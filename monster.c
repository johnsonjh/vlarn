/* =============================================================================
 * PROGRAM:  ularn
 * FILENAME: monster.c
 *
 * DESCRIPTION:
 * This module handles most aspects of monsters in the game.
 * It provides the monster definitions.
 * Handles monster creation.
 * All monster movement, including attacking the player (including special
 * attacks) is handled here.
 * Also handles loading and saving monster state data.
 *
 * =============================================================================
 * EXPORTED VARIABLES
 *
 * monstnamelist : The character code for displaying each monster
 * monsttilelist : The gfx tile for each monster
 * monster       : The monster data
 * lastmonst     : The name of the last monster being processed.
 * last_monst_id : The id of the last monster being processed.
 * last_monst_hx : The x location of the last monster hit by the player
 * last_monst_hy : The y location of the last monster hit by the player
 * rmst          : The random monster creation countdown.
 *
 * =============================================================================
 * EXPORTED FUNCTIONS
 *
 * createmonster
 * mon_has_item  : Checks if a monster has a specific item
 * fullhit       : Do full damage to a monster
 * ifblind       : Display the monster hir, accounting for blindness
 * hitmonster    : Function to hit a monster
 * hitm          : Function to just hit a monster (no checks for AC)
 * hitplayer     : Function for a monster to hit the player
 * makemonst     : Make a monster number appropriate to a dungeon level
 * randmonst     : Create a random monster on the current cave level
 * teleportmonst : Teleport a monster
 * movemonst     : Move monsters.
 * parse2        : Function to call when player is not to move, but monsters are
 * write_monster_data : Function to write the monster data to the save file
 * read_monster_data  : Function to read the monster data from the save file
 *
 * =============================================================================
 */

#include "monster.h"
#include "dungeon.h"
#include "header.h"
#include "itm.h"
#include "player.h"
#include "saveutils.h"
#include "show.h"
#include "sphere.h"
#include "ularn_game.h"
#include "ularn_win.h"

/* =============================================================================
 * Exported variables
 */

char monstnamelist[MONST_COUNT] = {
    ' ', 'l', 'G', 'H', 'J', 'K', 'O', 'S', 'c', 'j', 't', 'A', 'E', 'L',
    'N', 'Q', 'R', 'Z', 'a', 'b', 'h', 'i', 'C', 'T', 'Y', 'd', 'e', 'g',
    'm', 'v', 'z', 'F', 'W', 'f', 'l', 'o', 'r', 'X', 'V', ' ', 'p', 'q',
    's', 'y', 'U', 'k', 'M', 'w', 'D', 'D', 'P', 'x', 'n', 'D', 'D', 'u',
    'D', '1', '2', '3', '4', '5', '6', '7', '9', '0'};

/*
 * Tile numbers to use for monsters
 */
int monsttilelist[MONST_COUNT] = {
    0,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15,
    16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32,
    33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49,
    50, 51, 52, 53, 54, 55, 56, 56, 57, 58, 59, 60, 61, 62, 63};

/*
 *  for the monster data
 *
 *  array to do rnd() to create monsters <= a given level
 */
char monstlevel[] = {5, 11, 17, 22, 27, 33, 39, 42, 46, 50, 53, 56};

struct monst monster[MONST_COUNT] = {

   /* NAME                   LV   AC  DAM  ATT  INT   GOLD    HP     EXP  FLAGS
    *-----------------------------------------------------------------------------------------------------------------------------*/
    {"unseen attacker",      0,    0,   0,  0,   1,    0,     0,       0, 0},
    {"lemming",              1,    0,   0,  0,   1,    0,     0,       1, FL_HEAD},
    {"gnome",                1,   10,   1,  0,   8,   30,     2,       2, FL_HEAD | FL_INFRAVIS},
    {"hobgoblin",            1,   13,   2,  0,   5,   25,     3,       2, FL_HEAD | FL_SLOW},
    {"jackal",               1,    7,   1,  0,   4,    0,     1,       1, FL_HEAD},
    {"kobold",               1,   15,   1,  0,   7,   10,     1,       1, FL_HEAD | FL_INFRAVIS},
    {"orc",                  2,   15,   3,  0,   9,   40,     5,       2, FL_HEAD},
    {"snake",                2,   10,   1,  0,   3,    0,     3,       1, FL_HEAD},
    {"giant centipede",      2,   13,   1,  4,   3,    0,     2,       2, FL_HEAD},
    {"jaculi",               2,    9,   1,  0,   3,    0,     2,       1, FL_HEAD},
    {"troglodyte",           2,   10,   2,  0,   5,   80,     5,       3, FL_HEAD | FL_SLOW},
    {"giant ant",            2,    8,   1,  4,   4,    0,     5,       4, FL_HEAD},
    {"floating eye",         3,    8,   2,  0,   3,    0,     7,       2, FL_FLY},
    {"leprechaun",           3,    3,   0,  8,   3, 1500,    15,      40, FL_HEAD},
    {"nymph",                3,    3,   0, 14,   9,    0,    20,      40, FL_HEAD},
    {"quasit",               3,    5,   3,  0,   3,    0,    14,      10, FL_HEAD},
    {"rust monster",         3,    5,   0,  1,   3,    0,    18,      20, FL_HEAD},
    {"zombie",               3,   12,   3,  0,   3,    0,     9,       7, FL_HEAD | FL_UNDEAD},
    {"assassin bug",         4,    4,   3,  0,   3,    0,    23,      13, FL_HEAD},
    {"bugbear",              4,    5,   4, 15,   5,   40,    24,      33, FL_HEAD | FL_INFRAVIS},
    {"hell hound",           4,    5,   2,  2,   6,    0,    20,      33, FL_HEAD},
    {"ice lizard",           4,   11,   3, 10,   6,   50,    19,      23, FL_HEAD | FL_SLOW},
    {"centaur",              4,    6,   4,  0,  10,   40,    25,      43, FL_HEAD},
    {"troll",                5,    9,   5,  0,   9,   80,    55,     250, FL_HEAD},
    {"yeti",                 5,    8,   4,  0,   5,   50,    45,      90, FL_HEAD},
    {"white dragon",         5,    4,   5,  5,  16,  500,    65,    1000, FL_HEAD},
    {"elf",                  5,    3,   3,  0,  15,   50,    25,      33, FL_HEAD | FL_INFRAVIS},
    {"gelatinous cube",      5,    9,   3,  0,   3,    0,    24,      43, 0},
    {"metamorph",            6,    9,   3,  0,   3,    0,    32,      40, FL_SLOW},
    {"vortex",               6,    5,   4,  0,   3,    0,    33,      53, 0},
    {"ziller",               6,   15,   3,  0,   3,    0,    34,      33, FL_HEAD},
    {"violet fungus",        6,   12,   3,  0,   3,    0,    39,      90, 0},
    {"wraith",               6,    3,   1,  6,   3,    0,    36,     300, FL_HEAD | FL_FLY | FL_UNDEAD},
    {"forvalaka",            6,    3,   5,  0,   7,    0,    55,     270, FL_HEAD | FL_INFRAVIS},
    {"lama nobe",            7,   14,   7,  0,   6,    0,    36,      70, FL_HEAD},
    {"osequip",              7,    4,   7, 16,   4,    0,    36,      90, FL_HEAD},
    {"rothe",                7,   15,   5,  0,   3,  100,    53,     230, FL_HEAD | FL_INFRAVIS},
    {"xorn",                 7,    6,   7,  0,  13,    0,    63,     290, FL_INFRAVIS},
    {"vampire",              7,    5,   4,  6,  17,    0,    55,     950, FL_HEAD | FL_FLY | FL_UNDEAD},
    {"invisible stalker",    7,    5,   6,  0,   5,    0,    55,     330, FL_HEAD | FL_SLOW},
    {"poltergeist",          8,    1,   8,  0,   3,    0,    55,     430, FL_SPIRIT},
    {"disenchantress",       8,    3,   1,  9,   3,    0,    57,     500, FL_HEAD},
    {"shambling mound",      8,   13,   5,  0,   6,    0,    47,     390, 0},
    {"yellow mold",          8,   12,   4,  0,   3,    0,    37,     240, 0},
    {"umber hulk",           8,    6,   7, 11,  14,    0,    67,     600, FL_HEAD | FL_INFRAVIS},
    {"gnome king",           9,   -1,  10,  0,  18, 2000,   120,    3000, FL_HEAD | FL_INFRAVIS},
    {"mimic",                9,    9,   7,  0,   8,    0,    57,     100, 0},
    {"water lord",           9,  -10,  15,  7,  20,    0,   155,   15000, FL_HEAD | FL_NOBEHEAD},
    {"bronze dragon",        9,    5,   9,  3,  16,  300,    90,    4000, FL_HEAD | FL_FLY},
    {"green dragon",         9,    4,   4, 10,  15,  200,    80,    2500, FL_HEAD | FL_FLY},
    {"purple worm",          9,   -1,  13,  0,   3,  100,   130,   15000, FL_HEAD},
    {"xvart",                9,   -2,  14,  0,  13,    0,   100,    1000, FL_HEAD | FL_SLOW},
    {"spirit naga",         10,  -20,  15, 12,  23,    0,   100,   20000, FL_HEAD | FL_NOBEHEAD | FL_FLY | FL_SPIRIT | FL_INFRAVIS},
    {"silver dragon",       10,   -4,  10,  3,  20,  700,   110,   10000, FL_HEAD | FL_FLY},
    {"platinum dragon",     10,   -7,  15, 13,  22, 1000,   150,   25000, FL_HEAD | FL_FLY},
    {"green urchin",        10,   -5,  12,  0,   3,    0,    95,    5000, 0},
    {"red dragon",          10,   -4,  13,  3,  19,  800,   120,   14000, FL_HEAD | FL_FLY},
    {"type I demon lord",   12,  -40,  20,  3,  20,    0,   150,   50000, FL_HEAD | FL_NOBEHEAD | FL_FLY | FL_INFRAVIS},
    {"type II demon lord",  13,  -45,  25,  5,  22,    0,   200,   75000, FL_HEAD | FL_NOBEHEAD | FL_FLY | FL_INFRAVIS},
    {"type III demon lord", 14,  -50,  30,  9,  24,    0,   250,  100000, FL_HEAD | FL_NOBEHEAD | FL_FLY | FL_INFRAVIS},
    {"type IV demon lord",  15,  -55,  35, 11,  26,    0,   300,  125000, FL_HEAD | FL_NOBEHEAD | FL_FLY | FL_INFRAVIS},
    {"type V demon lord",   16,  -60,  40, 13,  28,    0,   350,  150000, FL_HEAD | FL_NOBEHEAD | FL_FLY | FL_INFRAVIS},
    {"type VI demon lord",  17,  -65,  45, 13,  30,    0,   400,  175000, FL_HEAD | FL_NOBEHEAD | FL_FLY | FL_INFRAVIS},
    {"type VII demon lord", 18,  -70,  50,  6,  32,    0,   450,  200000, FL_HEAD | FL_NOBEHEAD | FL_FLY | FL_INFRAVIS},
    {"demon prince",        19,  -90,  80,  6,  40,    0,  1000,  500000, FL_HEAD | FL_NOBEHEAD | FL_FLY | FL_INFRAVIS},
    {"God of Hellfire",     20, -120, 100,  6, 100,    0, 32767, 1000000, FL_HEAD | FL_NOBEHEAD | FL_FLY | FL_INFRAVIS}};

