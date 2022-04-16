/* =============================================================================
 * PROGRAM:  ularn
 * FILENAME: dungeon.c
 *
 * DESCRIPTION:
 * Dungeon levels module.
 * This module provides functions to create, load, save etc dungeon levels.
 *
 * =============================================================================
 * EXPORTED VARIABLES
 *
 * item      : The item at each location on the current level
 * know      : What the player beleives is at each location
 * moved     : The monster moved status for each dungeon location
 * stealth   : The monster stealth status for each dungeon location
 * hitp      : The monster hit points for each dungeon location
 * iarg      : The item arg for each dungeon location
 * screen    : Screen data used in moving monsters
 * mitem     : The monster and items it has stolen for each dungeon location
 * beenhere  : Which dungeon levels have been visited
 * level     : The current dungeon level
 * levelname : The name of each dungeon level
 *
 * =============================================================================
 * EXPORTED FUNCTIONS
 *
 * init_cells     : Allocate dungeon storage
 * free_cells     : Free dungeon storage
 * cgood          : Check if a cell is empty (monster and/or item)
 * dropgold       : Drop gold around the player
 * fillmonst      : Attempt to put a monster into the dungeon
 * eat            : Eat a maze in a level filled with walls
 * savelevel      : Save the current dungeon level into storage
 * getlevel       : Get the current level from storage.
 * AnalyseWalls   : Calculate wall tiles based on adjacent walls.
 * newcavelevel   : Function to go to a different cave level, creating if reqd
 * verifyxy       : Verify x and y coordinates are on the map, adjusting if reqd
 * createitem     : Create an item
 * something      : Create a random item
 * newobject      : Return a randomly selected item
 * write_levels   : Write dungeon levels to the save file
 * read_levels    : Read dungeon levels from the save file
 *
 * =============================================================================
 */

#include <stdio.h>

#include "dungeon.h"
#include "header.h"
#include "itm.h"
#include "monster.h"
#include "player.h"
#include "potion.h"
#include "saveutils.h"
#include "scores.h"
#include "scroll.h"
#include "ularn_game.h"
#include "ularn_win.h"

/* =============================================================================
 * Exported variables
 */

char item[MAXX][MAXY];          /* objects in maze if any */
char know[MAXX][MAXY];          /* 1 or 0 if here before  */
char moved[MAXX][MAXY];         /* monster movement flags  */
char stealth[MAXX][MAXY];       /* See Stealth flags */
short hitp[MAXX][MAXY];         /* monster hp on level  */
short iarg[MAXX][MAXY];         /* arg for the item array */
short screen[MAXX][MAXY];       /* The screen as the player knows it */
struct_mitem mitem[MAXX][MAXY]; /* Items stolen by monstes array */

char beenhere[NLEVELS] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

int level = 0; /* cavelevel player is on = c[CAVELEVEL]*/

char *levelname[] = {" H", " 1", " 2", " 3", " 4", " 5", " 6",
                     " 7", " 8", " 9", "10", "11", "12", "13",
                     "14", "15", "V1", "V2", "V3", "V4", "V5"};

/* =============================================================================
 * Local variables
 */

/*
 * Data and macros for finding the number of +s for items.
 */
#define NUM_LEATHER_PTS 15
#define NUM_H_LEATHER_PTS 12
static char nlpts[NUM_LEATHER_PTS] = {0, 0, 0, 0, 0, 1, 1, 2,
                                      2, 3, 3, 4, 5, 6, 7};

#define NUM_CHAIN_PTS 10
static char nch[NUM_CHAIN_PTS] = {0, 0, 0, 1, 1, 1, 2, 2, 3, 4};

#define NUM_PLATE_PTS 10
#define NUM_H_PLATE_PTS 3
static char nplt[NUM_PLATE_PTS] = {0, 0, 0, 0, 1, 1, 2, 2, 3, 4};

#define NUM_DAGGER_PTS 13
static char ndgg[NUM_DAGGER_PTS] = {0, 0, 0, 1, 1, 1, 1, 2, 2, 3, 3, 4, 5};

#define NUM_SWORD_PTS 13
#define NUM_H_SWORD_PTS 6
static char nsw[] = {0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 2, 3};

/* return the + points on created leather armor */
#define newleather()                                                           \
  (nlpts[rund(c[HARDGAME] ? (NUM_H_LEATHER_PTS) : (NUM_LEATHER_PTS))])

/* return the + points on chain armor */
#define newchain() (nch[rund(NUM_CHAIN_PTS)])

/* return + points on plate armor */
#define newplate()                                                             \
  (nplt[rund(c[HARDGAME] ? (NUM_H_PLATE_PTS) : (NUM_PLATE_PTS))])

/* return + points on new daggers */
#define newdagger() (ndgg[rund(NUM_DAGGER_PTS)])

/* return + points on new swords */
#define newsword()                                                             \
  (nsw[rund(c[HARDGAME] ? (NUM_H_SWORD_PTS) : (NUM_SWORD_PTS))])

typedef char Char_Ary[MAXX][MAXY];
typedef short Short_Ary[MAXX][MAXY];
typedef long Long_Ary[MAXX][MAXY];
typedef struct_mitem Mitem_Ary[MAXX][MAXY];

/* this is the structure that holds the entire dungeon specifications */
typedef struct save_lev_str {
  Short_Ary hitp;
  Mitem_Ary mitem;
  Char_Ary item;
  Short_Ary iarg; /* must be long for goldpiles */
  Char_Ary know;
} Saved_Level;

static Saved_Level *saved_levels[NLEVELS] = {
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};

static unsigned int level_sums[NLEVELS];

/* =============================================================================
 * Local functions
 */

