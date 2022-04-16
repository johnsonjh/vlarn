/* =============================================================================
 * PROGRAM:  ularn
 * FILENAME: monster.h
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

#ifndef __MONSTER_H
# define __MONSTER_H

# include "dungeon.h"

# define MAXMONST 57 /* maximum # monsters in the dungeon  */

struct monst {
  char *name;
  char level;
  char armorclass;
  char damage;
  char attack;
  char intelligence;
  short gold;
  short hitpoints;
  long experience;
  long flags;
};

/*
 *  Flags for monst structure
 */
# define FL_NONE 0x00 /* Nothing of interest                     */
# define FL_GENOCIDED 0x01 /* Monster has been genocided              */
# define FL_HEAD 0x02 /* Monster has a head                      */
# define FL_NOBEHEAD 0x04 /* Monster cannot be beheaded by Vorpy     */
# define FL_SLOW 0x08 /* Monster moves at 1/2 nornal speed       */
# define FL_FLY 0x10 /* Monster can fly over pits and trapdoors */
# define FL_SPIRIT 0x20 /* Is a spirit (affected by spirit prot)   */
# define FL_UNDEAD 0x40 /* Is undead (affected by undead prot)     */
# define FL_INFRAVIS 0x80 /* Monster has infravision (see invisible) */

/************* Defines for the monsters as objects *************/

typedef enum {
  MONST_NONE,
  LEMMING,
  GNOME,
  HOBGOBLIN,
  JACKAL,
  KOBOLD,
  ORC,
  SNAKE,
  CENTIPEDE,
  JACULI,
  TROGLODYTE,
  ANT,
  EYE,
  LEPRECHAUN,
  NYMPH,
  QUASIT,
  RUSTMONSTER,
  ZOMBIE,
  ASSASSINBUG,
  BUGBEAR,
  HELLHOUND,
  ICELIZARD,
  CENTAUR,
  TROLL,
  YETI,
  WHITEDRAGON,
  ELF,
  CUBE,
  METAMORPH,
  VORTEX,
  ZILLER,
  VIOLETFUNGI,
  WRAITH,
  FORVALAKA,
  LAMANOBE,
  OSEQUIP,
  ROTHE,
  XORN,
  VAMPIRE,
  INVISIBLESTALKER,
  POLTERGEIST,
  DISENCHANTRESS,
  SHAMBLINGMOUND,
  YELLOWMOLD,
  UMBERHULK,
  GNOMEKING,
  MIMIC,
  WATERLORD,
  BRONZEDRAGON,
  GREENDRAGON,
  PURPLEWORM,
  XVART,
  SPIRITNAGA,
  SILVERDRAGON,
  PLATINUMDRAGON,
  GREENURCHIN,
  REDDRAGON,
  DEMONLORD,
  DEMONLORD_II,
  DEMONLORD_III,
  DEMONLORD_IV,
  DEMONLORD_V,
  DEMONLORD_VI,
  DEMONLORD_VII,
  DEMONPRINCE,
  LUCIFER,
  MONST_COUNT
} MonsterIdType;

/*
 * Character codes to use for monsters
 */
extern char monstnamelist[MONST_COUNT];

/*
 * Tile numbers to use for monsters
 */
extern int monsttilelist[MONST_COUNT];

/*
 * The monster data
 */
extern struct monst monster[MONST_COUNT];

/*
 * Name of the current monster
 */
extern char lastmonst[40];

/* number of the last monster hitting the player */
extern MonsterIdType last_monst_id;

extern int last_monst_hx; /* x location of the last monster hit by player */
extern int last_monst_hy; /* y location of the last monster hit by player */

extern char rmst; /* Random monster creation timer */

/* =============================================================================
 * FUNCTION: createmonster
 *
 * DESCRIPTION:
 * Function to create a monster next to the player.
 *
 * PARAMETERS:
 *
 *   mon : The id of the monster to create.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void createmonster(MonsterIdType mon);

/* =============================================================================
 * FUNCTION: mon_has_item
 *
 * DESCRIPTION:
 * This function returns true if the monster at location x, y has Item
 * in its inventory.
 * If there is no monster at x, y then the function returns false.
 *
 * PARAMETERS:
 *
 *   x    : The x location to check
 *
 *   y    : The y location to check
 *
 *   Item : The item to check for.
 *
 * RETURN VALUE:
 *
 *   true if the monster at (x, y) has Item, false otherwise.
 */