char lastmonst[40]; /* name of the current monster */
MonsterIdType last_monst_id =
    MONST_NONE;    /* the last monster hitting the player */
int last_monst_hx; /* x location of the last monster hit by player */
int last_monst_hy; /* y location of the last monster hit by player */
char rmst = 80;    /* Random monster creation timer */

/* =============================================================================
 * Local variables
 */

/*
 * Monster movement area for the current monster movement
 */
static int move_xl, move_yl;
static int move_xh, move_yh;

/*
 * The maximum path distance for smart monster movement
 */
static int distance;

/*
 * The new location for the last monster moved
 * These will be set to -1 if the monster died.
 */
static int movedx, movedy;

/*
 * Rustable armour data
 */
#define ARMORTYPES 6

static short rustarm[ARMORTYPES][2] = {{OSTUDLEATHER, -2}, {ORING, -4},
                                       {OCHAIN, -5},       {OSPLINT, -6},
                                       {OPLATE, -8},       {OPLATEARMOR, -9}};

/* =============================================================================
 * Local functions
 */

/* =============================================================================
 * FUNCTION: checkloss
 *
 * DESCRIPTION:
 * Function to subtract hp from user and flag bottomline display.
 * Note: if x > c[HP] this routine could kill the player!
 *
 * PARAMETERS:
 *
 *   x : The number of hitpoints for the player to lose.
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void checkloss(int x) {
  if (x > 0) {
    losehp(DIED_MONSTER, x);
    UpdateStatus();
  }
}

/* =============================================================================
 * FUNCTION: dropsomething
 *
 * DESCRIPTION:
 * Function to create an object when a monster dies
 *
 * PARAMETERS:
 *
 *   x     : The x location
 *
 *   y     : The y location
 *
 *   monst : The monster id.
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void dropsomething(int x, int y, MonsterIdType monst) {
  switch (monst) {
  case ORC:
  case NYMPH:
  case ELF:
  case TROGLODYTE:
  case TROLL:
  case ROTHE:
  case VIOLETFUNGI:
  case PLATINUMDRAGON:
  case GNOMEKING:
  case REDDRAGON:
    something(x, y, level);
    return;
  case LEPRECHAUN:
    if (rnd(101) >= 75)
      creategem();
    if (rnd(5) == 1)
      dropsomething(x, y, LEPRECHAUN);
  case LEMMING:
    return;
  default:
    break;
  }
}

/* =============================================================================
 * FUNCTION: rust_armour
 *
 * DESCRIPTION:
 * Function to rust armour.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   Returns a pointer to the format string to print.
 */
static char *rust_armour(void) {
  int Armour;
  int ArmourId;
  int Shield;
  int Rusted = 0;
  int i;
  int Found;
  char *p;

  Armour = c[WEAR];
  Shield = c[SHIELD];

  /*
   * Rust the shield, if present and rustable, first
   */
  if (Shield != -1) {
    if (ivenarg[Shield] > -1) {
      adjustivenarg(Shield, -1);
      Rusted = 1;
    }
  }

  /*
   * If nothing rusted yet then rust the armour if possible
   */
  if (!Rusted && (Armour != -1)) {
    /* find the armor in table */
    ArmourId = iven[Armour];
    i = 0;
    Found = 0;

    while ((i < ARMORTYPES) && !Found) {
      if (ArmourId == rustarm[i][0])
        Found = 1;
      else
        i++;
    }

    if (Found) {
      if (ivenarg[Armour] > rustarm[i][1]) {
        adjustivenarg(Armour, -1);
        Rusted = 1;
      }
    }
  }

  /* if rusting did not occur */
  if (!Rusted) {
    switch (ArmourId) {
    case OLEATHER:
      p = "\nThe %s hit you -- you are lucky you have leather on.";
      break;
    case OSSPLATE:
      p = "\nThe %s hit you -- you are fortunate to have stainless steel "
          "armor!";
      break;
    case OELVENCHAIN:
      p = "\nThe %s hit you -- you are very lucky to have such strong elven "
          "chain!";
      break;
    default:
      p = "\nThe %s hit you.";
      break;
    }
  } else {
    UlarnBeep();
    p = "\nThe %s hit you -- your armor feels weaker.";
  }

  return p;
}