/* =============================================================================
 * FUNCTION: cannedlevel
 *
 * DESCRIPTION:
 * Function to read in a maze from a data file
 *
 * Only read in a maze 50% of time.
 *
 * Format of maze data file:
 *  Must contain 21 mazes
 *    For each maze:
 *        18 lines (1st 17 used)
 *        67 characters per line
 *
 *  Line seperating maps must be single newline character
 *
 *  Special characters in maze data file:
 *
 *    # wall                D door
 *    . random monster      ~ eye of larn
 *    ! cure dianthroritis  - random object
 *
 * PARAMETERS:
 *
 *   lev : The dungeon level being read.
 *
 * RETURN VALUE:
 *
 *   1 for success
 *  -1 for error/use random maze
 */
static int cannedlevel(int lev) {
  int i, j, k;
  int it, arg, marg;
  FILE *fp;
  char *row, buf[128];
  MonsterIdType Monst;

  if ((lev != DBOTTOM) && (lev != VBOTTOM))
    /*
     * The bottom levels are always read from the file.
     * Only read a maze from file around half the time for regular levels.
     */
    if (rnd(100) < 50)
      return -1;

  fp = fopen(larnlevels, "r");

  if (fp == (FILE *)NULL)
    return -1;

  /*
   * Umap format
   * - lines must be MAXX characters long
   * - must be MAXY characters per map
   * - each map must be seperated by 1 blank line
   *  (a single newline character)
   */

  /*
   * Decide which map to use and move to the location of the map in the
   * levels file.
   * A direct seek is a little non-portable as different operating systems
   * use different EOL (CR/LF vs CR or LF).
   * Also, be a bit more forgiving of white space after the map line.
   */
  i = rund(20);
  for (j = 0; j < i; j++) {
    /*
    ** Skip a level + the blank line
    */
    for (k = 0; k < (MAXY + 1); k++) {
      row = fgets(buf, 128, fp);
      if (row == (char *)NULL) {
        perror("fgets");
        fclose(fp);
        return -1;
      }
    }
  }

  /*
   * Read the level from the file
   */
  for (i = 0; i < MAXY; i++) {
    row = fgets(buf, 128, fp);
    if (row == (char *)NULL) {
      perror("fgets");
      fclose(fp);
      return -1;
    }

    for (j = 0; j < MAXX; j++) {
      it = ONOTHING;
      Monst = MONST_NONE;
      arg = 0;
      marg = 0;
      switch (*row++) {
      case '#':
        it = OWALL;
        break;
      case 'D':
        it = OCLOSEDDOOR;
        arg = rnd(30);
        break;
      case '~':
        if (lev != DBOTTOM)
          break;
        it = OLARNEYE;
        Monst = DEMONPRINCE;
        marg = monster[Monst].hitpoints;
        break;
      case '!':
        if (lev != VBOTTOM)
          break;
        it = OPOTION;
        arg = 21;
        Monst = LUCIFER;
        marg = monster[Monst].hitpoints;
        break;
      case '.':
        if (lev <= DBOTTOM - 5)
          break;
        Monst = makemonst(lev + 1);
        marg = monster[Monst].hitpoints;
        break;
      case '-':
        it = newobject(lev + 1, &arg);
        break;
      }
      item[j][i] = (char)it;
      iarg[j][i] = (short)arg;
      mitem[j][i].mon = (char)Monst;
      hitp[j][i] = (short)marg;
      know[j][i] = (char)((wizard) ? item[j][i] : OUNKNOWN);
    }
  }

  fclose(fp);
  return 1;
}

/* =============================================================================
 * FUNCTION: troom
 *
 * DESCRIPTION:
 * Function to create a treasure room of any size at a given location.
 * The room is filled with objects and monsters
 * The coordinate given is that of the upper left corner of the room.
 *
 * PARAMETERS:
 *
 *   lv     : The dungeon level for the treasure room
 *
 *   xsize  : The x size of the room
 *
 *   ysize  : The y size of the room
 *
 *   tx     : The Leftmost x position of the room
 *
 *   ty     : The Topmost y position og the room
 *
 *   glyph  : The door object to use.
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void troom(int lv, int xsize, int ysize, int tx, int ty, int glyph) {
  int i, j;
  int bupx, bupy;

  /* clear out space for room */
  for (j = (ty - 1); j <= (ty + ysize); j++)
    for (i = (tx - 1); i <= (tx + xsize); i++)
      item[i][j] = ONOTHING;

  /* now put in the walls */
  for (j = ty; j < (ty + ysize); j++) {
    for (i = tx; i < (tx + xsize); i++) {
      item[i][j] = OWALL;
      mitem[i][j].mon = MONST_NONE;
    }
  }

  /* now clear out interior */
  for (j = (ty + 1); j < (ty + ysize - 1); j++)
    for (i = (tx + 1); i < (tx + xsize - 1); i++)
      item[i][j] = ONOTHING;

  /* locate the door on the treasure room */
  switch (rnd(2)) {
  case 1:
    i = tx + rund(xsize);
    j = ty + (ysize - 1) * rund(2);
    item[i][j] = OCLOSEDDOOR;
    iarg[i][j] = (short)glyph; /* on horizontal walls */
    break;
  case 2:
    i = tx + (xsize - 1) * rund(2);
    j = ty + rund(ysize);
    item[i][j] = OCLOSEDDOOR;
    iarg[i][j] = (short)glyph; /* on vertical walls */
    break;
  }

  /*
   * must save and use playerx, playery because that's what
   * createmonster() uses
   */
  bupx = playerx;
  bupy = playery;

  playery = (char)(ty + (ysize >> 1));
  if (c[HARDGAME] < 3) {
    for (playerx = (char)(tx + 1); playerx <= (char)(tx + xsize - 2);
         playerx += (char)2) {
      j = rnd(6);
      for (i = 0; i <= j; i++) {
        something(playerx, playery, lv + 2);
        createmonster(makemonst(lv + 2));
      }
    }
  } else {
    for (playerx = (char)(tx + 1); playerx <= (char)(tx + xsize - 2);
         playerx += (char)2) {
      j = rnd(4);
      for (i = 0; i <= j; i++) {
        something(playerx, playery, lv + 2);
        createmonster(makemonst(lv + 4));
      }
    }
  }

  playerx = bupx;
  playery = bupy;
}

