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
 * GetUser                : Get teh username and user id.
 *
 * =============================================================================
 */

#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/xpm.h>
#include <stdarg.h>
#include <stdio.h>

#include "cursor.bm"

#include "header.h"
#include "ularn_game.h"

#include "x11_simple_menu.h"

#include "config.h"
#include "dungeon.h"
#include "itm.h"
#include "monster.h"
#include "player.h"
#include "ularn_win.h"

// Default size of the ularn window in characters
#define WINDOW_WIDTH 80
#define WINDOW_HEIGHT 25
#define SEPARATOR_WIDTH 8
#define SEPARATOR_HEIGHT 8
#define BORDER_SIZE 8

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
#define M_CTRL 2
#define M_ASCII 255

#define MAX_KEY_BINDINGS 3

struct KeyCodeType {
  int VirtKey;
  int ModKey;
};

#define NUM_DIRS 8
static ActionType DirActions[NUM_DIRS] = {
    ACTION_MOVE_WEST,      ACTION_MOVE_EAST,      ACTION_MOVE_SOUTH,
    ACTION_MOVE_NORTH,     ACTION_MOVE_NORTHEAST, ACTION_MOVE_NORTHWEST,
    ACTION_MOVE_SOUTHEAST, ACTION_MOVE_SOUTHWEST};

/* Default keymap */
/* Allow up to MAX_KEY_BINDINGS per action */
static struct KeyCodeType KeyMap[ACTION_COUNT][MAX_KEY_BINDINGS] = {
    {{0, 0}, {0, 0}, {0, 0}},         // ACTION_NULL
    {{'~', M_ASCII}, {0, 0}, {0, 0}}, // ACTION_DIAG
    {{'h', M_ASCII},
     {XK_KP_Left, M_NONE},
     {XK_Left, M_NONE}},                          // ACTION_MOVE_WEST
    {{'H', M_ASCII}, {XK_Left, M_SHIFT}, {0, 0}}, // ACTION_RUN_WEST
    {{'l', M_ASCII},
     {XK_KP_Right, M_NONE},
     {XK_Right, M_NONE}},                          // ACTION_MOVE_EAST,
    {{'L', M_ASCII}, {XK_Right, M_SHIFT}, {0, 0}}, // ACTION_RUN_EAST,
    {{'j', M_ASCII},
     {XK_KP_Down, M_NONE},
     {XK_Down, M_NONE}},                                   // ACTION_MOVE_SOUTH,
    {{'J', M_ASCII}, {XK_Down, M_SHIFT}, {0, 0}},          // ACTION_RUN_SOUTH,
    {{'k', M_ASCII}, {XK_KP_Up, M_NONE}, {XK_Up, M_NONE}}, // ACTION_MOVE_NORTH,
    {{'K', M_ASCII}, {XK_Up, M_SHIFT}, {0, 0}},            // ACTION_RUN_NORTH,
    {{'u', M_ASCII},
     {XK_KP_Page_Up, M_NONE},
     {XK_Prior, M_NONE}},                          // ACTION_MOVE_NORTHEAST,
    {{'U', M_ASCII}, {XK_Prior, M_SHIFT}, {0, 0}}, // ACTION_RUN_NORTHEAST,
    {{'y', M_ASCII},
     {XK_KP_Home, M_NONE},
     {XK_Home, M_NONE}},                          // ACTION_MOVE_NORTHWEST,
    {{'Y', M_ASCII}, {XK_Home, M_SHIFT}, {0, 0}}, // ACTION_RUN_NORTHWEST,
    {{'n', M_ASCII},
     {XK_KP_Page_Down, M_NONE},
     {XK_Next, M_NONE}},                          // ACTION_MOVE_SOUTHEAST,
    {{'N', M_ASCII}, {XK_Next, M_SHIFT}, {0, 0}}, // ACTION_RUN_SOUTHEAST,
    {{'b', M_ASCII},
     {XK_KP_End, M_NONE},
     {XK_End, M_NONE}},                          // ACTION_MOVE_SOUTHWEST,
    {{'B', M_ASCII}, {XK_End, M_SHIFT}, {0, 0}}, // ACTION_RUN_SOUTHWEST,
    {{'.', M_ASCII}, {0, 0}, {0, 0}},            // ACTION_WAIT,
    {{' ', M_ASCII}, {0, 0}, {0, 0}},            // ACTION_NONE,
    {{'w', M_ASCII}, {0, 0}, {0, 0}},            // ACTION_WIELD,
    {{'W', M_ASCII}, {0, 0}, {0, 0}},            // ACTION_WEAR,
    {{'r', M_ASCII}, {0, 0}, {0, 0}},            // ACTION_READ,
    {{'q', M_ASCII}, {0, 0}, {0, 0}},            // ACTION_QUAFF,
    {{'d', M_ASCII}, {0, 0}, {0, 0}},            // ACTION_DROP,
    {{'c', M_ASCII}, {0, 0}, {0, 0}},            // ACTION_CAST_SPELL,
    {{'o', M_ASCII}, {0, 0}, {0, 0}},            // ACTION_OPEN_DOOR
    {{'C', M_ASCII}, {0, 0}, {0, 0}},            // ACTION_CLOSE_DOOR,
    {{'O', M_ASCII}, {0, 0}, {0, 0}},            // ACTION_OPEN_CHEST
    {{'i', M_ASCII}, {0, 0}, {0, 0}},            // ACTION_INVENTORY,
    {{'e', M_ASCII}, {0, 0}, {0, 0}},            // ACTION_EAT_COOKIE,
    {{'\\', M_ASCII}, {0, 0}, {0, 0}},           // ACTION_LIST_SPELLS,
    {{'?', M_ASCII}, {0, 0}, {0, 0}},            // ACTION_HELP,
    {{'S', M_ASCII}, {0, 0}, {0, 0}},            // ACTION_SAVE,
    {{'Z', M_ASCII}, {0, 0}, {0, 0}},            // ACTION_TELEPORT,
    {{'^', M_ASCII}, {0, 0}, {0, 0}},            // ACTION_IDENTIFY_TRAPS,
    {{'_', M_ASCII}, {0, 0}, {0, 0}},            // ACTION_BECOME_CREATOR,
    {{'+', M_ASCII}, {0, 0}, {0, 0}},            // ACTION_CREATE_ITEM,
    {{'-', M_ASCII}, {0, 0}, {0, 0}},            // ACTION_TOGGLE_WIZARD,
    {{'`', M_ASCII}, {0, 0}, {0, 0}},            // ACTION_DEBUG_MODE,
    {{'T', M_ASCII}, {0, 0}, {0, 0}},            // ACTION_REMOVE_ARMOUR,
    {{'g', M_ASCII}, {0, 0}, {0, 0}},            // ACTION_PACK_WEIGHT,
    {{'v', M_ASCII}, {0, 0}, {0, 0}},            // ACTION_VERSION,
    {{'Q', M_ASCII}, {0, 0}, {0, 0}},            // ACTION_QUIT,
    {{'r', M_CTRL}, {0, 0}, {0, 0}},             // ACTION_REDRAW_SCREEN,
    {{'P', M_ASCII}, {0, 0}, {0, 0}}             // ACTION_SHOW_TAX
};

static struct KeyCodeType RunKeyMap = {XK_KP_Begin, M_NONE};

//
// Variables for X11
//

#define MENU_GAME_SAVE 101
#define MENU_GAME_QUIT 102

#define MENU_ACTION_WAIT 201
#define MENU_ACTION_WIELD 202
#define MENU_ACTION_WEAR 203
#define MENU_ACTION_TAKEOFF 204
#define MENU_ACTION_QUAFF 205
#define MENU_ACTION_READ 206
#define MENU_ACTION_CAST 207
#define MENU_ACTION_EAT 208
#define MENU_ACTION_DROP 209
#define MENU_ACTION_CLOSEDOOR 210

#define MENU_SHOW_DISCOVERIES 301
#define MENU_SHOW_INVENTORY 302
#define MENU_SHOW_TAX 303
#define MENU_SHOW_PACKWEIGHT 304

#define MENU_DISPLAY_REDRAW 401
#define MENU_DISPLAY_BEEP 402
#define MENU_DISPLAY_FONT 403

#define MENU_HELP_HELP 501
#define MENU_HELP_VERSION 502
#define MENU_HELP_ABOUT 503

/* Help menu definitions */

struct XMENU_Item Help_About_Item = /* unused */
    {"About", MENU_HELP_ABOUT, XMENU_UNCHECKED, NULL};

struct XMENU_Item Help_Version_Item = {"Version", MENU_HELP_VERSION,
                                       XMENU_UNCHECKED, NULL};