/* =============================================================================
 * FUNCTION: spattack
 *
 * DESCRIPTION:
 * Function to process special attacks from monsters
 *
 * atckno   monster     effect
 * ---------------------------------------------------
 * 0  none
 * 1  rust      eat armor
 * 2  hell hound    breathe light fire
 * 3  dragon      breathe fire
 * 4  giant centipede   weakening strength
 * 5  white dragon    cold breath
 * 6  wraith      drain level
 * 7  waterlord   water gusher
 * 8  leprechaun    steal gold
 * 9  disenchantress    disenchant weapon or armor
 * 10 ice lizard    hits with barbed tail
 * 11 umber hulk    confusion
 * 12 spirit naga   cast spells taken from special attacks
 * 13 platinum dragon   psionics
 * 14 nymph       steal objects
 * 15 bugbear     bite
 * 16 osequip
 *
 * PARAMETERS:
 *
 *   x  : THe special attack number.
 *
 *   xx : The x coordinate of the monster.
 *
 *   yy : The y coordinate of the monster.
 *
 * RETURN VALUE:
 *
 *   Returns 1 if must do a show1cell(xx,yy)
 *   return, 0 otherwise
 */

static char spsel[] = {1, 2, 3, 5, 6, 8, 9, 11, 13, 14};

static int spattack(int x, int xx, int yy) {
  int i;
  int m;
  char *p = 0;
  int need_beep = 0;
  int need_show = 0;
  int Attempt;
  int Disenchant;
  MonsterIdType monst;

  vxy(xx, yy); /* verify x & y coordinates */

  monst = mitem[xx][yy].mon;

  /*
   * cancel only works 5% of time for demon prince and god
   */
  if (c[CANCELLATION]) {
    if (monst >= DEMONPRINCE) {
      if (rnd(100) >= 95)
        return 0;
    } else
      return 0;
  }

  /* staff of power cancels demonlords/wraiths/vampires 75% of time */
  /* lucifer is unaffected */
  if (monst != LUCIFER) {
    if ((monst >= DEMONLORD) || (monst == WRAITH) || (monst == VAMPIRE))
      if (player_has_item(OPSTAFF))
        if (rnd(100) < 75)
          return 0;
  }

  /* if have cube of undead control, undead monsters do nothing */
  if ((monster[monst].flags & FL_UNDEAD) != 0)
    if ((c[CUBEofUNDEAD]) || (c[UNDEADPRO]))
      return 0;

  switch (x) {
  case 1:
    p = rust_armour();
    /* Recalculate the AC and WC */
    recalc();
    break;

  case 2:
    i = rnd(15) + 8 - c[AC];
    if (c[FIRERESISTANCE])
      p = "\nThe %s's flame doesn't faze you!";
    else {
      p = "\nThe %s breathes fire at you!";
      need_beep = 1;
    }
    checkloss(i);
    break;

  case 3:
    i = rnd(20) + 25 - c[AC];

    if (c[FIRERESISTANCE])
      p = "\nThe %s's flame doesn't faze you!";
    else {
      p = "\nThe %s breathes fire at you!";
      need_beep = 1;
    }
    checkloss(i);
    break;

  case 4:
    if (c[STRENGTH] > 3) {
      p = "\nThe %s stung you!  You feel weaker.";
      need_beep = 1;
      adjust_ability(STRENGTH, -1);
    } else
      p = "\nThe %s stung you!";
    break;

  case 5:
    p = "\nThe %s blasts you with his cold breath.";
    i = rnd(15) + 18 - c[AC];
    need_beep = 1;
    checkloss(i);
    break;

  case 6:
    if (player_has_item(OLIFEPRESERVER))
      /* Life preserver prevents all drain life attacks */
      return 0;

    p = "\nThe %s drains you of your life energy!";
    loselevel();
    if (monst == DEMONPRINCE)
      losemspells(1);
    if (monst == LUCIFER) {
      loselevel();
      losemspells(2);
      for (i = STRENGTH; i <= DEXTERITY; i++)
        adjust_ability(i, -1);
    }
    need_beep = 1;
    break;

  case 7:
    p = "\nThe %s got you with a gusher!";
    i = rnd(15) + 25 - c[AC];
    need_beep = 1;
    checkloss(i);
    break;

  case 8:
    /* he has a device of no theft */
    if (c[NOTHEFT])
      return 0;

    if (c[GOLD]) {
      p = "\nThe %s hit you.  Your purse feels lighter.";
      if (c[GOLD] > 32767)
        c[GOLD] >>= 1;
      else
        c[GOLD] -= rnd((int)(1 + (c[GOLD] >> 1)));

      if (c[GOLD] < 0)
        c[GOLD] = 0;
    } else
      p = "\nThe %s couldn't find any gold to steal.";
    disappear(xx, yy);
    need_beep = 1;
    need_show = 1;
    break;

  case 9:
    /* disenchant */
    Attempt = 50;
    Disenchant = 0;

    while ((Attempt > 0) && (!Disenchant)) {
      /* randomly select item */
      i = rund(IVENSIZE);
      m = iven[i];
      if ((m != ONOTHING) && (ivenarg[i] > 0) && (m != OSCROLL) &&
          (m != OPOTION)) {
        /* This item can be disenchanted */
        Disenchant = 3;
        if (ivenarg[i] < Disenchant)
          Disenchant = ivenarg[i];

        adjustivenarg(i, -Disenchant);

        p = "\nThe %s hits you with a spell of disenchantment! ";

        need_beep = 1;
        show3(i);

        Disenchant = 1;
      }

      Attempt--;
    }

    /*
     * Set the message if the disenchant failed.
     */
    if (!Disenchant)
      p = "\nThe %s nearly misses.";
    break;

  case 10:
    p = "\nThe %s hit you with its barbed tail.";
    i = rnd(25) - c[AC];
    need_beep = 1;
    checkloss(i);
    break;

  case 11:
    if (wizard)
      return 0;

    p = "\nThe %s has confused you.";
    need_beep = 1;
    c[CONFUSE] += 10 + rnd(10);
    break;

  case 12:
    /* performs any number of other special attacks  */
    need_show = spattack(spsel[rund(10)], xx, yy);
    break;

  case 13:
    p = "\nThe %s flattens you with it's psionics!";
    i = rnd(15) + 30 - c[AC];
    need_beep = 1;
    checkloss(i);
    break;

  case 14:
    /* he has device of no theft */
    if (c[NOTHEFT])
      return 0;

    if (emptyhanded() == 1)
      p = "\nThe %s couldn't find anything to steal.";
    else {
      Printf("\nThe %s picks your pocket and takes:", lastmonst);
      if (stealsomething(xx, yy) == 0)
        Print("  nothing");
      teleportmonst(xx, yy, mitem[xx][yy].mon);

      need_beep = 1;
      need_show = 1;
    }
    break;

  case 15:
    i = rnd(10) + 5 - c[AC];
    p = "\nThe %s bit you!";
    need_beep = 1;
    checkloss(i);
    break;

  case 16:
    i = rnd(15) + 10 - c[AC];
    p = "\nThe %s bit you!";
    need_beep = 1;
    checkloss(i);
    break;
  }

  /*
   * This attack needs a beep to be played to alert the player
   * to bad things.
   */
  if (need_beep)
    UlarnBeep();

  /*
   * If a message has been set then display the message
   */
  if (p) {
    Printf(p, lastmonst);

    recalc();
    UpdateStatus();
  }

  return need_show;
}