/* =============================================================================
 * FUNCTION: treasureroom
 *
 * DESCRIPTION:
 * Make a treasure room on a level
 *  - level 10's treasure room has the eye in it and demon lords
 *  - level V5 has potion of cure dianthroritis and demon prince
 *
 * PARAMETERS:
 *
 *   lv : The dungeon level containing the room
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void treasureroom(int lv) {
  int tx, ty, xsize, ysize;

  for (tx = 1 + rnd(10); tx < MAXX - 10; tx += 10) {
    if ((lv == DBOTTOM) || (lv == VBOTTOM) || (rnd(10) <= 2)) {
      /* 20% chance */
      xsize = rnd(6) + 3;
      ysize = rnd(3) + 3;
      ty = rnd(MAXY - 9) + 1; /* upper left corner of room */
      if ((lv == DBOTTOM) || (lv == VBOTTOM))
        troom(lv, xsize, ysize, (tx = tx + rnd(MAXX - 24)), ty, rnd(3) + 6);
      else
        troom(lv, xsize, ysize, tx, ty, rnd(9));
    }
  }
}

/* =============================================================================
 * FUNCTION: fillroom
 *
 * DESCRIPTION:
 * Function to put an object into an empty room.
 * Uses a random walk to find an empty room.
 *
 * PARAMETERS:
 *
 *   what : The item to create
 *
 *   arg  : The item's argument.
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void fillroom(int what, int arg) {
  int x, y;

  x = rnd(MAXX - 2);
  y = rnd(MAXY - 2);

  while (item[x][y] != ONOTHING) {
    x += rnd(3) - 2;
    y += rnd(3) - 2;

    if (x > MAXX - 2)
      x = 1;
    if (x < 1)
      x = MAXX - 2;
    if (y > MAXY - 2)
      y = 1;
    if (y < 1)
      y = MAXY - 2;
  }
  item[x][y] = (char)what;
  iarg[x][y] = (short)arg;
}

/* =============================================================================
 * FUNCTION: fillmroom
 *
 * DESCRIPTION:
 * Function to fill in a number of objects of the same kind
 *
 * PARAMETERS:
 *
 *   n    : The number of items to fill
 *
 *   what : The type of item
 *
 *   arg  : The item argument
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void fillmroom(int n, int what, int arg) {
  int i;

  for (i = 0; i < n; i++)
    fillroom(what, arg);
}

/* =============================================================================
 * FUNCTION: froom
 *
 * DESCRIPTION:
 * Fill a room with an item with a certain probability.
 *
 * PARAMETERS:
 *
 *   n   : The chance in 151 of the item being created
 *
 *   itm : The item to be filled
 *
 *   arg : The item argument.
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void froom(int n, int itm, int arg) {
  if (rnd(151) < n)
    fillroom(itm, arg);
}

/*
 * Data for the creation of unique items in the game.
 */

#define UNIQUE_1_COUNT 10
#define UNIQUE_COUNT 16

/*
 * A list of unique items in the game
 * The items in the set from which only one can be created per dungeon level
 * must be specified first.
 */
static int UniqueItem[UNIQUE_COUNT] = {
    OBRASSLAMP,    OWWAND,      OORBOFDRAGON,     OSPIRITSCARAB,
    OCUBEofUNDEAD, ONOTHEFT,    OSPHTALISMAN,     OHANDofFEAR,
    OORB,          OELVENCHAIN, OSWORDofSLASHING, OHAMMER,
    OSLAYER,       OVORPAL,     OPSTAFF,          OLIFEPRESERVER};

/* The character flags associated with the creation of each unique item */
static AttributeType UniqueFlag[UNIQUE_COUNT] = {
    LAMP,     WAND,   DRAGSLAY, NEGATE,        CUBEUNDEAD, DEVICE,
    TALISMAN, HAND,   ORB,      ELVEN,         SLASH,      BESSMANN,
    SLAY,     VORPAL, STAFF,    LIFE_PRESERVER};

/* The minimum dungeon level for this item to occur */
static int UniqueMinLevel[UNIQUE_COUNT] = {0, 0, 0, 0, 0,  0, 0, 0,
                                           0, 0, 0, 0, 10, 0, 8, 5};

/* The die to roll for determining if this item is created */
static int UniqueRoll[UNIQUE_COUNT] = {120, 120, 120, 120, 120, 120, 120, 120,
                                       120, 120, 120, 120, 100, 120, 100, 100};

/* The max roll for item creation */
static int UniqueProb[UNIQUE_COUNT] = {8, 8, 8, 8, 8,  8, 8,  8,
                                       8, 8, 8, 8, 15, 8, 15, 15};

/* The increase in probability for each level in the dungeon above the minimum
 * required for the item
 */
static int UniqueProbLevelMod[UNIQUE_COUNT] = {0, 0, 0, 0, 0, 0, 0, 0,
                                               0, 0, 0, 0, 1, 0, 1, 0};