struct XMENU_Item Help_Help_Item = {"Help", MENU_HELP_HELP, XMENU_UNCHECKED,
                                    &Help_Version_Item};

struct XMENU_Menu Help_Menu = {"Help", NULL, &Help_Help_Item, 0, 0, 0, 0};

/* Display menu definitions */

struct XMENU_Item Display_Font_Item = /* unused */
    {"Font", MENU_DISPLAY_FONT, XMENU_UNCHECKED, NULL};

struct XMENU_Item Display_Beep_Item = {"Beep", MENU_DISPLAY_BEEP,
                                       XMENU_UNCHECKED, NULL};

struct XMENU_Item Display_Redraw_Item = {"Redraw", MENU_DISPLAY_REDRAW,
                                         XMENU_UNCHECKED, &Display_Beep_Item};

struct XMENU_Menu Display_Menu = {
    "Display", &Help_Menu, &Display_Redraw_Item, 0, 0, 0, 0};

/* Show menu definitions */

struct XMENU_Item Show_Packweight_Item = {"Pack weight", MENU_SHOW_PACKWEIGHT,
                                          XMENU_UNCHECKED, NULL};

struct XMENU_Item Show_Tax_Item = {"Tax", MENU_SHOW_TAX, XMENU_UNCHECKED,
                                   &Show_Packweight_Item};

struct XMENU_Item Show_Inventory_Item = {"Inventory", MENU_SHOW_INVENTORY,
                                         XMENU_UNCHECKED, &Show_Tax_Item};

struct XMENU_Item Show_Discoveries_Item = {"Discoveries", MENU_SHOW_DISCOVERIES,
                                           XMENU_UNCHECKED,
                                           &Show_Inventory_Item};

struct XMENU_Menu Show_Menu = {
    "Show", &Display_Menu, &Show_Discoveries_Item, 0, 0, 0, 0};

/* Action Menu definitions */

struct XMENU_Item Action_Closedoor_Item = {"Close door", MENU_ACTION_CLOSEDOOR,
                                           XMENU_UNCHECKED, NULL};

struct XMENU_Item Action_Drop_Item = {"Drop", MENU_ACTION_DROP, XMENU_UNCHECKED,
                                      &Action_Closedoor_Item};

struct XMENU_Item Action_Eat_Item = {"Eat", MENU_ACTION_EAT, XMENU_UNCHECKED,
                                     &Action_Drop_Item};

struct XMENU_Item Action_Cast_Item = {"Cast", MENU_ACTION_CAST, XMENU_UNCHECKED,
                                      &Action_Eat_Item};

struct XMENU_Item Action_Read_Item = {"Read", MENU_ACTION_READ, XMENU_UNCHECKED,
                                      &Action_Cast_Item};

struct XMENU_Item Action_Quaff_Item = {"Quaff", MENU_ACTION_QUAFF,
                                       XMENU_UNCHECKED, &Action_Read_Item};

struct XMENU_Item Action_Takeoff_Item = {"Take off", MENU_ACTION_TAKEOFF,
                                         XMENU_UNCHECKED, &Action_Quaff_Item};

struct XMENU_Item Action_Wear_Item = {"Wear", MENU_ACTION_WEAR, XMENU_UNCHECKED,
                                      &Action_Takeoff_Item};

struct XMENU_Item Action_Wield_Item = {"Wield", MENU_ACTION_WIELD,
                                       XMENU_UNCHECKED, &Action_Wear_Item};

struct XMENU_Item Action_Wait_Item = {"Wait", MENU_ACTION_WAIT, XMENU_UNCHECKED,
                                      &Action_Wield_Item};

struct XMENU_Menu Action_Menu = {"Action", &Show_Menu, &Action_Wait_Item, 0, 0,
                                 0,        0};

/* Game menu definitions */

struct XMENU_Item Game_Quit_Item = {"Quit", MENU_GAME_QUIT, XMENU_UNCHECKED,
                                    NULL};

struct XMENU_Item Game_Save_Item = {"Save", MENU_GAME_SAVE, XMENU_UNCHECKED,
                                    &Game_Quit_Item};

struct XMENU_Menu Game_Menu = {"Game", &Action_Menu, &Game_Save_Item, 0, 0, 0,
                               0};

int ularn_menu_height;

#define INITIAL_WIDTH 400
#define INITIAL_HEIGHT 300

static Display *display = NULL;

static int screen_num;
static int screen_width;
static int screen_height;

static Window root_window = None;

static Window ularn_window = None;

static GC ularn_gc = None;
static XGCValues gc_values;
static unsigned long gc_values_mask;

static unsigned long white_pixel;
static unsigned long black_pixel;

static Colormap colormap;
static XColor LtGrey;
static XColor MidGrey;
static XColor DkGrey;
static XColor Red;
static XColor Green;
static XColor Blue;

static XFontStruct *font_info;
char *font_name = "*-courier-medium-r-*-12-*";

static Pixmap TilePixmap = None;
static Pixmap TilePShape = None;
static XpmAttributes TileAttributes;

static Pixmap CursorPixmap = None;

static int CaretActive = 0;

static int TileWidth = 32;
static int TileHeight = 32;
static int CharHeight;
static int CharWidth;
static int CharAscent;
static int LarnWindowWidth = INITIAL_WIDTH;
static int LarnWindowHeight = INITIAL_HEIGHT;
static int MinWindowWidth;
static int MinWindowHeight;

static int Runkey;
static ActionType Event;
static int GotChar;
static char EventChar;

//
// Bitmaps for tiles
//

static char *TileFilename = LIBDIR "/vlarn_gfx.xpm";

/* Tiles for different character classes, (female, male) */
static int PlayerTiles[8][2] = {
    {165, 181}, /* Ogre */
    {166, 182}, /* Wizard */
    {167, 183}, /* Klingon */
    {168, 184}, /* Elf */
    {169, 185}, /* Rogue */
    {170, 186}, /* Adventurer */
    {171, 187}, /* Dwarf */
    {172, 188}  /* Rambo */
};

#define TILE_CURSOR1 174
#define TILE_CURSOR2 190
#define WALL_TILES 352

/* Tiles for directional effects */
static int EffectTile[EFFECT_COUNT][9] = {
    {191, 198, 196, 194, 192, 195, 193, 197, 199},
    {191, 206, 204, 202, 200, 203, 201, 205, 207},
    {191, 214, 212, 210, 208, 211, 209, 213, 215},
    {191, 222, 220, 218, 216, 219, 217, 221, 223},
    {191, 230, 228, 226, 224, 227, 225, 229, 231}};

#define MAX_MAGICFX_FRAME 8

struct MagicEffectDataType {
  int Overlay;                  /* 0 = no overlay, 1 = overlay     */
  int Frames;                   /* Number of frames in the effect  */
  int Tile1[MAX_MAGICFX_FRAME]; /* The primary tile for this frame */
  int Tile2[MAX_MAGICFX_FRAME]; /* Only used for overlay effects   */
};

static struct MagicEffectDataType magicfx_tile[MAGIC_COUNT] = {
    /* Sparkle */
    {1, /* Overlay this on current tile */
     8,
     {240, 241, 242, 243, 244, 245, 246, 247},
     {248, 249, 250, 251, 252, 253, 254, 255}},

    /* Sleep */
    {0, 6, {256, 272, 288, 304, 320, 336, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}},

    /* Web */
    {0, 6, {257, 273, 289, 305, 321, 337, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}},

    /* Phantasmal forces */
    {0, 6, {258, 274, 290, 306, 322, 338, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}},

    /* Cloud kill */
    {0, 6, {259, 275, 291, 307, 323, 339, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}},

    /* Vaporize rock */
    {0, 6, {260, 276, 292, 308, 324, 340, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}},

    /* Dehydrate */
    {0, 6, {261, 277, 293, 309, 325, 341, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}},

    /* Drain life */
    {0, 6, {262, 278, 294, 310, 326, 342, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}},

    /* Flood */
    {0, 6, {263, 279, 295, 311, 327, 343, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}},

    /* Finger of death */
    {0, 6, {264, 280, 296, 312, 328, 344, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}},

    /* Teleport away */
    {0, 6, {265, 281, 297, 313, 329, 345, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}},

    /* Magic fire */
    {0, 6, {266, 282, 298, 314, 330, 346, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}},

    /* Make wall */
    {0, 6, {267, 283, 299, 315, 331, 347, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}},

    /* Summon demon */
    {0, 6, {268, 284, 300, 316, 332, 348, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}},

    /* Annihilate (scroll) */
    {0, 6, {269, 285, 301, 317, 333, 349, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}}};