/* =============================================================================
 * FUNCTION:
 *
 * DESCRIPTION:
 * Function to actually perform the monster movement.
 * This function sets MovedX & MovedY to the new monster position, or -1
 * if the monster was killed or otherwise removed from the level.
 *
 * PARAMETERS:
 *
 *   sx : The starting x coordinate of the monster
 *
 *   sy : The starting y coordinate of the monster
 *
 *   dx : The destination x coordinate for the monster
 *
 *   dy : The destination y coordinate for the monster
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void mmove(int sx, int sy, int dx, int dy) {
  MonsterIdType monst_id;
  int monst_killed = 0;
  int have_talisman;
  int trap_damage;
  int it;
  int i;
  int n;
  char *who;
  char *trap_msg;

  if ((dx == playerx) && (dy == playery)) {
    /* The destination is the player, so the monster attacks */
    hitplayer(sx, sy);
    moved[sx][sy] = 1;
    movedx = sx;
    movedy = sy;
    return;
  }

  monst_id = mitem[sx][sy].mon;
  it = item[dx][dy];

  /* Copy the monster and items it is carrying to the new location */
  for (i = 0; i < mitem[sx][sy].n; i++) {
    mitem[dx][dy].it[i].item = mitem[sx][sy].it[i].item;
    mitem[dx][dy].it[i].itemarg = mitem[sx][sy].it[i].itemarg;
    mitem[sx][sy].it[i].item = ONOTHING;
    mitem[sx][sy].it[i].itemarg = 0;
  }
  mitem[dx][dy].n = mitem[sx][sy].n;
  mitem[dx][dy].mon = mitem[sx][sy].mon;

  /* monsters that move are obviously awake */
  stealth[dx][dy] |= STEALTH_AWAKE;
  hitp[dx][dy] = hitp[sx][sy];

  /* clear the monster from the old location */
  mitem[sx][sy].mon = MONST_NONE;
  mitem[sx][sy].n = 0;
  hitp[sx][sy] = 0;

  /* mark this monster as moved */
  moved[dx][dy] = 1;

  /* perform special processing for monsters */

  if (monst_id == LEMMING) {
    if (rnd(150) <= 1) {
      mitem[sx][sy].mon = LEMMING;
      hitp[sx][sy] = hitp[dx][dy];
    }
  }

  if (monst_id == LEPRECHAUN) {
    /* leprechaun takes gold and gems */
    switch (it) {
    case OGOLDPILE:
    case OMAXGOLD:
    case OKGOLD:
    case ODGOLD:
    case ODIAMOND:
    case ORUBY:
    case OEMERALD:
    case OSAPPHIRE:
      if (mitem[dx][dy].n < 6) {
        n = mitem[dx][dy].n;
        mitem[dx][dy].it[n].item = item[dx][dy];
        mitem[dx][dy].it[n].itemarg = iarg[dx][dy];
        mitem[dx][dy].n++;
      }
      item[dx][dy] = ONOTHING;
      iarg[dx][dy] = 0;
      break;

    default:
      break;
    }
  }

  if (monst_id == TROLL) {
    /* if a troll regenerate him */
    if ((gtime & 1) == 0)
      if (monster[monst_id].hitpoints > hitp[dx][dy])
        hitp[dx][dy]++;
  }

  /* check for monsters walking into traps and spheres of annihilation */

  trap_damage = 0;
  trap_msg = NULL;
  who = "";

  if (it == OANNIHILATION) {
    /* demons dispel spheres */
    if (monst_id >= DEMONLORD) {
      have_talisman = player_has_item(OSPHTALISMAN);

      if ((!have_talisman) ||
          (have_talisman && (monst_id == LUCIFER) && (rnd(10) > 7))) {
        /* delete the sphere */
        trap_msg = "\nThe %s%s dispels the sphere!";
        rmsphere(dx, dy);
      } else {
        /* monster annihilated */
        trap_msg = "\nThe %s%s is destroyed by the sphere of annihilation!";
        mitem[dx][dy].mon = MONST_NONE;
        mitem[dx][dy].n = 0;
        hitp[dx][dy] = 0;
        monst_killed = 1;
      }
    } else {
      /* monster annihilated */
      trap_msg = "\nThe %s%s is destroyed by the sphere of annihilation!";
      mitem[dx][dy].mon = MONST_NONE;
      mitem[dx][dy].n = 0;
      hitp[dx][dy] = 0;
      monst_killed = 1;
    }
  } else if (it == OTRAPARROW) {
    who = "An arrow";
    trap_damage = rnd(10) + level;
  } else if (it == ODARTRAP) {
    who = "A dart";
    trap_damage = rnd(6);
  } else if (it == OTELEPORTER) {
    /* monster hits teleport trap */
    if (monst_id < DEMONLORD) {
      trap_msg = "\nThe %s%s gets teleported.";
      teleportmonst(dx, dy, mitem[dx][dy].mon);
      dx = movedx;
      dy = movedy;
    }
  } else if (it == OPIT) {
    if ((monster[monst_id].flags & FL_FLY) == 0) {
      /* non-flying monsters can fall into pits and trap doors */
      trap_msg = "\nThe %s%s fell into a pit.";
      mitem[dx][dy].mon = MONST_NONE;
      hitp[dx][dy] = 0;
      monst_killed = 1;
    }
  } else if (it == OTRAPDOOR) {
    if ((monster[monst_id].flags & FL_FLY) == 0) {
      /* non-flying monsters can fall into pits and trap doors */
      trap_msg = "\nThe %s%s fell through a trapdoor.";
      mitem[dx][dy].mon = MONST_NONE;
      hitp[dx][dy] = 0;
      monst_killed = 1;
    }
  } else if ((it == OELEVATORUP) || (it == OELEVATORDOWN)) {
    /* The monster enters an elevator */
    if (monst_id < DEMONLORD) {
      trap_msg = "\nThe %s%s is carried away by an elevator!";
      mitem[dx][dy].mon = MONST_NONE;
      hitp[dx][dy] = 0;
      monst_killed = 1;
    }
  }

  if (trap_damage > 0) {
    /* the monster was damaged by a trap */
    hitp[dx][dy] -= (short)trap_damage;

    if (hitp[dx][dy] <= 0) {
      /* the trap killed the monster */
      mitem[dx][dy].mon = MONST_NONE;
      trap_msg = "\n%s hits and kills the %s.";
      monst_killed = 1;
    } else
      trap_msg = "\n%s hits the %s.";
  }

  /*
   * Store the location to which this monster has been moved, or -1 if
   * the monster was destroyed
   */
  if (monst_killed) {
    movedx = -1;
    movedy = -1;
  } else {
    movedx = dx;
    movedy = dy;
  }

  /* if blind don't show where monsters are */
  if (c[BLINDCOUNT])
    return;

  if (know[dx][dy] != OUNKNOWN) {
    if (trap_msg != NULL) {
      Printf(trap_msg, who, monster[monst_id].name);
      UlarnBeep();
    }
  }

  if ((monst_id >= DEMONLORD) && (monst_id <= LUCIFER) && (c[EYEOFLARN] == 0))
    /*
     * don't update the screen for demonlords and above if the player
     * doesn't have the eye of larn as the player can't see the monster.
     */
    return;

  /* Update the screen */
  if (know[sx][sy] != OUNKNOWN)
    show1cell(sx, sy);
  if (know[dx][dy] != OUNKNOWN)
    show1cell(dx, dy);
}

