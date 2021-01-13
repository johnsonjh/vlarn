/* =============================================================================
 * PROGRAM:  ularn
 * FILENAME: ularn_win.c
 *
 * DESCRIPTION:
 * This module contains all operating system dependant code for input and
 * display update.
 * Each version of ularn should provide a different implementation of this
 * module.
 *
 * This is the Windows 32 window display and input module.
 *
 * =============================================================================
 * EXPORTED VARIABLES
 *
 * nonap         : Set to true if no time delays are to be used.
 * nosignal      : Set if ctrl-C is to be trapped to prevent exit.
 * enable_scroll : Probably superfluous
 * yrepcount     : Repeat count for input commands.
 *
 * =============================================================================
 * EXPORTED FUNCTIONS
 *
 * init_app               : Initialise the app
 * close_app              : Close the app and free resources
 * get_normal_input       : Get the next command input
 * get_prompt_input       : Get input in response to a question
 * get_password_input     : Get a password
 * get_num_input          : Geta number
 * get_dir_input          : Get a direction
 * set_display            : Set the display mode
 * UpdateStatus           : Update the status display
 * UpdateEffects          : Update the effects display
 * UpdateStatusAndEffects : Update both status and effects display
 * ClearText              : Clear the text output area
 * UlarnBeep              : Make a beep
 * Cursor                 : Set the cursor location
 * Printc                 : Print a single character
 * Print                  : Print a string
 * Printf                 : Print a formatted string
 * Standout               : Print a string is standout format
 * SetFormat              : Set the output text format
 * ClearEOL               : Clear to end of line
 * ClearEOPage            : Clear to end of page
 * show1cell              : Show 1 cell on the map
 * showplayer             : Show the player on the map
 * showcell               : Show the area around the player
 * drawscreen             : Redraw the screen
 * draws                  : Redraw a section of the screen
 * mapeffect              : Draw a directional effect
 * magic_effect_frames    : Get the number of animation frames in a magic fx
 * magic_effect           : Draw a frame in a magic fx
 * nap                    : Delay for a specified number of milliseconds
 * GetUser                : Get the username and user id.
 *
 * =============================================================================
 */

#include <windows.h>   // include important windows stuff
#include <stdio.h>

#include "header.h"
#include "ularn_game.h"

#include "config.h"
#include "dungeon.h"
#include "player.h"
#include "ularn_win.h"
#include "ularnpc.rh"
#include "monster.h"
#include "itm.h"

//
// Defines for windows
//
#define WINDOW_CLASS_NAME "ULARN_WINCLASS"  // class name

// Default size of the ularn window in characters
#define WINDOW_WIDTH    80
#define WINDOW_HEIGHT   25
#define SEPARATOR_WIDTH   8
#define SEPARATOR_HEIGHT  8
#define BORDER_SIZE       8

/* =============================================================================
 * Exported variables
 */

int nonap = 0;
int nosignal = 0;

char enable_scroll = 0;

int yrepcount = 0;

/* =============================================================================
 * Local variables
 */

#define M_NONE 0
#define M_SHIFT 1
#define M_CTRL  2
#define M_ASCII 255

#define MAX_KEY_BINDINGS 3

struct KeyCodeType {
	int VirtKey;
	int ModKey;
};

#define NUM_DIRS 8
static ActionType DirActions[NUM_DIRS] =
{
	ACTION_MOVE_WEST,
	ACTION_MOVE_EAST,
	ACTION_MOVE_SOUTH,
	ACTION_MOVE_NORTH,
	ACTION_MOVE_NORTHEAST,
	ACTION_MOVE_NORTHWEST,
	ACTION_MOVE_SOUTHEAST,
	ACTION_MOVE_SOUTHWEST
};

/* Default keymap */
/* Allow up to MAX_KEY_BINDINGS per action */
static struct KeyCodeType KeyMap[ACTION_COUNT][MAX_KEY_BINDINGS] =
{
	{ { 0,	  0	  },	     { 0,	   0	   },	  { 0,	      0	     } },       // ACTION_NULL
	{ { '~',  M_ASCII },	     { 0,	   0	   },	  { 0,	      0	     } },       // ACTION_DIAG
	{ { 'h',  M_ASCII },	     { VK_NUMPAD4, M_NONE  },	  { VK_LEFT,  M_NONE } },       // ACTION_MOVE_WEST
	{ { 'H',  M_ASCII },	     { VK_LEFT,	   M_SHIFT },	  { 0,	      0	     } },       // ACTION_RUN_WEST
	{ { 'l',  M_ASCII },	     { VK_NUMPAD6, M_NONE  },	  { VK_RIGHT, M_NONE } },       // ACTION_MOVE_EAST,
	{ { 'L',  M_ASCII },	     { VK_RIGHT,   M_SHIFT },	  { 0,	      0	     } },       // ACTION_RUN_EAST,
	{ { 'j',  M_ASCII },	     { VK_NUMPAD2, M_NONE  },	  { VK_DOWN,  M_NONE } },       // ACTION_MOVE_SOUTH,
	{ { 'J',  M_ASCII },	     { VK_DOWN,	   M_SHIFT },	  { 0,	      0	     } },       // ACTION_RUN_SOUTH,
	{ { 'k',  M_ASCII },	     { VK_NUMPAD8, M_NONE  },	  { VK_UP,    M_NONE } },       // ACTION_MOVE_NORTH,
	{ { 'K',  M_ASCII },	     { VK_UP,	   M_SHIFT },	  { 0,	      0	     } },       // ACTION_RUN_NORTH,
	{ { 'u',  M_ASCII },	     { VK_NUMPAD9, M_NONE  },	  { VK_PRIOR, M_NONE } },       // ACTION_MOVE_NORTHEAST,
	{ { 'U',  M_ASCII },	     { VK_PRIOR,   M_SHIFT },	  { 0,	      0	     } },       // ACTION_RUN_NORTHEAST,
	{ { 'y',  M_ASCII },	     { VK_NUMPAD7, M_NONE  },	  { VK_HOME,  M_NONE } },       // ACTION_MOVE_NORTHWEST,
	{ { 'Y',  M_ASCII },	     { VK_HOME,	   M_SHIFT },	  { 0,	      0	     } },       // ACTION_RUN_NORTHWEST,
	{ { 'n',  M_ASCII },	     { VK_NUMPAD3, M_NONE  },	  { VK_NEXT,  M_NONE } },       // ACTION_MOVE_SOUTHEAST,
	{ { 'N',  M_ASCII },	     { VK_NEXT,	   M_SHIFT },	  { 0,	      0	     } },       // ACTION_RUN_SOUTHEAST,
	{ { 'b',  M_ASCII },	     { VK_NUMPAD1, M_NONE  },	  { VK_END,   M_NONE } },       // ACTION_MOVE_SOUTHWEST,
	{ { 'B',  M_ASCII },	     { VK_END,	   M_SHIFT },	  { 0,	      0	     } },       // ACTION_RUN_SOUTHWEST,
	{ { '.',  M_ASCII },	     { 0,	   0	   },	  { 0,	      0	     } },       // ACTION_WAIT,
	{ { ' ',  M_ASCII },	     { 0,	   0	   },	  { 0,	      0	     } },       // ACTION_NONE,
	{ { 'w',  M_ASCII },	     { 0,	   0	   },	  { 0,	      0	     } },       // ACTION_WIELD,
	{ { 'W',  M_ASCII },	     { 0,	   0	   },	  { 0,	      0	     } },       // ACTION_WEAR,
	{ { 'r',  M_ASCII },	     { 0,	   0	   },	  { 0,	      0	     } },       // ACTION_READ,
	{ { 'q',  M_ASCII },	     { 0,	   0	   },	  { 0,	      0	     } },       // ACTION_QUAFF,
	{ { 'd',  M_ASCII },	     { 0,	   0	   },	  { 0,	      0	     } },       // ACTION_DROP,
	{ { 'c',  M_ASCII },	     { 0,	   0	   },	  { 0,	      0	     } },       // ACTION_CAST_SPELL,
	{ { 'o',  M_ASCII },	     { 0,	   0	   },	  { 0,	      0	     } },       // ACTION_OPEN_DOOR,
	{ { 'C',  M_ASCII },	     { 0,	   0	   },	  { 0,	      0	     } },       // ACTION_CLOSE_DOOR,
	{ { 'O',  M_ASCII },	     { 0,	   0	   },	  { 0,	      0	     } },       // ACTION_OPEN_CHEST,
	{ { 'i',  M_ASCII },	     { 0,	   0	   },	  { 0,	      0	     } },       // ACTION_INVENTORY,
	{ { 'e',  M_ASCII },	     { 0,	   0	   },	  { 0,	      0	     } },       // ACTION_EAT_COOKIE,
	{ { '\\', M_ASCII },	     { 0,	   0	   },	  { 0,	      0	     } },       // ACTION_LIST_SPELLS,
	{ { '?',  M_ASCII },	     { 0,	   0	   },	  { 0,	      0	     } },       // ACTION_HELP,
	{ { 'S',  M_ASCII },	     { 0,	   0	   },	  { 0,	      0	     } },       // ACTION_SAVE,
	{ { 'Z',  M_ASCII },	     { 0,	   0	   },	  { 0,	      0	     } },       // ACTION_TELEPORT,
	{ { '^',  M_ASCII },	     { 0,	   0	   },	  { 0,	      0	     } },       // ACTION_IDENTIFY_TRAPS,
	{ { '_',  M_ASCII },	     { 0,	   0	   },	  { 0,	      0	     } },       // ACTION_BECOME_CREATOR,
	{ { '+',  M_ASCII },	     { 0,	   0	   },	  { 0,	      0	     } },       // ACTION_CREATE_ITEM,
	{ { '-',  M_ASCII },	     { 0,	   0	   },	  { 0,	      0	     } },       // ACTION_TOGGLE_WIZARD,
	{ { '`',  M_ASCII },	     { 0,	   0	   },	  { 0,	      0	     } },       // ACTION_DEBUG_MODE,
	{ { 'T',  M_ASCII },	     { 0,	   0	   },	  { 0,	      0	     } },       // ACTION_REMOVE_ARMOUR,
	{ { 'g',  M_ASCII },	     { 0,	   0	   },	  { 0,	      0	     } },       // ACTION_PACK_WEIGHT,
	{ { 'v',  M_ASCII },	     { 0,	   0	   },	  { 0,	      0	     } },       // ACTION_VERSION,
	{ { 'Q',  M_ASCII },	     { 0,	   0	   },	  { 0,	      0	     } },       // ACTION_QUIT,
	{ { 'R',  M_CTRL  },	     { 0,	   0	   },	  { 0,	      0	     } },       // ACTION_REDRAW_SCREEN,
	{ { 'P',  M_ASCII },	     { 0,	   0	   },	  { 0,	      0	     } } // ACTION_SHOW_TAX
};

static struct KeyCodeType RunKeyMap = { VK_NUMPAD5, M_NONE };

//
// Variables for windows
//

#define INITIAL_WIDTH 400
#define INITIAL_HEIGHT 300

static HINSTANCE my_instance = 0;

static HWND frame_window_handle = NULL;         // save the window handle
static HINSTANCE main_instance = NULL;          // save the instance
static HDC frame_dc = 0;

static LOGFONT LogFont;
static HFONT LarnFont = 0;

static int use_palette = 0;
static int peused[256];
static PALETTEENTRY pe[256];
static LOGPALETTE *palette = NULL;
static HPALETTE HPalette;

static int CaretActive = 0;

static int TileWidth = 32;
static int TileHeight = 32;
static int CharHeight;
static int CharWidth;
static int LarnWindowLeft = 0;
static int LarnWindowTop = 0;
static int LarnWindowWidth = INITIAL_WIDTH;
static int LarnWindowHeight = INITIAL_HEIGHT;
static int LarnWindowMaximized = 0;
static int MinWindowWidth;
static int MinWindowHeight;

static int Runkey;
static ActionType Event;
static int GotChar;
static char EventChar;

//
// player id file
//
static char *PIDName = LIBDIR "\\vlarn.pid";
#define FIRST_PID 1001