//
// Current display mode
//
DisplayModeType CurrentDisplayMode = DISPLAY_TEXT;

//
// Map window position and size
//
static int MapLeft;
static int MapTop;
static int MapAreaLeft;
static int MapAreaTop;
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
#define MAX_MSG_LINES 5
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
static TextLine *Text;
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
static int mimicmonst = MIMIC;

/* =============================================================================
 * Local functions
 */

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
static int calc_scroll(void) {
  int ox, oy;

  ox = MapTileLeft;
  oy = MapTileTop;

  if (MapTileHeight < MAXY) {
    MapTileTop = playery - MapTileHeight / 2;
    if (MapTileTop < 0)
      MapTileTop = 0;

    if ((MapTileTop + MapTileHeight) > MAXY)
      MapTileTop = MAXY - MapTileHeight;
  } else
    MapTileTop = 0;

  if (MapTileWidth < MAXX) {
    MapTileLeft = playerx - MapTileWidth / 2;
    if (MapTileLeft < 0)
      MapTileLeft = 0;

    if ((MapTileLeft + MapTileWidth) > MAXX)
      MapTileLeft = MAXX - MapTileWidth;
  } else
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
 * The new minimum windows size is stored in MinWindowWidth and MinWindowHeight.
 * If the current window size is smaller than this then it is resized.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void CalcMinWindowSize(void) {
  XSizeHints size_hints;

  CharWidth = font_info->max_bounds.width;
  CharHeight = font_info->max_bounds.ascent + font_info->max_bounds.descent;
  CharAscent = font_info->max_bounds.ascent;

  MinWindowWidth = WINDOW_WIDTH * CharWidth;
  MinWindowHeight =
      WINDOW_HEIGHT * CharHeight + 2 * SEPARATOR_HEIGHT + ularn_menu_height;

  //
  // Update the window size
  //
  if (MinWindowWidth > LarnWindowWidth)
    LarnWindowWidth = MinWindowWidth;
  if (MinWindowHeight > LarnWindowHeight)
    LarnWindowHeight = MinWindowHeight;

  size_hints.flags = PMinSize | PBaseSize;
  size_hints.min_width = MinWindowWidth;
  size_hints.min_height = MinWindowHeight;
  size_hints.base_width = LarnWindowWidth;
  size_hints.base_height = LarnWindowHeight;

  XSetNormalHints(display, ularn_window, &size_hints);

  XResizeWindow(display, ularn_window, LarnWindowWidth, LarnWindowHeight);
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
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void PaintStatus(void) {
  char Line[81];
  char Buf[81];
  int i;

  XSetForeground(display, ularn_gc, white_pixel);
  XSetBackground(display, ularn_gc, black_pixel);
  XSetFillStyle(display, ularn_gc, FillSolid);

  XFillRectangle(display, ularn_window, ularn_gc, StatusLeft, StatusTop,
                 StatusWidth, StatusHeight);

  XSetForeground(display, ularn_gc, black_pixel);
  XSetBackground(display, ularn_gc, white_pixel);
  XSetFillStyle(display, ularn_gc, FillSolid);

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

  XDrawString(display, ularn_window, ularn_gc, StatusLeft,
              StatusTop + CharAscent, Line, strlen(Line));

  //
  // Format the second line of the status
  //
  sprintf(Buf, "%d (%d)", (int)c[HP], (int)c[HPMAX]);

  sprintf(Line,
          "HP: %11.11s STR=%-2ld INT=%-2ld WIS=%-2ld CON=%-2ld DEX=%-2ld "
          "CHA=%-2ld LV:",
          Buf, c[STRENGTH] + c[STREXTRA], c[INTELLIGENCE], c[WISDOM],
          c[CONSTITUTION], c[DEXTERITY], c[CHARISMA]);

  if ((level == 0) || (wizard))
    c[TELEFLAG] = 0;

  if (c[TELEFLAG])
    strcat(Line, " ?");
  else
    strcat(Line, levelname[level]);

  sprintf(Buf, "  Gold: %-8ld", c[GOLD]);
  strcat(Line, Buf);

  XDrawString(display, ularn_window, ularn_gc, StatusLeft,
              StatusTop + CharHeight + CharAscent, Line, strlen(Line));

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
} bot_data[] = {{STEALTH, "Stealth   "},      {UNDEADPRO, "Undead Pro"},
                {SPIRITPRO, "Spirit Pro"},    {CHARMCOUNT, "Charm     "},
                {TIMESTOP, "Time Stop "},     {HOLDMONST, "Hold Monst"},
                {GIANTSTR, "Giant Str "},     {FIRERESISTANCE, "Fire Resit"},
                {DEXCOUNT, "Dexterity "},     {STRCOUNT, "Strength  "},
                {SCAREMONST, "Scare     "},   {HASTESELF, "Haste Self"},
                {CANCELLATION, "Cancel    "}, {INVISIBILITY, "Invisible "},
                {ALTPRO, "Protect 3 "},       {PROTECTIONTIME, "Protect 2 "},
                {WTW, "Wall-Walk "}};

/* =============================================================================
 * FUNCTION: PaintEffects
 *
 * DESCRIPTION:
 * Paint the effects display.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void PaintEffects(void) {
  int i, idx;
  int WasSet;
  int IsSet;

  if (Repaint) {
    XSetForeground(display, ularn_gc, white_pixel);
    XSetBackground(display, ularn_gc, black_pixel);
    XSetFillStyle(display, ularn_gc, FillSolid);

    XFillRectangle(display, ularn_window, ularn_gc, EffectsLeft, EffectsTop,
                   EffectsWidth, EffectsHeight);
  }

  for (i = 0; i < 17; i++) {
    idx = bot_data[i].typ;
    WasSet = (cbak[idx] != 0);
    IsSet = (c[idx] != 0);

    if ((Repaint) || (IsSet != WasSet)) {
      if (IsSet) {
        XSetForeground(display, ularn_gc, black_pixel);
        XSetBackground(display, ularn_gc, white_pixel);
        XSetFillStyle(display, ularn_gc, FillSolid);

        XDrawString(display, ularn_window, ularn_gc, EffectsLeft,
                    EffectsTop + i * CharHeight + CharAscent,
                    bot_data[i].string, strlen(bot_data[i].string));
      } else {
        XSetForeground(display, ularn_gc, white_pixel);
        XSetBackground(display, ularn_gc, black_pixel);
        XSetFillStyle(display, ularn_gc, FillSolid);

        XFillRectangle(display, ularn_window, ularn_gc, EffectsLeft,
                       EffectsTop + i * CharHeight, EffectsWidth, CharHeight);
      }
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
static void GetTile(int x, int y, int *TileId) {
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
  } else {
    k = mitem[x][y].mon;
    if (k != 0) {
      if ((c[BLINDCOUNT] == 0) && (((stealth[x][y] & STEALTH_SEEN) != 0) ||
                                   ((stealth[x][y] & STEALTH_AWAKE) != 0))) {
        //
        // There is a monster here and the player is not blind and the
        // monster is seen or awake.
        //
        if (k == MIMIC) {
          if ((gtime % 10) == 0)
            while ((mimicmonst = rnd(MAXMONST)) == INVISIBLESTALKER)
              ;

          *TileId = monsttilelist[mimicmonst];
        } else if ((k == INVISIBLESTALKER) && (c[SEEINVISIBLE] == 0))
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
    else {
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
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void PaintMap(void) {
  int x, y;
  int sx, sy;
  int mx, my;
  int TileId;
  int TileX;
  int TileY;

  if (Repaint) {
    XSetForeground(display, ularn_gc, black_pixel);

    XFillRectangle(display, ularn_window, ularn_gc, MapAreaLeft, MapAreaTop,
                   MapWidth, MapHeight);
  }

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

      XCopyArea(display, TilePixmap, ularn_window, ularn_gc, TileX, TileY,
                TileWidth, TileHeight, MapLeft + sx * TileWidth,
                MapTop + sy * TileHeight);

      sy++;
    }

    sx++;
  }

  sx = playerx - MapTileLeft;
  sy = playery - MapTileTop;

  if ((sx >= 0) && (sx < MapTileWidth) && (sy >= 0) && (sy < MapTileHeight)) {
    TileId = TILE_CURSOR1;
    TileX = (TileId % 16) * TileWidth;
    TileY = (TileId / 16) * TileHeight;

    XSetClipOrigin(display, ularn_gc, MapLeft + sx * TileWidth - TileX,
                   MapTop + sy * TileHeight - TileY);
    XSetClipMask(display, ularn_gc, TilePShape);
    XCopyArea(display, TilePixmap, ularn_window, ularn_gc, TileX, TileY,
              TileWidth, TileHeight, MapLeft + sx * TileWidth,
              MapTop + sy * TileHeight);

    XSetClipOrigin(display, ularn_gc, 0, 0);
    XSetClipMask(display, ularn_gc, None);
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
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void PaintTextWindow(void) {
  int sx, ex, y;
  FormatType Fmt;
  int FillX, FillY;
  int FillWidth, FillHeight;
  XGCValues values;

  FillX = TLeft;
  FillY = TTop;
  FillWidth = TWidth;
  FillHeight = THeight;

  if (CurrentDisplayMode == DISPLAY_TEXT) {
    if (ShowTextBorder) {
      //
      // Clear the drawable area
      //
      FillX = 0;
      FillY = ularn_menu_height;
      FillWidth = LarnWindowWidth;
      FillHeight = LarnWindowHeight - ularn_menu_height;

      XSetForeground(display, ularn_gc, black_pixel);
      XSetBackground(display, ularn_gc, white_pixel);
      XSetFillStyle(display, ularn_gc, FillSolid);

      XFillRectangle(display, ularn_window, ularn_gc, FillX, FillY, FillWidth,
                     FillHeight);

      XSetForeground(display, ularn_gc, white_pixel);
      XSetBackground(display, ularn_gc, black_pixel);

      values.line_width = 2;
      XChangeGC(display, ularn_gc, GCLineWidth, &values);

      XDrawArc(display, ularn_window, ularn_gc, TLeft - 8, TTop - 8, 16, 16,
               90 * 64, 90 * 64);

      XDrawArc(display, ularn_window, ularn_gc, TLeft - 8, TTop + THeight - 8,
               16, 16, 180 * 64, 90 * 64);

      XDrawArc(display, ularn_window, ularn_gc, TLeft + TWidth - 8, TTop - 8,
               16, 16, 0 * 64, 90 * 64);

      XDrawArc(display, ularn_window, ularn_gc, TLeft + TWidth - 8,
               TTop + THeight - 8, 16, 16, 270 * 64, 90 * 64);

      XDrawLine(display, ularn_window, ularn_gc, TLeft, TTop - 8,
                TLeft + TWidth, TTop - 8);

      XDrawLine(display, ularn_window, ularn_gc, TLeft, TTop + THeight + 8,
                TLeft + TWidth, TTop + THeight + 8);

      XDrawLine(display, ularn_window, ularn_gc, TLeft - 8, TTop, TLeft - 8,
                TTop + THeight);

      XDrawLine(display, ularn_window, ularn_gc, TLeft + TWidth + 8, TTop,
                TLeft + TWidth + 8, TTop + THeight);

      values.line_width = 0;
      XChangeGC(display, ularn_gc, GCLineWidth, &values);

      FillX = TLeft;
      FillY = TTop;
      FillWidth = TWidth;
      FillHeight = THeight;

    } else {
      //
      // Not enough space around the text area for a border
      // Fill the entire drawable area
      //
      FillX = 0;
      FillY = ularn_menu_height;
      FillWidth = LarnWindowWidth;
      FillHeight = LarnWindowHeight - ularn_menu_height;
    }
  }

  XSetForeground(display, ularn_gc, white_pixel);
  XSetBackground(display, ularn_gc, black_pixel);
  XSetFillStyle(display, ularn_gc, FillSolid);

  XFillRectangle(display, ularn_window, ularn_gc, FillX, FillY, FillWidth,
                 FillHeight);

  XSetForeground(display, ularn_gc, black_pixel);
  XSetBackground(display, ularn_gc, white_pixel);
  XSetFillStyle(display, ularn_gc, FillSolid);

  for (y = 0; y < MaxLine; y++) {
    sx = 0;

    while (sx < LINE_LENGTH) {
      Fmt = Format[y][sx];
      ex = sx;

      while ((ex < LINE_LENGTH) && (Format[y][ex] == Fmt))
        ex++;

      switch (Fmt) {
      case FORMAT_NORMAL:
        XSetForeground(display, ularn_gc, black_pixel);
        break;
      case FORMAT_STANDOUT:
        XSetForeground(display, ularn_gc, Red.pixel);
        break;
      case FORMAT_STANDOUT2:
        XSetForeground(display, ularn_gc, Green.pixel);
        break;
      case FORMAT_STANDOUT3:
        XSetForeground(display, ularn_gc, Blue.pixel);
        break;
      default:
        break;
      }

      XDrawString(display, ularn_window, ularn_gc, TLeft + sx * CharWidth,
                  TTop + y * CharHeight + CharAscent, Text[y] + sx, ex - sx);

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
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void PaintMapWindow(void) {

  //
  // Draw separators between the different window areas
  //

  //
  // Message area
  //
  XSetForeground(display, ularn_gc, LtGrey.pixel);
  XFillRectangle(display, ularn_window, ularn_gc, MessageLeft,
                 MessageTop - SEPARATOR_HEIGHT, MessageWidth, 2);

  XSetForeground(display, ularn_gc, MidGrey.pixel);
  XFillRectangle(display, ularn_window, ularn_gc, MessageLeft,
                 MessageTop - SEPARATOR_HEIGHT + 2, MessageWidth, 4);

  XSetForeground(display, ularn_gc, DkGrey.pixel);
  XFillRectangle(display, ularn_window, ularn_gc, MessageLeft,
                 MessageTop - SEPARATOR_HEIGHT + 6, MessageWidth, 2);

  //
  // Status area
  //
  XSetForeground(display, ularn_gc, LtGrey.pixel);
  XFillRectangle(display, ularn_window, ularn_gc, StatusLeft,
                 StatusTop - SEPARATOR_HEIGHT, StatusWidth, 2);

  XSetForeground(display, ularn_gc, MidGrey.pixel);
  XFillRectangle(display, ularn_window, ularn_gc, StatusLeft,
                 StatusTop - SEPARATOR_HEIGHT + 2, StatusWidth, 4);

  XSetForeground(display, ularn_gc, DkGrey.pixel);
  XFillRectangle(display, ularn_window, ularn_gc, StatusLeft,
                 StatusTop - SEPARATOR_HEIGHT + 6, StatusWidth, 2);

  //
  // Effects area
  //
  XSetForeground(display, ularn_gc, LtGrey.pixel);
  XFillRectangle(display, ularn_window, ularn_gc, EffectsLeft - SEPARATOR_WIDTH,
                 EffectsTop, 2, EffectsHeight);

  XSetForeground(display, ularn_gc, MidGrey.pixel);
  XFillRectangle(display, ularn_window, ularn_gc,
                 EffectsLeft - SEPARATOR_WIDTH + 2, EffectsTop, 4,
                 EffectsHeight + 2);

  XSetForeground(display, ularn_gc, DkGrey.pixel);
  XFillRectangle(display, ularn_window, ularn_gc,
                 EffectsLeft - SEPARATOR_WIDTH + 6, EffectsTop, 2,
                 EffectsHeight);

  PaintStatus();
  PaintEffects();
  PaintMap();
  PaintTextWindow();
}

/* =============================================================================
 * FUNCTION: PaintWindow
 *
 * DESCRIPTION:
 * Repaint the window.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void PaintWindow(void) {

  Repaint = 1;

  XMENU_Redraw();

  if (CurrentDisplayMode == DISPLAY_MAP)
    PaintMapWindow();
  else
    PaintTextWindow();

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
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void Resize(void) {
  XWindowAttributes win_attr;
  int ClientWidth;
  int ClientHeight;

  /* XXX trn - check status rc return val */
  /*Status rc;

     rc = */
  XGetWindowAttributes(display, ularn_window, &win_attr);

  LarnWindowWidth = win_attr.width;
  LarnWindowHeight = win_attr.height;

  ClientWidth = LarnWindowWidth;
  ClientHeight = LarnWindowHeight;

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
  EffectsTop = ularn_menu_height;
  EffectsWidth = CharWidth * 10;
  EffectsHeight = StatusTop - SEPARATOR_HEIGHT - EffectsTop;

  //
  // Calculate the size and position of the map window
  //
  MapAreaLeft = 0;
  MapAreaTop = ularn_menu_height;
  MapLeft = 0;
  MapTop = MapAreaTop;
  MapWidth = EffectsLeft - SEPARATOR_WIDTH;
  MapHeight = StatusTop - SEPARATOR_HEIGHT - MapAreaTop;
  MapTileWidth = MapWidth / TileWidth;
  MapTileHeight = MapHeight / TileHeight;

  //
  // Calculate the size and position of the text window
  //

  TextWidth = CharWidth * LINE_LENGTH;
  TextHeight = CharHeight * MAX_TEXT_LINES;

  TextLeft = (ClientWidth - TextWidth) / 2;
  TextTop =
      ularn_menu_height + (ClientHeight - TextHeight - ularn_menu_height) / 2;

  //
  // Check if should draw a border around the text page when it is displayed
  //
  ShowTextBorder = (TextLeft >= BORDER_SIZE) &&
                   (TextTop >= (ularn_menu_height + BORDER_SIZE));

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
    MapTop = MapAreaTop + (MapHeight - MapTileHeight * TileHeight) / 2;
  }

  if (CurrentDisplayMode == DISPLAY_MAP) {
    TLeft = MessageLeft;
    TTop = MessageTop;
    TWidth = MessageWidth;
    THeight = MessageHeight;
  } else {
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
  PaintWindow();

  if (CaretActive) {

    XSetForeground(display, ularn_gc, black_pixel);
    XSetClipOrigin(display, ularn_gc, TLeft + (CursorX - 1) * CharWidth,
                   TTop + (CursorY - 1) * CharHeight + CharAscent);
    XSetClipMask(display, ularn_gc, CursorPixmap);
    XCopyPlane(display, CursorPixmap, ularn_window, ularn_gc, 0, 0,
               cursor_width, cursor_height, TLeft + (CursorX - 1) * CharWidth,
               TTop + (CursorY - 1) * CharHeight + CharAscent, 1);

    XSetClipOrigin(display, ularn_gc, 0, 0);
    XSetClipMask(display, ularn_gc, None);
  }
}

/* =============================================================================
 * FUNCTION: handle_event
 *
 * DESCRIPTION:
 * This procedure handles the processing for X events.
 *
 * PARAMETERS:
 *
 *   event : The X event to process.
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void handle_event(XEvent *event) {
  int MenuId;

  /* Handle menu input */
  MenuId = XMENU_HandleEvent(event);
  if (MenuId >= 0) {

    switch (MenuId) {
    case MENU_GAME_SAVE:
      Event = ACTION_SAVE;
      break;

    case MENU_GAME_QUIT:
      Event = ACTION_QUIT;
      break;

    case MENU_ACTION_WAIT:
      Event = ACTION_WAIT;
      break;

    case MENU_ACTION_WIELD:
      Event = ACTION_WIELD;
      break;

    case MENU_ACTION_WEAR:
      Event = ACTION_WEAR;
      break;

    case MENU_ACTION_TAKEOFF:
      Event = ACTION_REMOVE_ARMOUR;
      break;

    case MENU_ACTION_QUAFF:
      Event = ACTION_QUAFF;
      break;

    case MENU_ACTION_READ:
      Event = ACTION_READ;
      break;

    case MENU_ACTION_CAST:
      Event = ACTION_CAST_SPELL;
      break;

    case MENU_ACTION_EAT:
      Event = ACTION_EAT_COOKIE;
      break;

    case MENU_ACTION_DROP:
      Event = ACTION_DROP;
      break;

    case MENU_ACTION_CLOSEDOOR:
      Event = ACTION_CLOSE_DOOR;
      break;

    case MENU_SHOW_DISCOVERIES:
      Event = ACTION_LIST_SPELLS;
      break;

    case MENU_SHOW_INVENTORY:
      Event = ACTION_INVENTORY;
      break;

    case MENU_SHOW_TAX:
      Event = ACTION_SHOW_TAX;
      break;

    case MENU_SHOW_PACKWEIGHT:
      Event = ACTION_PACK_WEIGHT;
      break;

    case MENU_DISPLAY_REDRAW:
      Event = ACTION_REDRAW_SCREEN;
      break;

    case MENU_DISPLAY_BEEP:
      nobeep = !nobeep;
      /* Set menu checkmarks */

      if (nobeep)
        XMENU_SetCheck(MENU_DISPLAY_BEEP, XMENU_UNCHECKED);
      else
        XMENU_SetCheck(MENU_DISPLAY_BEEP, XMENU_CHECKED);
      break;

    case MENU_DISPLAY_FONT:
      break;

    case MENU_HELP_HELP:
      Event = ACTION_HELP;
      break;

    case MENU_HELP_VERSION:
      Event = ACTION_VERSION;
      break;

    case MENU_HELP_ABOUT:
      break;

    default:
      break;
    }

    return;
  }

  //
  // What is the message
  //
  switch (event->type) {
  case Expose:

    if (event->xexpose.count == 0)
      PaintWindow();
    break;

  case ConfigureNotify:
    Resize();
    break;

  case KeyPress: {
    ActionType Action;
    int ModKey = 0;
    int Found = 0;
    KeySym key_symbol;
    XComposeStatus compose;
    char KeyString[40];
    int i;

    XLookupString(&(event->xkey), KeyString, 40, &key_symbol, &compose);

    if ((event->xkey.state & ShiftMask) != 0)
      ModKey |= M_SHIFT;

    if ((event->xkey.state & ControlMask) != 0)
      ModKey |= M_CTRL;

    /* Get ASCII character */

    EventChar = KeyString[0];
    GotChar = (EventChar != 0);

    /* Decode key press as a ULarn Action */

    Action = ACTION_NULL;

    /* Check virtual key bindings */

    while ((Action < ACTION_COUNT) && (!Found)) {
      for (i = 0; i < MAX_KEY_BINDINGS; i++) {

        if (KeyMap[Action][i].ModKey != M_ASCII) {
          /* Virtual key binding */
          if ((key_symbol == KeyMap[Action][i].VirtKey) &&
              (KeyMap[Action][i].ModKey == ModKey))
            Found = 1;
        }
      }

      if (!Found)
        Action++;
    }

    /*
     * Check ASCII key bindings if no virtual key matches and
     * got a valid ASCII char
     */

    if (!Found && GotChar) {
      Action = ACTION_NULL;

      while ((Action < ACTION_COUNT) && (!Found)) {
        for (i = 0; i < MAX_KEY_BINDINGS; i++) {
          if (KeyMap[Action][i].ModKey == M_ASCII) {
            /* ASCII key binding */
            if (EventChar == KeyMap[Action][i].VirtKey)
              Found = 1;
          }
        }

        if (!Found)
          Action++;
      }
    }

    if (Found)
      Event = Action;
    else {
      /* check run key */
      if ((key_symbol == RunKeyMap.VirtKey) && (RunKeyMap.ModKey == ModKey))
        Runkey = 1;
    }

    break;
  }

  default:
    break;
  }
}