/* =============================================================================
 * FUNCTION: makeobject
 *
 * DESCRIPTION:
 * Create the objects in a dungeon level.
 *
 * PARAMETERS:
 *
 *   j : The dungeon level.
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void makeobject(int j) {
  int i;
  int MadeUnique;
  int Flag;
  int Item;
  int Idx;
  int Prob;

  if (j == 0) {
    /* The town level */
    fillroom(OENTRANCE, 0);  /* entrance to dungeon*/
    fillroom(ODNDSTORE, 0);  /* the DND STORE  */
    fillroom(OSCHOOL, 0);    /* college of Larn  */
    fillroom(OBANK, 0);      /* 1st national bank of larn*/
    fillroom(OVOLDOWN, 0);   /* volcano shaft to temple*/
    fillroom(OHOME, 0);      /* the players home & family*/
    fillroom(OTRADEPOST, 0); /* the trading post */
    fillroom(OLRS, 0);       /* the larn revenue service */

    return;
  }

  if (j == DBOTTOM + 1) { /* V1 */
    fillroom(OVOLUP, 0);  /* volcano shaft up from the temple */
  }

  /* make the fixed object in the maze STAIRS and
     random object ELEVATORS */

  /* stairs down only on V1 and V2 */
  if ((j > 0) && (j != DBOTTOM) && (j < VBOTTOM - 2))
    fillroom(OSTAIRSDOWN, 0);

  if ((j > 1) && (j != DBOTTOM))
    fillroom(OSTAIRSUP, 0);

  /* > 3, not on V1 or V5 or 15 */
  if ((j > 3) && (j != DBOTTOM + 1) && (j != VBOTTOM) && (j != DBOTTOM)) {
    if (c[ELVUP] == 0) {
      if (rnd(100) > 85) {
        fillroom(OELEVATORUP, 0);
        c[ELVUP]++;
      }
    }
  }

  /* < lev 10, or 15 or V5 */
  if ((j > 0) && (j <= DBOTTOM - 5 || j == DBOTTOM || j == VBOTTOM)) {
    if (c[ELVDOWN] == 0) {
      if (rnd(100) > 85) {
        fillroom(OELEVATORDOWN, 0);
        c[ELVDOWN]++;
      }
    }
  }

  /*  make the random objects in the maze */
  fillmroom(rund(3), OBOOK, j);
  fillmroom(rund(3), OCOOKIE, 0);
  fillmroom(rund(3), OALTAR, 0);
  fillmroom(rund(3), OSTATUE, 0);
  fillmroom(rund(3), OFOUNTAIN, 0);
  fillmroom(rund(2), OTHRONE, 0);
  fillmroom(rund(2), OMIRROR, 0);

  /* be sure to have pits on V3, V4, and V5 */
  /* because there are no stairs on those levels */
  if (j >= VBOTTOM - 2)
    fillroom(OPIT, 0);
  fillmroom(rund(3), OPIT, 0);

  /* be sure to have trapdoors on V3, V4, and V5 */
  if (j >= VBOTTOM - 2)
    fillroom(OIVTRAPDOOR, 0);
  fillmroom(rund(2), OIVTRAPDOOR, 0);
  fillmroom(rund(2), OTRAPARROWIV, 0);
  fillmroom(rnd(3) - 2, OIVTELETRAP, 0);
  fillmroom(rnd(3) - 2, OIVDARTRAP, 0);

  if (j == 1)
    fillmroom(1, OCHEST, j);
  else
    fillmroom(rund(2), OCHEST, j);

  if (j <= DBOTTOM) {
    fillmroom(rund(2), ODIAMOND, rnd(10 * j + 1) + 10);
    fillmroom(rund(2), ORUBY, rnd(6 * j + 1) + 6);
    fillmroom(rund(2), OEMERALD, rnd(4 * j + 1) + 4);
    fillmroom(rund(2), OSAPPHIRE, rnd(3 * j + 1) + 2);
  }

  Prob = rnd(4) + 3;
  for (i = 0; i < Prob; i++) {
    fillroom(OPOTION, newpotion()); /*  make a POTION */
  }

  Prob = rnd(5) + 3;
  for (i = 0; i < Prob; i++) {
    fillroom(OSCROLL, newscroll()); /*  make a SCROLL */
  }

  Prob = rnd(12) + 11;
  for (i = 0; i < Prob; i++) {
    fillroom(OGOLDPILE, 12 * rnd(j + 1) + (j << 3) + 10); /* make GOLD  */
  }

  if (j == 8) {
    fillroom(OBANK2, 0); /*  branch office of the bank */
  }

  froom(2, ORING, 0);            /* a ring mail  */
  froom(1, OSTUDLEATHER, 0);     /* a studded leather  */
  froom(3, OSPLINT, 0);          /* a splint mail*/
  froom(5, OSHIELD, rund(3));    /* a shield */
  froom(2, OBATTLEAXE, rund(3)); /* a battle axe */
  froom(5, OLONGSWORD, rund(3)); /* a long sword */
  froom(5, OFLAIL, rund(3));     /* a flail  */
  froom(7, OSPEAR, rnd(5));      /* a spear  */
  froom(4, OREGENRING, rund(3)); /* ring of regeneration */
  froom(1, OPROTRING, rund(3));  /* ring of protection */
  froom(2, OSTRRING, rund(5));   /* ring of strength  */
  froom(2, ORINGOFEXTRA, 0);     /* ring of extra regen  */

  /*
  ** Unique items.
  ** Only one of the following items can be created on a level.
  */

  /*
  ** Check if this level contains an item from the 1 of per level set
  */
  MadeUnique = 0;
  for (Idx = 0; Idx < UNIQUE_1_COUNT && !MadeUnique; Idx++) {
    Flag = UniqueFlag[Idx];
    Item = UniqueItem[Idx];

    if (c[Flag] == 0) {
      if (j >= UniqueMinLevel[Idx]) {
        Prob = UniqueProb[Idx] +
               UniqueProbLevelMod[Idx] * (j - UniqueMinLevel[Idx]);

        if (rnd(UniqueRoll[Idx]) < Prob) {
          fillroom(Item, 0);
          c[Flag]++;
          MadeUnique = 1;
        }
      }
    }
  }

  /*
  ** Check for creation of other unique items
  */
  for (Idx = UNIQUE_1_COUNT; Idx < UNIQUE_COUNT; Idx++) {
    Flag = UniqueFlag[Idx];
    Item = UniqueItem[Idx];

    if (c[Flag] == 0) {
      if (j >= UniqueMinLevel[Idx]) {
        Prob = UniqueProb[Idx] +
               UniqueProbLevelMod[Idx] * (j - UniqueMinLevel[Idx]);

        if (rnd(UniqueRoll[Idx]) < Prob) {
          fillroom(Item, 0);
          c[Flag]++;
        }
      }
    }
  }

  /*
  ** we don't get these if the difficulty level
  ** is >= 3
  */
  if (c[HARDGAME] < 3 || (rnd(4) == 3)) {
    if (j > 3) {
      /* only on levels 3 or below */
      froom(3, OSWORD, rund(6));      /* sunsword */
      froom(5, O2SWORD, rnd(6));      /* a two handed sword */
      froom(3, OBELT, rund(7));       /* belt of striking */
      froom(3, OENERGYRING, rund(6)); /* energy ring  */
      froom(4, OPLATE, rund(8));      /* platemail */
    }
  }
}

