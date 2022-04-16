/* =============================================================================
 * PROGRAM:  ularn
 * FILENAME: player.h
 *
 * DESCRIPTION:
 * This module contains functions to maintain information about the player
 * and to process the effects of various actions.
 *
 * =============================================================================
 * EXPORTED VARIABLES
 *
 * char_class        : The name of the player's selected class
 * class_num         : The number to identify the character class
 * ramboflag         : Is the character rambo?
 * sex               : Character's gender
 * wizard            : The wizard flag
 * cheat             : Has the player cheated
 * char_picked       : The character class letter picked
 * playerx           : The player's current x coordiante
 * playery           : The player's current y coordinate
 * lastpx            : The player's previous x coordinate
 * lastpy            : The player's previous y coordiante
 * initialtime       : The time play started
 * gtime             : The clock for the game
 * outstanding_taxes : The taxes owed from the score file
 * c                 : The character attributes array
 * cbak              : The last displayed attributes array
 * iven              : The inventory items
 * ivenarg           : The inventory item args
 * class             : The character class (level) strings
 * skill             : Experience required for each level
 * potionknown       : Array indicating potions discovered
 * spelknow          : Array indicating spells learnt by the player
 * hitflag           : Hit flags for player, used for move processing
 * hit2flag          :
 * hit3flag          :
 *
 * =============================================================================
 * EXPORTED FUNCTIONS
 *
 * identify_class  : Identify the class from the class name
 * makeplayer      : Make a new player
 * moveplayer      : Move the player
 * player_has_item : Check if the player has a particular item
 * raiselevel      : Raise experience level
 * loselevel       : Lose an experience level
 * raiseexperience : Increase experience
 * loseexperience  : Decrease experience
 * losehp          : Lose hitpoints
 * losemhp         : Lower max hitpoints
 * raisehp         : Raise hitpoints
 * raisemhp        : Increase max hitpoints
 * raisespells     : Raise spell points
 * raisemspells    : Raise max spell points
 * losespells      : Lose spell points
 * losemspells     : Decrease max spell points
 * positionplayer  : Position the player on a level
 * recalc          : Recalculate AC and WC
 * take            : Take an item
 * drop_object     : Drop an object
 * adjustivenarg   : Adjust the ivenarg for an item the player is carrying
 * enchantarmour   : Enchant armour being worn
 * enchweapon      : Enchant item being wielded
 * pocketfull      : Check if pockets are full
 * nearbymonst     : Check if the player is near a monster
 * stealsomething  : Steal an item from the player
 * emptyhanded     : Check if the player is empty handed
 * creategem       : Creat a gem by the player
 * adjustcvalues   : Adjust attributes when dropping/losing an item
 * packweight      : Get the weight of the player's inventory
 * adjust_ability  : Adjust an ability score
 * regen           : Regen player for passing of a turn
 * removecurse     : Remove curses inflicted upon player
 * adjusttime      : Adjust time base effects for the passage of time
 * write_player    : Write the player data to the save file
 * read_player     : Read the player data from the save file
 *
 * =============================================================================
 */

#ifndef __PLAYER_H
#define __PLAYER_H

#include "potion.h"
#include "scroll.h"
#include "spell.h"
#include "scores.h"

#define INNATE_TELEPORT_LEVEL 10

/*************** Defines for the character attibute array c[] ******/

