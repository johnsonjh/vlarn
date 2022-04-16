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
 * This is the curses based TTY display module.
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
 * beep                   : Make a beep
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

#include <curses.h>

#include <stdarg.h>
#include <stdio.h>

#include "config.h"

#include "header.h"
#include "ularn_game.h"

#include "dungeon.h"
#include "itm.h"
#include "monster.h"
#include "player.h"
#include "ularn_win.h"

//
// player id file
//
#ifndef UNIX
static char *PIDName = LIBDIR "\\vlarn.pid";
#endif
#define FIRST_PID 1001

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

int UseColor = 0;
int CaretActive = 0;

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
    {{0, 0}, {0, 0}, {0, 0}},                       // ACTION_NULL
    {{'~', M_ASCII}, {0, 0}, {0, 0}},               // ACTION_DIAG
    {{'h', M_ASCII}, {KEY_LEFT, M_ASCII}, {0, 0}},  // ACTION_MOVE_WEST
    {{'H', M_ASCII}, {0, 0}, {0, 0}},               // ACTION_RUN_WEST
    {{'l', M_ASCII}, {KEY_RIGHT, M_ASCII}, {0, 0}}, // ACTION_MOVE_EAST,
    {{'L', M_ASCII}, {0, 0}, {0, 0}},               // ACTION_RUN_EAST,
    {{'j', M_ASCII}, {KEY_DOWN, M_ASCII}, {0, 0}},  // ACTION_MOVE_SOUTH,
    {{'J', M_ASCII}, {0, 0}, {0, 0}},               // ACTION_RUN_SOUTH,
    {{'k', M_ASCII}, {KEY_UP, M_ASCII}, {0, 0}},    // ACTION_MOVE_NORTH,
    {{'K', M_ASCII}, {0, 0}, {0, 0}},               // ACTION_RUN_NORTH,
    {{'u', M_ASCII}, {KEY_A3, M_ASCII}, {0, 0}},    // ACTION_MOVE_NORTHEAST,
    {{'U', M_ASCII}, {0, 0}, {0, 0}},               // ACTION_RUN_NORTHEAST,
    {{'y', M_ASCII}, {KEY_A1, M_ASCII}, {0, 0}},    // ACTION_MOVE_NORTHWEST,
    {{'Y', M_ASCII}, {0, 0}, {0, 0}},               // ACTION_RUN_NORTHWEST,
    {{'n', M_ASCII}, {KEY_C3, M_ASCII}, {0, 0}},    // ACTION_MOVE_SOUTHEAST,
    {{'N', M_ASCII}, {0, 0}, {0, 0}},               // ACTION_RUN_SOUTHEAST,
    {{'b', M_ASCII}, {KEY_C1, M_ASCII}, {0, 0}},    // ACTION_MOVE_SOUTHWEST,
    {{'B', M_ASCII}, {0, 0}, {0, 0}},               // ACTION_RUN_SOUTHWEST,
    {{'.', M_ASCII}, {0, 0}, {0, 0}},               // ACTION_WAIT,
    {{' ', M_ASCII}, {0, 0}, {0, 0}},               // ACTION_NONE,
    {{'w', M_ASCII}, {0, 0}, {0, 0}},               // ACTION_WIELD,
    {{'W', M_ASCII}, {0, 0}, {0, 0}},               // ACTION_WEAR,
    {{'r', M_ASCII}, {0, 0}, {0, 0}},               // ACTION_READ,
    {{'q', M_ASCII}, {0, 0}, {0, 0}},               // ACTION_QUAFF,
    {{'d', M_ASCII}, {0, 0}, {0, 0}},               // ACTION_DROP,
    {{'c', M_ASCII}, {0, 0}, {0, 0}},               // ACTION_CAST_SPELL,
    {{'o', M_ASCII}, {0, 0}, {0, 0}},               // ACTION_OPEN_DOOR
    {{'C', M_ASCII}, {0, 0}, {0, 0}},               // ACTION_CLOSE_DOOR,
    {{'O', M_ASCII}, {0, 0}, {0, 0}},               // ACTION_OPEN_CHEST
    {{'i', M_ASCII}, {0, 0}, {0, 0}},               // ACTION_INVENTORY,
    {{'e', M_ASCII}, {0, 0}, {0, 0}},               // ACTION_EAT_COOKIE,
    {{'\\', M_ASCII}, {0, 0}, {0, 0}},              // ACTION_LIST_SPELLS,
    {{'?', M_ASCII}, {0, 0}, {0, 0}},               // ACTION_HELP,
    {{'S', M_ASCII}, {0, 0}, {0, 0}},               // ACTION_SAVE,
    {{'Z', M_ASCII}, {0, 0}, {0, 0}},               // ACTION_TELEPORT,
    {{'^', M_ASCII}, {0, 0}, {0, 0}},               // ACTION_IDENTIFY_TRAPS,
    {{'_', M_ASCII}, {0, 0}, {0, 0}},               // ACTION_BECOME_CREATOR,
    {{'+', M_ASCII}, {0, 0}, {0, 0}},               // ACTION_CREATE_ITEM,
    {{'-', M_ASCII}, {0, 0}, {0, 0}},               // ACTION_TOGGLE_WIZARD,
    {{'`', M_ASCII}, {0, 0}, {0, 0}},               // ACTION_DEBUG_MODE,
    {{'T', M_ASCII}, {0, 0}, {0, 0}},               // ACTION_REMOVE_ARMOUR,
    {{'g', M_ASCII}, {0, 0}, {0, 0}},               // ACTION_PACK_WEIGHT,
    {{'v', M_ASCII}, {0, 0}, {0, 0}},               // ACTION_VERSION,
    {{'Q', M_ASCII}, {0, 0}, {0, 0}},               // ACTION_QUIT,
    {{18, M_ASCII}, {0, 0}, {0, 0}},                // ACTION_REDRAW_SCREEN,
    {{'P', M_ASCII}, {0, 0}, {0, 0}}                // ACTION_SHOW_TAX
};