/* =============================================================================
 * FUNCTION: sethp
 *
 * DESCRIPTION:
 * Creates an entire set of monsters for a level.
 * Must be done when entering a new level.
 *
 * PARAMETERS:
 *
 *   flg : true  if this is the first time on this level
 *         false if this is a revisit to the level,.
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void sethp(int flg) {
  int i, j;

  if (flg) {
    /* All initial monsters are unseen and asleep */
    for (i = 0; i < MAXY; i++)
      for (j = 0; j < MAXX; j++)
        stealth[j][i] = 0;
  }

  if (level == 0)
    /* Don't spawn extra onsters on the home level */
    return;

  if (flg)
    /* Get the initial numb er of monsters for this level */
    j = rnd(12) + 2 + (level >> 1);
  else
    /* Calculate the number of new monsters to create on revisit */
    j = (level >> 1) + 1;

  for (i = 0; i < j; i++)
    fillmonst(makemonst(level));

  if (flg) {
    /*
     * Only spawn demonlords on first visit to a level otherwise things can
     * get out of hand rather rapidly.
     */

    /*
     * level 11 gets 1 demon lord
     * level 12 gets 2 demon lords
     * level 13 gets 3 demon lords
     * level 14 gets 4 demon lords
     * level 15 gets 5 demon lords
     */
    if ((level >= (DBOTTOM - 4)) && (level <= DBOTTOM)) {
      i = level - 10;
      for (j = 1; j <= i; j++)
        while (fillmonst(DEMONLORD + rund(7)) == -1)
          ;
    }

    /*
     * level V1 gets 1 demon prince
     * level V2 gets 2 demon princes
     * level V3 gets 3 demon princes
     * level V4 gets 4 demon princes
     * level V5 gets 5 demon princes
     */
    if (level > DBOTTOM) {
      i = level - DBOTTOM;
      for (j = 1; j <= i; j++)
        /* Keep trying until the demon prince has been placed */
        while (fillmonst(DEMONPRINCE) == -1)
          ;
    }
  }
}

/* =============================================================================
 * FUNCTION: checkgen
 *
 * DESCRIPTION:
 * Function to destroy all genocided monsters on the present level.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void checkgen(void) {
  int x, y;

  for (y = 0; y < MAXY; y++) {
    for (x = 0; x < MAXX; x++) {
      if ((monster[(int)mitem[x][y].mon].flags & FL_GENOCIDED) != 0) {
        mitem[x][y].mon = 0; /* no more monster */
      }
    }
  }
}

/* =============================================================================
 * FUNCTION: makemaze
 *
 * DESCRIPTION:
 * Function to make the caverns for a given level.
 * Only walls are made (except for canned levels).
 *
 * PARAMETERS:
 *
 *   lev : The dungeon level to be made.
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void makemaze(int lev) {
  int mx, mxl, mxh;
  int my, myl, myh;
  int tmp2;
  int i, j;
  int tmp;
  MonsterIdType Monst;

  if (lev > 0) {
    /* read maze from data file */
    if (cannedlevel(lev) == 1)
      return;
  }

  if (lev == 0)
    tmp = ONOTHING;
  else
    tmp = OWALL;

  /* fill up maze */
  for (i = 0; i < MAXY; i++)
    for (j = 0; j < MAXX; j++)
      item[j][i] = (char)tmp;

  /* don't need to do anymore for level 0 */
  if (lev == 0)
    return;

  eat(1, 1);

  /*  now for open spaces -- not on level 15 or V5 */
  if ((lev != DBOTTOM) && (lev != VBOTTOM)) {
    tmp2 = rnd(3) + 3;
    for (tmp = 0; tmp < tmp2; tmp++) {
      my = rnd(11) + 2;
      myl = my - rnd(2);
      myh = my + rnd(2);
      if (lev <= DBOTTOM) {
        /* in dungeon */
        mx = rnd(44) + 5;
        mxl = mx - rnd(4);
        mxh = mx + rnd(12) + 3;
        Monst = 0;
      } else {
        /* in volcano */
        mx = rnd(60) + 3;
        mxl = mx - rnd(2);
        mxh = mx + rnd(2);
        Monst = makemonst(lev);
      }

      for (i = mxl; i < mxh; i++) {
        for (j = myl; j < myh; j++) {
          item[i][j] = ONOTHING;
          if (Monst != MONST_NONE) {
            mitem[i][j].mon = (char)Monst;
            hitp[i][j] = monster[Monst].hitpoints;
          }
        }
      }
    }
  }

  if (lev != DBOTTOM && lev != VBOTTOM) {
    my = rnd(MAXY - 2);
    for (i = 1; i < MAXX - 1; i++)
      item[i][my] = ONOTHING;
  }

  /* no treasure rooms above level 5 */
  if (lev > 4)
    treasureroom(lev);
}

