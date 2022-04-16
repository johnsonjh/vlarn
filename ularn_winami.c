/* =============================================================================
 * PROGRAM:  ularn
 * FILENAME: ularn_winami.c
 *
 * DESCRIPTION:
 * This module contains all operating system dependant code for input and
 * display update.
 * Each version of ularn should provide a different implementation of this
 * module.
 *
 * This is the Amiga window display and input module.
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

#include <stdarg.h>
#include <stdio.h>

#include <graphics/gfxbase.h>
#include <graphics/modeid.h>
#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>
#include <intuition/screens.h>
#include <proto/asl.h>
#include <proto/diskfont.h>
#include <proto/exec.h>
#include <proto/graphics.h>
#include <proto/intuition.h>

#include "header.h"
#include "ularn_game.h"

#include "config.h"
#include "dungeon.h"
#include "itm.h"
#include "monster.h"
#include "player.h"
#include "ularn_win.h"
#include "ularnpc.rh"

#include "ifftools.h"
#include "smart_menu.h"

//
// Defines for windows
//
#define BLACK_PEN 0
#define RED_PEN 1
#define GREEN_PEN 2
#define BLUE_PEN 3
#define DARK_PEN 252
#define MID_PEN 253
#define LIGHT_PEN 254
#define WHITE_PEN 255

// Default size of the ularn window in characters
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

extern void Delay(long TickCount);

#define M_NONE 0
#define M_SHIFT 1
#define M_CTRL 2
#define M_NUMPAD 4
#define M_ASCII 8

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
    {{0, 0}, {0, 0}, {0, 0}},                          // ACTION_NULL
    {{'~', M_ASCII}, {0, 0}, {0, 0}},                  // ACTION_DIAG
    {{'h', M_ASCII}, {0x4f, M_NONE}, {'4', M_NUMPAD}}, // ACTION_MOVE_WEST
    {{'H', M_ASCII},
     {0x4f, M_SHIFT},
     {'4', M_NUMPAD | M_SHIFT}},                       // ACTION_RUN_WEST
    {{'l', M_ASCII}, {0x4e, M_NONE}, {'6', M_NUMPAD}}, // ACTION_MOVE_EAST,
    {{'L', M_ASCII},
     {0x4e, M_SHIFT},
     {'6', M_NUMPAD | M_SHIFT}},                       // ACTION_RUN_EAST,
    {{'j', M_ASCII}, {0x4d, M_NONE}, {'2', M_NUMPAD}}, // ACTION_MOVE_SOUTH,
    {{'J', M_ASCII},
     {0x4d, M_SHIFT},
     {'2', M_NUMPAD | M_SHIFT}},                       // ACTION_RUN_SOUTH,
    {{'k', M_ASCII}, {0x4c, M_NONE}, {'8', M_NUMPAD}}, // ACTION_MOVE_NORTH,
    {{'K', M_ASCII},
     {0x4c, M_SHIFT},
     {'8', M_NUMPAD | M_SHIFT}},               // ACTION_RUN_NORTH,
    {{'u', M_ASCII}, {'9', M_NUMPAD}, {0, 0}}, // ACTION_MOVE_NORTHEAST,
    {{'U', M_ASCII},
     {'9', M_NUMPAD | M_SHIFT},
     {0, 0}},                                  // ACTION_RUN_NORTHEAST,
    {{'y', M_ASCII}, {'7', M_NUMPAD}, {0, 0}}, // ACTION_MOVE_NORTHWEST,
    {{'Y', M_ASCII},
     {'7', M_NUMPAD | M_SHIFT},
     {0, 0}},                                  // ACTION_RUN_NORTHWEST,
    {{'n', M_ASCII}, {'3', M_NUMPAD}, {0, 0}}, // ACTION_MOVE_SOUTHEAST,
    {{'N', M_ASCII},
     {'3', M_NUMPAD | M_SHIFT},
     {0, 0}},                                  // ACTION_RUN_SOUTHEAST,
    {{'b', M_ASCII}, {'1', M_NUMPAD}, {0, 0}}, // ACTION_MOVE_SOUTHWEST,
    {{'B', M_ASCII},
     {'1', M_NUMPAD | M_SHIFT},
     {0, 0}},                          // ACTION_RUN_SOUTHWEST,
    {{'.', M_ASCII}, {0, 0}, {0, 0}},  // ACTION_WAIT,
    {{' ', M_ASCII}, {0, 0}, {0, 0}},  // ACTION_NONE,
    {{'w', M_ASCII}, {0, 0}, {0, 0}},  // ACTION_WIELD,
    {{'W', M_ASCII}, {0, 0}, {0, 0}},  // ACTION_WEAR,
    {{'r', M_ASCII}, {0, 0}, {0, 0}},  // ACTION_READ,
    {{'q', M_ASCII}, {0, 0}, {0, 0}},  // ACTION_QUAFF,
    {{'d', M_ASCII}, {0, 0}, {0, 0}},  // ACTION_DROP,
    {{'c', M_ASCII}, {0, 0}, {0, 0}},  // ACTION_CAST_SPELL,
    {{'o', M_ASCII}, {0, 0}, {0, 0}},  // ACTION_OPEN_DOOR,
    {{'C', M_ASCII}, {0, 0}, {0, 0}},  // ACTION_CLOSE_DOOR,
    {{'O', M_ASCII}, {0, 0}, {0, 0}},  // ACTION_OPEN_CHEST,
    {{'i', M_ASCII}, {0, 0}, {0, 0}},  // ACTION_INVENTORY,
    {{'e', M_ASCII}, {0, 0}, {0, 0}},  // ACTION_EAT_COOKIE,
    {{'\\', M_ASCII}, {0, 0}, {0, 0}}, // ACTION_LIST_SPELLS,
    {{'?', M_ASCII}, {0, 0}, {0, 0}},  // ACTION_HELP,
    {{'S', M_ASCII}, {0, 0}, {0, 0}},  // ACTION_SAVE,
    {{'Z', M_ASCII}, {0, 0}, {0, 0}},  // ACTION_TELEPORT,
    {{'^', M_ASCII}, {0, 0}, {0, 0}},  // ACTION_IDENTIFY_TRAPS,
    {{'_', M_ASCII}, {0, 0}, {0, 0}},  // ACTION_BECOME_CREATOR,
    {{'+', M_ASCII}, {0, 0}, {0, 0}},  // ACTION_CREATE_ITEM,
    {{'-', M_ASCII}, {0, 0}, {0, 0}},  // ACTION_TOGGLE_WIZARD,
    {{'`', M_ASCII}, {0, 0}, {0, 0}},  // ACTION_DEBUG_MODE,
    {{'T', M_ASCII}, {0, 0}, {0, 0}},  // ACTION_REMOVE_ARMOUR,
    {{'g', M_ASCII}, {0, 0}, {0, 0}},  // ACTION_PACK_WEIGHT,
    {{'v', M_ASCII}, {0, 0}, {0, 0}},  // ACTION_VERSION,
    {{'Q', M_ASCII}, {0, 0}, {0, 0}},  // ACTION_QUIT,
    {{0x12, M_ASCII}, {0, 0}, {0, 0}}, // ACTION_REDRAW_SCREEN,
    {{'P', M_ASCII}, {0, 0}, {0, 0}}   // ACTION_SHOW_TAX
};

static struct KeyCodeType RunKeyMap = {'5', M_NUMPAD};

//
// Amiga stuff
//

struct IntuitionBase *IntuitionBase = NULL;
struct GfxBase *GfxBase = NULL;
struct Library *AslBase = NULL;

struct Screen *UlarnScreen = NULL;
struct Window *UlarnWindow = NULL;
struct RastPort *UlarnRP = NULL;

static struct BitMap *UlarnGfx = NULL;
static ULONG UlarnPalette[256];

static struct TextAttr UlarnTextAttr = {"topaz.font", 8, 0, NULL};

static struct TextFont *UlarnFont = NULL;

UWORD UlarnPens[NUMDRIPENS + 1] = {
    255,   // DETAILPEN
    253,   // BLOCKPEN
    255,   // TEXTPEN
    254,   // SHINEPEN
    252,   // SHADOWPEN
    1,     // FILLPEN
    2,     // FILLTEXTPEN
    253,   // BACKGROUNDPEN
    0,     // HIGHLIGHTPEN
    255,   // BARDETAILPEN
    253,   // PARBLOCKPEN
    252,   // BARTRIMPEN
    0xFFFF // Terminator
};

//
// Variables for windows
//

#define INITIAL_WIDTH 400
#define INITIAL_HEIGHT 300

static int CaretActive = 0;

static int TileWidth = 32;
static int TileHeight = 32;
static int CharHeight = 8;
static int CharWidth = 8;
static int CharBaseline = 6;
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
// Smartmenu structures and handling functions.
//

static void DoMenuSave(void) { Event = ACTION_SAVE; }
static void DoMenuQuit(void) { Event = ACTION_QUIT; }

static void DoMenuWait(void) { Event = ACTION_WAIT; }
static void DoMenuWield(void) { Event = ACTION_WIELD; }
static void DoMenuWear(void) { Event = ACTION_WEAR; }
static void DoMenuTakeoff(void) { Event = ACTION_REMOVE_ARMOUR; }
static void DoMenuQuaff(void) { Event = ACTION_QUAFF; }
static void DoMenuRead(void) { Event = ACTION_READ; }
static void DoMenuCast(void) { Event = ACTION_CAST_SPELL; }
static void DoMenuEat(void) { Event = ACTION_EAT_COOKIE; }
static void DoMenuDrop(void) { Event = ACTION_DROP; }
static void DoMenuClose(void) { Event = ACTION_CLOSE_DOOR; }

static void DoMenuDiscoveries(void) { Event = ACTION_LIST_SPELLS; }
static void DoMenuInventory(void) { Event = ACTION_INVENTORY; }
static void DoMenuTax(void) { Event = ACTION_SHOW_TAX; }
static void DoMenuPackweight(void) { Event = ACTION_PACK_WEIGHT; }

static void DoMenuRedraw(void) { Event = ACTION_REDRAW_SCREEN; }
static void DoMenuBeep(void) {
  if (nobeep)
    nobeep = 0;
  else
    nobeep = 1;
}

static void DoMenuHelp(void) { Event = ACTION_HELP; }
static void DoMenuVersion(void) { Event = ACTION_VERSION; }

static struct SmartMenuItem UlarnGameMenu[3] = {
    {"Save", 'S', 255, 253, DoMenuSave, NULL},
    {"Quit", 'Q', 255, 253, DoMenuQuit, NULL},
    {NULL, 0, 0, 0, NULL, NULL}};

static struct SmartMenuItem UlarnCommandMenu[11] = {
    {"Wait", 0, 255, 253, DoMenuWait, NULL},
    {"Wield", 0, 255, 253, DoMenuWield, NULL},
    {"Wear", 0, 255, 253, DoMenuWear, NULL},
    {"Take off", 0, 255, 253, DoMenuTakeoff, NULL},
    {"Quaff", 0, 255, 253, DoMenuQuaff, NULL},
    {"Read", 0, 255, 253, DoMenuRead, NULL},
    {"Cast", 0, 255, 253, DoMenuCast, NULL},
    {"Eat", 0, 255, 253, DoMenuEat, NULL},
    {"Drop", 0, 255, 253, DoMenuDrop, NULL},
    {"Close", 0, 255, 253, DoMenuClose, NULL},
    {NULL, 0, 0, 0, NULL, NULL}};

static struct SmartMenuItem UlarnShowMenu[5] = {
    {"Discoveries", 0, 255, 253, DoMenuDiscoveries, NULL},
    {"Inventory", 0, 255, 253, DoMenuInventory, NULL},
    {"Tax", 0, 255, 253, DoMenuTax, NULL},
    {"Pack weight", 0, 255, 253, DoMenuPackweight, NULL},
    {NULL, 0, 0, 0, NULL, NULL}};

static struct SmartMenuItem UlarnDisplayMenu[3] = {
    {"Redraw", 0, 255, 253, DoMenuRedraw, NULL},
    {"Beep", 0, 255, 253, DoMenuBeep, NULL},
    {NULL, 0, 0, 0, NULL, NULL}};

static struct SmartMenuItem UlarnHelpMenu[3] = {
    {"Help", 0, 255, 253, DoMenuHelp, NULL},
    {"Version", 0, 255, 253, DoMenuVersion, NULL},
    {NULL, 0, 0, 0, NULL, NULL}};

static struct SmartMenu UlarnMenu[6] = {
    {"Game", UlarnGameMenu}, {"Commands", UlarnCommandMenu},
    {"Show", UlarnShowMenu}, {"Display", UlarnDisplayMenu},
    {"Help", UlarnHelpMenu}, {NULL, NULL}};

//
// player id file
//
static char *PIDName = LIBDIR "/vlarn.pid";
#define FIRST_PID 1001

//
// ularn.ini file for window position & font selection
//
static char *IniName = "vlarn.ini";

//
// Bitmaps for tiles
//

static char *TileBMName = LIBDIR "/vlarn_gfx.iff";

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
static TextLine *AText;
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
static void PaintStatus(void) {
  char Line[81];
  char Buf[81];
  int i;

  if (Repaint) {

    SetAPen(UlarnRP, WHITE_PEN);
    SetDrMd(UlarnRP, JAM1);
    RectFill(UlarnRP, StatusLeft, StatusTop, StatusLeft + StatusWidth - 1,
             StatusTop + StatusHeight - 1);
  }

  SetDrMd(UlarnRP, JAM2);
  SetAPen(UlarnRP, BLACK_PEN);

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

  Move(UlarnRP, StatusLeft, StatusTop + CharBaseline);
  Text(UlarnRP, Line, strlen(Line));

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

  Move(UlarnRP, StatusLeft, StatusTop + CharHeight + CharBaseline);
  Text(UlarnRP, Line, strlen(Line));

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
 *   DC : The DC to be painted.
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

    SetAPen(UlarnRP, WHITE_PEN);
    SetDrMd(UlarnRP, JAM1);
    RectFill(UlarnRP, EffectsLeft, EffectsTop, EffectsLeft + EffectsWidth - 1,
             EffectsTop + EffectsHeight - 1);
  }

  SetAPen(UlarnRP, BLACK_PEN);
  SetDrMd(UlarnRP, JAM2);

  for (i = 0; i < 17; i++) {
    idx = bot_data[i].typ;
    WasSet = (cbak[idx] != 0);
    IsSet = (c[idx] != 0);

    if ((Repaint) || (IsSet != WasSet)) {
      if (IsSet) {
        Move(UlarnRP, EffectsLeft, EffectsTop + i * CharHeight + CharBaseline);
        Text(UlarnRP, bot_data[i].string, strlen(bot_data[i].string));
      } else {
        Move(UlarnRP, EffectsLeft, EffectsTop + i * CharHeight + CharBaseline);
        Text(UlarnRP, "          ", 10);
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
 *   DC : The device context to be painted.
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

      BltBitMapRastPort(UlarnGfx, TileX, TileY, UlarnRP,
                        MapLeft + sx * TileWidth, MapTop + sy * TileHeight,
                        TileWidth, TileHeight, 0xc0);

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
    BltBitMapRastPort(UlarnGfx, TileX, TileY, UlarnRP, MapLeft + sx * TileWidth,
                      MapTop + sy * TileHeight, TileWidth, TileHeight, 0x80);

    TileId = TILE_CURSOR2;
    TileX = (TileId % 16) * TileWidth;
    TileY = (TileId / 16) * TileHeight;
    BltBitMapRastPort(UlarnGfx, TileX, TileY, UlarnRP, MapLeft + sx * TileWidth,
                      MapTop + sy * TileHeight, TileWidth, TileHeight, 0xe0);
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
static void PaintTextWindow(void) {
  int sx, ex, y;
  FormatType Fmt;

  SetAPen(UlarnRP, WHITE_PEN);
  SetDrMd(UlarnRP, JAM1);
  RectFill(UlarnRP, TLeft, TTop, TLeft + TWidth - 1, TTop + THeight - 1);

  SetDrMd(UlarnRP, JAM2);

  for (y = 0; y < MaxLine; y++) {

    sx = 0;

    while (sx < LINE_LENGTH) {

      Fmt = Format[y][sx];
      ex = sx;

      while ((ex < LINE_LENGTH) && (Format[y][ex] == Fmt))
        ex++;

      switch (Fmt) {
      case FORMAT_NORMAL:
        SetAPen(UlarnRP, BLACK_PEN);
        break;
      case FORMAT_STANDOUT:
        SetAPen(UlarnRP, RED_PEN);
        break;
      case FORMAT_STANDOUT2:
        SetAPen(UlarnRP, GREEN_PEN);
        break;
      case FORMAT_STANDOUT3:
        SetAPen(UlarnRP, BLUE_PEN);
        break;
      default:
        break;
      }

      Move(UlarnRP, TLeft + sx * CharWidth,
           TTop + y * CharHeight + CharBaseline);
      Text(UlarnRP, AText[y] + sx, ex - sx);

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
static void PaintMapWindow(void) {
  int RectLeft;
  int RectTop;
  int RectRight;
  int RectBottom;

  //
  // Message area
  //

  RectLeft = MessageLeft;
  RectTop = MessageTop - SEPARATOR_HEIGHT;
  RectRight = MessageLeft + MessageWidth;
  RectBottom = RectTop + 2;
  SetAPen(UlarnRP, LIGHT_PEN);
  RectFill(UlarnRP, RectLeft, RectTop, RectRight - 1, RectBottom - 1);

  RectTop = RectBottom;
  RectBottom = RectTop + 4;
  SetAPen(UlarnRP, MID_PEN);
  RectFill(UlarnRP, RectLeft, RectTop, RectRight - 1, RectBottom - 1);

  RectTop = RectBottom;
  RectBottom = RectTop + 2;
  SetAPen(UlarnRP, DARK_PEN);
  RectFill(UlarnRP, RectLeft, RectTop, RectRight - 1, RectBottom - 1);

  //
  // Status area
  //
  RectLeft = StatusLeft;
  RectTop = StatusTop - SEPARATOR_HEIGHT;
  RectRight = StatusLeft + StatusWidth;
  RectBottom = RectTop + 2;

  SetAPen(UlarnRP, LIGHT_PEN);
  RectFill(UlarnRP, RectLeft, RectTop, RectRight - 1, RectBottom - 1);

  RectTop = RectBottom;
  RectBottom = RectTop + 4;
  SetAPen(UlarnRP, MID_PEN);
  RectFill(UlarnRP, RectLeft, RectTop, RectRight - 1, RectBottom - 1);

  RectTop = RectBottom;
  RectBottom = RectTop + 2;
  SetAPen(UlarnRP, DARK_PEN);
  RectFill(UlarnRP, RectLeft, RectTop, RectRight - 1, RectBottom - 1);

  //
  // Effects area
  //
  RectLeft = EffectsLeft - SEPARATOR_WIDTH;
  RectTop = EffectsTop;
  RectRight = RectLeft + 2;
  RectBottom = EffectsTop + EffectsHeight;

  SetAPen(UlarnRP, LIGHT_PEN);
  RectFill(UlarnRP, RectLeft, RectTop, RectRight - 1, RectBottom - 1);

  RectLeft = RectRight;
  RectRight = RectLeft + 4;
  RectBottom = EffectsTop + EffectsHeight + 2;
  SetAPen(UlarnRP, MID_PEN);
  RectFill(UlarnRP, RectLeft, RectTop, RectRight - 1, RectBottom - 1);

  RectLeft = RectRight;
  RectRight = RectLeft + 2;
  RectBottom = EffectsTop + EffectsHeight;
  SetAPen(UlarnRP, DARK_PEN);
  RectFill(UlarnRP, RectLeft, RectTop, RectRight - 1, RectBottom - 1);

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
 *   DC : The device context to be painted
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void PaintWindow(void) {

  Repaint = 1;

  SetAPen(UlarnRP, WHITE_PEN);
  RectFill(UlarnRP, 0, 0, UlarnScreen->Width - 1, UlarnScreen->Height - 1);

  SetBPen(UlarnRP, WHITE_PEN);
  SetDrMd(UlarnRP, JAM2);

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
 *   hwnd : The handle of the window being resized
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void Resize(void) {
  int ClientWidth = 0;
  int ClientHeight = 0;

  ClientWidth = UlarnScreen->Width;
  ClientHeight = UlarnScreen->Height;

  //
  // Calculate the message window size and position
  //
  MessageWidth = ClientWidth;
  MessageHeight = CharHeight * MAX_MSG_LINES + 2;
  MessageLeft = 0;
  MessageTop = ClientHeight - MessageHeight - 1;

  //
  // Calculate the Status window size and position
  //
  StatusLeft = 0;
  StatusHeight = CharHeight * 2 + 2;
  StatusTop = (MessageTop - SEPARATOR_HEIGHT) - StatusHeight;
  StatusWidth = ClientWidth;

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
  MapTop = UlarnScreen->BarHeight;
  MapWidth = EffectsLeft - SEPARATOR_WIDTH;
  MapHeight = (StatusTop - SEPARATOR_HEIGHT) - MapTop;
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

  //
  // Show the cursor if required
  //
  if (CaretActive) {
    SetAPen(UlarnRP, BLUE_PEN);
    RectFill(UlarnRP, TLeft + (CursorX - 1) * CharWidth,
             TTop + (CursorY - 1) * CharHeight + 2,
             TLeft + (CursorX)*CharWidth - 1,
             TTop + (CursorY)*CharHeight + 2 - 1);
  }
}

/* =============================================================================
 * FUNCTION: HandleInput
 *
 * DESCRIPTION:
 * This procedure handles the next IntuiMessage and performs input
 * processing for the event that was received.
 * The Event variable is set to the event corresponding to the input
 * received.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void HandleInput(void) {
  struct IntuiMessage *Msg;

  WaitPort(UlarnWindow->UserPort);

  Msg = (struct IntuiMessage *)GetMsg(UlarnWindow->UserPort);

  switch (Msg->Class) {
  case IDCMP_MENUPICK:
    /* A menu item has been selected. */
    DoMenuSelection(Msg->Code);
    break;

  case IDCMP_VANILLAKEY: {
    /* An ASCII keypress */
    ActionType Action;
    int Found = 0;
    int i;
    int ModKey = M_ASCII;

    if ((Msg->Qualifier & IEQUALIFIER_NUMERICPAD) != 0) {
      ModKey = M_NUMPAD;

      if (((Msg->Qualifier & IEQUALIFIER_LSHIFT) != 0) ||
          ((Msg->Qualifier & IEQUALIFIER_RSHIFT) != 0))
        ModKey |= M_SHIFT;

      if ((Msg->Qualifier & IEQUALIFIER_CONTROL) != 0)
        ModKey |= M_CTRL;
    }

    Action = ACTION_NULL;
    while ((Action < ACTION_COUNT) && (!Found)) {
      for (i = 0; i < MAX_KEY_BINDINGS; i++) {
        if ((Msg->Code == KeyMap[Action][i].VirtKey) &&
            (KeyMap[Action][i].ModKey == ModKey))
          Found = 1;
      }

      if (!Found)
        Action++;
    }

    if (Found)
      Event = Action;
    else {
      /* Check run key */
      if ((Msg->Code == RunKeyMap.VirtKey) && (RunKeyMap.ModKey == ModKey))
        Runkey = 1;
    }

    EventChar = (char)Msg->Code;
    GotChar = 1;

    break;
  }

  case IDCMP_RAWKEY: {
    /* A keypress that does not translate to a single ASCII character */
    ActionType Action;
    int ModKey = 0;
    int Found = 0;
    int i;

    if (((Msg->Qualifier & IEQUALIFIER_LSHIFT) != 0) ||
        ((Msg->Qualifier & IEQUALIFIER_RSHIFT) != 0))
      ModKey |= M_SHIFT;

    if ((Msg->Qualifier & IEQUALIFIER_CONTROL) != 0)
      ModKey |= M_CTRL;

    Action = ACTION_NULL;
    while ((Action < ACTION_COUNT) && (!Found)) {
      for (i = 0; i < MAX_KEY_BINDINGS; i++) {
        if ((Msg->Code == KeyMap[Action][i].VirtKey) &&
            (KeyMap[Action][i].ModKey == ModKey))
          Found = 1;
      }

      if (!Found)
        Action++;
    }

    if (Found)
      Event = Action;
    else {
      /* Check run key */
      if ((Msg->Code == RunKeyMap.VirtKey) && (RunKeyMap.ModKey == ModKey))
        Runkey = 1;
    }

    break;
  }
  }

  ReplyMsg((struct Message *)Msg);
}