static struct KeyCodeType RunKeyMap = {KEY_B2, M_ASCII};

typedef enum {
  C_BLACK,
  C_RED,
  C_GREEN,
  C_YELLOW,
  C_BLUE,
  C_MAGENTA,
  C_CYAN,
  C_WHITE,
  C_COUNT
} Ularn_Color_Type;

static int ColorPairs[C_COUNT][2] = {
    {COLOR_BLACK, COLOR_BLACK}, {COLOR_RED, COLOR_BLACK},
    {COLOR_GREEN, COLOR_BLACK}, {COLOR_YELLOW, COLOR_BLACK},
    {COLOR_BLUE, COLOR_BLACK},  {COLOR_MAGENTA, COLOR_BLACK},
    {COLOR_CYAN, COLOR_BLACK},  {COLOR_WHITE, COLOR_BLACK}};

static int Runkey;
static ActionType Event;
static int GotChar;
static int EventChar;

//
// Characters for tiles
//

int WallTile[16] = {
    '#', '#', '#', '#', '#', '#', '#', '#', '#',
    '#', '#', '#', '#', '#', '#', '#'
    //'#', '-', '|', '+', '-', '-', '+', '-', '|', '+', '|', '|', '+', '-', '|',
    //'#'
};

/* Tiles for directional effects */
static int EffectTile[EFFECT_COUNT][9] = {
    {'*', '|', '-', '|', '-', '/', '\\', '\\', '/'},
    {'*', '|', '-', '|', '-', '/', '\\', '\\', '/'},
    {'*', '|', '-', '|', '-', '/', '\\', '\\', '/'},
    {'*', '|', '-', '|', '-', '/', '\\', '\\', '/'},
    {'*', '|', '-', '|', '-', '/', '\\', '\\', '/'},
};

static int EffectColor[EFFECT_COUNT] = {C_GREEN, C_CYAN, C_RED, C_YELLOW,
                                        C_WHITE};

#define MAX_MAGICFX_FRAME 8

struct MagicEffectDataType {
  int Frames;                   /* Number of frames in the effect  */
  int Tile[MAX_MAGICFX_FRAME];  /* The primary tile for this frame */
  int Color[MAX_MAGICFX_FRAME]; /* Only used for overlay effects   */
};

static struct MagicEffectDataType magicfx_tile[MAGIC_COUNT] = {
    /* Sparkle */
    {8,
     {'-', '\\', '|', '/', '-', '\\', '|', '/'},
     {C_RED, C_YELLOW, C_GREEN, C_CYAN, C_BLUE, C_MAGENTA, C_RED, C_YELLOW}},

    /* Sleep */
    {6,
     {'z', 'Z', 'z', 'Z', 'z', 'Z', 0, 0},
     {C_RED, C_RED, C_GREEN, C_GREEN, C_BLUE, C_BLUE, 0, 0}},

    /* Web */
    {6,
     {'.', 'o', '*', '#', '#', '#', 0, 0},
     {C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_GREEN, C_BLUE, 0, 0}},

    /* Phantasmal forces */
    {6,
     {'.', ':', '^', 'A', 'A', 'A', 0, 0},
     {C_BLUE, C_BLUE, C_MAGENTA, C_MAGENTA, C_CYAN, C_CYAN, 0, 0}},

    /* Cloud kill */
    {6,
     {'.', 'o', '*', '#', '#', 'O', 0, 0},
     {C_GREEN, C_GREEN, C_GREEN, C_GREEN, C_YELLOW, C_GREEN, 0, 0}},