/* =============================================================================
 * Exported functions
 */

/* =============================================================================
 * FUNCTION: init_cells
 */
void init_cells(void) {
  int i;

  for (i = 0; i < NLEVELS; i++) {
    if ((saved_levels[i] = (Saved_Level *)malloc(sizeof(Saved_Level))) ==
        (Saved_Level *)NULL)
      died(DIED_MALLOC_FAILURE, 0);
  }
}

/* =============================================================================
 * FUNCTION: free_cells
 */
void free_cells(void) {
  int i;

  for (i = 0; i < NLEVELS; i++)
    if (saved_levels[i] != (Saved_Level *)NULL)
      free(saved_levels[i]);
}

/* =============================================================================
 * FUNCTION: cgood
 */
int cgood(int x, int y, int chkitm, int chkmonst) {
  int good = 0;

  if ((y >= 0) && (y <= MAXY - 1) && (x >= 0) &&
      (x <= MAXX - 1)) { /* within bounds? */
    if ((level == 1) && (x == 33) && (y == MAXY - 1))
      /* exit to level 1 is never good */
      return 0;

    if ((item[x][y] == OWALL) || (item[x][y] == OCLOSEDDOOR))
      /* can't make on walls or closed doors */
      return 0;

    /* Location is OK so far */
    good = 1;

    if (chkitm && (item[x][y] != ONOTHING))
      /* checking items and it is not free of items */
      good = 0;

    if (chkmonst && (mitem[x][y].mon != MONST_NONE))
      /* checking for monsters and not free of monsters */
      good = 0;
  }

  return good;
}

/* =============================================================================
 * FUNCTION: dropgold
 */
void dropgold(int amount) {
  if (amount > 250)
    createitem(playerx, playery, OMAXGOLD, (long)amount);
  else
    createitem(playerx, playery, OGOLDPILE, (long)amount);
}

/* =============================================================================
 * FUNCTION: fillmonst
 */
int fillmonst(int what) {
  int x, y, trys;

  /* max # of creation attempts */
  for (trys = 10; trys > 0; --trys) {
    x = rnd(MAXX - 2);
    y = rnd(MAXY - 2);

    if ((item[x][y] == ONOTHING) && (mitem[x][y].mon == MONST_NONE) &&
        ((playerx != x) || (playery != y))) {
      mitem[x][y].mon = (char)what;
      stealth[x][y] = 0;
      hitp[x][y] = monster[what].hitpoints;
      return 0;
    }
  }

  return -1; /* creation failure */
}

/* =============================================================================
 * FUNCTION: eat
 */
void eat(int xx, int yy) {
  int dir;
  int attempt;

  dir = rnd(4);
  attempt = 2;
  while (attempt) {
    switch (dir) {
    case 1:
      if (xx <= 2)
        break; /*  west  */
      if ((item[xx - 1][yy] != OWALL) || (item[xx - 2][yy] != OWALL))
        break;
      item[xx - 1][yy] = ONOTHING;
      item[xx - 2][yy] = ONOTHING;
      eat(xx - 2, yy);
      break;
    case 2:
      if (xx >= MAXX - 3)
        break; /*  east  */
      if ((item[xx + 1][yy] != OWALL) || (item[xx + 2][yy] != OWALL))
        break;
      item[xx + 1][yy] = ONOTHING;
      item[xx + 2][yy] = ONOTHING;
      eat(xx + 2, yy);
      break;
    case 3:
      if (yy <= 2)
        break; /*  south */
      if ((item[xx][yy - 1] != OWALL) || (item[xx][yy - 2] != OWALL))
        break;
      item[xx][yy - 1] = ONOTHING;
      item[xx][yy - 2] = ONOTHING;
      eat(xx, yy - 2);
      break;
    case 4:
      if (yy >= MAXY - 3)
        break; /*north */
      if ((item[xx][yy + 1] != OWALL) || (item[xx][yy + 2] != OWALL))
        break;
      item[xx][yy + 1] = ONOTHING;
      item[xx][yy + 2] = ONOTHING;
      eat(xx, yy + 2);
      break;
    }

    if (++dir > 4) {
      dir = 1;
      --attempt;
    }
  }
}

/* =============================================================================
 * FUNCTION: savelevel
 */
void savelevel(void) {
  Saved_Level *storage = saved_levels[level];

  memcpy((char *)storage->hitp, (char *)hitp, sizeof(Short_Ary));
  memcpy((char *)storage->mitem, (char *)mitem, sizeof(Mitem_Ary));
  memcpy((char *)storage->item, (char *)item, sizeof(Char_Ary));
  memcpy((char *)storage->iarg, (char *)iarg, sizeof(Short_Ary));
  memcpy((char *)storage->know, (char *)know, sizeof(Char_Ary));
  level_sums[level] = sum((unsigned char *)storage, sizeof(Saved_Level));
}

/* =============================================================================
 * FUNCTION: getlevel
 */
void getlevel(void) {
  unsigned int i;

  Saved_Level *storage = saved_levels[level];

  memcpy((char *)hitp, (char *)storage->hitp, sizeof(Short_Ary));
  memcpy((char *)mitem, (char *)storage->mitem, sizeof(Mitem_Ary));
  memcpy((char *)item, (char *)storage->item, sizeof(Char_Ary));
  memcpy((char *)iarg, (char *)storage->iarg, sizeof(Short_Ary));
  memcpy((char *)know, (char *)storage->know, sizeof(Char_Ary));

  if (level_sums[level] > 0) {
    if ((i = sum((unsigned char *)storage, sizeof(Saved_Level))) !=
        level_sums[level]) {
      Printf("\nOH NO!!!! INTERNAL MEMORY CORRUPTION!!!!\n");
      Printf("(sum %u of level %d does not match saved sum %u)\n", i, level,
             level_sums[level]);
      UlarnBeep();
      nap(5000);
      died(DIED_INTERNAL_COMPLICATIONS, 0);
    }
  } else
    level_sums[level] = sum((unsigned char *)storage, sizeof(Saved_Level));
}