//
// ularn.ini file for window position & font selection
//
static char *IniName = "vlarn.ini";

//
// Bitmaps for tiles
//

static char *TileBMName = LIBDIR "\\vlarn_gfx.bmp";
static HBITMAP TileBitmap = NULL;

static HDC TileDC;

/* Tiles for different character classes, (female, male) */
static int PlayerTiles[8][2] =
{
	{ 165, 181 },   /* Ogre */
	{ 166, 182 },   /* Wizard */
	{ 167, 183 },   /* Klingon */
	{ 168, 184 },   /* Elf */
	{ 169, 185 },   /* Rogue */
	{ 170, 186 },   /* Adventurer */
	{ 171, 187 },   /* Dwarf */
	{ 172, 188 } /* Rambo */
};

#define TILE_CURSOR1 174
#define TILE_CURSOR2 190
#define WALL_TILES   352

/* Tiles for directional effects */
static int EffectTile[EFFECT_COUNT][9] =
{
	{ 191, 198, 196, 194, 192, 195, 193, 197, 199 },
	{ 191, 206, 204, 202, 200, 203, 201, 205, 207 },
	{ 191, 214, 212, 210, 208, 211, 209, 213, 215 },
	{ 191, 222, 220, 218, 216, 219, 217, 221, 223 },
	{ 191, 230, 228, 226, 224, 227, 225, 229, 231 }
};

#define MAX_MAGICFX_FRAME 8

struct MagicEffectDataType {
	int Overlay;                    /* 0 = no overlay, 1 = overlay     */
	int Frames;                     /* Number of frames in the effect  */
	int Tile1[MAX_MAGICFX_FRAME];   /* The primary tile for this frame */
	int Tile2[MAX_MAGICFX_FRAME];   /* Only used for overlay effects   */
};

static struct MagicEffectDataType magicfx_tile[MAGIC_COUNT] =
{
	/* Sparkle */
	{ 1, /* Overlay this on current tile */
		  8,
		  { 240, 241, 242, 243, 244, 245, 246, 247 },
		  { 248, 249, 250, 251, 252, 253, 254, 255 } },

	/* Sleep */
	{
		0,
		6,
		{ 256, 272, 288, 304, 320, 336, 0,   0	 },
		{ 0,   0,   0,	 0,   0,   0,	0,   0	 }
	},

	/* Web */
	{
		0,
		6,
		{ 257, 273, 289, 305, 321, 337, 0,   0	 },
		{ 0,   0,   0,	 0,   0,   0,	0,   0	 }
	},

	/* Phantasmal forces */
	{
		0,
		6,
		{ 258, 274, 290, 306, 322, 338, 0,   0	 },
		{ 0,   0,   0,	 0,   0,   0,	0,   0	 }
	},

	/* Cloud kill */
	{
		0,
		6,
		{ 259, 275, 291, 307, 323, 339, 0,   0	 },
		{ 0,   0,   0,	 0,   0,   0,	0,   0	 }
	},

	/* Vaporize rock */
	{
		0,
		6,
		{ 260, 276, 292, 308, 324, 340, 0,   0	 },
		{ 0,   0,   0,	 0,   0,   0,	0,   0	 }
	},

	/* Dehydrate */
	{
		0,
		6,
		{ 261, 277, 293, 309, 325, 341, 0,   0	 },
		{ 0,   0,   0,	 0,   0,   0,	0,   0	 }
	},

	/* Drain life */
	{
		0,
		6,
		{ 262, 278, 294, 310, 326, 342, 0,   0	 },
		{ 0,   0,   0,	 0,   0,   0,	0,   0	 }
	},

	/* Flood */
	{
		0,
		6,
		{ 263, 279, 295, 311, 327, 343, 0,   0	 },
		{ 0,   0,   0,	 0,   0,   0,	0,   0	 }
	},

	/* Finger of death */
	{
		0,
		6,
		{ 264, 280, 296, 312, 328, 344, 0,   0	 },
		{ 0,   0,   0,	 0,   0,   0,	0,   0	 }
	},

	/* Teleport away */
	{
		0,
		6,
		{ 265, 281, 297, 313, 329, 345, 0,   0	 },
		{ 0,   0,   0,	 0,   0,   0,	0,   0	 }
	},

	/* Magic fire */
	{
		0,
		6,
		{ 266, 282, 298, 314, 330, 346, 0,   0	 },
		{ 0,   0,   0,	 0,   0,   0,	0,   0	 }
	},

	/* Make wall */
	{
		0,
		6,
		{ 267, 283, 299, 315, 331, 347, 0,   0	 },
		{ 0,   0,   0,	 0,   0,   0,	0,   0	 }
	},

	/* Summon demon */
	{
		0,
		6,
		{ 268, 284, 300, 316, 332, 348, 0,   0	 },
		{ 0,   0,   0,	 0,   0,   0,	0,   0	 }
	},

	/* Annihilate (scroll) */
	{
		0,
		6,
		{ 269, 285, 301, 317, 333, 349, 0,   0	 },
		{ 0,   0,   0,	 0,   0,   0,	0,   0	 }
	}
};

//
// Current display mode
//
DisplayModeType CurrentDisplayMode = DISPLAY_TEXT;

//
// Map window position and size
//
static int MapLeft;
static int MapTop;
static int MapWidth;
static int MapHeight;

static int MapTileLeft = 0;
static int MapTileTop = 0;
static int MapTileWidth;
static int MapTileHeight;

//
// Status lines window position and size
//
static int StatusLeft;
static int StatusTop;
static int StatusWidth;
static int StatusHeight;

//
// Effects window position and size
//
static int EffectsLeft;
static int EffectsTop;
static int EffectsWidth;
static int EffectsHeight;

//
// Message window position and size
//
static int MessageLeft;
static int MessageTop;
static int MessageWidth;
static int MessageHeight;

//
// Text window position, size
//
static int TextLeft;
static int TextTop;
static int TextWidth;
static int TextHeight;
static int ShowTextBorder;

// =============================================================================
// Text mode stuff
//

#define LINE_LENGTH 80

typedef char TextLine[LINE_LENGTH + 1];
typedef FormatType FormatLine[LINE_LENGTH + 1];

//
// Messages
//
#define MAX_MSG_LINES    5
static TextLine MessageChr[MAX_MSG_LINES];
static FormatLine MessageFmt[MAX_MSG_LINES];
static FormatType CurrentMsgFormat;
static int MsgCursorX = 1;
static int MsgCursorY = 1;

//
// Text
//
#define MAX_TEXT_LINES 25
#define TEXT_LINE_LENGTH 80
static TextLine TextChr[MAX_TEXT_LINES];
static FormatLine TextFmt[MAX_TEXT_LINES];
static FormatType CurrentTextFormat;
static int TextCursorX = 1;
static int TextCursorY = 1;

//
// Generalised text buffer
// Top left corner is x=1, y=1
//
static TextLine   *Text;
static FormatLine *Format;
static FormatType CurrentFormat;
static int CursorX = 1;
static int CursorY = 1;
static int MaxLine;
static int TTop;
static int TLeft;
static int TWidth;
static int THeight;

//
// The monster to use for showing mimics. Changes every 10 turns.
//
static MonsterIdType mimicmonst = MIMIC;

/* =============================================================================
 * Local functions
 */

/* =============================================================================
 * FUNCTION: ReadIniFile
 *
 * DESCRIPTION:
 * Read the ularn.ini file that specifies the window position and font to use.
 * The file is written on exit so that the ularn window position and font will
 * are restored the next time the game is played.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   0 => Read of ini file failed
 *   1 => Successfully read ini file.
 */
int ReadIniFile(void)
{
	FILE *fp;
	char Line[256];
	char String[256];
	int len;
	int f1, f2, f3, f4, f5, f6, f7, f8;

	fp = fopen(IniName, "r");
	if (fp == NULL)
		/*
		 * .ini file doesn't exist yet
		 */
		return 0;

	fgets(Line, 256, fp);
	if (strncmp(Line, "[ULARN WINDOW]", 14) != 0)
		/*
		 * Not what we expect here so exit. The file will be re-created
		 * when the program exits
		 */
		return 0;

	fscanf(fp, "%d %d %d %d %s\n",
	       &LarnWindowLeft, &LarnWindowTop, &LarnWindowWidth, &LarnWindowHeight,
	       String);

	LarnWindowMaximized = strncmp(String, "MAXIMIZED", 9) == 0;

	fgets(Line, 256, fp);
	if (strncmp(Line, "[ULARN FONT]", 12) != 0)
		/*
		 * Not what we expect here so exit. The file will be re-created
		 * when the program exits.
		 */
		return 0;

	fgets(Line, 256, fp);
	len = strlen(Line) - 1;
	while ((len > 0) && (!isprint(Line[len]))) {
		Line[len] = 0;
		len--;
	}
	strncpy(LogFont.lfFaceName, Line, LF_FACESIZE);

	fscanf(fp, "%ld %ld %ld %ld %ld\n",
	       &LogFont.lfWidth,
	       &LogFont.lfHeight,
	       &LogFont.lfEscapement,
	       &LogFont.lfOrientation,
	       &LogFont.lfWeight);

	fscanf(fp, "%d %d %d %d %d %d %d %d\n",
	       &f1, &f2, &f3, &f4, &f5, &f6, &f7, &f8);

	LogFont.lfItalic = (BYTE)f1;
	LogFont.lfUnderline = (BYTE)f2;
	LogFont.lfStrikeOut = (BYTE)f3;
	LogFont.lfCharSet = (BYTE)f4;
	LogFont.lfOutPrecision = (BYTE)f5;
	LogFont.lfClipPrecision = (BYTE)f6;
	LogFont.lfQuality = (BYTE)f7;
	LogFont.lfPitchAndFamily = (BYTE)f8;

	return 1;
}

/* =============================================================================
 * FUNCTION: WriteIniFile
 *
 * DESCRIPTION:
 * This function writes the ini file that specifies the window position and
 * font to use for ularn.
 * This is to be called on program exit so that ularn will remember the player's
 * preferred window position and font size.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void WriteIniFile(void)
{
	FILE *fp;
	int Top, Left;
	int Width, Height;
	RECT WRect;
	WINDOWPLACEMENT wp;
	int maximized;

	fp = fopen(IniName, "w");

	/* Outputthe window position */
	GetWindowRect(frame_window_handle, &WRect);

	Left = WRect.left;
	Top = WRect.top;
	Width = (WRect.right - WRect.left) - 1;
	Height = (WRect.bottom - WRect.top) - 1;

	wp.length = sizeof(WINDOWPLACEMENT);
	GetWindowPlacement(frame_window_handle, &wp);
	maximized = (wp.showCmd == SW_SHOWMAXIMIZED);

	fprintf(fp, "[ULARN WINDOW]\n");
	fprintf(fp, "%d %d %d %d %s\n", Left, Top, Width, Height,
		(maximized) ? "MAXIMIZED" : "NORMAL");

	/* Output the font specification */
	GetObject(LarnFont, sizeof(LOGFONT), &LogFont);
	fprintf(fp, "[ULARN FONT]\n");
	fprintf(fp, "%s\n", LogFont.lfFaceName);
	fprintf(fp, "%ld %ld %ld %ld %ld\n",
		LogFont.lfWidth,
		LogFont.lfHeight,
		LogFont.lfEscapement,
		LogFont.lfOrientation,
		LogFont.lfWeight);
	fprintf(fp, "%d %d %d %d %d %d %d %d\n",
		LogFont.lfItalic,
		LogFont.lfUnderline,
		LogFont.lfStrikeOut,
		LogFont.lfCharSet,
		LogFont.lfOutPrecision,
		LogFont.lfClipPrecision,
		LogFont.lfQuality,
		LogFont.lfPitchAndFamily);

	fclose(fp);

}