#define MASK_TILES 9
static int MaskTiles[MASK_TILES][2] = {
    {240, 248}, {241, 249}, {242, 250},
    {243, 251}, {244, 252}, {245, 253},
    {246, 254}, {247, 255}, {TILE_CURSOR1, TILE_CURSOR2}};

/* =============================================================================
 * FUNCTION: MakeTileMasks
 *
 * DESCRIPTION:
 * This procedure makes the transparency masks for overlay tiles.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void MakeTileMasks(void) {
  XImage *Image1;
  XImage *Image2;
  XImage *Mask;
  GC gc;
  XGCValues values;
  int Tile;
  int x, y;
  int t1x, t1y;
  int t2x, t2y;
  int p1, p2;

  if (TilePShape == None)
    printf("TilePShape not allocated!\n");

  values.foreground = 1;
  values.background = 0;
  gc = XCreateGC(display, TilePShape, GCForeground | GCBackground, &values);

  for (Tile = 0; Tile < MASK_TILES; Tile++) {
    t1x = (MaskTiles[Tile][0] % 16) * TileWidth;
    t1y = (MaskTiles[Tile][0] / 16) * TileHeight;

    t2x = (MaskTiles[Tile][1] % 16) * TileWidth;
    t2y = (MaskTiles[Tile][1] / 16) * TileHeight;

    Image1 = XGetImage(display, TilePixmap, t1x, t1y, TileWidth, TileHeight,
                       0xffffffff, XYPixmap);
    Image2 = XGetImage(display, TilePixmap, t2x, t2y, TileWidth, TileHeight,
                       0xffffffff, XYPixmap);
    Mask = XGetImage(display, TilePShape, t1x, t1y, TileWidth, TileHeight,
                     0xffffffff, XYPixmap);

    for (y = 0; y < TileHeight; y++) {

      for (x = 0; x < TileWidth; x++) {
        p1 = XGetPixel(Image1, x, y);
        p2 = XGetPixel(Image2, x, y);

        if (p1 != p2)
          XPutPixel(Mask, x, y, 0);
        else
          XPutPixel(Mask, x, y, 1);
      }

      XPutImage(display, TilePShape, gc, Mask, 0, 0, t1x, t1y, TileWidth,
                TileHeight);
    }
  }

  XFreeGC(display, gc);
}

/* =============================================================================
 * Exported functions
 */