    /* Vaporize rock */
    {6,
     {'.', 'o', '*', '#', '#', 'O', 0, 0},
     {C_RED, C_RED, C_RED, C_RED, C_YELLOW, C_RED, 0, 0}},

    /* Dehydrate */
    {6,
     {'.', ':', '|', 'T', '^', '~', 0, 0},
     {C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE, 0, 0}},

    /* Drain life */
    {6,
     {'#', '*', '|', ':', 'o', '.', 0, 0},
     {C_YELLOW, C_YELLOW, C_YELLOW, C_RED, C_RED, C_RED, 0, 0}},

    /* Flood */
    {6,
     {'.', 'o', 'O', 'o', 'O', 'o', 0, 0},
     {C_BLUE, C_BLUE, C_BLUE, C_CYAN, C_CYAN, C_BLUE, 0, 0}},

    /* Finger of death */
    {6,
     {'#', '*', '|', ':', 'o', '.', 0, 0},
     {C_RED, C_RED, C_MAGENTA, C_MAGENTA, C_BLUE, C_BLUE, 0, 0}},

    /* Teleport away */
    {6,
     {':', '|', 'H', 'H', '|', ':', 0, 0},
     {C_CYAN, C_CYAN, C_CYAN, C_BLUE, C_BLUE, C_BLUE, 0, 0}},

    /* Magic fire */
    {6,
     {'.', 'o', '*', '#', '#', 'O', 0, 0},
     {C_RED, C_RED, C_RED, C_RED, C_YELLOW, C_RED, 0, 0}},

    /* Make wall */
    {6,
     {'.', ':', 'H', '#', '#', '#', 0, 0},
     {C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_RED, C_WHITE, 0, 0}},

    /* Summon demon */
    {6,
     {'.', ':', '^', '8', '8', '8', 0, 0},
     {C_MAGENTA, C_MAGENTA, C_RED, C_RED, C_YELLOW, C_YELLOW, 0, 0}},

    /* Annihilate (scroll) */
    {6,
     {'-', '|', '-', '|', '-', '|', 0, 0},
     {C_RED, C_RED, C_YELLOW, C_YELLOW, C_GREEN, C_GREEN, 0, 0}}};

//
// Display attributes and colours
//

static int ItemAttr[OCOUNT] = {
    0, 0,
    /* Dungeon features */
    A_REVERSE, A_REVERSE, A_REVERSE, A_REVERSE, A_REVERSE, A_REVERSE, A_REVERSE,
    A_REVERSE, A_REVERSE, A_REVERSE, A_REVERSE, A_REVERSE, A_REVERSE, A_REVERSE,
    A_REVERSE, A_REVERSE, 0,
    /* gold piles */
    0, 0, 0, 0,
    /* eye of larn */
    A_STANDOUT,
    /* armour */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* weapons */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* rings */
    0, 0, 0, 0, 0, 0, 0, 0,
    /* magic items */
    0, 0, 0, A_REVERSE, A_REVERSE, A_BOLD, 0, A_BOLD, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* gems */
    0, 0, 0, 0,
    /* buildings/entrances */
    A_REVERSE, A_REVERSE, A_REVERSE, A_REVERSE, A_REVERSE, A_REVERSE, A_REVERSE,
    A_REVERSE, A_REVERSE, A_REVERSE, A_REVERSE,
    /* traps */
    0, 0, 0, 0, 0, 0, 0,
    /* misc */
    0, 0, 0,
    /* drugs */
    0, 0, 0, 0, 0};

static int ItemColor[OCOUNT] = {
    C_WHITE, C_WHITE,
    /* Dungeon features */
    C_WHITE, C_YELLOW, C_YELLOW, C_YELLOW, C_WHITE, C_GREEN, C_WHITE, C_BLUE,
    C_WHITE, C_RED, C_WHITE, C_RED, C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE,
    /* gold piles */
    C_YELLOW, C_YELLOW, C_YELLOW, C_YELLOW,
    /* eye of larn */
    C_WHITE,
    /* armour */
    C_CYAN, C_CYAN, C_WHITE, C_CYAN, C_WHITE, C_CYAN, C_CYAN, C_CYAN, C_WHITE,
    C_GREEN,
    /* weapons */
    C_CYAN, C_WHITE, C_CYAN, C_CYAN, C_WHITE, C_CYAN, C_CYAN, C_CYAN, C_WHITE,
    C_WHITE, C_MAGENTA, C_YELLOW,
    /* rings */
    C_WHITE, C_GREEN, C_BLUE, C_WHITE, C_CYAN, C_RED, C_MAGENTA, C_WHITE,
    /* magic items */
    C_WHITE, C_WHITE, C_YELLOW, C_CYAN, C_YELLOW, C_CYAN, C_WHITE, C_RED,
    C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE,
    C_WHITE,
    /* gems */
    C_WHITE, C_RED, C_GREEN, C_BLUE,
    /* buildings/entrances */
    C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE,
    C_WHITE, C_WHITE, C_WHITE,
    /* traps */
    C_RED, C_RED, C_RED, C_RED, C_RED, C_RED, C_RED,
    /* misc */
    C_WHITE, C_WHITE, C_WHITE,
    /* drugs */
    C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE};