/* =============================================================================
 * FUNCTION: AboutDialogProc
 *
 * DESCRIPTION:
 * Windows callback for the about dialog.
 *
 * PARAMETERS:
 *
 *   hwnd   : The window handle for the message
 *
 *   msg    : The message received
 *
 *   wparam : The wparam for the message
 *
 *   lparam : The lparam for the message.
 *
 * RETURN VALUE:
 *
 *   0 if the message was handled by this procedure.
 */
static BOOL CALLBACK AboutDialogProc(
	HWND hwnd,
	UINT msg,
	WPARAM wparam,
	LPARAM lparam)
{
	RECT WRect;
	RECT DRect;
	int WindowWidth;
	int WindowHeight;
	int DialogWidth;
	int DialogHeight;
	int DialogX;
	int DialogY;

	//
	// What is the message
	//
	switch (msg) {
	case WM_INITDIALOG:

		// hwndFocus = (HWND) wparam; // handle of control to receive focus
		// lInitParam = lparam;
		//
		// Do initialization stuff here
		//
		GetWindowRect(frame_window_handle, &WRect);
		WindowWidth = (WRect.right - WRect.left) + 1;
		WindowHeight = (WRect.bottom - WRect.top) + 1;

		GetWindowRect(hwnd, &DRect);
		DialogWidth = (DRect.right - DRect.left) + 1;
		DialogHeight = (DRect.bottom - DRect.top) + 1;

		DialogX = WRect.left + (WindowWidth - DialogWidth) / 2;
		DialogY = WRect.top + (WindowHeight - DialogHeight) / 2;

		SetWindowPos(hwnd, NULL, DialogX, DialogY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
		return TRUE;

	case WM_COMMAND:
	{
		// int wNotifyCode = HIWORD(wparam); // notification code
		int wID = LOWORD(wparam); // item, control, or accelerator identifier
		// HWND hwndCtl = (HWND) lparam;     // handle of control

		switch (wID) {
		case IDOK:

			EndDialog(hwnd, IDOK);

			return TRUE;

		default:
			break;
		}
		break;
	}

	default:
		break;

	}

	return FALSE;

}

/* =============================================================================
 * FUNCTION: DoAbout
 *
 * DESCRIPTION:
 * SHow the about dialog.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */

void DoAbout(void)
{
	DialogBox(
		main_instance,
		MAKEINTRESOURCE(IDD_ABOUT),
		frame_window_handle,
		AboutDialogProc);
}

/* =============================================================================
 * FUNCTION: calc_scroll
 *
 * DESCRIPTION:
 * Calculate the new scroll position of the map based on the player's current
 * position.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   true if the new scroll position differs from the previous scroll position.
 */
static int calc_scroll(void)
{
	int ox, oy;

	ox = MapTileLeft;
	oy = MapTileTop;

	if (MapTileHeight < MAXY) {
		MapTileTop = playery - MapTileHeight / 2;
		if (MapTileTop < 0)
			MapTileTop = 0;

		if ((MapTileTop + MapTileHeight) > MAXY)
			MapTileTop = MAXY - MapTileHeight;
	}else
		MapTileTop = 0;

	if (MapTileWidth < MAXX) {
		MapTileLeft = playerx - MapTileWidth / 2;
		if (MapTileLeft < 0)
			MapTileLeft = 0;

		if ((MapTileLeft + MapTileWidth) > MAXX)
			MapTileLeft = MAXX - MapTileWidth;
	}else
		MapTileLeft = 0;

	//
	// return true if the map requires scrolling
	//
	return (MapTileLeft != ox) || (MapTileTop != oy);
}

/* =============================================================================
 * FUNCTION: CalcMinWindowSize
 *
 * DESCRIPTION:
 * Calculate the minimum window size.
 * The new minimum windows isze is stored in MinWindowWidth and MinWindowHeight.
 * If the current window size is smaller than this then it is resized.
 *
 * PARAMETERS:
 *
 *   DC : The device context for the window.
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void CalcMinWindowSize(HDC DC)
{
	TEXTMETRIC tm;

	//
	// Calculate the new minimum window size
	//
	GetTextMetrics(DC, &tm);

	CharWidth = tm.tmAveCharWidth;
	CharHeight = tm.tmHeight;

	MinWindowWidth =
		WINDOW_WIDTH * CharWidth +
		2 * GetSystemMetrics(SM_CXFRAME);
	MinWindowHeight =
		WINDOW_HEIGHT * CharHeight  +
		2 * SEPARATOR_HEIGHT +
		2 * GetSystemMetrics(SM_CYFRAME) +
		GetSystemMetrics(SM_CYMENU) +
		GetSystemMetrics(SM_CYCAPTION);

	//
	// Update the window size
	//
	if (MinWindowWidth > LarnWindowWidth)
		LarnWindowWidth = MinWindowWidth;
	if (MinWindowHeight > LarnWindowHeight)
		LarnWindowHeight = MinWindowHeight;
	SetWindowPos(frame_window_handle, NULL, 0, 0, LarnWindowWidth, LarnWindowHeight, SWP_NOMOVE | SWP_NOZORDER);

	InvalidateRect(frame_window_handle, NULL, 1);
}

/*
 * Repaint flag to force redraw of everything, not just deltas
 */
static int Repaint = 0;

/* =============================================================================
 * FUNCTION: PaintStatus
 *
 * DESCRIPTION:
 * Paint the status area.
 *
 * PARAMETERS:
 *
 *   DC : The device context for the painting
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void PaintStatus(HDC DC)
{
	char Line[81];
	char Buf[81];
	int i;
	RECT StatusRect;

	if (Repaint) {
		StatusRect.left = StatusLeft;
		StatusRect.top = StatusTop;
		StatusRect.right = StatusLeft + StatusWidth;
		StatusRect.bottom = StatusTop + StatusHeight;

		FillRect(DC, &StatusRect, GetStockObject(WHITE_BRUSH));
	}

	//
	// Build the top status line
	//
	Line[0] = 0;

	/* Spells */
	if (c[SPELLMAX] > 99)
		sprintf(Buf, "Spells:%3ld(%3ld)", c[SPELLS], c[SPELLMAX]);
	else
		sprintf(Buf, "Spells:%3ld(%2ld) ", c[SPELLS], c[SPELLMAX]);

	strcat(Line, Buf);

	/* AC, WC */
	sprintf(Buf, " AC: %-3ld  WC: %-3ld  Level", c[AC], c[WCLASS]);
	strcat(Line, Buf);

	/* Level */
	if (c[LEVEL] > 99)
		sprintf(Buf, "%3ld", c[LEVEL]);
	else
		sprintf(Buf, " %-2ld", c[LEVEL]);
	strcat(Line, Buf);

	/* Exp, class */
	sprintf(Buf, " Exp: %-9ld %s", c[EXPERIENCE], class[c[LEVEL] - 1]);
	strcat(Line, Buf);

	TextOut(DC, StatusLeft, StatusTop, Line, strlen(Line));

	//
	// Format the second line of the status
	//
	sprintf(Buf, "%ld (%ld)", c[HP], c[HPMAX]);

	sprintf(Line, "HP: %11s STR=%-2ld INT=%-2ld WIS=%-2ld CON=%-2ld DEX=%-2ld CHA=%-2ld LV:",
		Buf,
		c[STRENGTH] + c[STREXTRA],
		c[INTELLIGENCE],
		c[WISDOM],
		c[CONSTITUTION],
		c[DEXTERITY],
		c[CHARISMA]);

	if ((level == 0) || (wizard))
		c[TELEFLAG] = 0;

	if (c[TELEFLAG])
		strcat(Line, " ?");
	else
		strcat(Line, levelname[level]);

	sprintf(Buf, "  Gold: %-8ld", c[GOLD]);
	strcat(Line, Buf);

	TextOut(DC, StatusLeft, StatusTop + CharHeight, Line, strlen(Line));

	//
	// Mark all character values as displayed.
	//
	c[TMP] = c[STRENGTH] + c[STREXTRA];
	for (i = 0; i < 100; i++)
		cbak[i] = c[i];
}

/* Effects strings */
static struct bot_side_def {
	int typ;
	char *string;
} bot_data[] =
{
	{ STEALTH,	  "Stealth   "		  },
	{ UNDEADPRO,	  "Undead Pro"		  },
	{ SPIRITPRO,	  "Spirit Pro"		  },
	{ CHARMCOUNT,	  "Charm     "		  },
	{ TIMESTOP,	  "Time Stop "		  },
	{ HOLDMONST,	  "Hold Monst"		  },
	{ GIANTSTR,	  "Giant Str "		  },
	{ FIRERESISTANCE, "Fire Resit"		  },
	{ DEXCOUNT,	  "Dexterity "		  },
	{ STRCOUNT,	  "Strength  "		  },
	{ SCAREMONST,	  "Scare     "		  },
	{ HASTESELF,	  "Haste Self"		  },
	{ CANCELLATION,	  "Cancel    "		  },
	{ INVISIBILITY,	  "Invisible "		  },
	{ ALTPRO,	  "Protect 3 "		  },
	{ PROTECTIONTIME, "Protect 2 "		  },
	{ WTW,		  "Wall-Walk "		  }
};

/* =============================================================================
 * FUNCTION: PaintEffects
 *
 * DESCRIPTION:
 * Paint the effects display.
 *
 * PARAMETERS:
 *
 *   DC : The DC to be painted.
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void PaintEffects(HDC DC)
{
	int i, idx;
	int WasSet;
	int IsSet;
	RECT EffectsRect;

	if (Repaint) {
		EffectsRect.left = EffectsLeft;
		EffectsRect.top = EffectsTop;
		EffectsRect.right = EffectsLeft + EffectsWidth;
		EffectsRect.bottom = EffectsTop + EffectsHeight;

		FillRect(DC, &EffectsRect, GetStockObject(WHITE_BRUSH));
	}

	for (i = 0; i < 17; i++) {
		idx = bot_data[i].typ;
		WasSet = (cbak[idx] != 0);
		IsSet = (c[idx] != 0);

		if ((Repaint) || (IsSet != WasSet)) {
			if (IsSet)
				TextOut(DC, EffectsLeft, EffectsTop + i * CharHeight,
					bot_data[i].string, strlen(bot_data[i].string));
			else
				TextOut(DC, EffectsLeft, EffectsTop + i * CharHeight,
					"          ", 10);
		}

		cbak[idx] = c[idx];
	}
}

/* =============================================================================
 * FUNCTION: GetTile
 *
 * DESCRIPTION:
 * Get the tile to be displayed for a location on the map.
 *
 * PARAMETERS:
 *
 *   x      : The x coordinate for the tile
 *
 *   y      : The y coordiante for the tile
 *
 *   TileId : This is set to the tile to be displayed for (x, y).
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void GetTile(int x, int y, int *TileId)
{
	MonsterIdType k;

	if ((x == playerx) && (y == playery) && (c[BLINDCOUNT] == 0)) {
		//
		// This is the square containing the player and the players isn't
		// blind, so return the player tile.
		//
		*TileId = PlayerTiles[class_num][(int)sex];
		return;
	}

	//
	// Work out what is here
	//
	if (know[x][y] == OUNKNOWN) {
		//
		// The player doesn't know what is at this position.
		//
		*TileId = objtilelist[OUNKNOWN];
	}else {
		k = mitem[x][y].mon;
		if (k != 0) {
			if ((c[BLINDCOUNT] == 0) &&
			    (((stealth[x][y] & STEALTH_SEEN) != 0) ||
			     ((stealth[x][y] & STEALTH_AWAKE) != 0))) {
				//
				// There is a monster here and the player is not blind and the
				// monster is seen or awake.
				//
				if (k == MIMIC) {
					if ((gtime % 10) == 0)
						while ((mimicmonst = rnd(MAXMONST)) == INVISIBLESTALKER);

					*TileId = monsttilelist[mimicmonst];
				}else if ((k == INVISIBLESTALKER) && (c[SEEINVISIBLE] == 0))
					*TileId = objtilelist[(int)know[x][y]];
				else if ((k >= DEMONLORD) && (k <= LUCIFER) && (c[EYEOFLARN] == 0))
					/* demons are invisible if not have the eye */
					*TileId = objtilelist[(int)know[x][y]];
				else
					*TileId = monsttilelist[k];

			} /* can see monster */
			else
				/*
				 * The monster at this location is not known to the player, so show
				 * the tile for the item at this location
				 */
				*TileId = objtilelist[(int)know[x][y]];
		} /* monster here */
		else{
			k = know[x][y];
			*TileId = objtilelist[k];
		}
	}

	/* Handle walls */
	if (*TileId == objtilelist[OWALL])
		*TileId = WALL_TILES + iarg[x][y];
}