typedef enum
{
  /* Player ability scores */
  STRENGTH,
  INTELLIGENCE,
  WISDOM,
  CONSTITUTION,
  DEXTERITY,
  CHARISMA,

  /* Hitpoints */
  HPMAX,
  HP,

  GOLD,
  EXPERIENCE,
  LEVEL,
  REGEN,
  WCLASS,
  AC,
  BANKACCOUNT,
  SPELLMAX,
  SPELLS,
  ENERGY,
  ECOUNTER,
  MOREDEFENSES,
  WEAR,
  PROTECTIONTIME,
  WIELD,
  AMULET,
  REGENCOUNTER,
  MOREDAM,
  DEXCOUNT,
  STRCOUNT,
  BLINDCOUNT,
  CAVELEVEL,
  CONFUSE,
  ALTPRO,
  HERO,
  CHARMCOUNT,
  INVISIBILITY,
  CANCELLATION,
  HASTESELF,
  EYEOFLARN,
  AGGRAVATE,
  GLOBE,
  TELEFLAG,
  SLAYING,
  NEGATESPIRIT,
  SCAREMONST,
  AWARENESS,
  HOLDMONST,
  TIMESTOP,
  HASTEMONST,
  CUBEofUNDEAD,
  GIANTSTR,
  FIRERESISTANCE,
  BESSMANN,
  NOTHEFT,
  HARDGAME,
  BYTESIN,
  UNUSED_55,
  UNUSED_56,
  UNUSED_57,
  UNISED_58,
  VORPAL,
  LANCEDEATH,
  SPIRITPRO,
  UNDEADPRO,
  SHIELD,
  STEALTH,
  /* Curses */
  ITCHING,
  LAUGHING,
  DRAINSTRENGTH,
  CLUMSINESS,
  INFEEBLEMENT,
  HALFDAM,
  /* More player flags */
  SEEINVISIBLE,
  FILLROOM,
  SPHCAST,
  WTW,
  STREXTRA,
  TMP,        /* Temporary work area for total str */
  LIFEPROT,   /* Number of life protections read   */
  /* Item creation flags for unique items */
  ORB,        /* Created orb of expanded awareness */
  ELVUP,      /* Created elevator up               */
  ELVDOWN,    /* Created elevator down             */
  HAND,       /* Created hand of fear              */
  CUBEUNDEAD, /* Created cube of undead control    */
  DRAGSLAY,   /* Created orb of dragon slaying     */
  NEGATE,     /* Created scarab of negate spirit   */
  URN,        /* Created golden urn                */
  LAMP,       /* Created brass lamp                */
  TALISMAN,   /* Created talisman of the sphere    */
  WAND,       /* Created wand of wonder            */
  STAFF,      /* Created staff of power            */
  DEVICE,     /* Created device of theft prevention*/
  SLASH,      /* Created sword of slashing         */
  ELVEN,      /* Created elven chain               */
  VORP,       /* Created vorpal sword              */
  SLAY,       /* Created slayer                    */
  LIFE_PRESERVER, /* Created amulet of life preservation */
  /* Misc */
  COKED,      /* Flag indicating player is coked   */
  TMP2,       /* Storage for original int before Bessmann's hammer */
  UNUSED_98,
  UNUSED_99,
  ATTRIBUTE_COUNT
} AttributeType;


#define ABILITY_FIRST STRENGTH
#define ABILITY_LAST  CHARISMA
#define ABILITY_COUNT (ABILITY_LAST - ABILITY_FIRST + 1)

#define IVENSIZE  26  /* max size of inventory */

/*
 * Exported variables
 */

extern char char_class[20];   /* character class */
extern int  class_num;        /* Number to identify the character class */

extern char ramboflag;
extern char sex;              /* default is man, 0=woman  */
extern char wizard;           /* the wizard mode flag */
extern char cheat;            /* 1 if the player has fudged save file */
extern char char_picked;      /* the character chosen */
extern int playerx, playery; /* the room on the present level of the player*/
extern int lastpx, lastpy;   /* 0 --- MAXX-1  or  0 --- MAXY-1   */

extern time_t initialtime;        /* time playing began   */
extern long gtime;              /* the clock for the game */
extern long outstanding_taxes;  /* present tax bill from score file */

extern long c[ATTRIBUTE_COUNT];    /* Character description array */
extern long cbak[ATTRIBUTE_COUNT]; /* Backup array for detecting changes */

extern char iven[IVENSIZE];     /* inventory for player */
extern short ivenarg[IVENSIZE]; /* inventory args for player  */

extern char *class[];
extern long skill[];

/*
 * Array of which potions are known to the player.
 *   0 = unknown
 *   1 = known
 */
extern int potionknown[MAXPOTION];

/*
 * Array of which scrolls are known to the player.
 *   0 = unknown
 *   1 = known
 */
extern int scrollknown[MAXSCROLL];

/*
 * Array of which spells are known to the player.
 *   0 = unknown
 *   1 = known
 */
extern int spelknow[SPELL_COUNT];

extern char hitflag;
extern char hit2flag;
extern char hit3flag;


/* =============================================================================
 * FUNCTION: identify_class
 *
 * DESCRIPTION:
 * Identify the class selection character ('a' .. 'h') from the class anme
 * string.
 *
 * PARAMETERS:
 *
 *   class_str : The class name to be identified
 *
 * RETURN VALUE:
 *
 *   The class selection character.
 */
char identify_class(char *class_str);

/* =============================================================================
 * FUNCTION: makeplayer
 *
 * DESCRIPTION:
 * Function to create the player and the players attributes.
 * This is called at the beginning of a game and at no other time
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void makeplayer (void);

/* =============================================================================
 * FUNCTION: moveplayer
 *
 * DESCRIPTION:
 * Function to move the player from one room to another.
 *
 * nomove is set to 1 to stop the next move (inadvertent monsters hitting
 * players when walking into walls) if player walks off screen or into wall
 *
 * PARAMETERS:
 *
 *   dir : The direction to move in diroff array format
 *
 * RETURN VALUE:
 *
 *   0 if can't move in that direction or hit a monster or on an object
 *   1 otherwise
 */