static int MonstColor[MONST_COUNT] = {
    C_WHITE,   C_WHITE,   C_GREEN,   C_GREEN,   C_WHITE, C_WHITE,  C_RED,
    C_GREEN,   C_GREEN,   C_RED,     C_RED,     C_RED,   C_CYAN,   C_GREEN,
    C_WHITE,   C_BLUE,    C_RED,     C_WHITE,   C_WHITE, C_RED,    C_RED,
    C_CYAN,    C_GREEN,   C_YELLOW,  C_WHITE,   C_WHITE, C_GREEN,  C_WHITE,
    C_CYAN,    C_YELLOW,  C_RED,     C_MAGENTA, C_WHITE, C_YELLOW, C_YELLOW,
    C_YELLOW,  C_YELLOW,  C_RED,     C_WHITE,   C_WHITE, C_WHITE,  C_CYAN,
    C_GREEN,   C_YELLOW,  C_MAGENTA, C_GREEN,   C_BLUE,  C_BLUE,   C_MAGENTA,
    C_GREEN,   C_MAGENTA, C_RED,     C_YELLOW,  C_CYAN,  C_YELLOW, C_GREEN,
    C_RED,     C_WHITE,   C_YELLOW,  C_GREEN,   C_CYAN,  C_BLUE,   C_RED,
    C_MAGENTA, C_CYAN,    C_MAGENTA};

//
// Current display mode
//
DisplayModeType CurrentDisplayMode = DISPLAY_TEXT;

WINDOW *MapWindow;
WINDOW *StatusWindow;
WINDOW *EffectsWindow;
WINDOW *MessageWindow;

WINDOW *TextWindow;

// =============================================================================
// Text mode stuff
//

/* XXX trn default is 80 */
#define LINE_LENGTH 80

//
// Messages
//
#define MAX_MSG_LINES 5
static FormatType CurrentMsgFormat;
static int MsgCursorX = 1;
static int MsgCursorY = 1;

//
// Text
//
/* XXX trn default is 24, 80 */
#define MAX_TEXT_LINES 24
#define TEXT_LINE_LENGTH 80
static FormatType CurrentTextFormat;
static int TextCursorX = 1;
static int TextCursorY = 1;

//
// Generalised text buffer
// Top left corner is x=1, y=1
//
static FormatType CurrentFormat;
static int CursorX = 1;
static int CursorY = 1;
static int MaxLine;

//
// The monster to use for showing mimics. Changes every 10 turns.
//
static int mimicmonst = MIMIC;

/* =============================================================================
 * Local functions
 */