/* =============================================================================
 * FUNCTION: PaintMap
 *
 * DESCRIPTION:
 * Repaint the map.
 *
 * PARAMETERS:
 *
 *   DC : The device context to be painted.
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void PaintMap(HDC DC)
{
	int x, y;
	int sx, sy;
	int mx, my;
	int TileId;
	int TileX;
	int TileY;

	mx = MapTileLeft + MapTileWidth;
	my = MapTileTop + MapTileHeight;

	if (my > MAXY)
		my = MAXY;

	if (mx > MAXX)
		mx = MAXX;

	sx = 0;
	for (x = MapTileLeft; x < mx; x++) {
		sy = 0;
		for (y = MapTileTop; y < my; y++) {
			GetTile(x, y, &TileId);

			TileX = (TileId % 16) * TileWidth;
			TileY = (TileId / 16) * TileHeight;

			BitBlt(DC,
			       MapLeft + sx * TileWidth, MapTop + sy * TileHeight,
			       TileWidth, TileHeight,
			       TileDC, TileX, TileY, SRCCOPY);

			sy++;
		}

		sx++;
	}

	sx = playerx - MapTileLeft;
	sy = playery - MapTileTop;

	if ((sx >= 0) && (sx < MapTileWidth) &&
	    (sy >= 0) && (sy < MapTileHeight)) {
		TileId = TILE_CURSOR1;
		TileX = (TileId % 16) * TileWidth;
		TileY = (TileId / 16) * TileHeight;
		BitBlt(DC, MapLeft + sx * TileWidth, MapTop + sy * TileHeight,
		       TileWidth, TileHeight,
		       TileDC, TileX, TileY, SRCAND);

		TileId = TILE_CURSOR2;
		TileX = (TileId % 16) * TileWidth;
		TileY = (TileId / 16) * TileHeight;
		BitBlt(DC, MapLeft + sx * TileWidth, MapTop + sy * TileHeight,
		       TileWidth, TileHeight,
		       TileDC, TileX, TileY, SRCPAINT);
	}
}

/* =============================================================================
 * FUNCTION: PaintTextWindow
 *
 * DESCRIPTION:
 * Repaint the window in text mode.
 *
 * PARAMETERS:
 *
 *   DC : The device contect to be painted.
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void PaintTextWindow(HDC DC)
{
	int sx, ex, y;
	FormatType Fmt;
	RECT TextRect;
	HPEN Pen;

	TextRect.left = TLeft;
	TextRect.top = TTop;
	TextRect.right = TLeft + TWidth;
	TextRect.bottom = TTop + THeight;

	if (CurrentDisplayMode == DISPLAY_TEXT) {
		if (ShowTextBorder) {
			Pen = CreatePen(PS_SOLID, 2, RGB(255, 255, 255));

			SelectObject(DC, Pen);
			SelectObject(DC, GetStockObject(NULL_BRUSH));

			RoundRect(DC,
				  TLeft - 8, TTop - 8,
				  TLeft + TWidth + 9, TTop + THeight + 9,
				  16, 16);

			DeleteObject(Pen);

		}else
			GetClientRect(frame_window_handle, &TextRect);
	}

	FillRect(DC, &TextRect, GetStockObject(WHITE_BRUSH));

	for (y = 0; y < MaxLine; y++) {

		sx = 0;

		while (sx < LINE_LENGTH) {

			Fmt = Format[y][sx];
			ex = sx;

			while ((ex < LINE_LENGTH) && (Format[y][ex] == Fmt)) ex++;

			switch (Fmt) {
			case FORMAT_NORMAL:
				SetTextColor(DC, RGB(0, 0, 0));
				break;
			case FORMAT_STANDOUT:
				SetTextColor(DC, RGB(255, 0, 0));
				break;
			case FORMAT_STANDOUT2:
				SetTextColor(DC, RGB(0, 127, 0));
				break;
			case FORMAT_STANDOUT3:
				SetTextColor(DC, RGB(0, 0, 255));
				break;
			default:
				break;
			}

			TextOut(DC,
				TLeft + sx * CharWidth,
				TTop + y * CharHeight,
				Text[y] + sx,
				ex - sx);

			sx = ex;
		}
	}

}

/* =============================================================================
 * FUNCTION: PaintMapWindow
 *
 * DESCRIPTION:
 * Repaint the window in map mode
 *
 * PARAMETERS:
 *
 *   DC : The device context to be painted.
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void PaintMapWindow(HDC DC)
{
	RECT Rect;

	HBRUSH LightBrush;
	HBRUSH MidBrush;
	HBRUSH DarkBrush;

	//
	// Draw separators between the different window areas
	//

	LightBrush = CreateSolidBrush(GetSysColor(COLOR_3DHILIGHT));
	MidBrush = CreateSolidBrush(GetSysColor(COLOR_3DFACE));
	DarkBrush = CreateSolidBrush(GetSysColor(COLOR_3DSHADOW));

	//
	// Message area
	//

	Rect.left = MessageLeft;
	Rect.top = MessageTop - SEPARATOR_HEIGHT;
	Rect.right = MessageLeft + MessageWidth;
	Rect.bottom = Rect.top + 2;

	FillRect(DC, &Rect, LightBrush);

	Rect.top = Rect.bottom;
	Rect.bottom = Rect.top + 4;
	FillRect(DC, &Rect, MidBrush);

	Rect.top = Rect.bottom;
	Rect.bottom = Rect.top + 2;
	FillRect(DC, &Rect, DarkBrush);

	//
	// Status area
	//
	Rect.left = StatusLeft;
	Rect.top = StatusTop - SEPARATOR_HEIGHT;
	Rect.right = StatusLeft + StatusWidth;
	Rect.bottom = Rect.top + 2;

	FillRect(DC, &Rect, LightBrush);

	Rect.top = Rect.bottom;
	Rect.bottom = Rect.top + 4;
	FillRect(DC, &Rect, MidBrush);

	Rect.top = Rect.bottom;
	Rect.bottom = Rect.top + 2;
	FillRect(DC, &Rect, DarkBrush);

	//
	// Effects area
	//
	Rect.left = EffectsLeft - SEPARATOR_WIDTH;
	Rect.top = EffectsTop;
	Rect.right = Rect.left + 2;
	Rect.bottom = EffectsTop + EffectsHeight;

	FillRect(DC, &Rect, LightBrush);

	Rect.left = Rect.right;
	Rect.right = Rect.left + 4;
	Rect.bottom = EffectsTop + EffectsHeight + 2;
	FillRect(DC, &Rect, MidBrush);

	Rect.left = Rect.right;
	Rect.right = Rect.left + 2;
	Rect.bottom = EffectsTop + EffectsHeight;
	FillRect(DC, &Rect, DarkBrush);

	PaintStatus(DC);
	PaintEffects(DC);
	PaintMap(DC);
	PaintTextWindow(DC);

	DeleteObject(LightBrush);
	DeleteObject(MidBrush);
	DeleteObject(DarkBrush);

}

/* =============================================================================
 * FUNCTION: PaintWindow
 *
 * DESCRIPTION:
 * Repaint the window.
 *
 * PARAMETERS:
 *
 *   DC : The device context to be painted
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void PaintWindow(HDC DC)
{
	SetMapMode(DC, MM_TEXT);
	SelectObject(DC, LarnFont);

	if (use_palette) {
		SelectPalette(DC, HPalette, 0);
		RealizePalette(DC);
	}

	Repaint = 1;

	if (CurrentDisplayMode == DISPLAY_MAP)
		PaintMapWindow(DC);
	else
		PaintTextWindow(DC);

	Repaint = 0;
}

/* =============================================================================
 * FUNCTION: Resize
 *
 * DESCRIPTION:
 * This procedure handles resizing the window in response to any event that
 * requires the sub-window size and position to be recalculated.
 *
 * PARAMETERS:
 *
 *   hwnd : The handle of the window being resized
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void Resize(HWND hwnd)
{
	RECT CRect;
	RECT WRect;
	int ClientWidth;
	int ClientHeight;

	GetWindowRect(hwnd, &WRect);
	LarnWindowWidth = (WRect.right - WRect.left) + 1;
	LarnWindowHeight = (WRect.bottom - WRect.top) + 1;

	GetClientRect(hwnd, &CRect);
	ClientWidth = (CRect.right - CRect.left);
	ClientHeight = (CRect.bottom - CRect.top);

	//
	// Calculate the message window size and position
	//
	MessageWidth = ClientWidth;
	MessageHeight = CharHeight * MAX_MSG_LINES;
	MessageLeft = 0;
	MessageTop = ClientHeight - MessageHeight - 1;

	//
	// Calculate the Status window size and position
	//
	StatusLeft = 0;
	StatusTop = (MessageTop - SEPARATOR_HEIGHT) - CharHeight * 2;
	StatusWidth = ClientWidth;
	StatusHeight = CharHeight * 2;

	//
	// Calculate the Effects window size and position
	//
	EffectsLeft = ClientWidth - CharWidth * 10;
	EffectsTop = 0;
	EffectsWidth = CharWidth * 10;
	EffectsHeight = StatusTop - SEPARATOR_HEIGHT;

	//
	// Calculate the size and position of the map window
	//
	MapLeft = 0;
	MapTop = 0;
	MapWidth = EffectsLeft - SEPARATOR_WIDTH;
	MapHeight = StatusTop - SEPARATOR_HEIGHT;
	MapTileWidth = MapWidth / TileWidth;
	MapTileHeight = MapHeight / TileHeight;

	//
	// Calculate the size and position of the text window
	//

	TextWidth = CharWidth * LINE_LENGTH;
	TextHeight = CharHeight * MAX_TEXT_LINES;

	TextLeft = (ClientWidth - TextWidth) / 2;
	TextTop = (ClientHeight - TextHeight) / 2;

	//
	// Check if should draw a border around the text page when it is displayed
	//
	ShowTextBorder = (TextLeft >= BORDER_SIZE) && (TextTop >= BORDER_SIZE);


	//
	// If the map window is bigger than required to display the map, then centre
	// the map in the window.
	//

	if (MapTileWidth > MAXX) {
		MapTileWidth = MAXX;
		MapLeft = (MapWidth - MapTileWidth * TileWidth) / 2;
	}

	if (MapTileHeight > MAXY) {
		MapTileHeight = MAXY;
		MapTop = (MapHeight - MapTileHeight * TileHeight) / 2;
	}

	if (CurrentDisplayMode == DISPLAY_MAP) {
		TLeft = MessageLeft;
		TTop = MessageTop;
		TWidth = MessageWidth;
		THeight = MessageHeight;
	}else {
		TLeft = TextLeft;
		TTop = TextTop;
		TWidth = TextWidth;
		THeight = TextHeight;
	}

	//
	// calculate the map scroll position for the current player position
	//

	calc_scroll();

	//
	// Force the window to redraw
	//
	InvalidateRect(hwnd, NULL, 1);

	//
	// Show the cursor if required
	//
	if (CaretActive) {
		CreateCaret(frame_window_handle,
			    NULL,
			    2, CharHeight);
		ShowCaret(frame_window_handle);
		SetCaretPos(TLeft + (CursorX - 1) * CharWidth,
			    TTop + (CursorY - 1) * CharHeight);

	}

}

/* =============================================================================
 * FUNCTION: WindowProc
 *
 * DESCRIPTION:
 * This is the main message handler of the system.
 *
 * PARAMETERS:
 *
 *   hwnd   : The window handle for the message
 *
 *   msg    : The message received
 *
 *   wparam : The wparam for the message
 *
 *   lparam : The lparam for the message.
 *
 * RETURN VALUE:
 *
 *   0 If the message was handled by this function.
 */