int moveplayer(int dir);

/* =============================================================================
 * FUNCTION: player_has_item
 *
 * DESCRIPTION:
 * Check if the player is currently carrying Item.
 *
 * PARAMETERS:
 *
 * RETURN VALUE:
 *
 *   1 if the player has the item, 0 otherwise.
 */
int player_has_item(int Item);

/* =============================================================================
 * FUNCTION: raiselevel
 *
 * DESCRIPTION:
 * Function to raise the player one level.
 * Uses the skill[] array to find level boundarys
 * Uses c[EXPERIENCE], c[LEVEL]
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void raiselevel(void);

/* =============================================================================
 * FUNCTION: loselevel
 *
 * DESCRIPTION:
 * Function to lower the players character level by one
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void loselevel(void);

/* =============================================================================
 * FUNCTION: raiseexperience
 *
 * DESCRIPTION:
 * Function to increase experience points
 *
 * PARAMETERS:
 *
 *   x : The amount of experience earned.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void raiseexperience (long x);

/* =============================================================================
 * FUNCTION: loseexperience
 *
 * DESCRIPTION:
 * Function to lose experience points
 *
 * PARAMETERS:
 *
 *   x : The amount of experience lost
 *
 * RETURN VALUE:
 *
 *   None.
 */
void loseexperience (long x);

/* =============================================================================
 * FUNCTION: losehp
 *
 * DESCRIPTION:
 * Function to remove hit points from the player
 * warning -- will kill player if hp goes to zero
 *
 * PARAMETERS:
 *
 *   Reason : The reason for the HP loss.
 *
 *   x      : The number of hitpoints to lose.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void losehp (DiedReasonType Reason, int x);

/* =============================================================================
 * FUNCTION: losemhp
 *
 * DESCRIPTION:
 * Reduce max hitpoints (won't kill player)
 *
 * PARAMETERS:
 *
 *   x : The number of max HP to lose.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void losemhp (int x);

/* =============================================================================
 * FUNCTION: raisehp
 *
 * DESCRIPTION:
 * Function to gain hitpoints.
 * This will never make HP exceed Max HP.
 *
 * PARAMETERS:
 *
 *   x : The number of HP to gain.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void raisehp (int x);

/* =============================================================================
 * FUNCTION: raisemhp
 *
 * DESCRIPTION:
 * Function to increase Max HP.
 *
 * PARAMETERS:
 *
 *   x : The amount to increaese Max HP.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void raisemhp (int x);

/* =============================================================================
 * FUNCTION: raisespells
 *
 * DESCRIPTION:
 * Function to gain spell points.
 * The will never incerase spell points beyon Max spell points.
 *
 * PARAMETERS:
 *
 *   x : The number of spell points to gain.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void raisespells (int x);

/* =============================================================================
 * FUNCTION: raisemspells
 *
 * DESCRIPTION:
 * Function to increase max spell points.
 *
 * PARAMETERS:
 *
 *   x : The amount to increase max spell points.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void raisemspells (int x);

/* =============================================================================
 * FUNCTION: losespells
 *
 * DESCRIPTION:
 * Function to decrease the number of spell points.
 *
 * PARAMETERS:
 *
 *   x : The number of spell points to lose.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void losespells (int x);

/* =============================================================================
 * FUNCTION: losemspells
 *
 * DESCRIPTION:
 * Function to decrease the maximum number of spell points.
 *
 * PARAMETERS:
 *
 *   x : The amount to reduce the max spell points.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void losemspells (int x);

/* =============================================================================
 * FUNCTION: positionplayer
 *
 * DESCRIPTION:
 * Function to be sure player is not in a wall.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void positionplayer (void);

/* =============================================================================
 * FUNCTION: recalc
 *
 * DESCRIPTION:
 * Function to recalculate the armor class and wield class of the player.
 * Also sets the LANCEDEATH attribute according to the currently wielded
 * item.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void recalc (void);

/* =============================================================================
 * FUNCTION: take
 *
 * DESCRIPTION:
 * Function to put something in the players inventory
 *
 * PARAMETERS:
 *
 *   itm : The item to put iin theplayer's inventory.
 *
 *   arg : THe item's arg.
 *
 * RETURN VALUE:
 *
 *   0 if success, 1 if a failure
 */
int take (int itm, int arg);

/* =============================================================================
 * FUNCTION: drop_object
 *
 * DESCRIPTION:
 * Function to drop an object
 *
 * PARAMETERS:
 *
 *   k : The index into iven list for the object to drop
 *
 * RETURN VALUE:
 *
 *   1 if something there already else 0
 */