/* =============================================================================
 * FUNCTION: SetWallTiles
 *
 * DESCRIPTION:
 * Set the wall tiles to the ACS line drawing values
 *
 * PARAMETERS:
 *
 *   None
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void SetWallTiles(void) {
  WallTile[0] = ACS_BLOCK;
  WallTile[1] = ACS_HLINE;
  WallTile[2] = ACS_VLINE;
  WallTile[3] = ACS_URCORNER;
  WallTile[4] = ACS_HLINE;
  WallTile[5] = ACS_HLINE;
  WallTile[6] = ACS_ULCORNER;
  WallTile[7] = ACS_TTEE;
  WallTile[8] = ACS_VLINE;
  WallTile[9] = ACS_LRCORNER;
  WallTile[10] = ACS_VLINE;
  WallTile[11] = ACS_RTEE;
  WallTile[12] = ACS_LLCORNER;
  WallTile[13] = ACS_BTEE;
  WallTile[14] = ACS_LTEE;
  WallTile[15] = ACS_PLUS;
}

/* =============================================================================
 * FUNCTION: SetCursesAttr
 *
 * DESCRIPTION:
 * Set curses text display attributes for the indicated format.
 *
 * PARAMETERS:
 *
 *   Format : The text format.
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void SetCursesAttr(FormatType Format) {
  int BaseAttr;

#ifdef W32_TTY

  BaseAttr = A_BOLD;

#else

  BaseAttr = 0;

#endif

  switch (Format) {
  case FORMAT_NORMAL:
    if (UseColor)
      wattrset(TextWindow, BaseAttr | COLOR_PAIR(C_WHITE));
    else
      wattrset(TextWindow, A_NORMAL);
    break;
  case FORMAT_STANDOUT:
    if (UseColor)
      wattrset(TextWindow, BaseAttr | COLOR_PAIR(C_RED));
    else
      wattrset(TextWindow, A_STANDOUT);
    break;
  case FORMAT_STANDOUT2:
    if (UseColor)
      wattrset(TextWindow, BaseAttr | COLOR_PAIR(C_GREEN));
    else
      wattrset(TextWindow, A_BOLD);
    break;
  case FORMAT_STANDOUT3:
    if (UseColor)
      wattrset(TextWindow, BaseAttr | COLOR_PAIR(C_BLUE));
    else
      wattrset(TextWindow, A_UNDERLINE);
    break;
  default:
    break;
  }
}

/* =============================================================================
 * FUNCTION: setup_colour_pairs
 *
 * DESCRIPTION:
 * Setup the colour pairs used by curses.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void setup_colour_pairs(void) {

  short i;

  for (i = 0; i < C_COUNT; i++)
    init_pair(i, ColorPairs[i][0], ColorPairs[i][1]);

#ifdef W32_TTY

  //
  // PD Curses on windoze doesn't seem to do reverse video correctly!
  // Need to set up the reverse colour combinations
  //

  for (i = 0; i < C_COUNT; i++)
    init_pair(C_COUNT + i, ColorPairs[i][1], ColorPairs[i][0]);

#endif
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

#ifdef W32_TTY

  wattrset(StatusWindow, A_BOLD | COLOR_PAIR(C_WHITE));

#else

  wattrset(StatusWindow, COLOR_PAIR(C_WHITE));

#endif

  if (Repaint) {
    wclear(StatusWindow);
    for (i = 0; i < 80; i++) {
      mvwaddch(StatusWindow, 0, i, ' ');
      mvwaddch(StatusWindow, 1, i, ' ');
    }
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

  mvwaddstr(StatusWindow, 0, 0, Line);

  //
  // Format the second line of the status
  //
  sprintf(Buf, "%d (%d)", (int)c[HP], (int)c[HPMAX]);

  sprintf(Line,
          "HP: %11.11s STR=%-2ld INT=%-2ld WIS=%-2ld CON=%-2ld DEX=%-2ld "
          "CHA=%-2ld LV:",
          Buf, c[STRENGTH] + c[STREXTRA], c[INTELLIGENCE],
          c[WISDOM], c[CONSTITUTION], c[DEXTERITY], c[CHARISMA]);

  if ((level == 0) || (wizard))
    c[TELEFLAG] = 0;

  if (c[TELEFLAG])
    strcat(Line, " ?");
  else
    strcat(Line, levelname[level]);

  sprintf(Buf, "  Gold: %-8ld", c[GOLD]);
  strcat(Line, Buf);

  mvwaddstr(StatusWindow, 1, 0, Line);

  wrefresh(StatusWindow);

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
} bot_data[] = {
    {STEALTH, "  Stealth    "},      {UNDEADPRO, "  Undead Pro "},
    {SPIRITPRO, "  Spirit Pro "},    {CHARMCOUNT, "  Charm      "},
    {TIMESTOP, "  Time Stop  "},     {HOLDMONST, "  Hold Monst "},
    {GIANTSTR, "  Giant Str  "},     {FIRERESISTANCE, "  Fire Resit "},
    {DEXCOUNT, "  Dexterity  "},     {STRCOUNT, "  Strength   "},
    {SCAREMONST, "  Scare      "},   {HASTESELF, "  Haste Self "},
    {CANCELLATION, "  Cancel     "}, {INVISIBILITY, "  Invisible  "},
    {ALTPRO, "  Protect 3  "},       {PROTECTIONTIME, "  Protect 2  "},
    {WTW, "  Wall-Walk  "}};

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

#ifdef W32_TTY

  wattrset(StatusWindow, A_BOLD | COLOR_PAIR(C_WHITE));

#else

  wattrset(StatusWindow, COLOR_PAIR(C_WHITE));

#endif

  if (Repaint)
    wclear(EffectsWindow);

  for (i = 0; i < 17; i++) {
    idx = bot_data[i].typ;
    WasSet = (cbak[idx] != 0);
    IsSet = (c[idx] != 0);

    if ((Repaint) || (IsSet != WasSet)) {
      if (IsSet)
        mvwaddstr(EffectsWindow, i, 0, bot_data[i].string);
      else
        mvwaddstr(EffectsWindow, i, 0, "             ");
    }

    cbak[idx] = c[idx];
  }

  wrefresh(EffectsWindow);
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
 *   Attr   : The curses attribute for this tile
 *
 *   ColorPair : The colour pair for this tile
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void GetTile(int x, int y, int *TileId, int *Attr, int *ColorPair) {
  MonsterIdType k;

  *Attr = 0;

  if ((x == playerx) && (y == playery) && (c[BLINDCOUNT] == 0)) {
    //
    // This is the square containing the player and the players isn't
    // blind, so return the player tile.
    //
    *TileId = '@';
    *ColorPair = C_RED;
    return;
  }

  //
  // Work out what is here
  //
  if (know[x][y] == OUNKNOWN) {
    //
    // The player doesn't know what is at this position.
    //
    *TileId = objnamelist[OUNKNOWN];
    *Attr = ItemAttr[(int)know[x][y]];
    *ColorPair = C_BLACK;
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

          *TileId = monstnamelist[mimicmonst];
          *ColorPair = MonstColor[mimicmonst];
        } else if ((k == INVISIBLESTALKER) && (c[SEEINVISIBLE] == 0)) {
          *TileId = objnamelist[(int)know[x][y]];
          *Attr = ItemAttr[(int)know[x][y]];
          *ColorPair = ItemColor[(int)know[x][y]];
        } else if ((k >= DEMONLORD) && (k <= LUCIFER) && (c[EYEOFLARN] == 0)) {
          /* demons are invisible if not have the eye */
          *TileId = objnamelist[(int)know[x][y]];
          *Attr = ItemAttr[(int)know[x][y]];
          *ColorPair = ItemColor[(int)know[x][y]];
        } else {
          *TileId = monstnamelist[k];
          *ColorPair = MonstColor[k];
        }

      } /* can see monster */
      else {
        /*
         * The monster at this location is not known to the player, so show
         * the tile for the item at this location
         */
        *TileId = objnamelist[(int)know[x][y]];
        *Attr = ItemAttr[(int)know[x][y]];
        *ColorPair = ItemColor[(int)know[x][y]];
      }
    } /* monster here */
    else {
      k = know[x][y];
      *TileId = objnamelist[k];
      *Attr = ItemAttr[(int)know[x][y]];
      *ColorPair = ItemColor[(int)know[x][y]];
    }
  }

  /* Handle walls */
  if (*TileId == objnamelist[OWALL])
    *TileId = WallTile[iarg[x][y]];