static LRESULT CALLBACK WindowProc(
	HWND hwnd,
	UINT msg,
	WPARAM wparam,
	LPARAM lparam)
{
	PAINTSTRUCT ps; // used in WM_PAINT
	HDC hdc;        // handle to a device context

	//
	// What is the message
	//
	switch (msg) {
	case WM_CREATE:
		//
		// Do initialization stuff here
		//
		return 0;

	case WM_INITMENU:
	{
		HMENU hMenu = (HMENU)wparam;

		CheckMenuItem(hMenu, IDM_DISPLAY_BEEP, (nobeep) ? MF_UNCHECKED : MF_CHECKED);
		break;
	}

	case WM_PAINT:
		//
		// Start painting
		//
		hdc = BeginPaint(hwnd, &ps);

		PaintWindow(hdc);

		//
		// End painting
		//
		EndPaint(hwnd, &ps);
		return 0;

	case WM_CLOSE:
		Event = ACTION_QUIT;
		return 0;

	case WM_DESTROY:
		//
		// kill the application
		//
		Event = ACTION_QUIT;
		return 0;

	case WM_GETMINMAXINFO:
	{
		MINMAXINFO *mmi;

		mmi = (MINMAXINFO *)lparam;
		mmi->ptMinTrackSize.x = MinWindowWidth;
		mmi->ptMinTrackSize.y =  MinWindowHeight;
		break;
	}

	case WM_SIZE:
		Resize(hwnd);
		break;

	case WM_COMMAND:
	{
		// int wNotifyCode = HIWORD(wparam); // notification code
		int wID = LOWORD(wparam); // item, control, or accelerator identifier
		// HWND hwndCtl = (HWND) lparam;     // handle of control

		switch (wID) {
		case IDM_GAME_SAVE:
			Event = ACTION_SAVE;
			break;

		case IDM_GAME_QUIT:
			Event = ACTION_QUIT;
			break;

		case IDM_COMMANDS_WAIT:
			Event = ACTION_WAIT;
			break;

		case IDM_COMMANDS_WIELD:
			Event = ACTION_WIELD;
			break;

		case IDM_COMMANDS_WEAR:
			Event = ACTION_WEAR;
			break;

		case IDM_COMMANDS_TAKEOFF:
			Event = ACTION_REMOVE_ARMOUR;
			break;

		case IDM_COMMANDS_QUAFF:
			Event = ACTION_QUAFF;
			break;

		case IDM_COMMANDS_READ:
			Event = ACTION_READ;
			break;

		case IDM_COMMANDS_CAST:
			Event = ACTION_CAST_SPELL;
			break;

		case IDM_COMMANDS_EAT:
			Event = ACTION_EAT_COOKIE;
			break;

		case IDM_COMMANDS_DROP:
			Event = ACTION_DROP;
			break;

		case IDM_COMMAND_CLOSE:
			Event = ACTION_CLOSE_DOOR;
			break;

		case IDM_SHOW_DISCOVERIES:
			Event = ACTION_LIST_SPELLS;
			break;

		case IDM_SHOW_INV:
			Event = ACTION_INVENTORY;
			break;

		case IDM_SHOW_TAX:
			Event = ACTION_SHOW_TAX;
			break;

		case IDM_SHOW_PACKWEIGHT:
			Event = ACTION_PACK_WEIGHT;
			break;

		case IDM_DISPLAY_REDRAW:
			Event = ACTION_REDRAW_SCREEN;
			break;

		case IDM_DISPLAY_BEEP:
			nobeep = !nobeep;
			break;

		case IDM_DISPLAY_FONT:
		{
			CHOOSEFONT cf;
			HFONT newfont;

			//
			// Get the current font data
			//
			GetObject(LarnFont, sizeof(LOGFONT), &LogFont);

			memset(&cf, 0, sizeof(CHOOSEFONT));

			cf.lStructSize = sizeof(CHOOSEFONT);
			cf.hwndOwner = frame_window_handle;
			cf.hDC = NULL;
			cf.lpLogFont = &LogFont;
			cf.iPointSize = 0;
			cf.Flags = CF_SCREENFONTS | CF_FORCEFONTEXIST | CF_EFFECTS | CF_FIXEDPITCHONLY;
			cf.rgbColors = 0;
			cf.lCustData = 0;
			cf.lpfnHook = 0;
			cf.lpTemplateName = 0;
			cf.hInstance = main_instance;
			cf.lpszStyle = 0;
			cf.nFontType = 0;
			cf.nSizeMin = 6;
			cf.nSizeMax = 24;

			if (ChooseFont(&cf)) {
				newfont = CreateFontIndirect(&LogFont);
				SelectObject(frame_dc, newfont);
				DeleteObject(LarnFont);
				LarnFont = newfont;

				CalcMinWindowSize(frame_dc);
			}
			break;
		}

		case IDM_HELP_HELP:
			Event = ACTION_HELP;
			break;

		case IDM_HELP_VERSION:
			Event = ACTION_VERSION;
			break;

		case IDM_HELP_ABOUT:
			DoAbout();
			break;

		default:
			break;
		}
		break;
	}

	case WM_KEYDOWN:
	{
		ActionType Action;
		int ModKey = 0;
		int Found = 0;
		int i;

		if ((GetKeyState(VK_SHIFT) & 0xFFFE) != 0)
			ModKey |= M_SHIFT;

		if ((GetKeyState(VK_CONTROL) & 0xFFFE) != 0)
			ModKey |= M_CTRL;

		Action = ACTION_NULL;
		while ((Action < ACTION_COUNT) && (!Found)) {
			for (i = 0; i < MAX_KEY_BINDINGS; i++) {
				if ((wparam == KeyMap[Action][i].VirtKey) &&
				    (KeyMap[Action][i].ModKey == ModKey))
					Found = 1;
			}

			if (!Found)
				Action++;
		}

		if (Found)
			Event = Action;
		else{
			/* check run key */
			if ((wparam == RunKeyMap.VirtKey) &&
			    (RunKeyMap.ModKey == ModKey))
				Runkey = 1;
		}

		break;
	}

	case WM_CHAR:
	{
		int chCharCode = (TCHAR)wparam; // character code
		// int lKeyData = lparam;              // key data
		ActionType Action;
		int Found = 0;
		int i;

		Action = ACTION_NULL;
		while ((Action < ACTION_COUNT) && (!Found)) {
			for (i = 0; i < MAX_KEY_BINDINGS; i++) {
				if ((chCharCode == KeyMap[Action][i].VirtKey) &&
				    (KeyMap[Action][i].ModKey == M_ASCII))
					Found = 1;
			}

			if (!Found)
				Action++;
		}

		if (Found)
			Event = Action;
		else{
			/* check run key */
			if ((chCharCode == RunKeyMap.VirtKey) &&
			    (RunKeyMap.ModKey == M_ASCII))
				Runkey = 1;
		}

		EventChar = (char)chCharCode;
		GotChar = 1;

		return 0;
	}

	default:
		break;

	}

	//
	// Process any messages that we didn't take care of
	//
	return DefWindowProc(hwnd, msg, wparam, lparam);

}



/* =============================================================================
 * FUNCTION: calculate_palette
 *
 * DESCRIPTION:
 * Calculate an palette for mapping from a 24 bpp bitmap to an 8 bpp bitmap.
 * This sets the pe used and pe defined above.
 *
 * PARAMETERS:
 *
 *   HSrcBitmap : A handle to the source 24 bit bitmap
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void calculate_palette(HBITMAP HSrcBmp)
{
	int *Cube;
	int ro, go, bo;
	int r, g, b;
	int ci;
	int x, y;
	int NumColours;
	int MaxC, MaxI;
	BITMAP SrcBitmap;
	unsigned char *SrcAddr;

	GetObject(HSrcBmp, sizeof(BITMAP), &SrcBitmap);

	//
	// Stage 1: Quantize 8x8x8 colours to 5x5x5 colours and fill in the 5x5x5
	//          colour cube.
	//

	Cube = malloc(32768 * sizeof(int));

	ci = 0;
	for (ro = 0; ro < 32; ro++) {
		for (go = 0; go < 32; go++)
			for (bo = 0; bo < 32; bo++)
				Cube[ci++] = 0;
	}

	for (y = 0; y < SrcBitmap.bmHeight; y++) {
		for (x = 0; x < SrcBitmap.bmWidth; x++) {
			SrcAddr =
				((unsigned char *)SrcBitmap.bmBits) + SrcBitmap.bmWidthBytes * y + x * 3;

			r = SrcAddr[2];
			g = SrcAddr[1];
			b = SrcAddr[0];

			ro = r / 8;
			go = g / 8;
			bo = b / 8;

			ci = ((ro * 32) + go) * 32 + bo;
			Cube[ci]++;
		}
	}

	//
	// Mark all palette entries as unused.
	//
	for (ci = 0; ci < 256; ci++)
		peused[ci] = 0;

	//
	// Set the entry for black and white to 0 and 255 respectively
	//
	peused[0] = 1;
	pe[0].peRed = 0;
	pe[0].peGreen = 0;
	pe[0].peBlue = 0;

	peused[255] = 1;
	pe[255].peRed = 255;
	pe[255].peGreen = 255;
	pe[255].peBlue = 255;

	//
	// Take the most popular colours.
	//

	NumColours = 0;
	while (NumColours < 236) {
		MaxC = 0;
		MaxI = 0;
		//
		// Traverse the colour cube, ignoring black (0) and white (32767) as these
		// have already been locked into the palette.
		//
		for (ci = 1; ci < 32767; ci++) {
			if (Cube[ci] > MaxC) {
				MaxC = Cube[ci];
				MaxI = ci;
			}
		}

		if (MaxC > 0) {
			peused[NumColours + 8] = 1;
			pe[NumColours + 8].peRed =  (unsigned char)(8 * ((MaxI & 0x7c00) >> 10));
			pe[NumColours + 8].peGreen = (unsigned char)(8 * ((MaxI & 0x03e0) >> 5));
			pe[NumColours + 8].peBlue = (unsigned char)(8 * (MaxI & 0x1f));

			Cube[MaxI] = 0;
			NumColours++;
		}else
			NumColours = 236;

	}

	palette = malloc(sizeof(LOGPALETTE) + sizeof(PALETTEENTRY) * 256);

	palette->palVersion = 0x300;
	palette->palNumEntries = 256;
	for (ci = 0; ci < 256; ci++)
		palette->palPalEntry[ci] = pe[ci];

	HPalette = CreatePalette(palette);

	free(Cube);
}

/* =============================================================================
 * Exported functions
 */

/* =============================================================================
 * FUNCTION: init_app
 */