/* =============================================================================
 * Exported functions
 */

/* =============================================================================
 * FUNCTION: init_app
 */
int init_app(void) {
  int x, y;
  struct ScreenModeRequester *req = NULL;
  long ScreenMode;

  //
  // Open the Amiga libraries required
  //
  IntuitionBase = (struct IntuitionBase *)OpenLibrary("intuition.library", 0L);
  GfxBase = (struct GfxBase *)OpenLibrary("graphics.library", 0L);

  AslBase = OpenLibrary("asl.library", 36L);

  if (AslBase == NULL) {
    printf("Error: ULarn requires the ASL Library\n");

    return 0;
  }

  //
  // Ask for the screen mode
  //

  req = AllocAslRequestTags(ASL_ScreenModeRequest, TAG_DONE);

  if (req == NULL) {
    printf("Error: Couldn't allocate the screen mode requester\n");
    return 0;
  }

  if (AslRequestTags(req, TAG_DONE))
    ScreenMode = req->sm_DisplayID;

  FreeAslRequest(req);

  //
  // Create the screen
  //

  if (ModeNotAvailable(ScreenMode)) {
    printf("Error: Requested screen mode 0x%X is not available\n", ScreenMode);

    return 0;
  }

  //
  // Open the font and get the font metrics
  //

  UlarnFont = OpenDiskFont(&UlarnTextAttr);
  if (UlarnFont == NULL) {
    printf("Error: Couldn't open font\n");
    return 0;
  }

  CharWidth = UlarnFont->tf_XSize;
  CharHeight = UlarnFont->tf_YSize;
  CharBaseline = UlarnFont->tf_Baseline + 2;

  UlarnScreen =
      OpenScreenTags(NULL, SA_Pens, UlarnPens, SA_DisplayID, ScreenMode,
                     SA_Width, STDSCREENWIDTH, SA_Height, STDSCREENHEIGHT,
                     SA_Depth, 8, SA_Title, "Ularn", SA_Type, CUSTOMSCREEN,
                     SA_ShowTitle, TRUE, SA_Font, &UlarnTextAttr, TAG_DONE);

  if (UlarnScreen == NULL) {
    printf("Error: Couldn't open Ularn screen\n");
    return 0;
  }

  //
  // Create the window
  //

  UlarnWindow = OpenWindowTags(
      NULL, WA_Left, 0, WA_Top, 0, WA_Width, UlarnScreen->Width, WA_Height,
      UlarnScreen->Height, WA_IDCMP,
      IDCMP_VANILLAKEY | IDCMP_RAWKEY | IDCMP_MENUPICK, WA_Title, "Ularn",
      WA_CustomScreen, UlarnScreen, WA_DetailPen, WHITE_PEN, WA_BlockPen,
      MID_PEN, WA_Borderless, TRUE, WA_Backdrop, TRUE, WA_Activate, TRUE,
      WA_SmartRefresh, TRUE, TAG_DONE);

  if (UlarnWindow == NULL) {
    printf("Error: Couldn't create Ularn window\n");
    return 0;
  }

  if (!MakeMenuStructure(UlarnWindow, UlarnMenu)) {
    printf("Error: Couldn't make menu\n");
    return 0;
  }

  UlarnRP = UlarnWindow->RPort;

  //
  // Load the graphics
  //
  UlarnGfx = ReadIff(TileBMName, UlarnPalette);

  if (UlarnGfx == NULL) {
    printf("Error: Could't open graphics tiles");
    return 0;
  }

  //
  // Setup the palette
  //

  for (x = 0; x < 256; x++) {
    SetRGB32(&(UlarnScreen->ViewPort), x, (UlarnPalette[x] & 0xff0000) << 8,
             (UlarnPalette[x] & 0x00ff00) << 16,
             (UlarnPalette[x] & 0x0000ff) << 24);
  }

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

  //
  // Set the initial text buffers
  //
  CursorX = MsgCursorX;
  CursorY = MsgCursorY;
  CurrentFormat = CurrentMsgFormat;

  AText = MessageChr;
  Format = MessageFmt;
  MaxLine = MAX_MSG_LINES;

  TLeft = MessageLeft;
  TTop = MessageTop;
  TWidth = MessageWidth;
  THeight = MessageHeight;

  //
  // Call resize to perform initial sizing and trigger the intial redraw
  //
  Resize();

  return 1;
}