int drop_object (int k);

/* =============================================================================
 * FUNCTION: adjustivenarg
 *
 * DESCRIPTION:
 * Function to adjust the inventory argument by the amount spicified, updating
 * characters attributes as required.
 *
 * PARAMETERS:
 *
 *   Idx    : The item index of the item being adjusted
 *
 *   Amount : The amount to change the attruibute
 *
 * RETURN VALUE:
 *
 *   None.
 */
void adjustivenarg(int Idx, int Amount);

/* =============================================================================
 * FUNCTION: enchantarmor
 *
 * DESCRIPTION:
 * Function to enchant armor player is currently wearing.
 *
 * PARAMETERS:
 *
 *   how : The cause of the enchantment (altar or scroll)
 *
 * RETURN VALUE:
 *
 *   None.
 */
void enchantarmor (int how);

/* =============================================================================
 * FUNCTION: enchweapon
 *
 * DESCRIPTION:
 * Function to enchant a weapon presently being wielded
 *
 * PARAMETERS:
 *
 *   how : The cause of the enchantment (altar or scroll)
 * RETURN VALUE:
 *
 *   None.
 */
void enchweapon (int how);

/* =============================================================================
 * FUNCTION: pocketfull
 *
 * DESCRIPTION:
 * Function to tell if player can carry one more thing
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   1 if pockets are full, else 0
 */
int pocketfull (void);

/* =============================================================================
 * FUNCTION: nearbymonst
 *
 * DESCRIPTION:
 * Function to check if there is a monster next to the player.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   1 if a monster is next to the player else returns 0
 */
int nearbymonst (void);

/* =============================================================================
 * FUNCTION: stealsomething
 *
 * DESCRIPTION:
 * Function to steal an item from the players pockets
 *
 * PARAMETERS:
 *
 *   x : The x coordinate of the monster stealing the item
 *
 *   y : The y coordinate of the monster stealing the item
 *
 * RETURN VALUE:
 *
 *   1 if steals something else returns 0
 */
int stealsomething (int x, int y);

/* =============================================================================
 * FUNCTION: emptyhanded
 *
 * DESCRIPTION:
 * Function to check if the player is carrying nothing.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   1 if player carrys nothing else return 0
 */
int emptyhanded (void);

/* =============================================================================
 * FUNCTION: creategem
 *
 * DESCRIPTION:
 * Function to create a gem on a square near the player
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void creategem (void);

/* =============================================================================
 * FUNCTION: adjustcvalues
 *
 * DESCRIPTION:
 * Function to change character attributes as needed when dropping an object
 * that affects these attributes.
 *
 * PARAMETERS:
 *
 *   itm : The item being dropped
 *
 *   arg : The item arg.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void adjustcvalues (int itm, int arg);

/* =============================================================================
 * FUNCTION: packweight
 *
 * DESCRIPTION:
 * Function to calculate the pack weight of the player
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   the number of pounds the player is carrying
 */
int packweight (void);

/* =============================================================================
 * FUNCTION: adjust_ability
 *
 * DESCRIPTION:
 * Adjust an ability socre of the player, sticky ending to 3 if the adjustment
 * would reduce the score below 3.
 *
 * PARAMETERS:
 *
 *   ability : The ability to adjust.
 *
 *   amount  : The amount to change the abiity
 *
 * RETURN VALUE:
 *
 *   None.
 */
void adjust_ability(AttributeType ability, int amount);

/* =============================================================================
 * FUNCTION: regen
 *
 * DESCRIPTION:
 * Function to regenerate player HP and spells, and to process time base
 * effects.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void regen(void);

/* =============================================================================
 * FUNCTION: removecurse
 *
 * DESCRIPTION:
 * Cure the player of curses.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void removecurse (void);

/* =============================================================================
 * FUNCTION: adjusttime
 *
 * DESCRIPTION:
 * Function to adjust time when time warping and taking courses in school
 *
 * PARAMETERS:
 *
 *   tim : The amount of time adjustment.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void adjusttime(long tim);

/* =============================================================================
 * FUNCTION: write_player
 *
 * DESCRIPTION:
 * Function to write the player data to a file
 *
 * PARAMETERS:
 *
 *   fp : A pointer to the save file being written.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void write_player(FILE *fp);

/* =============================================================================
 * FUNCTION: read_player
 *
 * DESCRIPTION:
 * Function to read the player data from a file
 *
 * PARAMETERS:
 *
 *  fp : A pointer to the save file being read.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void read_player(FILE *fp);

#endif