int init_app(HINSTANCE hinstance)
{
	WNDCLASS winclass;      // this will hold the class we create
	HWND hwnd;              // generic window handle
	int x, y;
	HBITMAP TmpBitmap;

	//
	// First fill in the window class stucture
	//
	winclass.style = CS_OWNDC;
	winclass.lpfnWndProc = WindowProc;
	winclass.cbClsExtra = 0;
	winclass.cbWndExtra = 0;
	winclass.hInstance = hinstance;
	winclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	winclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	winclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	winclass.lpszMenuName = NULL;
	winclass.lpszClassName = WINDOW_CLASS_NAME;

	//
	// Register the window class
	//
	if (!RegisterClass(&winclass))
		return 0;

	//
	// Store the instance for future reference
	//
	my_instance = hinstance;

	//
	// Create the window, note the use of WS_POPUP
	//
	hwnd = CreateWindow(
		WINDOW_CLASS_NAME,                              // class
		"ULarn Win32",                                  // title
		WS_CAPTION | WS_POPUPWINDOW | WS_VISIBLE | WS_THICKFRAME | WS_MAXIMIZEBOX | WS_MINIMIZEBOX,
		0, 0,                                           // x,y
		INITIAL_WIDTH,                                  // width
		INITIAL_HEIGHT,                                 // height
		NULL,                                           // handle to parent
		LoadMenu(hinstance, (const char *)IDM_MENU1),   // handle to menu
		hinstance,                                      // instance
		NULL);

	if (hwnd == NULL)
		return 0;

	//
	// Save the window handle and instance in a global
	//
	frame_window_handle = hwnd;
	main_instance = hinstance;

	frame_dc = GetDC(hwnd);
	SetMapMode(frame_dc, MM_TEXT);

	if (ReadIniFile()) {
		LarnFont = CreateFontIndirect(&LogFont);
		SelectObject(frame_dc, LarnFont);

		CalcMinWindowSize(frame_dc);

		if (LarnWindowMaximized) {
			ShowWindow(frame_window_handle, SW_SHOWMAXIMIZED);
			Resize(frame_window_handle);
		}else {
			SetWindowPos(frame_window_handle, NULL,
				     LarnWindowLeft, LarnWindowTop,
				     LarnWindowWidth, LarnWindowHeight, SWP_NOZORDER);
		}

	}else {
		//
		// Get the default fixed font
		//
		LarnFont = GetStockObject(ANSI_FIXED_FONT);
		SelectObject(frame_dc, LarnFont);

		//
		// Calculate the minimum window size and set the window to be that size
		//
		CalcMinWindowSize(frame_dc);
	}

	if (GetDeviceCaps(frame_dc, BITSPIXEL) <= 8) {
		BITMAPINFOHEADER bi;
		LPVOID lpDIBBits;
		BITMAP SrcBitmap;

		//
		// Display is not true colour or high colour.
		// The tiles will need to be remapped into fewer colours.
		//
		use_palette = 1;

		//
		// Load the tile images as a DIB into a temporory bitmap
		//
		TmpBitmap = LoadImage(
			hinstance,
			TileBMName,
			IMAGE_BITMAP,
			0,
			0,
			LR_LOADFROMFILE | LR_DEFAULTSIZE | LR_CREATEDIBSECTION );

		if (TmpBitmap == NULL) {
			MessageBox(NULL, "Failed to load tiles bitmap", "Error", MB_OK);
			return 0;
		}

		//
		// Calculate the palette to use
		//

		calculate_palette(TmpBitmap);

		//
		// Select the palette into the main window DC and realize the palette
		// into the display device.
		//

		SelectPalette(frame_dc, HPalette, 0);
		RealizePalette(frame_dc);

		//
		// Get info about the 24 bit tiles bitmap
		//
		GetObject(TmpBitmap, sizeof(BITMAP), &SrcBitmap);

		//
		// Create info about the tiles bitmap to use for blitting from info about
		// the 24 bit version of the tiles.
		//
		bi.biSize = sizeof(BITMAPINFOHEADER);
		bi.biWidth = SrcBitmap.bmWidth;
		bi.biHeight = SrcBitmap.bmHeight;
		bi.biPlanes = SrcBitmap.bmPlanes;
		bi.biBitCount = SrcBitmap.bmBitsPixel;
		bi.biCompression = BI_RGB;
		bi.biSizeImage = 0;
		bi.biXPelsPerMeter = 0;
		bi.biYPelsPerMeter = 0;
		bi.biClrUsed = 0;
		bi.biClrImportant = 0;

		lpDIBBits = (LPVOID)(SrcBitmap.bmBits);

		//
		// Create a device dependant version of the original bitmap using the
		// palette that has been selected to the main window.
		//
		TileBitmap = CreateDIBitmap(
			frame_dc,
			(LPBITMAPINFOHEADER)&bi,
			CBM_INIT,
			lpDIBBits,
			(LPBITMAPINFO)&bi,
			DIB_RGB_COLORS);

		DeleteObject(TmpBitmap);

	}else {
		//
		// Load the tile bitmap as a DDB and create device contexts for blitting
		//
		TileBitmap = LoadImage(
			hinstance,
			TileBMName,
			IMAGE_BITMAP,
			0,
			0,
			LR_LOADFROMFILE | LR_DEFAULTSIZE);

		if (TileBitmap == NULL) {
			MessageBox(NULL, "Failed to load tiles bitmap", "Error", MB_OK);
			return 0;
		}

	}

	TileDC = CreateCompatibleDC(frame_dc);
	SelectObject(TileDC, TileBitmap);

	if (use_palette)
		SelectPalette(TileDC, HPalette, 0);

	//
	// Clear the text buffers
	//
	for (y = 0; y < MAX_MSG_LINES; y++) {
		for (x = 0; x < LINE_LENGTH; x++) {
			MessageChr[y][x] = ' ';
			MessageFmt[y][x] = FORMAT_NORMAL;
		}

		MessageChr[y][LINE_LENGTH] = 0;
	}

	for (y = 0; y < MAX_TEXT_LINES; y++) {
		for (x = 0; x < LINE_LENGTH; x++) {
			TextChr[y][x] = ' ';
			TextFmt[y][x] = FORMAT_NORMAL;
		}

		TextChr[y][LINE_LENGTH] = 0;
	}

	return 1;
}

/* =============================================================================
 * FUNCTION: close_app
 */
void close_app(void)
{
	if (frame_window_handle != NULL)
		WriteIniFile();

	if (palette != NULL)
		free(palette);

	if (frame_dc != NULL)
		ReleaseDC(frame_window_handle, frame_dc);

	if (TileDC != NULL)
		DeleteDC(TileDC);

	if (TileBitmap != NULL)
		DeleteObject(TileBitmap);

	if (frame_window_handle != NULL)
		DestroyWindow(frame_window_handle);

	if (my_instance != NULL)
		UnregisterClass(WINDOW_CLASS_NAME, my_instance);
}

/* =============================================================================
 * FUNCTION: get_normal_input
 */
ActionType get_normal_input(void)
{
	MSG msg; // generic message
	int idx;
	int got_dir;

	Event = ACTION_NULL;
	Runkey = 0;

	while (Event == ACTION_NULL) {
		if (GetMessage(&msg, NULL, 0, 0)) {
			//
			// Test if this is a quit
			//
			if (msg.message == WM_QUIT)
				break;

			//
			// Translate any accelerator keys
			//
			TranslateMessage(&msg);

			//
			// Send the message to the window proc
			//
			DispatchMessage(&msg);
		}

		//
		// Clear enhanced interface events in enhanced interface is not active
		//
		if (!enhance_interface) {
			if ((Event == ACTION_OPEN_DOOR) ||
			    (Event == ACTION_OPEN_CHEST))
				Event = ACTION_NULL;
		}

	}

	if (Runkey) {
		idx = 0;
		got_dir = 0;

		while ((idx < NUM_DIRS) && (!got_dir)) {
			if (DirActions[idx] == Event)
				got_dir = 1;
			else
				idx++;
		}

		if (got_dir)
			/* modify into a run event */
			Event = Event + 1;
	}

	return Event;
}

/* =============================================================================
 * FUNCTION: get_prompt_input
 */
char get_prompt_input(char *prompt, char *answers, int ShowCursor)
{
	MSG msg;  // generic message
	char *ch;

	//
	// Display the prompt at the current position
	//
	Print(prompt);

	//
	// Show the cursor if required
	//
	if (ShowCursor) {
		CreateCaret(frame_window_handle,
			    NULL,
			    2, CharHeight);
		ShowCaret(frame_window_handle);
		SetCaretPos(TLeft + (CursorX - 1) * CharWidth,
			    TTop + (CursorY - 1) * CharHeight);

		CaretActive = 1;
	}

	//
	// Process events until a character in answers has been pressed.
	//
	GotChar = 0;
	while (!GotChar) {
		if (GetMessage(&msg, NULL, 0, 0)) {
			//
			// Test if this is a quit
			//
			if (msg.message == WM_QUIT)
				break;

			//
			// Translate any accelerator keys
			//
			TranslateMessage(&msg);

			//
			// Send the message to the window proc
			//
			DispatchMessage(&msg);
		}

		if (GotChar) {
			//
			// Search for the input character in the answers string
			//
			ch = strchr(answers, EventChar);
			if (ch == NULL) {
				//
				// Not an answer we want
				//
				GotChar = 0;
			}
		}

	}

	if (ShowCursor) {
		HideCaret(frame_window_handle);

		CaretActive = 0;
	}

	return EventChar;
}

/* =============================================================================
 * FUNCTION: get_password_input
 */
void get_password_input(char *password, int Len)
{
	char ch;
	char inputchars[256];
	int Pos;
	int value;

	/* get the printable characters on this system */
	Pos = 0;
	for (value = 0; value < 256; value++) {
		if (isprint(value)) {
			inputchars[Pos] = (char)value;
			Pos++;
		}
	}

	/* add CR, BS and null terminator */
	inputchars[Pos++] = '\010';
	inputchars[Pos++] = '\015';
	inputchars[Pos] = '\0';

	Pos = 0;
	do{
		ch = get_prompt_input("", inputchars, 1);

		if (isprint(ch) && (Pos < Len)) {
			password[Pos] = ch;
			Pos++;
			Printc('*');
		}else if (ch == '\010') {
			//
			// Backspace
			//

			if (Pos > 0) {
				CursorX--;
				Printc(' ');
				CursorX--;
				Pos--;
			}
		}

	} while (ch != '\015');

	password[Pos] = 0;

}

/* =============================================================================
 * FUNCTION: get_num_input
 */
int get_num_input(int defval)
{
	char ch;
	int Pos = 0;
	int value = 0;
	int neg = 0;

	do{
		ch = get_prompt_input("", "-*0123456789\010\015", 1);

		if ((ch == '-') && (Pos == 0)) {
			//
			// Minus
			//
			neg = 1;
			Printc(ch);
			Pos++;
		}
		if (ch == '*')
			return defval;
		else if (ch == '\010') {
			//
			// Backspace
			//

			if (Pos > 0) {
				if ((Pos == 1) && neg)
					neg = 0;
				else
					value = value / 10;

				CursorX--;
				Printc(' ');
				CursorX--;
				Pos--;
			}
		}else if ((ch >= '0') && (ch <= '9')) {
			//
			// digit
			//
			value = value * 10 + (ch - '0');
			Printc(ch);
			Pos++;
		}

	} while (ch != '\015');

	if (Pos == 0)
		return defval;
	else{
		if (neg) value = -value;

		return value;
	}
}

/* =============================================================================
 * FUNCTION: get_dir_input
 */
ActionType get_dir_input(char *prompt, int ShowCursor)
{
	MSG msg;  // generic message
	int got_dir;
	int idx;

	//
	// Display the prompt at the current position
	//
	Print(prompt);

	//
	// Show the cursor if required
	//
	if (ShowCursor) {
		CreateCaret(frame_window_handle,
			    NULL,
			    2, CharHeight);
		ShowCaret(frame_window_handle);
		SetCaretPos(TLeft + (CursorX - 1) * CharWidth,
			    TTop + (CursorY - 1) * CharHeight);

		CaretActive = 1;
	}

	Event = ACTION_NULL;
	got_dir = 0;

	while (!got_dir) {
		if (GetMessage(&msg, NULL, 0, 0)) {
			//
			// Test if this is a quit
			//
			if (msg.message == WM_QUIT)
				break;

			//
			// Translate any accelerator keys
			//
			TranslateMessage(&msg);

			//
			// Send the message to the window proc
			//
			DispatchMessage(&msg);
		}

		idx = 0;

		while ((idx < NUM_DIRS) && (!got_dir)) {
			if (DirActions[idx] == Event)
				got_dir = 1;
			else
				idx++;
		}

	}

	if (ShowCursor) {
		HideCaret(frame_window_handle);

		CaretActive = 0;
	}

	return Event;
}

/* =============================================================================
 * FUNCTION: UpdateStatus
 */
void UpdateStatus(void)
{
	if (CurrentDisplayMode == DISPLAY_TEXT)
		/* Don't redisplay if in text mode */
		return;

	PaintStatus(frame_dc);

}

/* =============================================================================
 * FUNCTION: UpdateEffects
 */
void UpdateEffects(void)
{
	if (CurrentDisplayMode == DISPLAY_TEXT)
		/* Don't redisplay if in text mode */
		return;

	PaintEffects(frame_dc);

}