/* =============================================================================
 * FUNCTION: valid_monst_move
 *
 * DESCRIPTION:
 * Check if the location x, y is a valid place for monster monst_id to move
 * or attack.
 * Does NOT check if the location is already occupied by another monster.
 *
 * PARAMETERS:
 *
 *   x        : The x coordinate to check
 *
 *   y        : THe y coordinate to check
 *
 *   monst_id : The monster id
 *
 * RETURN VALUE:
 *
 *   0 if the monster cannot occupy to this position
 *   1 if the monster can occupy to this postion
 */
static int valid_monst_move(int x, int y, MonsterIdType monst_id) {
  int tmpitem;
  int at_entrance;     /* flag indicating that this is the dungeon entrance */
  int at_player;       /* flag that this is the player's location */
  int blocked;         /* flag that the monster canot move here (wall etc) */
  int monster_special; /* flag that there is a special reason the monster  */

  /* can't move here */

  tmpitem = item[x][y];

  at_player = (x == playerx) && (y == playery);
  at_entrance = (x == 33) && (y == MAXY - 1) && (level == 1);

  /*
   * A monster cannot pass through a closed door or a wall.
   * However, while a monster cannot hit the player through a closed door
   * a monster can hit a player who is walking through walls.
   */
  blocked = (tmpitem == OCLOSEDDOOR) || ((tmpitem == OWALL) && !at_player);

  /*
   * Vampires will not move onto mirrors.
   */
  monster_special = (monst_id == VAMPIRE) && (tmpitem == OMIRROR);

  if (monst_id >= DEMONPRINCE)
    /* walls and closed doors are no hindrance to a demon prince or above */
    blocked = 0;

  if (at_entrance || blocked || monster_special)
    /* Return 0  (false) to indicate illegal position */
    return 0;
  else
    return 1;
}

/* =============================================================================
 * FUNCTION: smart_move
 *
 * DESCRIPTION:
 * Function to move smart monsters.
 *
 * PARAMETERS:
 *
 *   x : The x coordinate of the monster.
 *
 *   y : The y coordinate of the monster.
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void smart_move(int x, int y) {
  int sx, sy;
  int xl, yl;
  int xh, yh;
  int xtmp, ytmp;
  int path_dist;
  int found_path;
  MonsterIdType monst;
  int on_map;
  int z;

  monst = mitem[x][y].mon;

  /* get the screen region to check for monster movement */
  xl = move_xl - 2;
  yl = move_yl - 2;
  xh = move_xh + 2;
  yh = move_yh + 2;
  vxy(xl, yl);
  vxy(xh, yh);

  for (sy = yl; sy <= yh; sy++) {
    for (sx = xl; sx <= xh; sx++) {
      if (valid_monst_move(sx, sy, monst)) {
        if (monst >= DEMONPRINCE)
          /* Monsters of rank DEMONPRICE and above ignore traps etc */
          screen[sx][sy] = 0;
        else {
          /* smart monsters will avoid traps */
          switch (item[sx][sy]) {
          case OELEVATORUP:
          case OELEVATORDOWN:
          case OTRAPARROW:
          case ODARTRAP:
          case OTELEPORTER:
            /* all monsters avoid there traps */
            screen[sx][sy] = 127;
            break;

          case OPIT:
          case OTRAPDOOR:
            if ((monster[monst].flags & FL_FLY) != 0)
              /* flying monsters ignore pits and trap doors. */
              screen[sx][sy] = 0;
            else
              screen[sx][sy] = 127;
            break;

          default:
            screen[sx][sy] = 0;
            break;
          }
        }
      } else
        /* not valid for this monster to move here */
        screen[sx][sy] = 127;
      /* if valid move */
    } /* for sx */
  }   /* for sy */

  /* Mark the player's location */
  screen[playerx][playery] = 1;

  /* now perform proximity ripple from playerx, playery to monster */
  xl = move_xl - 1;
  yl = move_yl - 1;
  xh = move_xh + 1;
  yh = move_yh + 1;
  vxy(xl, yl);
  vxy(xh, yh);

  path_dist = 1;
  found_path = 0;

  while ((path_dist < distance) && (!found_path)) {
    for (sy = yl; sy <= yh; sy++) {
      for (sx = xl; sx <= xh; sx++) {
        if (screen[sx][sy] == path_dist) {
          /*
           * This square is path_dist steps from the player, so advance
           * the path by one in all directions not already part of a path.
           */
          for (z = 1; z < 9; z++) {
            xtmp = sx + diroffx[z];
            ytmp = sy + diroffy[z];

            if ((xtmp >= 0) && (xtmp < MAXX) && (ytmp >= 0) && (ytmp < MAXY)) {
              /* This square is within the map */
              if (screen[xtmp][ytmp] == 0) {
                /*
                 * a valid position that is not part of a path has been found,
                 * so mark it as path_dist + 1 away from the player
                 */
                screen[xtmp][ytmp] = (char)(path_dist + 1);

                if ((xtmp == x) && (ytmp == y))
                  found_path = 1;
              }
            } /* if xtmp, ytmp on map */
          }   /* for each direction */
        }     /* if found path end */
      }       /* for sx */
    }         /* for sy */

    path_dist++;
  }

  if (found_path) {
    /* did find connectivity */
    /*
     * Now find the square with a distance 1 lower than the distance to the
     * monster being moved.
     */
    path_dist = screen[x][y] - 1;

    for (z = 1; z < 9; z++) {
      xl = x + diroffx[z];
      yl = y + diroffy[z];

      if (level == 0)
        /*
         * On the home level monsters can move right to the edge
         * of the map
         */
        on_map = checkxy(xl, yl);
      else
        /*
         * In the dungeon and volcano monsters can not move onto the
         * outer border of walls.
         */
        on_map =
            ((xl >= 1) && (xl < (MAXX - 1)) && (yl >= 1) && (yl < (MAXY - 1)));

      if (on_map) {
        if (screen[xl][yl] == path_dist) {
          if (mitem[xl][yl].mon == MONST_NONE) {
            mmove(x, y, xl, yl);
            return;
          }
        }
      }
    }
  }
}

/* =============================================================================
 * FUNCTION: dumb_move
 *
 * DESCRIPTION:
 * Function to move dumb monsters.
 *
 * PARAMETERS:
 *
 *   x : The x coordinate of the monster.
 *
 *   y : The y coordinate of the monster.
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void dumb_move(int x, int y) {
  MonsterIdType monst_id;
  short w1[9], w1x[9], w1y[9];
  int xl, yl;
  int xh, yh;
  int tx, ty;
  int j, k;
  int tmp;

  monst_id = mitem[x][y].mon;
  xl = x - 1;
  yl = y - 1;
  xh = x + 2;
  yh = y + 2;

  if (x < playerx)
    xl++;
  else if (x > playerx)
    xh--;

  if (y < playery)
    yl++;
  else if (y > playery)
    yh--;

  if (xl < 0)
    xl = 0;
  if (xh > MAXX)
    xh = MAXX;
  if (yl < 0)
    yl = 0;
  if (yh > MAXY)
    yh = MAXY;

  for (k = 0; k < 9; k++)
    w1[k] = 10000;

  /* For each square compute distance to player */

  tmp = 0;
  for (tx = xl; tx < xh; tx++) {
    for (ty = yl; ty < yh; ty++) {
      if (valid_monst_move(tx, ty, monst_id) &&
          (mitem[tx][ty].mon == MONST_NONE)) {
        w1[tmp] = (short)((playerx - tx) * (playerx - tx) +
                          (playery - ty) * (playery - ty));
        w1x[tmp] = (short)tx;
        w1y[tmp] = (short)ty;
      }
      tmp++;
    }
  }

  /* find the closest square in the search area */
  j = 0;
  for (k = 1; k < 9; k++)
    if (w1[k] < w1[j])
      j = k;

  if ((w1[j] < 10000) && ((x != w1x[j]) || (y != w1y[j])))
    mmove(x, y, w1x[j], w1y[j]);
}