#ifdef W32_TTY

  if (*Attr & A_REVERSE) {
    /* If reverse on win32/pdcurses then use the reverse color pair */
    *Attr = (*Attr & (~A_REVERSE));
    *ColorPair += C_COUNT;
  } else
    /* PD Curses also needs bold to make bright colours on win32 */
    *Attr |= A_BOLD;

#endif
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
  int TileId;
  int Attr;
  int Color;

  if (Repaint) {
    wclear(MapWindow);

    for (y = 0; y < MAXY; y++) {
      for (x = 0; x < MAXX; x++) {
        GetTile(x, y, &TileId, &Attr, &Color);

        if (UseColor)
          wattrset(MapWindow, Attr | COLOR_PAIR(Color));
        else
          wattrset(MapWindow, Attr);

        mvwaddch(MapWindow, y, x, TileId);
      }
    }
  }

  wrefresh(MapWindow);
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
  touchwin(TextWindow);
  wrefresh(TextWindow);
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
  PaintStatus();
  PaintEffects();
  PaintTextWindow();
  PaintMap();

  showplayer();
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

  if (CurrentDisplayMode == DISPLAY_MAP)
    PaintMapWindow();
  else
    PaintTextWindow();

  Repaint = 0;
}

/* =============================================================================
 * Exported functions
 */

/* =============================================================================
 * FUNCTION: init_app
 */
int init_app(void) {
  int x, y;

  /* Initialise curses app */

  initscr();
  cbreak();
  noecho();

  nonl();
  intrflush(stdscr, FALSE);

  //  meta(stdscr, TRUE);
  start_color();
  setup_colour_pairs();

  keypad(stdscr, TRUE);

  /* Create windows */

  MapWindow = newwin(17, 67, 0, 0);
  StatusWindow = newwin(2, 80, 17, 0);
  EffectsWindow = newwin(17, 13, 0, 67);
  MessageWindow = newwin(5, 80, 19, 0);

  SetWallTiles();

  /* Start colour mode for slowlaris */
  wattrset(stdscr, A_STANDOUT | COLOR_PAIR(C_RED));
  mvwaddch(stdscr, 0, 0, '*');

  touchwin(stdscr);
  wrefresh(stdscr);

  UseColor = has_colors();

  refresh();

  //
  // Clear the text buffers
  //

  TextWindow = MessageWindow;

  SetCursesAttr(FORMAT_NORMAL);

  for (y = 0; y < MAX_MSG_LINES; y++) {
    for (x = 0; x < LINE_LENGTH; x++)
      mvwaddch(TextWindow, y, x, ' ');
  }
  wrefresh(TextWindow);

  TextWindow = stdscr;

  SetCursesAttr(FORMAT_NORMAL);

  for (y = 0; y < MAX_TEXT_LINES; y++) {
    for (x = 0; x < LINE_LENGTH; x++)
      mvwaddch(TextWindow, y, x, ' ');
  }
  wrefresh(TextWindow);

  return 1;
}

