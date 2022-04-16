/* =============================================================================
 * PROGRAM:  ularn
 * FILENAME: dungeon.h
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
 * checkxy        : Just check if x, y are on the map, making no changes.
 * createitem     : Create an item
 * something      : Create a random item
 * newobject      : Return a randomly selected item
 * write_levels   : Write dungeon levels to the save file
 * read_levels    : Read dungeon levels from the save file
 *
 * =============================================================================
 */

#ifndef __DUNGEON_H
# define __DUNGEON_H

# include <stdio.h>

# include "monster.h"

/* =============================================================================
 * Exported variables
 */

# define MAXLEVEL 16 /*  max # levels in the dungeon + 1 */
# define MAXVLEVEL 5 /*  max # of levels in the volcano */
# define NLEVELS (MAXLEVEL + MAXVLEVEL)
# define DBOTTOM (MAXLEVEL - 1)
# define VBOTTOM (MAXLEVEL + MAXVLEVEL - 1)

/*
 * Dungeon level size. MUST bo odd.
 */
# define MAXX 67
# define MAXY 17

/*
 * Items stolen by monsters data structure
 */
typedef struct _s {
  char item;
  short itemarg;
} StolenItem;

typedef struct {
  char mon;
  char n;
  StolenItem it[6];
} struct_mitem;

/*
 * Stealth flags for monsters.
 *   STEALTH_SEEN   indicates that the player has seen or somehow knowns
 *                  about the monster.
 *
 *   STEALTH_AWAKE  indicates that the monster is awake and will move
 *                  about the dungeon.
 *
 * A monster will be displayed on the map if
 *   1. The player's knowledge of the cell containing the monster is not
 *      OUNKNOWN,
 * and either:
 *   2a. The monster has been seen (STEALTH_SEEN set) or,
 *   2b. The monster is awake (STEALTH_AWAKE set)
 *       (Awake monsters are assumed to be making noise)
 */
# define STEALTH_SEEN 1
# define STEALTH_AWAKE 2

extern char item[MAXX][MAXY];    /* objects in maze if any */
extern char know[MAXX][MAXY];    /* contains what the player thinks is here */
extern char moved[MAXX][MAXY];   /* monster movement flags  */
extern char stealth[MAXX][MAXY]; /* 0=sleeping 1=awake monst    */
extern short hitp[MAXX][MAXY];   /* monster hp on level  */
extern short iarg[MAXX][MAXY];   /* arg for the item array */
extern short screen[MAXX][MAXY]; /* The screen as the player knows it */
extern struct_mitem mitem[MAXX][MAXY]; /* Items stolen by monstes array */

/*
 * This serves two purposes:
 *   1. Indicates which levels have been visited by the player.
 *        0  => not visited
 *        >0 => visited
 *   2. Indicates how many items have been stolen from the player on this level.
 *      This is used as a sanity check. Don't blame me, it was like this
 *      when I got here :-)
 *      Each time an item is stolen this is incremented.
 *      When the monster that stole the items is killed, this is decremented
 *      for each item the monster drops.
 */
extern char beenhere[NLEVELS];

extern int level; /* cavelevel player is on = c[CAVELEVEL]*/

extern char *levelname[]; /* Dungeon level names */

/*
 * MACRO: forget
 * Destroy object at present location
 * As the player is there, obviously it is known to be destroyed.
 */
# define forget()                                                               \
  {                                                                            \
    item[playerx][playery] = ONOTHING;                                         \
    know[playerx][playery] = ONOTHING;                                         \
  }

/*
 * MACRO: disappear
 * Wipe out a monster at a location.
 * Redraw the location if it is not unknown.
 */
# define disappear(x, y)                                                        \
  {                                                                            \
    mitem[x][y].mon = 0;                                                       \
    if (know[x][y] != OUNKNOWN)                                                \
      show1cell(x, y);                                                         \
  }

/* verify coordinates */
# define vxy(x, y)                                                              \
  {                                                                            \
    if (x < 0)                                                                 \
      x = 0;                                                                   \
    if (y < 0)                                                                 \
      y = 0;                                                                   \
    if (x >= MAXX)                                                             \
      x = MAXX - 1;                                                            \
    if (y >= MAXY)                                                             \
      y = MAXY - 1;                                                            \
  }

/* =============================================================================
 * FUNCTION: init_cells
 *
 * DESCRIPTION:
 * Allocate storage for levels.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void init_cells(void);

/* =============================================================================
 * FUNCTION: free_cells
 *
 * DESCRIPTION:
 * Free storage for levels.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void free_cells(void);

/* =============================================================================
 * FUNCTION:
 *
 * DESCRIPTION:
 * Function to check location for emptiness and for dungeon exit.
 * if itm==TRUE check for no item at this location
 * if monst==TRUE check for no monster at this location
 *
 * PARAMETERS:
 *
 *   x     : The x coordinate to check
 *
 *   y     : The y coordinate to check
 *
 *   chkitm   : True if check for empty of items
 *
 *   chkmonst : True if check for empty of monsters.
 *
 * RETURN VALUE:
 *
 *   TRUE if a location does not have itm or monst there,
 *   FALSE (0) otherwise
 */
int cgood(int x, int y, int chkitm, int chkmonst);

/* =============================================================================
 * FUNCTION: dropgold
 *
 * DESCRIPTION:
 * Function to drop some gold around player.
 *
 * PARAMETERS:
 *
 *   amount : The amount of gold to drop
 *
 * RETURN VALUE:
 *
 *   None.
 */