/* =============================================================================
 * FUNCTION: UpdateStatusAndEffects
 */
void UpdateStatusAndEffects(void)
{
	if (CurrentDisplayMode == DISPLAY_TEXT)
		/* Don't redisplay if in text mode */
		return;

	//
	// Do effects first as update status will mark all effects as current
	//
	PaintEffects(frame_dc);
	PaintStatus(frame_dc);

}

/* =============================================================================
 * FUNCTION: set_display
 */
void set_display(DisplayModeType Mode)
{
	//
	// Save the current settings
	//
	if (CurrentDisplayMode == DISPLAY_MAP) {
		MsgCursorX = CursorX;
		MsgCursorY = CursorY;
		CurrentMsgFormat = CurrentFormat;
	}else if (CurrentDisplayMode == DISPLAY_TEXT) {
		TextCursorX = CursorX;
		TextCursorY = CursorY;
		CurrentTextFormat = CurrentFormat;
	}

	CurrentDisplayMode = Mode;

	//
	// Set the text buffer settings for the new display mode
	//
	if (CurrentDisplayMode == DISPLAY_MAP) {
		CursorX = MsgCursorX;
		CursorY = MsgCursorY;
		CurrentFormat = CurrentMsgFormat;

		Text = MessageChr;
		Format = MessageFmt;
		MaxLine = MAX_MSG_LINES;

		TLeft = MessageLeft;
		TTop = MessageTop;
		TWidth = MessageWidth;
		THeight = MessageHeight;

	}else if (CurrentDisplayMode == DISPLAY_TEXT) {
		CursorX = TextCursorX;
		CursorY = TextCursorY;
		CurrentFormat = CurrentTextFormat;

		Text = TextChr;
		Format = TextFmt;
		MaxLine = MAX_TEXT_LINES;

		TLeft = TextLeft;
		TTop = TextTop;
		TWidth = TextWidth;
		THeight = TextHeight;

	}

	InvalidateRect(frame_window_handle, NULL, 1);
}

/* =============================================================================
 * FUNCTION: IncCursorY
 *
 * DESCRIPTION:
 * Increae the cursor y position, scrolling the text window if requried.
 *
 * PARAMETERS:
 *
 *   Count : The number of lines to increase the cursor y position
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void IncCursorY(int Count)
{
	int Scroll;
	int inc;
	int Line;
	int x;

	inc = Count;
	Scroll = 0;

	while (inc > 0) {
		CursorY = CursorY + 1;

		if (CursorY > MaxLine) {
			Scroll = 1;
			for (Line = 0; Line < (MaxLine - 1); Line++) {
				for (x = 0; x < LINE_LENGTH; x++) {
					Text[Line][x] = Text[Line + 1][x];
					Format[Line][x] = Format[Line + 1][x];
				}
			}
			CursorY--;

			for (x = 0; x < LINE_LENGTH; x++) {
				Text[MaxLine - 1][x] = ' ';
				Format[MaxLine - 1][x] = FORMAT_NORMAL;
			}
		}

		inc--;
	}

	if (Scroll)
		PaintTextWindow(frame_dc);
}

/* =============================================================================
 * FUNCTION: IncCursorX
 *
 * DESCRIPTION:
 * Increase the cursor x position, handling line wrap.
 *
 * PARAMETERS:
 *
 *   Count : The amount to increase the cursor x position.
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void IncCursorX(int Count)
{
	CursorX = CursorX + Count;
	if (CursorX > LINE_LENGTH) {
		CursorX = 1;
		IncCursorY(1);
	}
}

/* =============================================================================
 * FUNCTION: ClearText
 */
void ClearText(void)
{
	int x, y;

	//
	// Clear the text buffer
	//

	for (y = 0; y < MaxLine; y++) {
		for (x = 0; x < LINE_LENGTH; x++) {
			Text[y][x] = ' ';
			Format[y][x] = FORMAT_NORMAL;
		}

		Text[y][LINE_LENGTH] = 0;
	}

	CursorX = 1;
	CursorY = 1;

	//
	// Clear the text area
	//
	PaintTextWindow(frame_dc);
}

/* =============================================================================
 * FUNCTION: UlarnBeep
 */
void UlarnBeep(void)
{
	if (!nobeep) {
		//
		// Middle C, 1/4 second
		//
		Beep(440, 250);
	}
}

/* =============================================================================
 * FUNCTION: MoveCursor
 */
void MoveCursor(int x, int y)
{
	CursorX = x;
	CursorY = y;

	if (CursorX < 1) CursorX = 1;
	if (CursorY < 1) CursorY = 1;
	if (CursorX > LINE_LENGTH) CursorX = LINE_LENGTH;
	if (CursorY > MaxLine) CursorY = MaxLine;
}

/* =============================================================================
 * FUNCTION: Printc
 */
void Printc(char c)
{
	int incx;

	switch (c) {
	case '\t':
		incx = ((((CursorX - 1) / 8) + 1) * 8 + 1) - CursorX;
		IncCursorX(incx);
		break;

	case '\n':
		CursorX = 1;
		IncCursorY(1);
		break;

	default:
		Text[CursorY - 1][CursorX - 1] = c;
		Format[CursorY - 1][CursorX - 1] = CurrentFormat;

		switch (CurrentFormat) {
		case FORMAT_NORMAL:
			SetTextColor(frame_dc, RGB(0, 0, 0));
			break;
		case FORMAT_STANDOUT:
			SetTextColor(frame_dc, RGB(255, 0, 0));
			break;
		case FORMAT_STANDOUT2:
			SetTextColor(frame_dc, RGB(0, 127, 0));
			break;
		case FORMAT_STANDOUT3:
			SetTextColor(frame_dc, RGB(0, 0, 255));
			break;
		default:
			break;
		}

		TextOut(frame_dc,
			TLeft + (CursorX - 1) * CharWidth,
			TTop + (CursorY - 1) * CharHeight,
			&c, 1);

		IncCursorX(1);
		break;
	}
}

/* =============================================================================
 * FUNCTION: Print
 */
void Print(char *string)
{
	int Len;
	int pos;

	if (string == NULL) return;

	Len = strlen(string);

	if (Len == 0) return;

	for (pos = 0; pos < Len; pos++)
		Printc(string[pos]);
}

/* =============================================================================
 * FUNCTION: Printf
 */
void Printf(char *fmt, ...)
{
	char buf[2048];
	va_list argptr;

	va_start(argptr, fmt);
	vsprintf(buf, fmt, argptr);
	va_end(argptr);

	Print(buf);
}

/* =============================================================================
 * FUNCTION: Standout
 */
void Standout(char *String)
{
	CurrentFormat = FORMAT_STANDOUT;

	Print(String);

	CurrentFormat = FORMAT_NORMAL;
}

/* =============================================================================
 * FUNCTION: SetFormat
 */
void SetFormat(FormatType format)
{
	CurrentFormat = format;
}

/* =============================================================================
 * FUNCTION: ClearToEOL
 */
void ClearToEOL(void)
{
	int x;

	for (x = CursorX; x <= LINE_LENGTH; x++) {
		Text[CursorY - 1][x - 1] = ' ';
		Format[CursorY - 1][x - 1] = FORMAT_NORMAL;
	}

	TextOut(frame_dc,
		TLeft + (CursorX - 1) * CharWidth,
		TTop + (CursorY - 1) * CharHeight,
		&(Text[CursorY - 1][CursorX - 1]), (LINE_LENGTH - CursorX) + 1);

}

/* =============================================================================
 * FUNCTION: ClearToEOPage
 */
void ClearToEOPage(int x, int y)
{
	int tx, ty;

	for (tx = x; tx <= LINE_LENGTH; tx++) {
		Text[y - 1][tx - 1] = ' ';
		Format[y - 1][tx - 1] = FORMAT_NORMAL;
	}

	TextOut(frame_dc,
		TLeft + (x - 1) * CharWidth,
		TTop + (y - 1) * CharHeight,
		&(Text[y - 1][x - 1]), (LINE_LENGTH - x) + 1);

	for (ty = y + 1; ty <= MaxLine; ty++) {
		for (tx = 1; tx <= LINE_LENGTH; tx++) {
			Text[ty - 1][tx - 1] = ' ';
			Format[ty - 1][tx - 1] = FORMAT_NORMAL;
		}

		TextOut(frame_dc,
			TLeft,
			TTop + (ty - 1) * CharHeight,
			Text[ty - 1], LINE_LENGTH);

	}

}

/* =============================================================================
 * FUNCTION: show1cell
 */
void show1cell(int x, int y)
{
	int TileId;
	int sx, sy;
	int TileX, TileY;

	/* see nothing if blind   */
	if (c[BLINDCOUNT]) return;

	/* we end up knowing about it */
	know[x][y] = item[x][y];
	if (mitem[x][y].mon != MONST_NONE)
		stealth[x][y] |= STEALTH_SEEN;

	sx = x - MapTileLeft;
	sy = y - MapTileTop;

	if ((sx < 0) || (sx >= MapTileWidth) ||
	    (sy < 0) || (sy >= MapTileHeight)) {
		//
		// Tile is not currently in the visible part of the map,
		// so don't draw anything
		//
		return;
	}

	GetTile(x, y, &TileId);

	TileX = (TileId % 16) * TileWidth;
	TileY = (TileId / 16) * TileHeight;

	BitBlt(frame_dc,
	       MapLeft + sx * TileWidth, MapTop + sy * TileHeight,
	       TileWidth, TileHeight,
	       TileDC, TileX, TileY, SRCCOPY);

}

/* =============================================================================
 * FUNCTION: showplayer
 */
void showplayer(void)
{
	int sx, sy;
	int TileId;
	int TileX, TileY;
	int scroll;

	//
	// Determine if we need to scroll the map
	//
	scroll = calc_scroll();

	if (scroll)
		PaintMap(frame_dc);
	else{
		sx = playerx - MapTileLeft;
		sy = playery - MapTileTop;

		if ((sx >= 0) && (sx < MapTileWidth) &&
		    (sy >= 0) && (sy < MapTileHeight)) {
			if (c[BLINDCOUNT] == 0)
				TileId = PlayerTiles[class_num][(int)sex];
			else
				GetTile(playerx, playery, &TileId);

			TileX = (TileId % 16) * TileWidth;
			TileY = (TileId / 16) * TileHeight;

			BitBlt(frame_dc, MapLeft + sx * TileWidth, MapTop + sy * TileHeight,
			       TileWidth, TileHeight,
			       TileDC, TileX, TileY, SRCCOPY);

			TileId = TILE_CURSOR1;
			TileX = (TileId % 16) * TileWidth;
			TileY = (TileId / 16) * TileHeight;
			BitBlt(frame_dc, MapLeft + sx * TileWidth, MapTop + sy * TileHeight,
			       TileWidth, TileHeight,
			       TileDC, TileX, TileY, SRCAND);

			TileId = TILE_CURSOR2;
			TileX = (TileId % 16) * TileWidth;
			TileY = (TileId / 16) * TileHeight;
			BitBlt(frame_dc, MapLeft + sx * TileWidth, MapTop + sy * TileHeight,
			       TileWidth, TileHeight,
			       TileDC, TileX, TileY, SRCPAINT);

		} /* If player on visible map area */
	}

}

/* =============================================================================
 * FUNCTION: showcell
 */