/* =============================================================================
 * FUNCTION: close_app
 */
void close_app(void) {
  delwin(MessageWindow);
  delwin(EffectsWindow);
  delwin(StatusWindow);
  delwin(MapWindow);

  nl();

  echo();

  nocbreak();

  endwin();
}

/* =============================================================================
 * FUNCTION: get_normal_input
 */
ActionType get_normal_input(void) {
  int idx;
  int got_dir;
  int Found;
  ActionType Action;
  int i;

  Event = ACTION_NULL;
  Runkey = 0;

  while (Event == ACTION_NULL) {
    wrefresh(MapWindow);
    EventChar = getch();
    GotChar = 1;

    //
    // Decide the event corresponding to the key pressed
    //

    /* Decode key press as a ULarn Action */

    /*
     * Check ASCII key bindings if no virtual key matches and
     * got a valid ASCII char
     */
    Found = 0;

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
      if ((EventChar == RunKeyMap.VirtKey) && (RunKeyMap.ModKey == M_ASCII))
        Runkey = 1;
    }

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
  char *ch;

  Print(prompt);

  if (ShowCursor)
    CaretActive = 1;

  //
  // Process events until a character in answers has been pressed.
  //
  GotChar = 0;
  while (!GotChar) {
    wrefresh(TextWindow);

    EventChar = getch();
    GotChar = 1;

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

  if (ShowCursor)
    CaretActive = 0;

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

        wmove(TextWindow, CursorY - 1, CursorX - 1);
        wrefresh(TextWindow);
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

				wmove(TextWindow, CursorY - 1, CursorX - 1);
				wrefresh(TextWindow);
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
        wmove(TextWindow, CursorY - 1, CursorX - 1);
        wrefresh(TextWindow);
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

  //
  // Show the cursor if required
  //
  if (ShowCursor)

    CaretActive = 1;

  Event = ACTION_NULL;
  got_dir = 0;

  while (!got_dir) {
    get_normal_input();

    idx = 0;

    while ((idx < NUM_DIRS) && (!got_dir)) {
      if (DirActions[idx] == Event)
        got_dir = 1;
      else
        idx++;
    }
  }

  if (ShowCursor)
    CaretActive = 0;

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

    MaxLine = MAX_MSG_LINES;

    TextWindow = MessageWindow;

  } else if (CurrentDisplayMode == DISPLAY_TEXT) {
    CursorX = TextCursorX;
    CursorY = TextCursorY;
    CurrentFormat = CurrentTextFormat;

    MaxLine = MAX_TEXT_LINES;

    TextWindow = stdscr;
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
  /*int Scroll;*/
  int inc;
  int x;

  inc = Count;
  /*Scroll = 0;*/

  while (inc > 0) {
    CursorY = CursorY + 1;

    if (CursorY > MaxLine) {
      /*Scroll = 1;*/
      CursorY--;

      scrollok(TextWindow, 1);

      scroll(TextWindow);

      scrollok(TextWindow, 0);

      SetCursesAttr(FORMAT_NORMAL);
      for (x = 0; x < LINE_LENGTH; x++)
        mvwaddch(TextWindow, CursorY - 1, x, ' ');
    }

    inc--;
  }

  wrefresh(TextWindow);
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
  SetCursesAttr(FORMAT_NORMAL);

  for (y = 0; y < MaxLine; y++) {
    for (x = 0; x < LINE_LENGTH; x++)
      mvwaddch(TextWindow, y, x, ' ');
  }

  wrefresh(TextWindow);

  CursorX = 1;
  CursorY = 1;
}

/* =============================================================================
 * FUNCTION: beep
 */
void UlarnBeep(void) {
  //
  // Play a beep
  //
  if (!nobeep) {
#ifdef W32_TTY
    //
    // beep seems to cause problems under windows, so put a bell
    //
    putch(7);
#else
    beep();
#endif
  }
}

/* =============================================================================
 * FUNCTION: Cursor
 */
void MoveCursor(int x, int y) {
  CursorX = x;
  CursorY = y;
}

static int RefreshEachChar = 1;

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

  case '\015':
    break;

  default:

    SetCursesAttr(CurrentFormat);
    mvwaddch(TextWindow, CursorY - 1, CursorX - 1, c);

    if (RefreshEachChar)
      wrefresh(TextWindow);

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

  RefreshEachChar = 0;

  for (pos = 0; pos < Len; pos++)
    Printc(string[pos]);

  RefreshEachChar = 1;

  wrefresh(TextWindow);
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
    SetCursesAttr(FORMAT_NORMAL);
    mvwaddch(TextWindow, CursorY - 1, x - 1, ' ');
  }
}

/* =============================================================================
 * FUNCTION: ClearToEOPage
 */