/* =============================================================================
 * FUNCTION: scared_move
 *
 * DESCRIPTION:
 * Function to move scared monsters.
 *
 * PARAMETERS:
 *
 *   x : The x coordinate of the monster.
 *
 *   y : The y coordinate of the monster.
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void scared_move(int x, int y) {
  int nx, ny;

  /* move in random direction */
  nx = x + rnd(3) - 2;
  ny = y + rnd(3) - 2;

  if (nx < 0)
    nx = 0;
  if (nx >= MAXX)
    nx = MAXX - 1;
  if (ny < 0)
    ny = 0;
  if (ny >= MAXY)
    ny = MAXY - 1;

  if (valid_monst_move(nx, ny, mitem[x][y].mon) &&
      (mitem[nx][ny].mon == MONST_NONE))
    /* This is a valid place to move, so move there */
    mmove(x, y, nx, ny);

  return;
}

/* =============================================================================
 * FUNCTION: movemt
 *
 * DESCRIPTION:
 * This routine is responsible for determining where one monster at (x,y)
 * will move to.
 *
 * PARAMETERS:
 *
 *   x : The x coordinate of the monster.
 *
 *   y : The y coordinate of the monster.
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void movemt(int x, int y) {
  int Scared;
  MonsterIdType monst;

  monst = mitem[x][y].mon;

  /* half speed monsters only move every other turn */
  if ((monster[monst].flags & FL_SLOW) != 0)
    if ((gtime & 1) == 1)
      return;

  /* choose destination randomly if scared */

  /* Check for hand of fear */
  Scared = player_has_item(OHANDofFEAR);

  if (c[SCAREMONST])
    /*
     * hand of fear is scarier (may work on higher level monsters) if scare
     * monster is in effect.
     */
    Scared++;
  else if ((Scared == 1) && (rnd(10) > 4))
    /*
     * Hand of fear alone is only effective 60% of the time
     */
    Scared = 0;

  if ((monst > DEMONLORD) || (monst == PLATINUMDRAGON))
    /*
     * A demon lord or higher is only scared if the player has both
     * the hand of fear and scare monster active.
     * Even then, only half the time
     */
    Scared = (Scared <= 1) ? 0 : (rnd(10) > 5);

  /* Call the appropriate move routine */
  if (Scared)
    scared_move(x, y);
  else if (monster[monst].intelligence > 10 - c[HARDGAME])
    smart_move(x, y);
  else
    dumb_move(x, y);
}

/* =============================================================================
 * Exported functions
 */

/* =============================================================================
 * FUNCTION: createmonster
 */
void createmonster(MonsterIdType mon) {
  int x, y, k, i;

  if (mon < 1 || mon > MAXMONST + 8) {
    UlarnBeep();
    Printf("\ncan't createmonst(%d)\n", (long)mon);
    nap(3000);
    return;
  }

  while ((monster[mon].flags & FL_GENOCIDED) != 0 && (mon < MAXMONST)) {
    mon++; /* genocided? */
  }

  /* choose direction, then try all */
  for (k = rnd(8), i = -8; i < 0; i++, k++) {
    if (k > 8)
      k = 1; /* wraparound the diroff arrays */
    x = playerx + diroffx[k];
    y = playery + diroffy[k];

    /* if we can create a monster here */
    if (cgood(x, y, 0, 1)) {
      mitem[x][y].mon = (char)mon;
      hitp[x][y] = monster[mon].hitpoints;
      stealth[x][y] = 0; /* New monsters are not seen or awake */

      switch (mon) {
      case ROTHE:
      case POLTERGEIST:
      case VAMPIRE:
        /* These monsters are initially awake */
        stealth[x][y] |= STEALTH_AWAKE;
        break;
      default:
        break;
      };
      return;
    }
  }
}

/* =============================================================================
 * FUNCTION: mon_has_item
 */
int mon_has_item(int x, int y, int Item) {
  int has_item;
  int i;

  has_item = 0;

  if (mitem[x][y].mon != 0) {
    for (i = 0; i < mitem[x][y].n; i++)
      if (mitem[x][y].it[i].item == Item)
        has_item = 1;
  }

  return has_item;
}

/* =============================================================================
 * FUNCTION: fullhit
 */
int fullhit(int xx) {
  int i;

  if ((xx < 0) || (xx > 20))
    return 0; /* fullhits are out of range */

  if (c[LANCEDEATH])
    return 10000; /* lance of death */

  i = xx * ((c[WCLASS] >> 1) + c[STRENGTH] + c[STREXTRA] - c[HARDGAME] - 12 +
            c[MOREDAM]);

  return (i >= 1) ? i : xx;
}

/* =============================================================================
 * FUNCTION: ifblind
 */
void ifblind(int x, int y) {
  char *s;

  vxy(x, y); /* verify correct x,y coordinates */
  if (c[BLINDCOUNT]) {
    last_monst_id = MONST_NONE;
    s = "monster";
  } else {
    last_monst_id = mitem[x][y].mon;
    s = monster[last_monst_id].name;
  }
  strcpy(lastmonst, s);
}

/* =============================================================================
 * FUNCTION: hitmonster
 */
void hitmonster(int x, int y) {
  int tmp, damag, flag;
  MonsterIdType monst;

  if (c[TIMESTOP])
    return; /* not if time stopped */

  vxy(x, y); /* verify coordinates are within range */

  if ((monst = mitem[x][y].mon) == MONST_NONE)
    return;

  hit3flag = 1;
  ifblind(x, y);

  tmp = monster[monst].armorclass + c[LEVEL] + c[DEXTERITY] + c[WCLASS] / 4 -
        12 - c[HARDGAME];

  /* need at least random chance to hit */
  if ((rnd(20) < tmp) || (rnd(71) < 5)) {
    Print("\nYou hit");
    flag = 1;
    damag = fullhit(1);

    if (damag < 9999)
      damag = rnd(damag) + 1;
  } else {
    Print("\nYou missed");
    flag = 0;
  }
  Printf(" the %s.", lastmonst);

  /*
   *  If the monser was hit, deal with weapon dulling.
   */
  if (flag &&
      (monst == RUSTMONSTER || monst == DISENCHANTRESS || monst == CUBE) &&
      (c[WIELD] >= 0)) {
    /* if it's not already dulled to hell */
    if (((ivenarg[c[WIELD]] > -10) &&
         ((iven[c[WIELD]] == OSLAYER) || (iven[c[WIELD]] == ODAGGER) ||
          (iven[c[WIELD]] == OSPEAR) || (iven[c[WIELD]] == OFLAIL) ||
          (iven[c[WIELD]] == OBATTLEAXE) || (iven[c[WIELD]] == OLONGSWORD) ||
          (iven[c[WIELD]] == O2SWORD) || (iven[c[WIELD]] == OLANCE) ||
          (iven[c[WIELD]] == OHAMMER) || (iven[c[WIELD]] == OVORPAL) ||
          (iven[c[WIELD]] == OBELT))) ||
        (ivenarg[c[WIELD]] > 0)) {
      Printf("\nYour weapon is dulled by the %s.", lastmonst);
      UlarnBeep();

      adjustivenarg(c[WIELD], -1);
    } else if (ivenarg[c[WIELD]] <= -10) {
      /* Destroy the weapon if it is too badly damaged */
      Printf("\nYour weapon disintegrates!");
      /* Adjust character attributes for the destroyed item */
      adjustcvalues(iven[c[WIELD]], ivenarg[c[WIELD]]);
      /* Destroy the item */
      iven[c[WIELD]] = ONOTHING;
      ivenarg[c[WIELD]] = 0;
      /* No longer wielding anything */
      c[WIELD] = -1;
      /* Didn't hit after all... */
      flag = 0;
    }

    /* Recalculate WC */
    recalc();
    UpdateStatus();
  }

  if (flag) {
    hitm(x, y, damag, 0);
    if ((monst >= DEMONLORD) && (c[LANCEDEATH]) && (hitp[x][y]))
      Printf("\nYour lance of death tickles the %s!", lastmonst);
  }

  if (monst == METAMORPH) {
    if (hitp[x][y] < 25 && hitp[x][y] > 0) {
      mitem[x][y].mon = (char)(BRONZEDRAGON + rund(9));
      show1cell(x, y);
    }
  }

  if (mitem[x][y].mon == LEMMING)
    if (rnd(150) <= 10)
      createmonster(LEMMING); /* was 40, now 10 */
}