void showcell(int x, int y)
{
	int minx, maxx;
	int miny, maxy;
	int mx, my;
	int sx, sy;
	int TileX, TileY;
	int TileId;
	int scroll;

	//
	// Determine if we need to scroll the map
	//
	scroll = calc_scroll();

	/*
	 * Decide how much the player knows about around him/her.
	 */
	if (c[AWARENESS]) {
		minx = x - 3;
		maxx = x + 3;
		miny = y - 3;
		maxy = y + 3;
	}else {
		minx = x - 1;
		maxx = x + 1;
		miny = y - 1;
		maxy = y + 1;
	}

	if (c[BLINDCOUNT]) {
		minx = x;
		maxx = x;
		miny = y;
		maxy = y;

		//
		// Redraw the last player position to remove the cursor
		//

		if (!scroll) {
			//
			// Only redraw if the map is not going to be completely redrawn.
			//
			sx = lastpx - MapTileLeft;
			sy = lastpy - MapTileTop;

			if ((sx >= 0) && (sx < MapTileWidth) &&
			    (sy >= 0) && (sy < MapTileHeight)) {
				//
				// Tile is currently visible, so draw it
				//

				GetTile(lastpx, lastpy, &TileId);

				TileX = (TileId % 16) * TileWidth;
				TileY = (TileId / 16) * TileHeight;

				BitBlt(frame_dc,
				       MapLeft + sx * TileWidth, MapTop + sy * TileHeight,
				       TileWidth, TileHeight,
				       TileDC, TileX, TileY, SRCCOPY);
			}

		}

	}

	/*
	 * Limit the area to the map extents
	 */
	if (minx < 0) minx = 0;
	if (maxx > MAXX - 1) maxx = MAXX - 1;
	if (miny < 0) miny = 0;
	if (maxy > MAXY - 1) maxy = MAXY - 1;

	for (my = miny; my <= maxy; my++) {
		for (mx = minx; mx <= maxx; mx++) {
			if ((mx == playerx) && (my == playery)) {
				know[mx][my] = item[mx][my];
				if (!scroll) {
					//
					// Only draw if the entire map is not going to be scrolled
					//
					showplayer();
				}
			}else if ((know[mx][my] != item[mx][my]) ||     /* item changed    */
				  ((mx == lastpx) && (my == lastpy)) || /* last player pos */
				  ((mitem[mx][my].mon != MONST_NONE) && /* unseen monster  */
				   ((stealth[mx][my] & STEALTH_SEEN) == 0))) {
				//
				// Only draw areas not already known (and hence displayed)
				//
				know[mx][my] = item[mx][my];
				if (mitem[mx][my].mon != MONST_NONE)
					stealth[mx][my] |= STEALTH_SEEN;

				if (!scroll) {
					//
					// Only draw the tile if the map is not going to be scrolled
					//
					sx = mx - MapTileLeft;
					sy = my - MapTileTop;

					if ((sx >= 0) && (sx < MapTileWidth) &&
					    (sy >= 0) && (sy < MapTileHeight)) {
						//
						// Tile is currently visible, so draw it
						//

						GetTile(mx, my, &TileId);

						TileX = (TileId % 16) * TileWidth;
						TileY = (TileId / 16) * TileHeight;

						BitBlt(frame_dc,
						       MapLeft + sx * TileWidth, MapTop + sy * TileHeight,
						       TileWidth, TileHeight,
						       TileDC, TileX, TileY, SRCCOPY);

					}

				}

			} // if not known

		}
	}

	if (scroll)
		/* scrolling the map window, so repaint everything and return */
		PaintMap(frame_dc);


}

/* =============================================================================
 * FUNCTION: drawscreen
 */
void drawscreen(void)
{
	InvalidateRect(frame_window_handle, NULL, 1);
}

/* =============================================================================
 * FUNCTION: draws
 */
void draws(int minx, int miny, int maxx, int maxy)
{
	InvalidateRect(frame_window_handle, NULL, 1);
}

/* =============================================================================
 * FUNCTION: mapeffect
 */
void mapeffect(int x, int y, DirEffectsType effect, int dir)
{
	int TileId;
	int sx, sy;
	int TileX, TileY;

	/* see nothing if blind   */
	if (c[BLINDCOUNT]) return;

	sx = x - MapTileLeft;
	sy = y - MapTileTop;

	if ((sx < 0) || (sx >= MapTileWidth) ||
	    (sy < 0) || (sy >= MapTileHeight)) {
		//
		// Tile is not currently in the visible part of the map,
		// so don't draw anything
		//
		return;
	}

	TileId = EffectTile[effect][dir];

	TileX = (TileId % 16) * TileWidth;
	TileY = (TileId / 16) * TileHeight;

	BitBlt(frame_dc,
	       MapLeft + sx * TileWidth, MapTop + sy * TileHeight,
	       TileWidth, TileHeight,
	       TileDC, TileX, TileY, SRCCOPY);
}

/* =============================================================================
 * FUNCTION: magic_effect_frames
 */
int magic_effect_frames(MagicEffectsType fx)
{
	return magicfx_tile[fx].Frames;
}

/* =============================================================================
 * FUNCTION: magic_effect
 */
void magic_effect(int x, int y, MagicEffectsType fx, int frame)
{
	int TileId;
	int sx, sy;
	int TileX, TileY;

	if (frame > magicfx_tile[fx].Frames)
		return;

	/*
	 * draw the tile that is at this location
	 */

	/* see nothing if blind   */
	if (c[BLINDCOUNT]) return;

	sx = x - MapTileLeft;
	sy = y - MapTileTop;

	if ((sx < 0) || (sx >= MapTileWidth) ||
	    (sy < 0) || (sy >= MapTileHeight)) {
		//
		// Tile is not currently in the visible part of the map,
		// so don't draw anything
		//
		return;
	}

	if (magicfx_tile[fx].Overlay) {
		GetTile(x, y, &TileId);

		TileX = (TileId % 16) * TileWidth;
		TileY = (TileId / 16) * TileHeight;

		BitBlt(frame_dc,
		       MapLeft + sx * TileWidth, MapTop + sy * TileHeight,
		       TileWidth, TileHeight,
		       TileDC, TileX, TileY, SRCCOPY);

		TileId = magicfx_tile[fx].Tile1[frame];
		TileX = (TileId % 16) * TileWidth;
		TileY = (TileId / 16) * TileHeight;
		BitBlt(frame_dc, MapLeft + sx * TileWidth, MapTop + sy * TileHeight,
		       TileWidth, TileHeight,
		       TileDC, TileX, TileY, SRCAND);

		TileId = magicfx_tile[fx].Tile2[frame];
		TileX = (TileId % 16) * TileWidth;
		TileY = (TileId / 16) * TileHeight;
		BitBlt(frame_dc, MapLeft + sx * TileWidth, MapTop + sy * TileHeight,
		       TileWidth, TileHeight,
		       TileDC, TileX, TileY, SRCPAINT);
	}else {
		TileId = magicfx_tile[fx].Tile1[frame];
		TileX = (TileId % 16) * TileWidth;
		TileY = (TileId / 16) * TileHeight;
		BitBlt(frame_dc, MapLeft + sx * TileWidth, MapTop + sy * TileHeight,
		       TileWidth, TileHeight,
		       TileDC, TileX, TileY, SRCCOPY);
	}

}

/* =============================================================================
 * FUNCTION: nap
 */
void nap(int delay)
{
	MSG msg; // generic message
	int time_left;

	time_left = delay;
	while (time_left > 0) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			//
			// Test if this is a quit
			//
			if (msg.message == WM_QUIT)
				break;

			//
			// Translate any accelerator keys
			//
			TranslateMessage(&msg);

			//
			// Send the message to the window proc
			//
			DispatchMessage(&msg);
		}

		if (time_left > 100)
			Sleep(100);
		else
			Sleep(time_left);

		time_left -= 100;
	}
}

//
//
//

static char *UserName;

/* =============================================================================
 * FUNCTION: NameDialogProc
 *
 * DESCRIPTION:
 * Windows callback for the enter name dialog.
 *
 * PARAMETERS:
 *
 *   hwnd   : The window handle for the message
 *
 *   msg    : The message received
 *
 *   wparam : The wparam for the message
 *
 *   lparam : The lparam for the message.
 *
 * RETURN VALUE:
 *
 *   0 if the message was handled by this procedure.
 */
static BOOL CALLBACK NameDialogProc(
	HWND hwnd,
	UINT msg,
	WPARAM wparam,
	LPARAM lparam)
{
	int len;
	RECT WRect;
	RECT DRect;
	int WindowWidth;
	int WindowHeight;
	int DialogWidth;
	int DialogHeight;
	int DialogX;
	int DialogY;

	//
	// What is the message
	//
	switch (msg) {
	case WM_INITDIALOG:

		// hwndFocus = (HWND) wparam; // handle of control to receive focus
		// lInitParam = lparam;
		//
		// Do initialization stuff here
		//
		GetWindowRect(frame_window_handle, &WRect);
		WindowWidth = (WRect.right - WRect.left) + 1;
		WindowHeight = (WRect.bottom - WRect.top) + 1;

		GetWindowRect(hwnd, &DRect);
		DialogWidth = (DRect.right - DRect.left) + 1;
		DialogHeight = (DRect.bottom - DRect.top) + 1;

		DialogX = WRect.left + (WindowWidth - DialogWidth) / 2;
		DialogY = WRect.top + (WindowHeight - DialogHeight) / 2;

		SetWindowPos(hwnd, NULL, DialogX, DialogY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
		return TRUE;

	case WM_COMMAND:
	{
		// int wNotifyCode = HIWORD(wparam); // notification code
		int wID = LOWORD(wparam); // item, control, or accelerator identifier
		// HWND hwndCtl = (HWND) lparam;     // handle of control

		switch (wID) {
		case IDOK:

			//
			// Get the number of characeters entered.
			//
			len = (WORD)SendDlgItemMessage(
				hwnd,
				IDC_EDIT_NAME,
				EM_LINELENGTH,
				(WPARAM)0,
				(LPARAM)0);
			if (len > USERNAME_LENGTH)
				len = USERNAME_LENGTH;
			else if (len == 0) {
				EndDialog(hwnd, IDCANCEL);
				return TRUE;
			}

			//
			// Put the number of characters into first word of buffer.
			//

			*((LPWORD)UserName) = (WORD)len;

			/* Get the characters. */

			SendDlgItemMessage(
				hwnd,
				IDC_EDIT_NAME,
				EM_GETLINE,
				(WPARAM)0, /* line 0 */
				(LPARAM)UserName);

			/* Null-terminate the string. */

			UserName[len] = 0;

			EndDialog(hwnd, IDOK);

			return TRUE;

		case IDCANCEL:
			EndDialog(hwnd, IDCANCEL);
			return TRUE;

		default:
			break;
		}
		break;
	}

	default:
		break;

	}

	return FALSE;

}

/* =============================================================================
 * FUNCTION: GetUser
 */
void GetUser(char *username, int *uid)
{
	int result;
	FILE *fp;
	char TmpName[80];
	int TmpPid;
	int Found;
	int n;

	/* Set the buffer the name dialog is to use to store the input name */
	UserName = username;
	/* uid = -1 indicated failure to determine uid */
	*uid = -1;

	if (username[0] == 0) {
		//
		// Name is not yet specified, so ask player for the name
		//
		result = DialogBox(
			main_instance,
			MAKEINTRESOURCE(IDD_DIALOG1),
			frame_window_handle,
			NameDialogProc);

		if (result == IDCANCEL)
			strcpy(UserName, "Anon");

	}

	/* get the Player Id */

	fp = fopen(PIDName, "rb");
	if (fp == NULL) {
		/* Need to create the PID file. */
		fp = fopen(PIDName, "wb");
		if (fp != NULL) {
			*uid = FIRST_PID;
			fwrite(username, USERNAME_LENGTH + 1, 1, fp);
			fwrite(uid, sizeof(int), 1, fp);

			fclose(fp);
		}
	}else {
		/* search the PID file for this player id */
		Found = 0;
		TmpPid = FIRST_PID;
		while (!feof(fp) && !Found) {
			n = fread(TmpName, USERNAME_LENGTH + 1, 1, fp);
			if (n == 1)
				n = fread(&TmpPid, sizeof(int), 1, fp);

			if (n == 1) {
				if (strcmp(TmpName, username) == 0) {
					*uid = TmpPid;
					Found = 1;
				}
			}

		}

		fclose(fp);

		if (!Found) {
			*uid = TmpPid + 1;
			fp = fopen(PIDName, "ab");
			if (fp != NULL) {
				fwrite(username, USERNAME_LENGTH + 1, 1, fp);
				fwrite(uid, sizeof(int), 1, fp);

				fclose(fp);
			}
		}

	}

}