void ClearToEOPage(int x, int y) {
  int tx, ty;

  SetCursesAttr(FORMAT_NORMAL);

  for (tx = x; tx <= LINE_LENGTH; tx++)
    mvwaddch(TextWindow, y - 1, tx - 1, ' ');

  for (ty = y + 1; ty <= MaxLine; ty++)
    for (tx = 1; tx <= LINE_LENGTH; tx++)
      mvwaddch(TextWindow, ty - 1, tx - 1, ' ');
}

/* =============================================================================
 * FUNCTION: show1cell
 */
void show1cell(int x, int y) {
  int TileId;
  int Attr;
  int Color;

  /* see nothing if blind		*/
  if (c[BLINDCOUNT])
    return;

  /* we end up knowing about it */
  know[x][y] = item[x][y];
  if (mitem[x][y].mon != MONST_NONE)
    stealth[x][y] |= STEALTH_SEEN;

  GetTile(x, y, &TileId, &Attr, &Color);

  wattrset(MapWindow, Attr | COLOR_PAIR(Color));
  mvwaddch(MapWindow, y, x, TileId);

  wrefresh(MapWindow);
}

/* =============================================================================
 * FUNCTION: showplayer
 */
void showplayer(void) {
  int TileId;
  int Attr;
  int Color;

  //
  // Determine if we need to scroll the map
  //

  if (c[BLINDCOUNT] == 0) {
    TileId = '@';
#ifdef W32_TTY
    Attr = A_BOLD;
#else
    Attr = 0;
#endif
    Color = C_RED;
  } else
    GetTile(playerx, playery, &TileId, &Attr, &Color);

  wattrset(MapWindow, Attr | COLOR_PAIR(Color));
  mvwaddch(MapWindow, playery, playerx, TileId);
  wmove(MapWindow, playery, playerx);
  wrefresh(MapWindow);
}

/* =============================================================================
 * FUNCTION: showcell
 */
void showcell(int x, int y) {
  int minx, maxx;
  int miny, maxy;
  int mx, my;
  int TileId;
  int Attr;
  int Color;

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
      if ((mx == playerx) && (my == playery))
        know[mx][my] = item[mx][my];
      else if ((know[mx][my] != item[mx][my]) ||     /* item changed    */
               ((mx == lastpx) && (my == lastpy)) || /* last player pos */
               ((mitem[mx][my].mon != MONST_NONE) && /* unseen monster  */
                ((stealth[mx][my] & STEALTH_SEEN) == 0))) {
        //
        // Only draw areas not already known (and hence displayed)
        //
        know[mx][my] = item[mx][my];
        if (mitem[mx][my].mon != MONST_NONE)
          stealth[mx][my] |= STEALTH_SEEN;

        GetTile(mx, my, &TileId, &Attr, &Color);

        wattrset(MapWindow, Attr | COLOR_PAIR(Color));
        mvwaddch(MapWindow, my, mx, TileId);
      } // if not known
    }
  }

  showplayer();
}

/* =============================================================================
 * FUNCTION: drawscreen
 */
void drawscreen(void) { PaintWindow(); }

/* =============================================================================
 * FUNCTION: draws
 */
void draws(int minx, int miny, int maxx, int maxy) {
  (void)minx;
  (void)miny;
  (void)maxx;
  (void)maxy;
  PaintWindow();
}

/* =============================================================================
 * FUNCTION: mapeffect
 */
void mapeffect(int x, int y, DirEffectsType effect, int dir) {
  wattrset(MapWindow, A_NORMAL | COLOR_PAIR(EffectColor[effect]));
  mvwaddch(MapWindow, y, x, EffectTile[effect][dir]);
  wmove(MapWindow, 0, 0);

  touchwin(MapWindow);
  wrefresh(MapWindow);
}

/* =============================================================================
 * FUNCTION: magic_effect_frames
 */
int magic_effect_frames(MagicEffectsType fx) { return magicfx_tile[fx].Frames; }

/* =============================================================================
 * FUNCTION: magic_effect
 */
void magic_effect(int x, int y, MagicEffectsType fx, int frame) {
  wattrset(MapWindow, COLOR_PAIR(magicfx_tile[fx].Color[frame]));
  mvwaddch(MapWindow, y, x, magicfx_tile[fx].Tile[frame]);
  wmove(MapWindow, 0, 0);

  touchwin(MapWindow);
  wrefresh(MapWindow);
}

/* =============================================================================
 * FUNCTION: nap
 */
void nap(int delay) {
#ifdef UNIX
  usleep(delay * 1000);
#else
  napms(delay);
#endif
}

/* =============================================================================
 * FUNCTION: GetUser
 */
void GetUser(char *username, int *uid) {
#ifdef UNIX

  *uid = getuid();

  strcpy(username, getenv("USER"));

#else
  FILE *fp;
  char TmpName[80];
  int TmpPid;
  int Found;
  int n;

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

#endif
}