/* =============================================================================
 * FUNCTION: hitm
 */
int hitm(int x, int y, int amt, int SpellFlag) {
  MonsterIdType monst;
  int hpoints;
  int amt2;

  vxy(x, y);  /* verify coordinates are within range */
  amt2 = amt; /* save initial damage so we can return it */
  monst = mitem[x][y].mon;

  /* if half damage curse adjust damage points */
  if (c[HALFDAM])
    amt >>= 1;
  if (amt <= 0) {
    amt2 = 1;
    amt = 1;
  }

  last_monst_hx = (char)x;
  last_monst_hy = (char)y;

  /* make sure hitting monst wakes the monster */
  stealth[x][y] |= STEALTH_AWAKE;

  if (c[HOLDMONST] > 0) {
    /*
     * Hit a monster breaks hold monster spell.
     * Set HOLDMONST counter to 1 so the next regen will cancel the effect.
     */

    c[HOLDMONST] = 1;
  }

  /* if a dragon and orb(s) of dragon slaying  */
  if (c[SLAYING]) {
    switch (monst) {
    case WHITEDRAGON:
    case REDDRAGON:
    case GREENDRAGON:
    case BRONZEDRAGON:
    case PLATINUMDRAGON:
    case SILVERDRAGON:
      amt *= 3;
      break;

    default:
      break;
    }
  }

  if (!SpellFlag) {
    /* Deal with weapon based effects */

    /* Deal with Vorpy */
    if ((c[WIELD] > 0) && (iven[c[WIELD]] == OVORPAL) && (rnd(20) == 1) &&
        ((monster[monst].flags & FL_HEAD) != 0) &&
        ((monster[monst].flags & FL_NOBEHEAD) == 0)) {
      Printf("\nThe Vorpal Blade beheads the %s!", lastmonst);
      amt = hitp[x][y];
    }

    if (monst >= DEMONLORD) {
      if (c[LANCEDEATH])
        amt = 300;
      if (iven[c[WIELD]] == OSLAYER)
        amt = 10000;
    }
  }

  hpoints = hitp[x][y];
  if (hpoints <= amt) {
    int i;

    Printf("\nThe %s died!", lastmonst);
    raiseexperience((long)monster[monst].experience);
    disappear(x, y);

    if (mitem[x][y].n > 0) {
      for (i = 0; i < mitem[x][y].n; i++) {
        createitem(x, y, mitem[x][y].it[i].item, mitem[x][y].it[i].itemarg);
      }
      beenhere[level] -= mitem[x][y].n;

      if (beenhere[level] < 1)
        beenhere[level] = 1;

      mitem[x][y].n = 0;
      if ((amt = monster[monst].gold) > 0)
        dropgold(rnd(amt) + amt);
    } else {
      if ((amt = monster[monst].gold) > 0)
        dropgold(rnd(amt) + amt);
      dropsomething(x, y, monst);
    }
    show1cell(x, y);
    showcell(playerx, playery);
    recalc();
    UpdateStatus();
    hitp[x][y] = 0;

    return hpoints;
  }

  hitp[x][y] = (short)(hpoints - amt);
  return amt2;
}

/* =============================================================================
 * FUNCTION: hitplayer
 */
void hitplayer(int x, int y) {
  register int dam, bias;
  int hit_success;
  int at_chance;
  MonsterIdType mster;

  vxy(x, y); /* verify coordinates are within range */

  last_monst_id = mitem[x][y].mon;
  mster = mitem[x][y].mon;

  if (know[x][y] == OUNKNOWN)
    show1cell(x, y);

  bias = (c[HARDGAME]) + 1;
  hitflag = hit2flag = hit3flag = 1;
  yrepcount = 0;

  ifblind(x, y);

  if (mster == LEMMING)
    return;

  if ((monster[mster].flags & FL_INFRAVIS) == 0) {
    /* Monsters without infravision might miss invisible players */
    if (c[INVISIBILITY])
      if (rnd(33) < 20) {
        Printf("\nThe %s misses wildly!", lastmonst);
        return;
      }
  }

  if ((mster < DEMONLORD) && (mster != PLATINUMDRAGON)) {
    if (c[CHARMCOUNT]) {
      if (rnd(30) + 5 * monster[mster].level - c[CHARISMA] < 30) {
        Printf("\nThe %s is awestruck by your magnificence!", lastmonst);
        return;
      }
    }
  }

  dam = monster[mster].damage;
  dam += rnd((int)((dam < 1) ? 1 : dam)) + monster[mster].level;

  /*
   * demon lords/prince/god of hellfire damage is reduced if wielding
   * Slayer
   */
  if (mster >= DEMONLORD) {
    if (iven[c[WIELD]] == OSLAYER)
      /* does between 1/10 and 1/2 damage */
      dam = (rnd(5) * dam) / 10;
  }

  /*
   * spirit naga's and poltergeist's damage is halved if scarab of
   * negate spirit
   */
  if (c[NEGATESPIRIT] || c[SPIRITPRO])
    if ((monster[mster].flags & FL_SPIRIT) != 0)
      dam = (int)dam / 2;

  /*  halved if undead and cube of undead control */
  if (c[CUBEofUNDEAD] || c[UNDEADPRO])
    if ((monster[mster].flags & FL_UNDEAD) != 0)
      dam = (int)dam / 2;

  hit_success = 0;
  at_chance = max(c[AC], 1);

  if (monster[mster].attack > 0) {
    /*
     * If the hit isn't good enough to do a special attack on its own then
     * a monster has a 1:at_chance of performing a special attack
     */

    if (((dam + bias + 8) > c[AC]) || (rnd(at_chance) == 1)) {
      if (spattack(monster[mster].attack, x, y))
        /* The monster successfully performed a special attack */
        return;
      else {
        /*
         * The monster did a special attack and failed to affect the player,
         * so note that the monster hit and weaken the normal attack.
         */
        hit_success = 1;
        bias -= 2;
      }
    }
  }

  if (((dam + bias) > c[AC]) || (rnd(at_chance) == 1)) {
    Printf("\n  The %s hit you.", lastmonst);
    hit_success = 1;

    /* Reduce the damage by the player's AC */
    dam -= c[AC];
    if (dam < 0)
      dam = 0;

    if (dam > 0) {
      losehp(DIED_MONSTER, dam);
      UpdateStatus();
    }
  }

  if (hit_success == 0)
    Printf("\n  The %s missed.", lastmonst);
}