/* =============================================================================
 * FUNCTION: init_app
 */
int init_app(char *DisplayName) {
  int x, y;
  /* XXX trn */
  /*Visual *visual;*/
  int rc;
  char *LoadingText = "Loading data...";
  char *UlarnText = "VLarn";
  XTextProperty xtext;

  display = XOpenDisplay(DisplayName);

  if (display == NULL) {
    fprintf(stderr, "Error: Cannot connect to X server %s\n", DisplayName);
    /*return 0;*/
    exit(1);
  }

  screen_num = DefaultScreen(display);

  white_pixel = WhitePixel(display, screen_num);
  black_pixel = BlackPixel(display, screen_num);

  screen_width = DisplayWidth(display, screen_num);
  screen_height = DisplayHeight(display, screen_num);

  root_window = RootWindow(display, screen_num);

  ularn_window = XCreateSimpleWindow(
      display, root_window, 10, 10, INITIAL_WIDTH, INITIAL_HEIGHT, 2,
      BlackPixel(display, screen_num), WhitePixel(display, screen_num));

  /* Set the window name */

  if (XStringListToTextProperty(&LoadingText, 1, &xtext) == 0) {
    printf("XStringListToTextProperty ran out of memory\n");
    return 0;
  }

  XSetWMName(display, ularn_window, &xtext);

  XFree(xtext.value);

  XMapWindow(display, ularn_window);

  XFlush(display);
  XSync(display, 0);

  /* Create the graphics context */

  gc_values.cap_style = CapButt;
  gc_values.join_style = JoinBevel;
  gc_values_mask = GCCapStyle | GCJoinStyle;
  ularn_gc = XCreateGC(display, ularn_window, gc_values_mask, &gc_values);

  /*
   * Install the menu and set initial check states
   */

  XMENU_SetMenu(display, ularn_window, &Game_Menu, NULL, PaintWindow);
  ularn_menu_height = XMENU_GetMenuHeight();

  if (nobeep)
    XMENU_SetCheck(MENU_DISPLAY_BEEP, XMENU_UNCHECKED);
  else
    XMENU_SetCheck(MENU_DISPLAY_BEEP, XMENU_CHECKED);

  /* Get colours required */

  /*visual = */ DefaultVisual(display, DefaultScreen(display));
  colormap = XDefaultColormap(display, screen_num);

  LtGrey.red = 192 * 256;
  LtGrey.green = 192 * 256;
  LtGrey.blue = 192 * 256;
  XAllocColor(display, colormap, &LtGrey);

  MidGrey.red = 128 * 256;
  MidGrey.green = 128 * 256;
  MidGrey.blue = 128 * 256;
  XAllocColor(display, colormap, &MidGrey);

  DkGrey.red = 96 * 256;
  DkGrey.green = 96 * 256;
  DkGrey.blue = 96 * 256;
  XAllocColor(display, colormap, &DkGrey);

  Red.red = 255 * 256;
  Red.green = 0;
  Red.blue = 0;
  XAllocColor(display, colormap, &Red);

  Green.red = 0;
  Green.green = 128 * 256;
  Green.blue = 0;
  XAllocColor(display, colormap, &Green);

  Blue.red = 0;
  Blue.green = 0;
  Blue.blue = 255 * 256;
  XAllocColor(display, colormap, &Blue);

  font_info = XLoadQueryFont(display, font_name);
  if (!font_info) {
    fprintf(stderr, "Error: XLoadQueryFont: failed loading font '%s'\n",
            font_name);
    return 0;
  }

  XSetFont(display, ularn_gc, font_info->fid);

  XSelectInput(display, ularn_window,
               ExposureMask | KeyPressMask | StructureNotifyMask |
                   XMENU_EVENT_MASK);

  CursorPixmap = XCreateBitmapFromData(display, ularn_window, cursor_bits,
                                       cursor_width, cursor_height);

  TileAttributes.valuemask = XpmCloseness;
  TileAttributes.closeness = 25000;

  rc = XpmReadFileToPixmap(display, ularn_window, TileFilename, &TilePixmap,
                           &TilePShape, &TileAttributes);

  if (rc < XpmSuccess) {
    printf("Error reading pixmap: %s\n", XpmGetErrorString(rc));
    printf("Retrying with inferior colours\n");

    TileAttributes.valuemask = XpmCloseness;
    TileAttributes.closeness = 50000;

    rc = XpmReadFileToPixmap(display, ularn_window, TileFilename, &TilePixmap,
                             &TilePShape, &TileAttributes);
    if (rc < XpmSuccess)
      printf("Error reading pixmap: %s\n", XpmGetErrorString(rc));
  }

  MakeTileMasks();

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

  if (XStringListToTextProperty(&UlarnText, 1, &xtext) == 0) {
    printf("XStringListToTextProperty ran out of memory\n");
    return 0;
  }

  XSetWMName(display, ularn_window, &xtext);

  XFree(xtext.value);

  //
  // Set the initial window size
  //
  CalcMinWindowSize();

  Resize();

  return 1;
}