/* =============================================================================
 * FUNCTION: close_app
 */
void close_app(void) {

  if (UlarnGfx != NULL) {
    FreeBitmap(UlarnGfx);
    UlarnGfx = NULL;
  }

  if (UlarnWindow != NULL) {
    MenuQuit();
    CloseWindow(UlarnWindow);
    UlarnWindow = NULL;
  }

  if (UlarnScreen != NULL) {
    CloseScreen(UlarnScreen);
    UlarnScreen = NULL;
  }

  if (UlarnFont != NULL) {
    CloseFont(UlarnFont);
    UlarnFont = NULL;
  }

  if (AslBase != NULL) {
    CloseLibrary(AslBase);
    AslBase = NULL;
  }

  if (GfxBase != NULL) {
    CloseLibrary((struct Library *)GfxBase);
    GfxBase = NULL;
  }

  if (IntuitionBase != NULL) {
    CloseLibrary((struct Library *)IntuitionBase);
    IntuitionBase = NULL;
  }
}

/* =============================================================================
 * FUNCTION: get_normal_input
 */
ActionType get_normal_input(void) {
  int idx;
  int got_dir;

  Event = ACTION_NULL;
  Runkey = 0;

  /* Process input events until an action has been selected */

  while (Event == ACTION_NULL) {
    HandleInput();

    if (!enhance_interface) {
      if ((Event == ACTION_OPEN_DOOR) || (Event == ACTION_OPEN_CHEST))
        Event = ACTION_NULL;
    }
  }

  /* Check for the run key, and adjust events accordingly */
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
  char *ch;

  Print(prompt);

  //
  // Show the cursor if required
  //
  if (ShowCursor) {
    SetAPen(UlarnRP, BLUE_PEN);
    RectFill(UlarnRP, TLeft + (CursorX - 1) * CharWidth,
             TTop + (CursorY - 1) * CharHeight + 2,
             TLeft + (CursorX)*CharWidth - 1,
             TTop + (CursorY)*CharHeight + 2 - 1);
    CaretActive = 1;
  }

  GotChar = 0;

  while (!GotChar) {
    HandleInput();

    if (GotChar) {
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
    SetAPen(UlarnRP, WHITE_PEN);
    RectFill(UlarnRP, TLeft + (CursorX - 1) * CharWidth,
             TTop + (CursorY - 1) * CharHeight + 2,
             TLeft + (CursorX)*CharWidth - 1,
             TTop + (CursorY)*CharHeight + 2 - 1);
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

    if (isprint(ch) && (Pos < Len)) {
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
#if 0
/* =============================================================================
 * FUNCTION: get_string_input
 */
static void get_string_input(char *string, int Len)
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

    if (isprint((int)ch) && (Pos < Len)) {
      string[Pos] = ch;
      Pos++;
      Printc(ch);
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

  string[Pos] = 0;

}
#endif
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
  int got_dir;
  int idx;

  //
  // Display the prompt at the current position
  //
  Print(prompt);

  if (ShowCursor) {
    SetAPen(UlarnRP, BLUE_PEN);
    RectFill(UlarnRP, TLeft + (CursorX - 1) * CharWidth,
             TTop + (CursorY - 1) * CharHeight + 2,
             TLeft + (CursorX)*CharWidth - 1,
             TTop + (CursorY)*CharHeight + 2 - 1);
    CaretActive = 1;
  }

  Event = ACTION_NULL;
  got_dir = 0;

  while (!got_dir) {
    HandleInput();

    idx = 0;
    while ((idx < NUM_DIRS) && (!got_dir)) {
      if (DirActions[idx] == Event)
        got_dir = 1;
      else
        idx++;
    }
  }

  if (ShowCursor) {
    SetAPen(UlarnRP, WHITE_PEN);
    RectFill(UlarnRP, TLeft + (CursorX - 1) * CharWidth,
             TTop + (CursorY - 1) * CharHeight + 2,
             TLeft + (CursorX)*CharWidth - 1,
             TTop + (CursorY)*CharHeight + 2 - 1);
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

    AText = MessageChr;
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

    AText = TextChr;
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
          AText[Line][x] = AText[Line + 1][x];
          Format[Line][x] = Format[Line + 1][x];
        }
      }
      CursorY--;

      for (x = 0; x < LINE_LENGTH; x++) {
        AText[MaxLine - 1][x] = ' ';
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
      AText[y][x] = ' ';
      Format[y][x] = FORMAT_NORMAL;
    }

    AText[y][LINE_LENGTH] = 0;
  }

  CursorX = 1;
  CursorY = 1;

  //
  // Clear the text area
  //
  PaintTextWindow();
}

/* =============================================================================
 * FUNCTION: UlarnBeep
 */
void UlarnBeep(void) {
  if (!nobeep) {
    //
    //
    //
    DisplayBeep(UlarnScreen);
  }
}

/* =============================================================================
 * FUNCTION: MoveCursor
 */
void MoveCursor(int x, int y) {
  CursorX = x;
  CursorY = y;

  if (CursorX < 1)
    CursorX = 1;
  if (CursorY < 1)
    CursorY = 1;
  if (CursorX > LINE_LENGTH)
    CursorX = LINE_LENGTH;
  if (CursorY > MaxLine)
    CursorY = MaxLine;
}

/* =============================================================================
 * FUNCTION: Printc
 */
void Printc(char c) {
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

  case 13:
    /* Ignore LF */
    break;

  default:
    AText[CursorY - 1][CursorX - 1] = c;
    Format[CursorY - 1][CursorX - 1] = CurrentFormat;

    switch (CurrentFormat) {
    case FORMAT_NORMAL:
      SetAPen(UlarnRP, BLACK_PEN);
      break;
    case FORMAT_STANDOUT:
      SetAPen(UlarnRP, RED_PEN);
      break;
    case FORMAT_STANDOUT2:
      SetAPen(UlarnRP, GREEN_PEN);
      break;
    case FORMAT_STANDOUT3:
      SetAPen(UlarnRP, BLUE_PEN);
      break;
    default:
      break;
    }

    Move(UlarnRP, TLeft + (CursorX - 1) * CharWidth,
         TTop + (CursorY - 1) * CharHeight + CharBaseline);

    Text(UlarnRP, &c, 1);

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
    AText[CursorY - 1][x - 1] = ' ';
    Format[CursorY - 1][x - 1] = FORMAT_NORMAL;
  }

  Move(UlarnRP, TLeft + (CursorX - 1) * CharWidth,
       TTop + (CursorY - 1) * CharHeight + CharBaseline);

  Text(UlarnRP, &(AText[CursorY - 1][CursorX - 1]),
       (LINE_LENGTH - CursorX) + 1);
}