/* =============================================================================
 * FUNCTION: makemonst
 */
int makemonst(int lev) {
  int tmp;
  int min_monst_id;
  int max_monst_id;
  int num_ids;

  if (lev < 1)
    lev = 1;
  if (lev > 12)
    lev = 12;

  tmp = WATERLORD;

  if (lev < 5) {
    min_monst_id = 1;
    max_monst_id = monstlevel[lev - 1];
  } else {
    min_monst_id = monstlevel[lev - 4] + 1;
    max_monst_id = monstlevel[lev - 1];
  }

  num_ids = (max_monst_id - min_monst_id) + 1;

  while (tmp == WATERLORD)
    tmp = min_monst_id + rund(num_ids);

  while ((monster[tmp].flags & FL_GENOCIDED) != 0 && (tmp < MAXMONST))
    tmp++;

  if (level <= DBOTTOM)
    if (rnd(100) < 10)
      tmp = LEMMING;

  return tmp;
}

/* =============================================================================
 * FUNCTION: randmonst
 */
void randmonst(void) {
  /*  don't make monsters if time is stopped  */
  if (c[TIMESTOP])
    return;

  rmst--;
  if (rmst <= 0) {
    rmst = (char)(120 - (level << 2));
    fillmonst(makemonst(level));
  }
}

/* =============================================================================
 * FUNCTION: teleportmonst
 */
void teleportmonst(int xx, int yy, int monst) {
  int i;
  int x, y;
  int trys;

  /* max # of teleport attempts */
  for (trys = 10; trys > 0; --trys) {
    x = rnd(MAXX - 2);
    y = rnd(MAXY - 2);
    if ((item[x][y] == ONOTHING) && (mitem[x][y].mon == MONST_NONE) &&
        ((playerx != x) || (playery != y))) {
      mitem[x][y].mon = (char)monst;
      mitem[xx][yy].mon = MONST_NONE;

      hitp[x][y] = monster[monst].hitpoints;
      hitp[xx][yy] = 0;
      for (i = 0; i < mitem[xx][yy].n; i++) {
        mitem[x][y].it[i].item = mitem[xx][yy].it[i].item;
        mitem[x][y].it[i].itemarg = mitem[xx][yy].it[i].itemarg;
        mitem[xx][yy].it[i].item = ONOTHING;
        mitem[xx][yy].it[i].itemarg = 0;
      }
      mitem[x][y].n = mitem[xx][yy].n;
      mitem[xx][yy].n = 0;

      /* store the new location */
      movedx = x;
      movedy = y;

      show1cell(xx, yy);

      return;
    }
  }
}

/* =============================================================================
 * FUNCTION: movemonst
 */
void movemonst(void) {
  int i, j;

  /* no action if time is stopped */
  if (c[TIMESTOP])
    return;

  /* Check for haste self */
  if (c[HASTESELF])
    if ((c[HASTESELF] & 1) == 0)
      return;

  /* move the spheres of annihilation if any */
  movsphere();

  /* no action if monsters are held */
  if (c[HOLDMONST])
    return;

  if (c[AGGRAVATE]) {
    /* determine window of monsters to move */
    move_yl = playery - 5;
    move_yh = playery + 6;
    move_xl = playerx - 10;
    move_xh = playerx + 11;
    distance = 40; /* depth of intelligent monster movement */
  } else {
    move_yl = playery - 3;
    move_yh = playery + 4;
    move_xl = playerx - 5;
    move_xh = playerx + 6;
    distance = 17; /* depth of intelligent monster movement */
  }

  if (level == 0) {
    /* if on outside level monsters can move in perimeter */
    if (move_yl < 0)
      move_yl = 0;
    if (move_yh > MAXY)
      move_yh = MAXY;
    if (move_xl < 0)
      move_xl = 0;
    if (move_xh > MAXX)
      move_xh = MAXX;
  } else {
    /* if in a dungeon monsters can't be on the perimeter (wall there) */
    if (move_yl < 1)
      move_yl = 1;
    if (move_yh > MAXY - 1)
      move_yh = MAXY - 1;
    if (move_xl < 1)
      move_xl = 1;
    if (move_xh > MAXX - 1)
      move_xh = MAXX - 1;
  }

  /* now reset monster moved flags */
  for (j = move_yl; j < move_yh; j++)
    for (i = move_xl; i < move_xh; i++)
      moved[i][j] = 0;

  /*
   * Move the last monster hit by the player
   * This is mainly to make monster hit by spells move towards the player,
   * even if out of the normal movement range.
   * If the last monster hit by the player no longer exists then
   * last_monst_hx and last_monst_hy will be out of the map range
   * (actually set to -1).
   */
  if ((last_monst_hx >= 0) && (last_monst_hx < MAXX) && (last_monst_hy >= 0) &&
      (last_monst_hy < MAXY)) {
    moved[last_monst_hx][last_monst_hy] = 0;
    if (mitem[last_monst_hx][last_monst_hy].mon != MONST_NONE) {
      movemt(last_monst_hx, last_monst_hy);
      last_monst_hx = (char)movedx;
      last_monst_hy = (char)movedy;
    }
  }

  if (c[AGGRAVATE] || !c[STEALTH]) {
    /* move monsters for aggravation or not stealthed */
    for (j = move_yl; j < move_yh;
         j++) { /* look thru all locations in window */
      for (i = move_xl; i < move_xh; i++) {
        if ((mitem[i][j].mon != MONST_NONE) && (moved[i][j] == 0))
          /* if there is a monster to move and it isn't already moved */
          movemt(i, j);
      }
    }
  } else {
    for (j = move_yl; j < move_yh; j++) {
      for (i = move_xl; i < move_xh; i++) {
        if ((mitem[i][j].mon != MONST_NONE) && (moved[i][j] == 0)) {
          /* if there is a monster to move and it isn't already moved */
          if ((stealth[i][j] & STEALTH_AWAKE) != 0)
            /* Move awake monsters */
            movemt(i, j);
        }
      }
    }
  }
}

/* =============================================================================
 * FUNCTION: parse2
 */
void parse2(void) {
  /* move the monsters */
  if (c[HASTEMONST])
    movemonst();

  movemonst();
  randmonst();
  regen();
}

/* =============================================================================
 * FUNCTION: write_monster_data
 */
void write_monster_data(FILE *fp) {
  int i;
  char genocided;

  /* write the random nonster generation time */
  bwrite(fp, (char *)&rmst, sizeof(rmst));

  /* Write genocide status */
  for (i = 0; i < MAXMONST; i++) {
    genocided = (char)((monster[i].flags & FL_GENOCIDED) != 0);
    bwrite(fp, &genocided, sizeof(genocided));
  }
}

/* =============================================================================
 * FUNCTION: read_monster_data
 */
void read_monster_data(FILE *fp) {
  int i;
  char genocided;

  /* read the random nonster generation time */
  bread(fp, (char *)&rmst, sizeof(rmst));

  /* read genocide status */
  for (i = 0; i < MAXMONST; i++) {
    bread(fp, &genocided, sizeof(genocided));

    if (genocided)
      monster[i].flags |= FL_GENOCIDED;
  }
}