void dropgold(int amount);

/* =============================================================================
 * FUNCTION: fillmonst
 *
 * DESCRIPTION:
 * Function to put a monster into an empty room without walls or other
 * monsters
 *
 * PARAMETERS:
 *
 *   what : The monster to fill.
 *
 * RETURN VALUE:
 *
 *   0 if the monster could be created
 *  -1 if no location for the monster could be found
 */
int fillmonst(int what);

/* =============================================================================
 * FUNCTION: eat
 *
 * DESCRIPTION:
 * Function to eat a maze in a level filled with walls.
 *
 * PARAMETERS:
 *
 *   xx : The starting x coordinate
 *
 *   yy : The starting y coordinate
 *
 * RETURN VALUE:
 *
 *   None.
 */
void eat(int xx, int yy);

/* =============================================================================
 * FUNCTION: savelevel
 *
 * DESCRIPTION:
 * Routine to save the present level into storage.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void savelevel(void);

/* =============================================================================
 * FUNCTION: getlevel
 *
 * DESCRIPTION:
 * Routine to restore a level from storage.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void getlevel(void);

/* =============================================================================
 * FUNCTION: AnalyseWalls
 *
 * DESCRIPTION:
 * Function to analyse the tiles needed for walls based on the layout of
 * adjacent walls.
 *
 * PARAMETERS:
 *
 *   x1 : The leftmost position in the area to analyse
 *
 *   y1 : The topmose position in the area to analyse
 *
 *   x2 : The rightmost position in the area to analyse
 *
 *   y2 : The bottommost position in the area to analyse.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void AnalyseWalls(int x1, int y1, int x2, int y2);

/* =============================================================================
 * FUNCTION: newcavelevel
 *
 * DESCRIPTION:
 * Function to enter a new level.  This routine must be called anytime the
 * player changes levels.  If that level is unknown it will be created.
 * A new set of monsters will be created for a new level, and existing
 * levels will get a few more monsters.
 * Note that it is here we remove genocided monsters from the present level
 *
 * PARAMETERS:
 *
 *   x : The new cave level to visit.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void newcavelevel(int x);

/* =============================================================================
 * FUNCTION: verifyxy
 *
 * DESCRIPTION:
 * Function to verify x & y are within the bounds for a level If *x or *y is not
 * within the absolute bounds for a level, fix them so that they are on the
 * level. Returns TRUE if it was out of bounds, and the *x & *y in the
 * calling routine are affected.
 *
 * PARAMETERS:
 *
 *   x : The x coordinate to verify
 *
 *   y : The y coordiante to verify
 *
 * RETURN VALUE:
 *
 *   == 0 if no adjustment to x and y required.
 *   != 0 if x and/or y required adjustment.
 */
int verifyxy(int *x, int *y);

/* =============================================================================
 * FUNCTION: checkxy
 *
 * DESCRIPTION:
 * Function to check x & y are within the bounds for a level.
 *
 * PARAMETERS:
 *
 *   x : The x coordinate to check
 *
 *   y : The y coordiante to check
 *
 * RETURN VALUE:
 *
 *   == 1 if x, y is within the map bounds.
 *   == 0 if x, y is outside the map bounds.
 */
int checkxy(int x, int y);

/* =============================================================================
 * FUNCTION: createitem
 *
 * DESCRIPTION:
 * Routine to place an item next to the player.
 * When creating gold, arg is the number of gold pieces to drop, not the
 * value to be stored in iarg.
 *
 * PARAMETERS:
 *
 *   x   : The x location around which to create the object
 *
 *   y   : The y location around which to create the object
 *
 *   it  : The item to create
 *
 *   arg : The item arg.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void createitem(int x, int y, int it, int arg);

/* =============================================================================
 * FUNCTION: something
 *
 * DESCRIPTION:
 * Function to create a random item around coords x, y
 * Items are created from a designed probability for the input level.
 *
 * PARAMETERS:
 *
 *   x   : The x location around which to create the object
 *
 *   y   : The y location around which to create the object
 *
 *   lev : The dungeon level of the item
 *
 * RETURN VALUE:
 *
 *   None.
 */
void something(int x, int y, int lev);

/* =============================================================================
 * FUNCTION: newobject
 *
 * DESCRIPTION:
 * Function to return a randomly selected object to be created.
 *
 * PARAMETERS:
 *
 *   lev : The dungeon level for the item.
 *
 *   i   : This is set to the item arg.
 *
 * RETURN VALUE:
 *
 *   The item identifier of the selected item.
 */
int newobject(int lev, int *i);

/* =============================================================================
 * FUNCTION: write_levels
 *
 * DESCRIPTION:
 * Write the dungeon levels to a save file
 *
 * PARAMETERS:
 *
 *   fp : A pointer to the save file being written.
 *
 * RETURN VALUE:
 *
 *   == 0 if successful
 *   != 0 if error
 */
int write_levels(FILE *fp);

/* =============================================================================
 * FUNCTION: read_levels
 *
 * DESCRIPTION:
 * Read the dungeon levels from a save file.
 *
 * PARAMETERS:
 *
 *   fp : A pointer to the save file being read.
 *
 * RETURN VALUE:
 *
 *   == 0 if successful
 *   != 0 if error
 */
int read_levels(FILE *fp);

#endif