/* =============================================================================
 * FUNCTION: close_app
 */
void close_app(void) {

  if (TilePixmap != None)
    XFreePixmap(display, TilePixmap);

  if (TilePShape != None)
    XFreePixmap(display, TilePShape);

  if (CursorPixmap != None)
    XFreePixmap(display, CursorPixmap);

  if (ularn_gc != None) {
    XFreeColormap(display, colormap);
    XFreeGC(display, ularn_gc);
  }

  if (ularn_window != None)
    XDestroyWindow(display, ularn_window);

  sleep(4);

  XFlush(display);
  XSync(display, 1);

  if (display != NULL)
    XCloseDisplay(display);
}

/* =============================================================================
 * FUNCTION: get_normal_input
 */
ActionType get_normal_input(void) {
  XEvent xevent; // The X event
  int idx;
  int got_dir;

  Event = ACTION_NULL;
  Runkey = 0;

  while (Event == ACTION_NULL) {
    XNextEvent(display, &xevent);

    handle_event(&xevent);

    //
    // Clear enhanced interface events in enhanced interface is not active
    //
    if (!enhance_interface) {
      if ((Event == ACTION_OPEN_DOOR) || (Event == ACTION_OPEN_CHEST))
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
char get_prompt_input(char *prompt, char *answers, int ShowCursor) {
  XEvent xevent; // The X event
  char *ch;

  Print(prompt);

  if (ShowCursor) {

    XSetForeground(display, ularn_gc, black_pixel);
    XSetClipOrigin(display, ularn_gc, TLeft + (CursorX - 1) * CharWidth,
                   TTop + (CursorY - 1) * CharHeight + CharAscent);
    XSetClipMask(display, ularn_gc, CursorPixmap);
    XCopyPlane(display, CursorPixmap, ularn_window, ularn_gc, 0, 0,
               cursor_width, cursor_height, TLeft + (CursorX - 1) * CharWidth,
               TTop + (CursorY - 1) * CharHeight + CharAscent, 1);

    XSetClipOrigin(display, ularn_gc, 0, 0);
    XSetClipMask(display, ularn_gc, None);

    CaretActive = 1;
  }

  //
  // Process events until a character in answers has been pressed.
  //
  GotChar = 0;
  while (!GotChar) {
    XNextEvent(display, &xevent);

    handle_event(&xevent);

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

    XSetForeground(display, ularn_gc, white_pixel);
    XSetClipOrigin(display, ularn_gc, TLeft + (CursorX - 1) * CharWidth,
                   TTop + (CursorY - 1) * CharHeight + CharAscent);
    XSetClipMask(display, ularn_gc, CursorPixmap);
    XCopyPlane(display, CursorPixmap, ularn_window, ularn_gc, 0, 0,
               cursor_width, cursor_height, TLeft + (CursorX - 1) * CharWidth,
               TTop + (CursorY - 1) * CharHeight + CharAscent, 1);

    XSetClipOrigin(display, ularn_gc, 0, 0);
    XSetClipMask(display, ularn_gc, None);

    CaretActive = 0;
  }

  return EventChar;
}

/* =============================================================================
 * FUNCTION: get_password_input
 */
void get_password_input(char *password, int Len) {
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
  do {
    ch = get_prompt_input("", inputchars, 1);

    if (isprint((int)ch) && (Pos < Len)) {
      password[Pos] = ch;
      Pos++;
      Printc('*');
    } else if (ch == '\010') {
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
int get_num_input(int defval) {
  char ch;
  int Pos = 0;
  int value = 0;
  int neg = 0;

  do {
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
    } else if ((ch >= '0') && (ch <= '9')) {
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
  else {
    if (neg)
      value = -value;

    return value;
  }
}

/* =============================================================================
 * FUNCTION: get_dir_input
 */
ActionType get_dir_input(char *prompt, int ShowCursor) {
  XEvent xevent;
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

    XSetForeground(display, ularn_gc, black_pixel);
    XSetClipOrigin(display, ularn_gc, TLeft + (CursorX - 1) * CharWidth,
                   TTop + (CursorY - 1) * CharHeight + CharAscent);
    XSetClipMask(display, ularn_gc, CursorPixmap);
    XCopyPlane(display, CursorPixmap, ularn_window, ularn_gc, 0, 0,
               cursor_width, cursor_height, TLeft + (CursorX - 1) * CharWidth,
               TTop + (CursorY - 1) * CharHeight + CharAscent, 1);

    XSetClipOrigin(display, ularn_gc, 0, 0);
    XSetClipMask(display, ularn_gc, None);

    CaretActive = 1;
  }

  Event = ACTION_NULL;
  got_dir = 0;

  while (!got_dir) {
    XNextEvent(display, &xevent);

    handle_event(&xevent);

    idx = 0;

    while ((idx < NUM_DIRS) && (!got_dir)) {
      if (DirActions[idx] == Event)
        got_dir = 1;
      else
        idx++;
    }
  }

  if (ShowCursor) {

    XSetForeground(display, ularn_gc, white_pixel);
    XSetClipOrigin(display, ularn_gc, TLeft + (CursorX - 1) * CharWidth,
                   TTop + (CursorY - 1) * CharHeight + CharAscent);
    XSetClipMask(display, ularn_gc, CursorPixmap);
    XCopyPlane(display, CursorPixmap, ularn_window, ularn_gc, 0, 0,
               cursor_width, cursor_height, TLeft + (CursorX - 1) * CharWidth,
               TTop + (CursorY - 1) * CharHeight + CharAscent, 1);

    XSetClipOrigin(display, ularn_gc, 0, 0);
    XSetClipMask(display, ularn_gc, None);

    CaretActive = 0;
  }

  return Event;
}

/* =============================================================================
 * FUNCTION: UpdateStatus
 */
void UpdateStatus(void) {
  if (CurrentDisplayMode == DISPLAY_TEXT)
    /* Don't redisplay if in text mode */
    return;

  PaintStatus();
}

/* =============================================================================
 * FUNCTION: UpdateEffects
 */
void UpdateEffects(void) {
  if (CurrentDisplayMode == DISPLAY_TEXT)
    /* Don't redisplay if in text mode */
    return;

  PaintEffects();
}

/* =============================================================================
 * FUNCTION: UpdateStatusAndEffects
 */
void UpdateStatusAndEffects(void) {
  if (CurrentDisplayMode == DISPLAY_TEXT)
    /* Don't redisplay if in text mode */
    return;

  //
  // Do effects first as update status will mark all effects as current
  //
  PaintEffects();
  PaintStatus();
}

/* =============================================================================
 * FUNCTION: set_display
 */
void set_display(DisplayModeType Mode) {
  //
  // Save the current settings
  //
  if (CurrentDisplayMode == DISPLAY_MAP) {
    MsgCursorX = CursorX;
    MsgCursorY = CursorY;
    CurrentMsgFormat = CurrentFormat;
  } else if (CurrentDisplayMode == DISPLAY_TEXT) {
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

  } else if (CurrentDisplayMode == DISPLAY_TEXT) {
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

  PaintWindow();
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
static void IncCursorY(int Count) {
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
    PaintTextWindow();
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
static void IncCursorX(int Count) {
  CursorX = CursorX + Count;
  if (CursorX > LINE_LENGTH) {
    CursorX = 1;
    IncCursorY(1);
  }
}

/* =============================================================================
 * FUNCTION: ClearText
 */
void ClearText(void) {
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
  PaintTextWindow();

  XFlush(display);
  XSync(display, 0);
}

/* =============================================================================
 * FUNCTION: UlarnBeep
 */
void UlarnBeep(void) {
  //
  // Middle C, 1/4 second
  //
  if (!nobeep)
    XBell(display, 100);
}

/* =============================================================================
 * FUNCTION: Cursor
 */
void MoveCursor(int x, int y) {
  CursorX = x;
  CursorY = y;
}

/* =============================================================================
 * FUNCTION: Printc
 */
void Printc(char c) {
  int incx;
  char lc;

  switch (c) {
  case '\t':
    incx = ((((CursorX - 1) / 8) + 1) * 8 + 1) - CursorX;
    IncCursorX(incx);
    break;

  case '\n':
    CursorX = 1;
    IncCursorY(1);
    break;

  case '\015':
    break;

  default:

    lc = Text[CursorY - 1][CursorX - 1];

    if (lc != c) {
      /* Erase the char that was here */
      XSetForeground(display, ularn_gc, white_pixel);
      XSetBackground(display, ularn_gc, black_pixel);
      XSetFillStyle(display, ularn_gc, FillSolid);

      XFillRectangle(display, ularn_window, ularn_gc,
                     TLeft + (CursorX - 1) * CharWidth,
                     TTop + (CursorY - 1) * CharHeight, CharWidth, CharHeight);
    }

    Text[CursorY - 1][CursorX - 1] = c;
    Format[CursorY - 1][CursorX - 1] = CurrentFormat;

    switch (CurrentFormat) {
    case FORMAT_NORMAL:
      XSetForeground(display, ularn_gc, black_pixel);
      break;
    case FORMAT_STANDOUT:
      XSetForeground(display, ularn_gc, Red.pixel);
      break;
    case FORMAT_STANDOUT2:
      XSetForeground(display, ularn_gc, Green.pixel);
      break;
    case FORMAT_STANDOUT3:
      XSetForeground(display, ularn_gc, Blue.pixel);
      break;
    default:
      break;
    }

    XDrawString(display, ularn_window, ularn_gc,
                TLeft + (CursorX - 1) * CharWidth,
                TTop + (CursorY - 1) * CharHeight + CharAscent, &c, 1);

    IncCursorX(1);
    break;
  }
}

/* =============================================================================
 * FUNCTION: Print
 */
void Print(char *string) {
  int Len;
  int pos;

  if (string == NULL)
    return;

  Len = strlen(string);

  if (Len == 0)
    return;

  for (pos = 0; pos < Len; pos++)
    Printc(string[pos]);

  XFlush(display);
  XSync(display, 0);
}

/* =============================================================================
 * FUNCTION: Printf
 */
void Printf(char *fmt, ...) {
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
void Standout(char *String) {
  CurrentFormat = FORMAT_STANDOUT;

  Print(String);

  CurrentFormat = FORMAT_NORMAL;
}

/* =============================================================================
 * FUNCTION: SetFormat
 */
void SetFormat(FormatType format) { CurrentFormat = format; }

/* =============================================================================
 * FUNCTION: ClearToEOL
 */
void ClearToEOL(void) {
  int x;

  for (x = CursorX; x <= LINE_LENGTH; x++) {
    Text[CursorY - 1][x - 1] = ' ';
    Format[CursorY - 1][x - 1] = FORMAT_NORMAL;
  }

  XSetForeground(display, ularn_gc, white_pixel);
  XSetBackground(display, ularn_gc, black_pixel);
  XSetFillStyle(display, ularn_gc, FillSolid);

  XFillRectangle(display, ularn_window, ularn_gc,
                 TLeft + (CursorX - 1) * CharWidth,
                 TTop + (CursorY - 1) * CharHeight,
                 ((LINE_LENGTH - CursorX) + 1) * CharWidth, CharHeight);
}

/* =============================================================================
 * FUNCTION: ClearToEOPage
 */
void ClearToEOPage(int x, int y) {
  int tx, ty;

  for (tx = x; tx <= LINE_LENGTH; tx++) {
    Text[y - 1][tx - 1] = ' ';
    Format[y - 1][tx - 1] = FORMAT_NORMAL;
  }

  XSetForeground(display, ularn_gc, white_pixel);
  XSetBackground(display, ularn_gc, black_pixel);
  XSetFillStyle(display, ularn_gc, FillSolid);

  XFillRectangle(display, ularn_window, ularn_gc, TLeft + (x - 1) * CharWidth,
                 TTop + (y - 1) * CharHeight,
                 ((LINE_LENGTH - x) + 1) * CharWidth, CharHeight);

  for (ty = y + 1; ty <= MaxLine; ty++) {
    for (tx = 1; tx <= LINE_LENGTH; tx++) {
      Text[ty - 1][tx - 1] = ' ';
      Format[ty - 1][tx - 1] = FORMAT_NORMAL;
    }

    XFillRectangle(display, ularn_window, ularn_gc, TLeft,
                   TTop + (ty - 1) * CharHeight, LINE_LENGTH * CharWidth,
                   CharHeight);
  }
}

/* =============================================================================
 * FUNCTION: show1cell
 */
void show1cell(int x, int y) {
  int TileId;
  int sx, sy;
  int TileX, TileY;

  /* see nothing if blind		*/
  if (c[BLINDCOUNT])
    return;

  /* we end up knowing about it */
  know[x][y] = item[x][y];
  if (mitem[x][y].mon != MONST_NONE)
    stealth[x][y] |= STEALTH_SEEN;

  sx = x - MapTileLeft;
  sy = y - MapTileTop;

  if ((sx < 0) || (sx >= MapTileWidth) || (sy < 0) || (sy >= MapTileHeight)) {
    //
    // Tile is not currently in the visible part of the map,
    // so don't draw anything
    //
    return;
  }

  GetTile(x, y, &TileId);

  TileX = (TileId % 16) * TileWidth;
  TileY = (TileId / 16) * TileHeight;

  XCopyArea(display, TilePixmap, ularn_window, ularn_gc, TileX, TileY,
            TileWidth, TileHeight, MapLeft + sx * TileWidth,
            MapTop + sy * TileHeight);
}

/* =============================================================================
 * FUNCTION: showplayer
 */
void showplayer(void) {
  int sx, sy;
  int TileId;
  int TileX, TileY;
  int scroll;

  //
  // Determine if we need to scroll the map
  //
  scroll = calc_scroll();

  if (scroll)
    PaintMap();
  else {
    sx = playerx - MapTileLeft;
    sy = playery - MapTileTop;

    if ((sx >= 0) && (sx < MapTileWidth) && (sy >= 0) && (sy < MapTileHeight)) {
      if (c[BLINDCOUNT] == 0)
        TileId = PlayerTiles[class_num][(int)sex];
      else
        GetTile(playerx, playery, &TileId);

      TileX = (TileId % 16) * TileWidth;
      TileY = (TileId / 16) * TileHeight;

      XCopyArea(display, TilePixmap, ularn_window, ularn_gc, TileX, TileY,
                TileWidth, TileHeight, MapLeft + sx * TileWidth,
                MapTop + sy * TileHeight);

      TileId = TILE_CURSOR1;
      TileX = (TileId % 16) * TileWidth;
      TileY = (TileId / 16) * TileHeight;

      XSetClipOrigin(display, ularn_gc, MapLeft + sx * TileWidth - TileX,
                     MapTop + sy * TileHeight - TileY);
      XSetClipMask(display, ularn_gc, TilePShape);
      XCopyArea(display, TilePixmap, ularn_window, ularn_gc, TileX, TileY,
                TileWidth, TileHeight, MapLeft + sx * TileWidth,
                MapTop + sy * TileHeight);

      XSetClipOrigin(display, ularn_gc, 0, 0);
      XSetClipMask(display, ularn_gc, None);

    } /* If player on visible map area */
  }
}

/* =============================================================================
 * FUNCTION: showcell
 */
void showcell(int x, int y) {
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
  } else {
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

      if ((sx >= 0) && (sx < MapTileWidth) && (sy >= 0) &&
          (sy < MapTileHeight)) {
        //
        // Tile is currently visible, so draw it
        //

        GetTile(lastpx, lastpy, &TileId);

        TileX = (TileId % 16) * TileWidth;
        TileY = (TileId / 16) * TileHeight;

        XCopyArea(display, TilePixmap, ularn_window, ularn_gc, TileX, TileY,
                  TileWidth, TileHeight, MapLeft + sx * TileWidth,
                  MapTop + sy * TileHeight);
      }
    }
  }

  /*
   * Limit the area to the map extents
   */
  if (minx < 0)
    minx = 0;
  if (maxx > MAXX - 1)
    maxx = MAXX - 1;
  if (miny < 0)
    miny = 0;
  if (maxy > MAXY - 1)
    maxy = MAXY - 1;

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
      } else if ((know[mx][my] != item[mx][my]) ||     /* item changed    */
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

          if ((sx >= 0) && (sx < MapTileWidth) && (sy >= 0) &&
              (sy < MapTileHeight)) {
            //
            // Tile is currently visible, so draw it
            //

            GetTile(mx, my, &TileId);

            TileX = (TileId % 16) * TileWidth;
            TileY = (TileId / 16) * TileHeight;

            XCopyArea(display, TilePixmap, ularn_window, ularn_gc, TileX, TileY,
                      TileWidth, TileHeight, MapLeft + sx * TileWidth,
                      MapTop + sy * TileHeight);
          }
        }

      } // if not known
    }
  }

  if (scroll)
    /* scrolling the map window, so repaint everything and return */
    PaintMap();
}

/* =============================================================================
 * FUNCTION: drawscreen
 */
void drawscreen(void) { PaintWindow(); }

/* =============================================================================
 * FUNCTION: draws
 */
void draws(int minx, int miny, int maxx, int maxy) { PaintWindow(); }

/* =============================================================================
 * FUNCTION: mapeffect
 */
void mapeffect(int x, int y, DirEffectsType effect, int dir) {
  int TileId;
  int sx, sy;
  int TileX, TileY;

  /* see nothing if blind		*/
  if (c[BLINDCOUNT])
    return;

  sx = x - MapTileLeft;
  sy = y - MapTileTop;

  if ((sx < 0) || (sx >= MapTileWidth) || (sy < 0) || (sy >= MapTileHeight)) {
    //
    // Tile is not currently in the visible part of the map,
    // so don't draw anything
    //
    return;
  }

  TileId = EffectTile[effect][dir];

  TileX = (TileId % 16) * TileWidth;
  TileY = (TileId / 16) * TileHeight;

  XCopyArea(display, TilePixmap, ularn_window, ularn_gc, TileX, TileY,
            TileWidth, TileHeight, MapLeft + sx * TileWidth,
            MapTop + sy * TileHeight);
}

/* =============================================================================
 * FUNCTION: magic_effect_frames
 */
int magic_effect_frames(MagicEffectsType fx) { return magicfx_tile[fx].Frames; }

/* =============================================================================
 * FUNCTION: magic_effect
 */
void magic_effect(int x, int y, MagicEffectsType fx, int frame) {
  int TileId;
  int sx, sy;
  int TileX, TileY;

  if (frame > magicfx_tile[fx].Frames)
    return;

  /*
   * draw the tile that is at this location
   */

  /* see nothing if blind		*/
  if (c[BLINDCOUNT])
    return;

  sx = x - MapTileLeft;
  sy = y - MapTileTop;

  if ((sx < 0) || (sx >= MapTileWidth) || (sy < 0) || (sy >= MapTileHeight)) {
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

    XCopyArea(display, TilePixmap, ularn_window, ularn_gc, TileX, TileY,
              TileWidth, TileHeight, MapLeft + sx * TileWidth,
              MapTop + sy * TileHeight);

    TileId = magicfx_tile[fx].Tile1[frame];
    TileX = (TileId % 16) * TileWidth;
    TileY = (TileId / 16) * TileHeight;

    XSetClipOrigin(display, ularn_gc, MapLeft + sx * TileWidth - TileX,
                   MapTop + sy * TileHeight - TileY);
    XSetClipMask(display, ularn_gc, TilePShape);
    XCopyArea(display, TilePixmap, ularn_window, ularn_gc, TileX, TileY,
              TileWidth, TileHeight, MapLeft + sx * TileWidth,
              MapTop + sy * TileHeight);

    XSetClipOrigin(display, ularn_gc, 0, 0);
    XSetClipMask(display, ularn_gc, None);

  } else {
    TileId = magicfx_tile[fx].Tile1[frame];
    TileX = (TileId % 16) * TileWidth;
    TileY = (TileId / 16) * TileHeight;

    XCopyArea(display, TilePixmap, ularn_window, ularn_gc, TileX, TileY,
              TileWidth, TileHeight, MapLeft + sx * TileWidth,
              MapTop + sy * TileHeight);
  }
}

/* =============================================================================
 * FUNCTION: nap
 */
void nap(int delay) {
  XFlush(display);
  XSync(display, 0);
  usleep(delay * 1000);
}

/* =============================================================================
 * FUNCTION: GetUser
 */
void GetUser(char *username, int *uid) {

  *uid = getuid();

  strcpy(username, getenv("USER"));
}