int mon_has_item(int x, int y, int Item);

/* =============================================================================
 * FUNCTION: fullhit
 *
 * DESCRIPTION:
 * Function to return hp damage to monster due to a number of full hits.
 *
 * PARAMETERS:
 *
 *   xx : The number of full hits.
 *
 * RETURN VALUE:
 *
 *   The total damage done by xx full hits.
 */
int fullhit(int xx);

/* =============================================================================
 * FUNCTION: ifblind
 *
 * DESCRIPTION:
 * Subroutine to copy the word "monster" into lastmonst if the player is blind,
 * or the monster name if not.
 *
 * PARAMETERS:
 *
 *   x : The x location of the monster
 *
 *   y : The y location of the monster
 *
 * RETURN VALUE:
 *
 *   None.
 */
void ifblind(int x, int y);

/* =============================================================================
 * FUNCTION: hitmonster
 *
 * DESCRIPTION:
 * Function to hit a monster at the designated coordinates.
 * This routine is used for a bash & slash type attack on a monster.
 *
 * PARAMETERS:
 *
 *   x : The x location of the monster
 *
 *   y : The y location of the monster
 *
 * RETURN VALUE:
 *
 *   None
 */
void hitmonster(int x, int y);

/* =============================================================================
 * FUNCTION: hitm
 *
 * DESCRIPTION:
 * Function to just hit a monster at a given coordinates.
 * This routine is used to specifically damage a monster at a location.
 * The function handles damage from both weapons and spells.
 *
 * PARAMETERS:
 *
 *   x   : The x location of the monster.
 *
 *   y   : The y location of the monster.
 *
 *   amt : The amount of damage being dealt.
 *
 *   SpellFlag : This flag is to be set to true if the damage is from a spell.
 *
 * RETURN VALUE:
 *
 *   Returns the number of hitpoints the monster absorbed.
 */
int hitm(int x, int y, int amt, int SpellFlag);

/* =============================================================================
 * FUNCTION: hitplayer
 *
 * DESCRIPTION:
 * Function for the monster at the designated location to hit the player.
 *
 * PARAMETERS:
 *
 *   x   : The x location of the monster.
 *
 *   y   : THe y location of the monster.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void hitplayer(int x, int y);

/* =============================================================================
 * FUNCTION: makemonst
 *
 * DESCRIPTION:
 * Function to return monster number for a randomly selected monster for the
 * given cave level.
 *
 * PARAMETERS:
 *
 *   lev : The cave level for the monster
 *
 * RETURN VALUE:
 *
 *   None.
 */
int makemonst(int lev);

/* =============================================================================
 * FUNCTION: randmonst
 *
 * DESCRIPTION:
 * Function to randomly create monsters if needed.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void randmonst(void);

/* =============================================================================
 * FUNCTION: teleportmonst
 *
 * DESCRIPTION:
 * Function to teleport a monster.
 *
 * PARAMETERS:
 *
 *   xx    : The x location of the monster
 *
 *   yy    : The y location of the monster
 *
 *   monst : The monster id.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void teleportmonst(int xx, int yy, int monst);

/* =============================================================================
 * FUNCTION: movemonst
 *
 * DESCRIPTION:
 * This routine has the responsibility to determine which monsters are to
 * move, and to perform the movement.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void movemonst(void);

/* =============================================================================
 * FUNCTION: parse2
 *
 * DESCRIPTION:
 * Move things while player is out of action.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void parse2(void);

/* =============================================================================
 * FUNCTION: write_monster_data
 *
 * DESCRIPTION:
 * Function to write the monster data to the save file.
 *
 * PARAMETERS:
 *
 *   fp : A pointer to the save file being written.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void write_monster_data(FILE *fp);

/* =============================================================================
 * FUNCTION: read_monster_data
 *
 * DESCRIPTION:
 * Function to read the monster data from the save file.
 *
 * PARAMETERS:
 *
 *   fp : A pointer to the save file being read.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void read_monster_data(FILE *fp);

#endif