/* =============================================================================
 * FUNCTION: AnalyseWalls
 */
void AnalyseWalls(int x1, int y1, int x2, int y2) {
  int WallArg;
  int x, y;
  int sx, sy;
  int ex, ey;

  /* Limit area to the valid map area */
  sx = (x1 < 0) ? 0 : (x1);
  sy = (y1 < 0) ? 0 : (y1);
  ex = (x2 >= MAXX) ? (MAXX - 1) : (x2);
  ey = (y2 >= MAXY) ? (MAXY - 1) : (y2);

  for (x = sx; x <= ex; x++) {
    for (y = sy; y <= ey; y++) {
      if (item[x][y] == OWALL) {
        /* There is a wall here, so analyse connectivity with other walls */

        WallArg = 0;

        if (x > 0) {
          if ((item[x - 1][y] == OWALL) || (item[x - 1][y] == OOPENDOOR) ||
              (item[x - 1][y] == OCLOSEDDOOR))
            WallArg += 1;
        }

        if (y < (MAXY - 1)) {
          if ((item[x][y + 1] == OWALL) || (item[x][y + 1] == OOPENDOOR) ||
              (item[x][y + 1] == OCLOSEDDOOR))
            WallArg += 2;
        }

        if (x < (MAXX - 1)) {
          if ((item[x + 1][y] == OWALL) || (item[x + 1][y] == OOPENDOOR) ||
              (item[x + 1][y] == OCLOSEDDOOR))
            WallArg += 4;
        }

        if (y > 0) {
          if ((item[x][y - 1] == OWALL) || (item[x][y - 1] == OOPENDOOR) ||
              (item[x][y - 1] == OCLOSEDDOOR))
            WallArg += 8;
        }

        iarg[x][y] = (short)WallArg;

      } /* if a wall is here */
    }
  }
}

/* =============================================================================
 * FUNCTION: newcavelevel
 */
void newcavelevel(int x) {
  int i, j;

  if (beenhere[level]) {
    savelevel(); /* put the level back into storage  */
  }

  if (level == 0)
    /* if teleported and found the home level then know level we are on */
    c[TELEFLAG] = 0;

  /*
   * Forget which monster was last hit by the player as it is no longer
   * on this level.
   */
  last_monst_hx = -1;
  last_monst_hy = -1;

  level = (char)x;
  c[CAVELEVEL] = level;
  if (beenhere[x]) {
    /* get the new level and put in working storage */
    getlevel();
    /* spawn new monsters */
    sethp(0);
    /* remove any genocided monsters */
    checkgen();

    /* Position the player on the map */
    positionplayer();
  } else {
    /* never been here before, so don't know anything, and no monsters */
    for (i = 0; i < MAXY; i++) {
      for (j = 0; j < MAXX; j++) {
        know[j][i] = OUNKNOWN;
        mitem[j][i].mon = MONST_NONE;
      }
    }

    makemaze(x);

    /* if this is level 1 */
    if (x == 1)
      /* exit from dungeon */
      item[33][MAXY - 1] = ONOTHING;

    AnalyseWalls(0, 0, MAXX - 1, MAXY - 1);
    makeobject(x);

    beenhere[x] = 1;
    /* create monsters for this level */
    sethp(1);

    if (wizard || x == 0) {
      for (j = 0; j < MAXY; j++) {
        for (i = 0; i < MAXX; i++) {
          know[i][j] = item[i][j];
          stealth[i][j] |= STEALTH_SEEN;
        }
      }
    }

    /* wipe out any genocided monsters */
    checkgen();

    /* Position the player on the map */
    positionplayer();
  }
}

/* =============================================================================
 * FUNCTION: verifyxy
 */
int verifyxy(int *x, int *y) {
  int flag = 0;

  if (*x < 0) {
    *x = 0;
    flag++;
  }
  if (*y < 0) {
    *y = 0;
    flag++;
  }
  if (*x >= MAXX) {
    *x = MAXX - 1;
    flag++;
  }
  if (*y >= MAXY) {
    *y = MAXY - 1;
    flag++;
  }
  return flag;
}

/* =============================================================================
 * FUNCTION: checkxy
 */
int checkxy(int x, int y) {
  return (x >= 0) && (x < MAXX) && (y >= 0) && (y < MAXY);
}

/* =============================================================================
 * FUNCTION: createitem
 */
void createitem(int x, int y, int it, int arg) {
  int i;
  int dir, dir_try;
  int ox, oy;

  if (it >= OCOUNT)
    return; /* no such object */

  /* Select the initial direction to try at random */
  dir = rnd(8);

  for (dir_try = 0; dir_try < 8; dir_try++) {
    /* choose direction, try all */
    if (dir > 8)
      dir = 1; /* wraparound the diroff arrays */

    ox = x + diroffx[dir];
    oy = y + diroffy[dir];

    if ((it != OKGOLD) && (it != OMAXGOLD) && (it != ODGOLD) &&
        (it != OGOLDPILE)) {
      if (cgood(ox, oy, 1, 0)) {
        /* if we can create an item here */
        item[ox][oy] = (char)it;
        iarg[ox][oy] = (short)arg;
        return;
      }
    } else {
      /* arg contains the number of GP to drop */
      switch (item[ox][oy]) {
      case OGOLDPILE:
        if ((iarg[ox][oy] + arg) < 32767) {
          iarg[ox][oy] += (short)arg;
          return;
        }
        /* fallthrough */
      case ODGOLD:
        if ((10L * iarg[ox][oy] + arg) < 327670L) {
          i = iarg[ox][oy];
          iarg[ox][oy] = (short)((10L * i + arg) / 10);
          item[ox][oy] = ODGOLD;
          return;
        }
        /* fallthrough */
      case OMAXGOLD:
        if ((100L * iarg[ox][oy] + arg) < 3276700L) {
          i = (int)((100L * iarg[ox][oy]) + arg);
          iarg[ox][oy] = (short)(i / 100);
          item[ox][oy] = OMAXGOLD;
          return;
        }
        /* fallthrough */
      case OKGOLD:
        if ((1000L * iarg[ox][oy] + arg) <= 32767000L) {
          i = iarg[ox][oy];
          iarg[ox][oy] = (short)((1000L * i + arg) / 1000);
          item[ox][oy] = OKGOLD;
          return;
        } else
          iarg[ox][oy] = 32767;
        return;
      default:
        if (cgood(ox, oy, 1, 0)) {
          item[ox][oy] = (char)it;
          if (it == OMAXGOLD)
            iarg[ox][oy] = (short)(arg / 100);
          else
            iarg[ox][oy] = (short)arg;
          return;
        }
        break;
      } /* end switch */
    }   /* end else */

    /* try the next direction */
    dir++;
  } /* end for */
}