/* =============================================================================
 * FUNCTION: ClearToEOPage
 */
void ClearToEOPage(int x, int y) {
  int tx, ty;

  for (tx = x; tx <= LINE_LENGTH; tx++) {
    AText[y - 1][tx - 1] = ' ';
    Format[y - 1][tx - 1] = FORMAT_NORMAL;
  }

  Move(UlarnRP, TLeft + (x - 1) * CharWidth,
       TTop + (y - 1) * CharHeight + CharBaseline);
  Text(UlarnRP, &(AText[y - 1][x - 1]), (LINE_LENGTH - x) + 1);

  for (ty = y + 1; ty <= MaxLine; ty++) {
    for (tx = 1; tx <= LINE_LENGTH; tx++) {
      AText[ty - 1][tx - 1] = ' ';
      Format[ty - 1][tx - 1] = FORMAT_NORMAL;
    }

    Move(UlarnRP, TLeft, TTop + (ty - 1) * CharHeight + CharBaseline);
    Text(UlarnRP, AText[ty - 1], LINE_LENGTH);
  }
}

/* =============================================================================
 * FUNCTION: show1cell
 */
void show1cell(int x, int y) {
  int TileId;
  int sx, sy;
  int TileX, TileY;

  /* see nothing if blind   */
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

  BltBitMapRastPort(UlarnGfx, TileX, TileY, UlarnRP, MapLeft + sx * TileWidth,
                    MapTop + sy * TileHeight, TileWidth, TileHeight, 0xc0);
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

      BltBitMapRastPort(UlarnGfx, TileX, TileY, UlarnRP,
                        MapLeft + sx * TileWidth, MapTop + sy * TileHeight,
                        TileWidth, TileHeight, 0xc0);

      TileId = TILE_CURSOR1;
      TileX = (TileId % 16) * TileWidth;
      TileY = (TileId / 16) * TileHeight;
      BltBitMapRastPort(UlarnGfx, TileX, TileY, UlarnRP,
                        MapLeft + sx * TileWidth, MapTop + sy * TileHeight,
                        TileWidth, TileHeight, 0x80);

      TileId = TILE_CURSOR2;
      TileX = (TileId % 16) * TileWidth;
      TileY = (TileId / 16) * TileHeight;
      BltBitMapRastPort(UlarnGfx, TileX, TileY, UlarnRP,
                        MapLeft + sx * TileWidth, MapTop + sy * TileHeight,
                        TileWidth, TileHeight, 0xe0);

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

        BltBitMapRastPort(UlarnGfx, TileX, TileY, UlarnRP,
                          MapLeft + sx * TileWidth, MapTop + sy * TileHeight,
                          TileWidth, TileHeight, 0xc0);
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

            BltBitMapRastPort(
                UlarnGfx, TileX, TileY, UlarnRP, MapLeft + sx * TileWidth,
                MapTop + sy * TileHeight, TileWidth, TileHeight, 0xc0);
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

  /* see nothing if blind   */
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

  BltBitMapRastPort(UlarnGfx, TileX, TileY, UlarnRP, MapLeft + sx * TileWidth,
                    MapTop + sy * TileHeight, TileWidth, TileHeight, 0xc0);
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

  /* see nothing if blind   */
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

    BltBitMapRastPort(UlarnGfx, TileX, TileY, UlarnRP, MapLeft + sx * TileWidth,
                      MapTop + sy * TileHeight, TileWidth, TileHeight, 0xc0);

    TileId = magicfx_tile[fx].Tile1[frame];
    TileX = (TileId % 16) * TileWidth;
    TileY = (TileId / 16) * TileHeight;
    BltBitMapRastPort(UlarnGfx, TileX, TileY, UlarnRP, MapLeft + sx * TileWidth,
                      MapTop + sy * TileHeight, TileWidth, TileHeight, 0x80);

    TileId = magicfx_tile[fx].Tile2[frame];
    TileX = (TileId % 16) * TileWidth;
    TileY = (TileId / 16) * TileHeight;
    BltBitMapRastPort(UlarnGfx, TileX, TileY, UlarnRP, MapLeft + sx * TileWidth,
                      MapTop + sy * TileHeight, TileWidth, TileHeight, 0xe0);
  } else {
    TileId = magicfx_tile[fx].Tile1[frame];
    TileX = (TileId % 16) * TileWidth;
    TileY = (TileId / 16) * TileHeight;

    BltBitMapRastPort(UlarnGfx, TileX, TileY, UlarnRP, MapLeft + sx * TileWidth,
                      MapTop + sy * TileHeight, TileWidth, TileHeight, 0xc0);
  }
}

/* =============================================================================
 * FUNCTION: nap
 */
void nap(int delay) {
  //
  // Delay for delay/20 ticks (50 ticks per second)
  //
  Delay(delay / 20);
}

//
//
//

static char *UserName;

/* =============================================================================
 * FUNCTION: GetUser
 */
void GetUser(char *username, int *uid) {
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
    Print("Who are you? ");
    get_string_input(username, USERNAME_LENGTH);

    if (strlen(username) == 0)
      strcpy(username, "Anon");
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
  } else {
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