/* =============================================================================
 * FUNCTION: something
 */
void something(int x, int y, int lev) {
  int item_id, item_arg;

  /* correct level? */
  if (lev < 0 || lev > VBOTTOM)
    return;

  /* possibly more than one item */
  do {

    item_id = newobject(lev, &item_arg);
    createitem(x, y, item_id, (long)item_arg);

  } while (rnd(101) < 8);
}

/* =============================================================================
 * FUNCTION: newobject
 */

#define LEV_0to3_LAST 32
#define LEV_4to6_LAST 35
#define LEV_6plus_LAST 37
static char nobjtab[] = {
    0,         OSCROLL,      OSCROLL,  OSCROLL,   OSCROLL,   OPOTION,
    OPOTION,   OPOTION,      OPOTION,  OGOLDPILE, OGOLDPILE, OGOLDPILE,
    OGOLDPILE, OBOOK,        OBOOK,    OBOOK,     OBOOK,     ODAGGER,
    ODAGGER,   ODAGGER,      OLEATHER, OLEATHER,  OLEATHER,  OREGENRING,
    OPROTRING, OENERGYRING,  ODEXRING, OSTRRING,  OSPEAR,    OBELT,
    ORING,     OSTUDLEATHER, OSHIELD,  OFLAIL,    OCHAIN,    O2SWORD,
    OPLATE,    OLONGSWORD}; /* 38 */

int newobject(int lev, int *i) {
  int LastObj;
  int ObjType;

  /* correct level? */
  if (level < 0 || level > VBOTTOM)
    return 0;

  /* Decide what types of objects can be created based on the current level */

  if (lev <= 3)
    LastObj = LEV_0to3_LAST;
  else if (lev <= 6)
    LastObj = LEV_4to6_LAST;
  else
    LastObj = LEV_6plus_LAST;

  /* select the object type */
  ObjType = nobjtab[rnd(LastObj)];

  switch (ObjType) {
  case OSCROLL:
    *i = newscroll();
    break;
  case OPOTION:
    *i = newpotion();
    break;
  case OGOLDPILE:
    *i = rnd((lev + 1) * 10) + lev * 10 + 10;
    break;
  case OBOOK:
    *i = lev;
    break;
  case ODAGGER:
    *i = newdagger();
    break;
  case OLEATHER:
    *i = newleather();
    break;
  case OREGENRING:
  case OSHIELD:
  case O2SWORD:
    *i = rund(lev / 3 + 1);
    break;
  case OPROTRING:
  case ODEXRING:
    *i = rnd(lev / 4 + 1);
    break;
  case OENERGYRING:
    *i = rund(lev / 4 + 1);
    break;
  case OSTRRING:
    *i = rnd(lev / 2 + 1);
    break;
  case OSPEAR:
    *i = rund(lev / 3 + 1);
    break;
  case OBELT:
  case OSTUDLEATHER:
    *i = rund(lev / 2 + 1);
    break;
  case ORING:
  case OFLAIL:
    *i = rund(lev / 2 + 1);
    break;
  case OCHAIN:
    *i = newchain();
    break;
  case OPLATE:
    *i = newplate();
    break;
  case OLONGSWORD:
    *i = newsword();
    break;
  default:
    break;
  }
  return ObjType;
}

/* =============================================================================
 * FUNCTION: write_levels
 */
int write_levels(FILE *fp) {
  int i;
  Saved_Level *storage;

  /*
   * Put the current level into storage
   */
  savelevel();

  /*
   * save which level the player is currently on
   */
  bwrite(fp, (char *)&level, sizeof(int));

  /*
   * Save which levels have been visited by the player
   */
  bwrite(fp, (char *)beenhere, sizeof(char) * NLEVELS);

  /*
   * Save each of the visited levels
   */
  for (i = 0; i < NLEVELS; i++) {
    if (beenhere[i]) {
      storage = saved_levels[i];
      bwrite(fp, (char *)storage, sizeof(Saved_Level));
    }
  }

  return 0;
}

/* =============================================================================
 * FUNCTION: read_levels
 */
int read_levels(FILE *fp) {
  int i;
  Saved_Level *storage;

  /*
   * Read which level the player is currently on
   */
  bread(fp, (char *)&level, sizeof(int));

  /*
   * Read which levels have been visited by the player
   */
  bread(fp, (char *)beenhere, sizeof(char) * NLEVELS);

  /*
   * Read each of the visited levels
   */
  for (i = 0; i < NLEVELS; i++) {
    if (beenhere[i]) {
      storage = saved_levels[i];
      bread(fp, (char *)storage, sizeof(Saved_Level));
    }
  }

  return 0;
}
