/* =============================================================================
 * PROGRAM:  ularn
 * FILENAME: player.c
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

#include "ularn_game.h"
#include "ularn_win.h"
#include "ularn_ask.h"
#include "header.h"
#include "saveutils.h"
#include "scores.h"
#include "player.h"
#include "monster.h"
#include "dungeon.h"
#include "dungeon_obj.h"
#include "scroll.h"
#include "potion.h"
#include "spell.h"
#include "show.h"
#include "itm.h"

/* =============================================================================
 * Exported variables
 */

/*
 * class[c[LEVEL]-1] gives the correct name of the player's experience level
 */
static char aa1[] = " mighty evil master";
static char aa2[] = "apprentice demi-god";
static char aa3[] = "  minor demi-god   ";
static char aa4[] = "  major demi-god   ";
static char aa5[] = "    minor deity    ";
static char aa6[] = "    major deity    ";
static char aa7[] = "  novice guardian  ";
static char aa8[] = "apprentice guardian";
static char aa9[] = "    The Creator    ";

char *class[] =
{
	"  novice explorer  ", "apprentice explorer", " practiced explorer",    /*  -3*/
	"  expert explorer  ", " novice adventurer ", "     adventurer    ",    /*  -6*/
	"apprentice conjurer", "     conjurer      ", "  master conjurer  ",    /*  -9*/
	"  apprentice mage  ", "       mage        ", " experienced mage  ",    /* -12*/
	"    master mage    ", " apprentice warlord", "  novice warlord   ",    /* -15*/
	"  expert warlord   ", "   master warlord  ", " apprentice gorgon ",    /* -18*/
	"      gorgon       ", "  practiced gorgon ", "   master gorgon   ",    /* -21*/
	"    demi-gorgon    ", "    evil master    ", " great evil master ",    /* -24*/
	aa1,		       aa1,   aa1,                                      /* -27*/
	aa1,		       aa1,   aa1,                                      /* -30*/
	aa1,		       aa1,   aa1,                                      /* -33*/
	aa1,		       aa1,   aa1,                                      /* -36*/
	aa1,		       aa1,   aa1,                                      /* -39*/
	aa2,		       aa2,   aa2,                                      /* -42*/
	aa2,		       aa2,   aa2,                                      /* -45*/
	aa2,		       aa2,   aa2,                                      /* -48*/
	aa3,		       aa3,   aa3,                                      /* -51*/
	aa3,		       aa3,   aa3,                                      /* -54*/
	aa3,		       aa3,   aa3,                                      /* -57*/
	aa4,		       aa4,   aa4,                                      /* -60*/
	aa4,		       aa4,   aa4,                                      /* -63*/
	aa4,		       aa4,   aa4,                                      /* -66*/
	aa5,		       aa5,   aa5,                                      /* -69*/
	aa5,		       aa5,   aa5,                                      /* -72*/
	aa5,		       aa5,   aa5,                                      /* -75*/
	aa6,		       aa6,   aa6,                                      /* -78*/
	aa6,		       aa6,   aa6,                                      /* -81*/
	aa6,		       aa6,   aa6,                                      /* -84*/
	aa7,		       aa7,   aa7,                                      /* -87*/
	aa8,		       aa8,   aa8,                                      /* -90*/
	aa8,		       aa8,   aa8,                                      /* -93*/
	"  earth guardian   ", "   air guardian    ", "   fire guardian   ",    /* -96*/
	"  water guardian   ", "   time guardian   ", " ethereal guardian ",    /* -99*/
	aa9,		       aa9,   aa9,                                      /* -102*/
};

/*
 * table of experience needed to be a certain level of player
 * skill[c[LEVEL]] is the experience required to attain the next level
 */
#define MEG 1000000
long skill[] = {
	0,	   10,	      20,	 40,	    80,	       160,	  320,	     640,      1280, 2560, 5120,        /*  1-11 */
	10240,	   20480,     40960,	 100000,    200000,    400000,	  700000,    1 * MEG,                           /* 12-19 */
	2 * MEG,   3 * MEG,   4 * MEG,	 5 * MEG,   6 * MEG,   8 * MEG,	  10 * MEG,                                     /* 20-26 */
	12 * MEG,  14 * MEG,  16 * MEG,	 18 * MEG,  20 * MEG,  22 * MEG,  24 * MEG,  26 * MEG, 28 * MEG,                /* 27-35 */
	30 * MEG,  32 * MEG,  34 * MEG,	 36 * MEG,  38 * MEG,  40 * MEG,  42 * MEG,  44 * MEG, 46 * MEG,                /* 36-44 */
	48 * MEG,  50 * MEG,  52 * MEG,	 54 * MEG,  56 * MEG,  58 * MEG,  60 * MEG,  62 * MEG, 64 * MEG,                /* 45-53 */
	66 * MEG,  68 * MEG,  70 * MEG,	 72 * MEG,  74 * MEG,  76 * MEG,  78 * MEG,  80 * MEG, 82 * MEG,                /* 54-62 */
	84 * MEG,  86 * MEG,  88 * MEG,	 90 * MEG,  92 * MEG,  94 * MEG,  96 * MEG,  98 * MEG, 100 * MEG,               /* 63-71 */
	105 * MEG, 110 * MEG, 115 * MEG, 120 * MEG, 125 * MEG, 130 * MEG, 135 * MEG, 140 * MEG,                         /* 72-79 */
	145 * MEG, 150 * MEG, 155 * MEG, 160 * MEG, 165 * MEG, 170 * MEG, 175 * MEG, 180 * MEG,                         /* 80-87 */
	185 * MEG, 190 * MEG, 195 * MEG, 200 * MEG, 210 * MEG, 220 * MEG, 230 * MEG, 240 * MEG,                         /* 88-95 */
	260 * MEG, 280 * MEG, 300 * MEG, 320 * MEG, 340 * MEG, 370 * MEG                                                /* 96-101*/
};
#undef MEG


/*
 * Exported variables
 */

char hitflag = 0;       /* flag for if player has been hit when running */
char hit2flag = 0;      /* flag for if player has been hit when running */
char hit3flag = 0;      /* flag for if player has been hit flush input*/

char char_class[20];    /* character class */
int class_num;          /* character class number */

char ramboflag = 0;
char sex = 1;                   /* default is man, 0=woman  */
char wizard = 0;                /* the wizard mode flag */
char cheat = 0;                 /* 1 if the player has fudged save file */
char char_picked;               /* the character chosen */
int playerx, playery;           /* the room on the present level of the player*/
int lastpx, lastpy;             /* 0 --- MAXX-1  or  0 --- MAXY-1   */

time_t initialtime = 0;         /* time playing began   */
long gtime = 0;                 /* the clock for the game */
long outstanding_taxes = 0;     /* present tax bill from score file */

long c[ATTRIBUTE_COUNT];        /* Character description array */
long cbak[ATTRIBUTE_COUNT];     /* Backup array for detecting changes */

char iven[IVENSIZE];            /* inventory for player */
short ivenarg[IVENSIZE];        /* inventory args for player  */

int potionknown[MAXPOTION];

int scrollknown[MAXSCROLL];

int spelknow[SPELL_COUNT];

/* =============================================================================
 * Local variables
 */

#define MAXPLEVEL 100   /* maximum player level allowed   */

/*
 * Haste step to keep track of frations of moves when hasted
 */
static int HasteStep = 0;

/*
 * Character fields affected by the passage of time
 */
#define TIME_CHANGED_COUNT 28
static AttributeType time_change[TIME_CHANGED_COUNT] =
{
	HERO,
	ALTPRO,
	PROTECTIONTIME,
	DEXCOUNT,
	STRCOUNT,
	GIANTSTR,
	CHARMCOUNT,
	INVISIBILITY,
	CANCELLATION,
	HASTESELF,
	AGGRAVATE,
	SCAREMONST,
	STEALTH,
	AWARENESS,
	HOLDMONST,
	HASTEMONST,
	FIRERESISTANCE,
	GLOBE,
	SPIRITPRO,
	UNDEADPRO,
	HALFDAM,
	SEEINVISIBLE,
	ITCHING,
	CLUMSINESS,
	WTW,
	COKED,
	BLINDCOUNT,
	CONFUSE
};

/*
 * Character fields for curses
 */
#define CURSE_COUNT 10
static AttributeType curse[CURSE_COUNT] =
{
	BLINDCOUNT,
	CONFUSE,
	AGGRAVATE,
	HASTEMONST,
	ITCHING,
	LAUGHING,
	DRAINSTRENGTH,
	CLUMSINESS,
	INFEEBLEMENT,
	HALFDAM
};

#define MAX_CLASSES        8
#define MAX_INITIAL_SPELLS 2
#define MAX_INITIAL_ITEMS  3

struct InitialClassDataType {
	char *ClassName;
	long Attr[6];
	int InitialSP;
	int InitialHP;
	SpellType KnownSpells[MAX_INITIAL_SPELLS];
	int Item[MAX_INITIAL_ITEMS];
	int ItemArg[MAX_INITIAL_ITEMS];
	int WearItem;
	int WieldItem;
};

static struct InitialClassDataType InitialClassData[MAX_CLASSES] =
{
	{
		"Ogre",                                         /* Class name */
		{ 18,		4,	  6,  16, 6,  4	 },     /* STR, INT, WIS, CON, DEX, CHA */
		1, 16,                                          /* SP, HP */
		{ SPELL_MLE,	SPELL_COUNT },                  /* Spells */
		{ OPOTION,	OPOTION,  ONOTHING },           /* Items  */
		{ -6,		-6,	  0 },                  /* Item args */
		-1, -1                                          /* Wear, Wield*/
	},
	{
		"Wizard",                                       /* Class name */
		{ 8,		16,	  16, 6,  6,  8	 },     /* STR, INT, WIS, CON, DEX, CHA */
		2, 8,                                           /* SP, HP */
		{ SPELL_MLE,	SPELL_CHM },                    /* Spells */
		{ OPOTION,	OSCROLL,  OSCROLL },            /* Items  */
		{ PTREASURE,	-6,	  -6 },                 /* Item args */
		-1, -1                                          /* Wear, Wield */
	},
	{
		"Klingon",                                      /* Class name */
		{ 14,		12,	  4,  12, 8,  3	 },     /* STR, INT, WIS, CON, DEX, CHA */
		1, 14,                                          /* SP, HP */
		{ SPELL_SSP,	SPELL_COUNT },                  /* Spells */
		{ OSTUDLEATHER, OPOTION,  ONOTHING },           /* Items  */
		{ 0,		-6,	  0 },                  /* Item args */
		0, -1                                           /* Wear, Wield */
	},
	{
		"Elf",                                          /* Class name */
		{ 8,		14,	  12, 8,  8,  14 },     /* STR, INT, WIS, CON, DEX, CHA */
		2, 8,                                           /* SP, HP */
		{ SPELL_MLE,	SPELL_COUNT },                  /* Spells */
		{ OLEATHER,	OSCROLL,  ONOTHING },           /* Items  */
		{ 0,		-6,	  0 },                  /* Item args */
		0, -1                                           /* Wear, Wield */
	},
	{
		"Rogue",                                        /* Class name */
		{ 8,		12,	  8,  10, 14, 6	 },     /* STR, INT, WIS, CON, DEX, CHA */
		1, 12,                                          /* SP, HP */
		{ SPELL_MLE,	SPELL_COUNT },                  /* Spells */
		{ OLEATHER,	ODAGGER,  OSCROLL },            /* Items  */
		{ 0,		0,	  SSTEALTH },           /* Item args */
		0, 1                                            /* Wield, wear */
	},
	{
		"Adventurer",                                   /* Class name */
		{ 12,		12,	  12, 12, 12, 12 },     /* STR, INT, WIS, CON, DEX, CHA */
		1, 10,                                          /* SP, HP */
		{ SPELL_PRO,	SPELL_MLE },                    /* Spells */
		{ OLEATHER,	ODAGGER,  ONOTHING },           /* Items  */
		{ 0,		0,	  0 },                  /* Item args */
		0, 1                                            /* Wear, Wield */
	},
	{
		"Dwarf",                                        /* Class name */
		{ 16,		6,	  8,  16, 4,  4	 },     /* STR, INT, WIS, CON, DEX, CHA */
		1, 12,                                          /* SP, HP */
		{ SPELL_PRO,	SPELL_COUNT },                  /* Spells */
		{ OSPEAR,	ONOTHING, ONOTHING },           /* Items  */
		{ 0,		0,	  0 },                  /* Item args */
		-1, 0                                           /* Wear, Wield */
	},
	{
		"Rambo",                                        /* Class name */
		{ 3,		3,	  3,  3,  3,  3	 },     /* STR, INT, WIS, CON, DEX, CHA */
		0, 1,                                           /* SP, HP */
		{ SPELL_MLE,	SPELL_COUNT },                  /* Spells */
		{ OLANCE,	ONOTHING, ONOTHING },           /* Items  */
		{ 0,		0,	  0 },                  /* Item args */
		-1, 0                                           /* Wear, Wield */
	}
};

/* =============================================================================
 * Local functions
 */

/* =============================================================================
 * FUNCTION: pick_char
 *
 * DESCRIPTION:
 * Pick the player's character class and set initial attributes and inventory.
 *
 * PARAMETERS:
 *
 *   foo : This is the character selection specified in the ularn opts file.
 *         If no character is specified then this should be set to 0.
 *         If a character is selected then this should be set to the key
 *         selection for the character ('a' .. 'h').
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void pick_char(int foo)
{
	int i;
	int j;

	set_display(DISPLAY_TEXT);

	nosignal = 1; /* disable signals */

	if (foo == 0) {
		ClearText();
		MoveCursor(29, 1);
		Standout("The Addiction of VLarn\n\n");
		Print("Pick a character class...\n\n");

		Print("\ta)  Ogre       - Exceptional strength, but thick as a brick\n");
		Print("\n");
		Print("\tb)  Wizard     - Smart, good at magic, but very weak\n");
		Print("\n");
		Print("\tc)  Klingon    - Strong and average IQ, but unwise & very ugly\n");
		Print("\n");
		Print("\td)  Elf        - OK at magic, but a mediocre fighter\n");
		Print("\n");
		Print("\te)  Rogue      - Nimble and smart, but only average strength\n");
		Print("\n");
		Print("\tf)  Adventurer - Jack of all trades, master of none\n");
		Print("\n");
		Print("\tg)  Dwarf      - Strong and healthy, but not good at magic\n");
		Print("\n");
		Print("\th)  Rambo      - A special challenge.\n");
		Print("\t                 Bad at everything, but has a lance of death");

		MoveCursor(1, 24);
		i = get_prompt_input("So, what are ya? ", "abcdefgh", 1);

	}else {
		//
		// Character selected in ops
		//
		i = foo;
		c[SHIELD] = c[WEAR] = c[WIELD] = -1;
	}

	i = i - 'a';
	class_num = i;

	strcpy(char_class, InitialClassData[i].ClassName);
	c[SPELLMAX] = InitialClassData[i].InitialSP;
	c[SPELLS] = InitialClassData[i].InitialSP;
	c[HPMAX] = InitialClassData[i].InitialHP;
	c[HP] = InitialClassData[i].InitialHP;;

	for (j = ABILITY_FIRST; j <= ABILITY_LAST; j++)
		c[j] = InitialClassData[i].Attr[j - ABILITY_FIRST];

	for (j = 0; j < MAX_INITIAL_SPELLS; j++)
		if (InitialClassData[i].KnownSpells[j] < SPELL_COUNT)
			spelknow[InitialClassData[i].KnownSpells[j]] = 1;

	for (j = 0; j < MAX_INITIAL_ITEMS; j++) {
		iven[j] = (char)InitialClassData[i].Item[j];

		if (iven[j] == OLANCE) {
			c[LANCEDEATH] = 1;
			ramboflag = 1;
		}

		if (InitialClassData[i].ItemArg[j] >= 0)
			ivenarg[j] = (short)InitialClassData[i].ItemArg[j];
		else
			ivenarg[j] = (short)rund(-InitialClassData[i].ItemArg[j]);
	}

	c[WEAR] = InitialClassData[i].WearItem;
	c[WIELD] = InitialClassData[i].WieldItem;

	nosignal = 0;

} /* end pick_char */


/* =============================================================================
 * Exported functions
 */


/* =============================================================================
 * FUNCTION: identify_class
 */
char identify_class(char *class_str)
{
	int cls;
	int found;
	int i;
	int len;
	char class_cmp_str[20];

	if (class_str == NULL)
		/* no class string specified */
		return 0;

	len = strlen(class_str);
	for (i = 0; i < len; i++)
		class_str[i] = (char)tolower(class_str[i]);

	cls = 0;
	found = 0;

	while ((cls < MAX_CLASSES) && (!found)) {
		strcpy(class_cmp_str, InitialClassData[cls].ClassName);

		len = strlen(class_cmp_str);
		for (i = 0; i < len; i++)
			class_cmp_str[i] = (char)tolower(class_cmp_str[i]);

		if (strcmp(class_str, class_cmp_str) == 0)
			found = 1;
		else
			cls++;

	}

	if (cls < MAX_CLASSES)
		return (char)('a' + cls);
	else
		return 0;
}

/* =============================================================================
 * FUNCTION: makeplayer
 */
void makeplayer(void)
{
	int i;

	/*
	 * Clear the player's attributes
	 */
	for (i = 0; i < ATTRIBUTE_COUNT; i++) {
		c[i] = 0;
		cbak[i] = 0;
	}

	/*
	 * Clear the player's inventory
	 */
	for (i = 0; i < IVENSIZE; i++) {
		iven[i] = ONOTHING;
		ivenarg[i] = 0;
	}

	/* Clear spells, potions and scrolls known */

	for (i = 0; i < MAXPOTION; i++)
		potionknown[i] = 0;

	for (i = 0; i < MAXSCROLL; i++)
		scrollknown[i] = 0;

	for (i = 0; i < SPELL_COUNT; i++)
		spelknow[i] = 0;

	c[LEVEL] = 1;           /* player starts at level one   */
	c[REGENCOUNTER] = 16;
	c[ECOUNTER] = 96;       /* start regeneration correctly */

	c[SHIELD] = -1;
	c[WEAR] = -1;
	c[WIELD] = -1;

	if (char_picked >= 'a' && char_picked <= 'h')
		/* Class specified on command line */
		pick_char(char_picked);
	else
		/* Ask for the class */
		pick_char(0);

	/*
	 * Position the player in the home level
	 */
	playerx = (char)rnd(MAXX - 2);
	playery = (char)rnd(MAXY - 2);
	lastpx = 0;
	lastpy = 0;

	/*  time clock starts at zero */
	gtime = 0;
	cbak[SPELLS] = -50;
	recalc();
}

/* =============================================================================
 * FUNCTION: moveplayer
 */
int moveplayer(int dir)
{
	int k, m, i, j;

	if (c[CONFUSE]) {
		if (c[LEVEL] < rnd(30)) {
			dir = rund(9); /*if confused any dir*/
		}
	}

	k = playerx + diroffx[dir];
	m = playery + diroffy[dir];

	if ((k < 0) || (k >= MAXX) || (m < 0) || (m >= MAXY)) {
		nomove = 1;
		yrepcount = 0;
		return 0;
	}

	i = item[k][m];
	j = mitem[k][m].mon;

	/*  hit a wall  */
	if ((i == OWALL) && (c[WTW] == 0)) {
		nomove = 1;
		yrepcount = 0;
		return 0;
	}

	/* Can't move onto closed doors if using the enhanced interface */
	if ((enhance_interface) && (i == OCLOSEDDOOR)) {
		nomove = 1;
		yrepcount = 0;
		return 0;
	}

	if ((k == 33) && (m == MAXY - 1) && (level == 1)) {
		newcavelevel(0);

		/*
		 * Returning to the home level from the dungeon, so find the
		 * dungeon entrance and locate the player at the dungeon entrance.
		 */
		for (k = 0; k < MAXX; k++) {
			for (m = 0; m < MAXY; m++) {
				if (item[k][m] == OENTRANCE) {
					playerx = (char)k;
					playery = (char)m;
					positionplayer();
					drawscreen();
					return 0;
				}
			}
		}

	}

	if (j > 0) {
		hitmonster(k, m);
		yrepcount = 0;
		return 0;
	} /* hit a monster*/

	lastpx = playerx;
	lastpy = playery;
	playerx = (char)k;
	playery = (char)m;

	if ((i != ONOTHING) &&
	    (i != OTRAPARROWIV) &&
	    (i != OIVTELETRAP) &&
	    (i != OIVDARTRAP) &&
	    (i != OIVTRAPDOOR)) {
		yrepcount = 0;
		return 0;
	}else
		return 1;
}

/* =============================================================================
 * FUNCTION: player_has_item
 */
int player_has_item(int Item)
{
	int i;
	int have_item;

	have_item = 0;
	for (i = 0; i < IVENSIZE; i++)
		if (iven[i] == Item)
			have_item = 1;

	return have_item;
}

/* =============================================================================
 * FUNCTION: raiselevel
 */
void raiselevel(void)
{
	if (c[LEVEL] < MAXPLEVEL)
		raiseexperience((long)(skill[c[LEVEL]] - c[EXPERIENCE]));
}

/* =============================================================================
 * FUNCTION: loselevel
 */
void loselevel(void)
{
	if (c[LEVEL] > 1)
		loseexperience((long)(c[EXPERIENCE] - skill[c[LEVEL] - 1] + 1));
}

/* =============================================================================
 * FUNCTION: raiseexperience
 */
void raiseexperience(long x)
{
	int i, tmp;

	i = c[LEVEL];
	c[EXPERIENCE] += x;
	while (c[EXPERIENCE] >= skill[c[LEVEL]] && (c[LEVEL] < MAXPLEVEL)) {
		tmp = (c[CONSTITUTION] - c[HARDGAME]) >> 1;
		c[LEVEL]++;
		raisemhp((int)(rnd(3) + rnd((tmp > 0)?tmp:1)));
		raisemspells((int)rund(3));
		if (c[LEVEL] < 7 - c[HARDGAME])
			raisemhp((int)(c[CONSTITUTION] >> 2));
	}

	if (c[LEVEL] != i) {
		/* Alert the player to level up */
		UlarnBeep();
		Printf("\nWelcome to level %d!", (long)c[LEVEL]);
		/* if we changed levels */

		switch ((int)c[LEVEL]) {
		case 94: /* earth guardian */
			c[WTW] = 99999L;
			break;
		case 95: /* air guardian */
			c[INVISIBILITY] = 99999L;
			break;
		case 96: /* fire guardian */
			c[FIRERESISTANCE] = 99999L;
			break;
		case 97: /* water guardian */
			c[CANCELLATION] = 99999L;
			break;
		case 98: /* time guardian */
			c[HASTESELF] = 99999L;
			break;
		case 99: /* ethereal guardian */
			c[STEALTH] = 99999L;
			c[SPIRITPRO] = 99999L;
			break;
		case 100:
			Print("\nYou are now The Creator!");
			{
				int i, j;

				for (i = 0; i < MAXY; i++) for (j = 0; j < MAXX; j++) know[j][i] = item[j][i];
				for (i = 0; i < SPELL_COUNT; i++) spelknow[i] = 1;
				for (i = 0; i < MAXSCROLL; i++) scrollknown[i] = 1;
				for (i = 0; i < MAXPOTION; i++) potionknown[i] = 1;
			}
			break;
		default:
			break;
		}
	}

	UpdateStatusAndEffects();
}

/* =============================================================================
 * FUNCTION: loseexperience
 */
void loseexperience(long x)
{
	int i, tmp;

	i = c[LEVEL];
	c[EXPERIENCE] -= x;
	if (c[EXPERIENCE] < 0) c[EXPERIENCE] = 0;
	while (c[EXPERIENCE] < skill[c[LEVEL] - 1]) {
		if (--c[LEVEL] <= 1) c[LEVEL] = 1;              /*  down one level    */

		tmp = (c[CONSTITUTION] - c[HARDGAME]) >> 1;     /* lose hpoints */
		losemhp((int)rnd((tmp > 0)?tmp:1));             /* lose hpoints */
		if (c[LEVEL] < 7 - c[HARDGAME]) losemhp((int)(c[CONSTITUTION] >> 2));
		losemspells((int)rund(3));                      /*  lose spells */
	}

	if (i != c[LEVEL])
		Printf("\nYou went down to level %d!", (long)c[LEVEL]);

	recalc();
	UpdateStatus();
}

/* =============================================================================
 * FUNCTION: losehp
 */
void losehp(DiedReasonType Reason, int x)
{
	if ((c[HP] -= x) <= 0) {
		UlarnBeep();
		Print("\nAlas, you have died.\n");
		Print("\n");
		nap(3000);

		if ((Reason == DIED_MONSTER) && (last_monst_id == MONST_NONE))
			Reason = DIED_UNSEEN_ATTACKER;
		died(Reason, last_monst_id);
	}
}

/* =============================================================================
 * FUNCTION: losemhp
 */
void losemhp(int x)
{
	c[HP] -= x;
	if (c[HP] < 1) c[HP] = 1;

	c[HPMAX] -= x;
	if (c[HPMAX] < 1) c[HPMAX] = 1;
}

/* =============================================================================
 * FUNCTION: raisehp
 */
void raisehp(int x)
{
	if ((c[HP] += x) > c[HPMAX])
		c[HP] = c[HPMAX];
}

/* =============================================================================
 * FUNCTION: raisemhp
 */
void raisemhp(int x)
{
	c[HPMAX] += x;
	c[HP] += x;
}

/* =============================================================================
 * FUNCTION: raisespells
 */
void raisespells(int x)
{
	if ((c[SPELLS] += x) > c[SPELLMAX])
		c[SPELLS] = c[SPELLMAX];
}

/* =============================================================================
 * FUNCTION: raisemspells
 */
void raisemspells(int x)
{
	c[SPELLMAX] += x;
	c[SPELLS] += x;
}

/* =============================================================================
 * FUNCTION: losespells
 */
void losespells(int x)
{
	if ((c[SPELLS] -= x) < 0)
		c[SPELLS] = 0;
}

/* =============================================================================
 * FUNCTION: losemspells
 */
void losemspells(int x)
{
	if ((c[SPELLMAX] -= x) < 0)
		c[SPELLMAX] = 0;
	if ((c[SPELLS] -= x) < 0)
		c[SPELLS] = 0;
}

/* =============================================================================
 * FUNCTION: positionplayer
 */
void positionplayer(void)
{
	int x, y;
	int cx = 0, cy = 0;
	int dist, closest;

	if ((item[playerx][playery] == ONOTHING) &&
	    (mitem[playerx][playery].mon == MONST_NONE))
		/* location is clear, so nothing to do */
		return;

	closest = 10000;

	for (x = 0; x < MAXX; x++) {
		for (y = 0; y < MAXY; y++) {
			if ((item[x][y] == ONOTHING) &&
			    (mitem[x][y].mon == MONST_NONE)) {
				/*
				 * This location is empty, so see how far it is from the desired
				 * location.
				 */
				dist = (playerx - x) * (playerx - x) + (playery - y) * (playery - y);

				if (dist < closest) {
					/* This is the closest location found so far */
					cx = x;
					cy = y;
					closest = dist;
				}
			}
		}
	}


	if (closest == 10000)
		Print("Failure in positionplayer\n");
	else{
		playerx = cx;
		playery = cy;
	}
}

/* =============================================================================
 * FUNCTION: recalc
 */
void recalc(void)
{
	int i;

	c[AC] = c[MOREDEFENSES];
	if (c[WEAR] >= 0) {
		switch (iven[c[WEAR]]) {
		case OSHIELD:
			c[AC] += 2 + ivenarg[c[WEAR]];
			break;
		case OLEATHER:
			c[AC] += 2 + ivenarg[c[WEAR]];
			break;
		case OSTUDLEATHER:
			c[AC] += 3 + ivenarg[c[WEAR]];
			break;
		case ORING:
			c[AC] += 5 + ivenarg[c[WEAR]];
			break;
		case OCHAIN:
			c[AC] += 6 + ivenarg[c[WEAR]];
			break;
		case OSPLINT:
			c[AC] += 7 + ivenarg[c[WEAR]];
			break;
		case OPLATE:
			c[AC] += 9 + ivenarg[c[WEAR]];
			break;
		case OPLATEARMOR:
			c[AC] += 10 + ivenarg[c[WEAR]];
			break;
		case OSSPLATE:
			c[AC] += 12 + ivenarg[c[WEAR]];
			break;
		case OELVENCHAIN:
			c[AC] += 15 + ivenarg[c[WEAR]];
			break;
		default:
			break;
		}
	}

	if (c[SHIELD] >= 0 && iven[c[SHIELD]] == OSHIELD)
		c[AC] += 2 + ivenarg[c[SHIELD]];

	c[LANCEDEATH] = 0;

	if (c[WIELD] < 0)
		c[WCLASS] = 0;
	else{
		i = ivenarg[c[WIELD]];
		switch (iven[c[WIELD]]) {
		case ODAGGER:
			c[WCLASS] =  3 + i;
			break;
		case OBELT:
			c[WCLASS] =  7 + i;
			break;
		case OSHIELD:
			c[WCLASS] =  8 + i;
			break;
		case OPSTAFF:
		case OSPEAR:
			c[WCLASS] = 10 + i;
			break;
		case OFLAIL:
			c[WCLASS] = 14 + i;
			break;
		case OBATTLEAXE:
			c[WCLASS] = 17 + i;
			break;
		case OLANCE:
			c[LANCEDEATH] = 1;
			c[WCLASS] = 20 + i;
			break;
		case OLONGSWORD:
		case OVORPAL:
			c[WCLASS] = 22 + i;
			break;
		case O2SWORD:
			c[WCLASS] = 26 + i;
			break;
		case OSWORDofSLASHING:
			c[WCLASS] = 30 + i;
			break;
		case OSLAYER:
			c[WCLASS] = 30 + i;
			break;
		case OSWORD:
			c[WCLASS] = 32 + i;
			break;
		case OHAMMER:
			c[WCLASS] = 35 + i;
			break;
		default:
			c[WCLASS] = 0;
		}
	}
	c[WCLASS] += c[MOREDAM];

	/*  now for regeneration abilities based on rings */
	c[REGEN] = 1;
	c[ENERGY] = 0;

	for (i = 0; i <= IVENSIZE; i++) {
		switch (iven[i]) {
		case OPROTRING:
			c[AC] += ivenarg[i] + 1;
			break;
		case ODAMRING:
			c[WCLASS] += ivenarg[i] + 1;
			break;
		case OBELT:
			c[WCLASS] += ((ivenarg[i] << 1)) + 2;
			break;
		case OREGENRING:
			c[REGEN] += ivenarg[i] + 1;
			break;
		case ORINGOFEXTRA:
			c[REGEN] += 5 * (ivenarg[i] + 1);
			break;
		case OENERGYRING:
			c[ENERGY] += ivenarg[i] + 1;
			break;
		default:
			break;
		}
	}
}

/* =============================================================================
 * FUNCTION: take
 */
int take(int itm, int arg)
{
	int i;
	int limit;
	int slot;
	int need_recalc;
	char eyeflag = 0;

	/*
	 * Decide how many items the player can carry
	 */
	limit = 16 + (c[LEVEL] >> 1);
	if (limit > IVENSIZE)
		limit = IVENSIZE;

	/*
	 * find a free inventory slot
	 */
	i = 0;
	slot = -1;
	while ((i < limit) && (slot == -1)) {
		if (iven[i] == ONOTHING)
			slot = i;
		i++;
	}

	if (slot != -1) {
		/* found a free inventory slot, so store the item */
		need_recalc = 0;
		iven[slot] = (char)itm;
		ivenarg[slot] = (short)arg;
		switch (itm) {
		case OPROTRING:
		case ODAMRING:
		case OBELT:
			need_recalc = 1;
			break;
		case ODEXRING:
			c[DEXTERITY] += ivenarg[slot] + 1;
			need_recalc = 1;
			break;
		case OSTRRING:
			c[STREXTRA] += ivenarg[slot] + 1;
			need_recalc = 1;
			break;
		case OCLEVERRING:
			c[INTELLIGENCE] += ivenarg[slot] + 1;
			need_recalc = 1;
			break;
		case OHAMMER:
			c[DEXTERITY] += 10;
			c[STREXTRA] += 10;
			c[TMP2] = c[INTELLIGENCE];
			c[INTELLIGENCE] = (c[TMP2] - 10 <= 2)?3:c[TMP2] - 10;
			need_recalc = 1;
			break;
		case OORB:
			c[ORB]++;
			c[AWARENESS]++;
			break;
		case OORBOFDRAGON:
			c[SLAYING]++;
			break;
		case OSPIRITSCARAB:
			c[NEGATESPIRIT]++;
			break;
		case OCUBEofUNDEAD:
			c[CUBEofUNDEAD]++;
			break;
		case ONOTHEFT:
			c[NOTHEFT]++;
			break;
		case OSWORDofSLASHING:
			c[DEXTERITY] += 5;
			need_recalc = 1;
			break;
		case OSLAYER:
			c[INTELLIGENCE] += 10;
			need_recalc = 1;
			break;
		case OPSTAFF:
			c[WISDOM] += 10;
			need_recalc = 1;
			break;
		case  OLARNEYE:
			c[EYEOFLARN] = 1;
			eyeflag = 1;
			break;
		}

		Print("\nYou pick up:");
		show3(slot);
		if (need_recalc) {
			recalc();
			UpdateStatus();
		}

		if ((c[BLINDCOUNT] == 0) && eyeflag) {
			Print("\nYour sight fades for a moment...");
			nap(2000);
			drawscreen();
			Print("\nYour sight returns, and everything looks crystal-clear!");
		}
		return 0;
	}

	Print("\nYou can't carry anything else.");
	return 1;
}

/* =============================================================================
 * FUNCTION: drop_object
 */
int drop_object(int k)
{
	int itm, pitflag = 0;

	if ((k < 0) || (k > 25))
		return 0;

	itm = iven[k];

	if (itm == ONOTHING) {
		Printf("\nYou don't have item %c! ", k + 'a');
		return 1;
	}

	if (item[playerx][playery] == OPIT)
		pitflag = 1;
	else if (item[playerx][playery] > ONOTHING) {
		UlarnBeep();
		Print("\nThere's something here already.");
		return 1;
	}

	if (playery == MAXY - 1 && playerx == 33) {
		return 1; /* not in entrance */
	}

	if (!pitflag) {
		item[playerx][playery] = (char)itm;
		iarg[playerx][playery] = (short)ivenarg[k];
	}

	/* show what item you dropped*/
	Print("\n  You drop:");
	show3(k);

	/* Update the player's inventory and status */
	iven[k] = ONOTHING;

	if (c[WIELD] == k) c[WIELD] = -1;
	if (c[WEAR] == k) c[WEAR] = -1;
	if (c[SHIELD] == k) c[SHIELD] = -1;

	adjustcvalues(itm, ivenarg[k]);
	recalc();
	UpdateStatus();

	if (pitflag)
		Printf("\nIt disappears down the pit.");

	/* say dropped an item so wont ask to pick it up right away */
	dropflag = 1;
	return 0;
}

/* =============================================================================
 * FUNCTION: adjustivenarg
 */
void adjustivenarg(int Idx, int Amount)
{
	ivenarg[Idx] += (short)Amount;

	switch (iven[Idx]) {
	/* Armour */
	case OPLATE:
	case OCHAIN:
	case OLEATHER:
	case ORING:
	case OSTUDLEATHER:
	case OSPLINT:
	case OPLATEARMOR:
	case OSSPLATE:
	case OSHIELD:
	case OELVENCHAIN:
		recalc();
		UpdateStatus();
		break;

	/* Weapons */
	case OSWORDofSLASHING:
	case OHAMMER:
	case OSWORD:
	case O2SWORD:
	case OSPEAR:
	case ODAGGER:
	case OBATTLEAXE:
	case OLONGSWORD:
	case OFLAIL:
	case OLANCE:
	case OVORPAL:
	case OSLAYER:
	case OBELT:
		recalc();
		UpdateStatus();
		break;

	/* Rings */

	case ORINGOFEXTRA:
		recalc();
		break;

	case OREGENRING:
		recalc();
		break;

	case OPROTRING:
		recalc();
		UpdateStatus();
		break;

	case OENERGYRING:
		recalc();
		break;

	case ODEXRING:
		c[DEXTERITY] += Amount;
		recalc();
		UpdateStatus();
		break;

	case OSTRRING:
		c[STREXTRA] += Amount;
		recalc();
		UpdateStatus();
		break;

	case OCLEVERRING:
		c[INTELLIGENCE] += Amount;
		recalc();
		UpdateStatus();
		break;

	case ODAMRING:
		recalc();
		UpdateStatus();
		break;

	default:
		break;
	}

}


/* =============================================================================
 * FUNCTION: enchantarmor
 */
void enchantarmor(int how)
{
	int which;

	/*
	 *  Bomb out if we're not wearing anything.
	 */
	if (c[WEAR] < 0 && c[SHIELD] < 0) {
		UlarnBeep();
		Print("\nYou feel a sense of loss.");
		return;
	}

	/*
	 *  Choose what to enchant.
	 */
	which = (rund(100) < 50) ? SHIELD : WEAR;
	if (c[which] < 0)
		which = (which == SHIELD) ? WEAR : SHIELD;

	/*
	 *  Enchant it and check for destruction at >= +10.
	 */
	if (ivenarg[c[which]] >= 9) {
		if (how == ENCH_ALTAR)
			Printf("\nYour %s glows briefly.", objectname[(int)iven[c[which]]]);
		else if (rnd(10) <= 9) {
			Printf("\nYour %s vibrates violently and crumbles into dust!",
			       objectname[(int)iven[c[which]]]);

			/* Recalculated any attributes affected by destroying the armour */
			adjustcvalues(iven[c[which]], ivenarg[c[which]]);

			/* Destroy the armour */
			iven[c[which]] = ONOTHING;
			ivenarg[c[which]] = 0;
			c[which] = -1;

			/* Recalculate player's AC */
			recalc();
		}
	}else {
		Printf("\nYour %s glows for a moment.", objectname[(int)iven[c[which]]]);
		/* Increase enchantment level by 1 */
		adjustivenarg(c[which], 1);
	}

	UpdateStatus();
}

/* =============================================================================
 * FUNCTION: enchweapon
 */
void enchweapon(int how)
{
	ObjectIdType Obj;
	int Idx;

	if (c[WIELD] < 0) {
		UlarnBeep();
		Print("\nYou feel depressed.");
		return;
	}

	Idx = c[WIELD];
	Obj = iven[c[WIELD]];
	if ((Obj != OSCROLL) && (Obj != OPOTION)) {

		if ((ivenarg[Idx] >= 9) && (rnd(10) <= 9)) {
			if (how == ENCH_ALTAR)
				Print("\nYour weapon glows a little.");
			else{
				Print("\nYour weapon vibrates violently and crumbles into dust!");

				/* Recalculate any attributes affected by destroying the weapon */
				adjustcvalues(iven[Idx], ivenarg[Idx]);

				/* destroy the weapon */
				iven[Idx] = ONOTHING;
				ivenarg[Idx] = 0;
				c[WIELD] = -1;

				/* Recalculate the player's WC */
				recalc();
			}
		}else {
			Print("\nYour weapon glows for a moment.");
			adjustivenarg(Idx, 1);
		}

	}
	UpdateStatus();
}

/* =============================================================================
 * FUNCTION: pocketfull
 */
int pocketfull(void)
{
	int i, limit;

	if ((limit = 15 + (c[LEVEL] >> 1)) > IVENSIZE)
		limit = IVENSIZE;

	for (i = 0; i < limit; i++)
		if (iven[i] == ONOTHING) return 0;
	return 1;
}

/* =============================================================================
 * FUNCTION: nearbymonst
 */
int nearbymonst(void)
{
	int tx, ty;
	int x1, y1;
	int x2, y2;

	x1 = playerx - 1;
	x2 = playerx + 1;
	y1 = playery - 1;
	y2 = playery + 1;

	if (x1 < 0) x1 = 0;
	if (x2 >= MAXX) x2 = (MAXX - 1);
	if (y1 < 0) y1 = 0;
	if (y2 >= MAXY) y2 = (MAXY - 1);

	for (tx = x1; tx <= x2; tx++)
		for (ty = y1; ty <= y2; ty++)
			if (mitem[tx][ty].mon != MONST_NONE) return 1; /* monster nearby */

	return 0;
}

/* =============================================================================
 * FUNCTION: stealsomething
 */
int stealsomething(int x, int y)
{
	int i, n = 100;

	/* max of 6 stolen items per monster */
	if (mitem[x][y].n >= 6)
		return 0;

	while (n--) {
		i = rund(IVENSIZE);
		if (iven[i] != ONOTHING) {
			if (c[WEAR] != i && c[WIELD] != i && c[SHIELD] != i) {
				show3(i);
				adjustcvalues(iven[i], ivenarg[i]);
				n = mitem[x][y].n++;
				mitem[x][y].it[n].item = iven[i];
				mitem[x][y].it[n].itemarg = ivenarg[i];

				iven[i] = ONOTHING;
				ivenarg[i] = 0;
				beenhere[level]++;

				return 1;
			}
		}
	}
	return 0;
}

/* =============================================================================
 * FUNCTION: emptyhanded
 */
int emptyhanded(void)
{
	int i;

	for (i = 0; i < IVENSIZE; i++) {
		if (iven[i] != ONOTHING)
			if ((i != c[WIELD]) && (i != c[WEAR]) && (i != c[SHIELD]))
				return 0;
	}
	return 1;
}

/* =============================================================================
 * FUNCTION: creategem
 */
void creategem(void)
{
	int i, j;

	switch (rnd(4)) {
	case 1:
		i = ODIAMOND;
		j = 50;
		break;
	case 2:
		i = ORUBY;
		j = 40;
		break;
	case 3:
		i = OEMERALD;
		j = 30;
		break;
	default:
		i = OSAPPHIRE;
		j = 20;
		break;
	}
	;

	createitem(playerx, playery, i, (long)(rndl(j) + (j / 10)));
}

/* =============================================================================
 * FUNCTION: adjustcvalues
 */
void adjustcvalues(int itm, int arg)
{
	int flag, i;

	flag = 0;
	switch (itm) {
	case ODEXRING:
		c[DEXTERITY] -= arg + 1;
		flag = 1;
		break;
	case OSTRRING:
		c[STREXTRA] -= arg + 1;
		flag = 1;
		break;
	case OCLEVERRING:
		c[INTELLIGENCE] -= arg + 1;
		flag = 1;
		break;
	case OHAMMER:
		c[DEXTERITY] -= 10;
		c[STREXTRA] -= 10;
		c[INTELLIGENCE] = c[TMP2];
		flag = 1;
		break;
	case OORB:
		c[ORB]--;
		c[AWARENESS]--;
		break;
	case OSWORDofSLASHING:
		c[DEXTERITY] -= 5;
		flag = 1;
		break;
	case OSLAYER:
		c[INTELLIGENCE] -= 10;
		flag = 1;
		break;
	case OPSTAFF:
		c[WISDOM] -= 10;
		flag = 1;
		break;
	case OORBOFDRAGON:
		--c[SLAYING];
		break;
	case OSPIRITSCARAB:
		--c[NEGATESPIRIT];
		break;
	case OCUBEofUNDEAD:
		--c[CUBEofUNDEAD];
		break;
	case ONOTHEFT:
		--c[NOTHEFT];
		break;
	case OLANCE:
		c[LANCEDEATH] = 0;
		break;
	case OLARNEYE:
		c[EYEOFLARN] = 0;
		if (c[BLINDCOUNT] == 0) {
			Print("\nYour sight fades for a moment...");
			nap(2000);
			drawscreen();
			Print("\nYour sight returns but everything looks dull and faded.");
		}
		break;
	case OPOTION:
	case OSCROLL:
		break;

	default:
		flag = 1;
		break;
	}

	for (i = ABILITY_FIRST; i <= ABILITY_LAST; i++)
		if (c[i] < 3)
			c[i] = 3;

	if (flag) {
		recalc();
		UpdateStatusAndEffects();
	}

}

/* =============================================================================
 * FUNCTION: packweight
 */
int packweight(void)
{
	int i, j, k;

	k = c[GOLD] / 1000;
	j = 25;
	while ((iven[j] == ONOTHING) && (j > 0)) --j;

	for (i = 0; i <= j; i++) {
		switch (iven[i]) {
		case ONOTHING: break;
		case OSSPLATE:
		case OPLATEARMOR:
			k += 40;
			break;
		case OPLATE:
			k += 35;
			break;
		case OHAMMER:
			k += 30;
			break;
		case OSPLINT:
			k += 26;
			break;
		case OCHAIN:
		case OBATTLEAXE:
		case O2SWORD:
			k += 23;
			break;
		case OLONGSWORD:
		case OPSTAFF:
		case OSWORD:
		case ORING:
		case OFLAIL:
			k += 20;
			break;
		case OELVENCHAIN:
		case OSWORDofSLASHING:
		case OLANCE:
		case OSLAYER:
		case OSTUDLEATHER:
			k += 15;
			break;
		case OLEATHER:
		case OSPEAR:
			k += 8;
			break;
		case OORBOFDRAGON:
		case OORB:
		case OBELT:
			k += 4;
			break;
		case OSHIELD:
			k += 7;
			break;
		case OCHEST:
			k += 30 + ivenarg[i];
			break;
		default:
			k++;
		}
	}

	return k;
}

/* =============================================================================
 * FUNCTION: adjust_ability
 */
void adjust_ability(AttributeType ability, int amount)
{
	/* check that it is an ability being adjusted */
	if ((ability < ABILITY_FIRST) || (ability > ABILITY_LAST))
		return;

	c[ability] += amount;
	if (c[ability] < 3)
		c[ability] = 3;
}

/* =============================================================================
 * FUNCTION: regen
 */
void regen(void)
{
	int i, j;
	AttributeType Attr;
	int flag; // indicates whether effect and/or status need update.

	//   1 = Status, 2 = effects, 3 = both

	/* Handle stop time spell */

	if (c[TIMESTOP]) {
		if (--c[TIMESTOP] <= 0) {
			recalc();
			UpdateStatusAndEffects();
		}
		return;
	}


	flag = 0;

	if (c[HASTESELF] == 0)
		gtime++;
	else{
		/*
		 * Player is moving at double speed, so time only passes every
		 * second move
		 */
		HasteStep++;
		if (HasteStep == 2) {
			gtime++;
			HasteStep = 0;
		}
	}

	/* regenerate hit points  */
	if (c[HP] != c[HPMAX]) {
		c[REGENCOUNTER]--;
		if (c[REGENCOUNTER] < 0) {
			c[REGENCOUNTER] = 22 + (c[HARDGAME] << 1) - c[LEVEL];
			if ((c[HP] += c[REGEN]) > c[HPMAX])
				c[HP] = c[HPMAX];

			flag |= 1;
		}
	}

	/* regenerate spells  */
	if (c[SPELLS] < c[SPELLMAX]) {
		c[ECOUNTER]--;
		if (c[ECOUNTER] < 0) {
			c[ECOUNTER] = 100 + 4 * (c[HARDGAME] - c[LEVEL] - c[ENERGY]);
			c[SPELLS]++;

			flag |= 1;
		}
	}

	for (j = 0; j < TIME_CHANGED_COUNT; j++) {
		Attr = time_change[j];
		if (c[Attr] != 0) {
			/* Reduce time remaining for this effect */
			c[Attr]--;

			/* AWARENESS doesn't wear off if the player has the orb */
			if (Attr == AWARENESS)
				if (c[ORB] != 0) c[AWARENESS]++;

			if (c[Attr] == 0) {
				/* Effect has worn off, so perform appropriate action */

				switch (Attr) {
				case HERO:
					for (i = ABILITY_FIRST; i <= ABILITY_LAST; i++)
						adjust_ability(i, -(PHEROISM_BOOST - 1));
					flag |= 1;
					break;

				case COKED:
					for (i = ABILITY_FIRST; i <= ABILITY_LAST; i++)
						adjust_ability(i, -34);
					flag |= 1;
					break;

				case ALTPRO:
					c[MOREDEFENSES] -= ALTAR_PRO_BOOST;
					flag |= 3;
					break;

				case PROTECTIONTIME:
					c[MOREDEFENSES] -= SPELL_PRO_BOOST;
					flag |= 3;
					break;

				case DEXCOUNT:
					adjust_ability(DEXTERITY, SDEXTERITY_BOOST);
					flag |= 3;
					break;

				case STRCOUNT:
					c[STREXTRA] -= SSTRENGTH_BOOST;
					flag |= 3;
					break;

				case BLINDCOUNT:
					Print("\nThe blindness lifts.");
					UlarnBeep();
					break;

				case CONFUSE:
					Print("\nYou regain your senses.");
					UlarnBeep();
					break;

				case GIANTSTR:
					/*
					 * Giant strength wears off, but the playre gets a permanent +1
					 * boost to extra strength.
					 * (unaffected by effects that lower strength).
					 */
					c[STREXTRA] -= (PGIANTSTR_BOOST - 1);
					flag |= 3;
					break;

				case GLOBE:
					c[MOREDEFENSES] -= SPELL_GLOBE_BOOST;
					flag |= 1;
					break;

				case HALFDAM:
					Print("\nYou now feel better.");
					UlarnBeep();
					break;

				case SEEINVISIBLE:
					if (player_has_item(OAMULET))
						/*
						 * See inv doesn't wear off if player has amulet of invisibility
						 */
						c[SEEINVISIBLE]++;
					break;

				case ITCHING:
					Print("\nThe irritation subsides.");
					UlarnBeep();
					break;

				case CLUMSINESS:
					Print("\nYou now feel less awkward.");
					UlarnBeep();
					break;

				case CHARMCOUNT:
					flag |= 3;
					break;

				case INVISIBILITY:
				case CANCELLATION:
				case WTW:
				case HASTESELF:
				case SCAREMONST:
				case STEALTH:
				case HOLDMONST:
				case FIRERESISTANCE:
				case SPIRITPRO:
				case UNDEADPRO:
					flag |= 2;
					break;

				case AGGRAVATE:
				case HASTEMONST:
				case AWARENESS:
					break;

				default:
					break;

				}       /* switch */
			}               /* if now expired */
		}                       /* if active */
	}                               /* for each time based effect */

	/*
	 * If the player is still itching then check to see if armour is to be
	 * removed
	 */
	if (c[ITCHING]) {
		if ((c[WEAR] != -1) || (c[SHIELD] != -1)) {
			if (rnd(100) < 50) {
				c[WEAR] = -1;
				c[SHIELD] = -1;
				Print("\nThe hysteria of itching forces you to remove your armor!");
				UlarnBeep();
				recalc();
				flag |= 1;
			}
		}
	}

	/*
	 * If the player is still clumsy, check to see if the weapon is dropped
	 */
	if (c[CLUMSINESS]) {
		if (c[WIELD] != -1) {
			/* if nothing there */
			if (item[playerx][playery] == ONOTHING) {
				/* drop your weapon 1/3 of the time */
				if (rnd(100) < 33)
					drop_object((int)c[WIELD]);
			}
		}
	}


	if (flag == 1)
		UpdateStatus();
	else if (flag == 2)
		UpdateEffects();
	else if (flag == 3)
		UpdateStatusAndEffects();
}

/* =============================================================================
 * FUNCTION: removecurse
 */
void removecurse(void)
{
	int i;

	for (i = 0; i < CURSE_COUNT; i++) {
		if (c[curse[i]])
			/*
			 * Set the time remaining on each curse to 1 so that the next regen
			 * will clear the curse
			 */
			c[curse[i]] = 1;
	}
}

/* =============================================================================
 * FUNCTION: adjusttime
 */
void adjusttime(long tim)
{
	int j;

	/* adjust time related parameters */
	for (j = 0; j < TIME_CHANGED_COUNT; j++) {
		if (c[time_change[j]]) {
			c[time_change[j]] -= tim;

			if (c[time_change[j]] <= 0)
				/*
				 * if effect would wear off then set the time remaining to 1 so that
				 * the next regen will cancel the effect
				 */
				c[time_change[j]] = 1;
		}
	}

	regen();
}

/* =============================================================================
 * FUNCTION: write_player
 */
void write_player(FILE *fp)
{
	bwrite(fp, char_class, 20);
	bwrite(fp, &ramboflag, 1);
	bwrite(fp, &wizard, 1);
	bwrite(fp, &cheat, 1);
	bwrite(fp, (char *)&playerx, sizeof(int));
	bwrite(fp, (char *)&playery, sizeof(int));
	bwrite(fp, (char *)&initialtime, sizeof(long));
	bwrite(fp, (char *)&gtime, sizeof(long));
	bwrite(fp, (char *)&outstanding_taxes, sizeof(long));
	bwrite(fp, (char *)c, ATTRIBUTE_COUNT * sizeof(long));
	bwrite(fp, iven, IVENSIZE);
	bwrite(fp, (char *)ivenarg, IVENSIZE * sizeof(short));
	bwrite(fp, (char *)potionknown, MAXPOTION * sizeof(int));
	bwrite(fp, (char *)scrollknown, MAXSCROLL * sizeof(int));
	bwrite(fp, (char *)spelknow, SPELL_COUNT * sizeof(int));
}

/* =============================================================================
 * FUNCTION: read_player
 */
void read_player(FILE *fp)
{
	bread(fp, char_class, 20);
	bread(fp, &ramboflag, 1);
	bread(fp, &wizard, 1);
	bread(fp, &cheat, 1);
	bread(fp, (char *)&playerx, sizeof(int));
	bread(fp, (char *)&playery, sizeof(int));
	bread(fp, (char *)&initialtime, sizeof(long));
	bread(fp, (char *)&gtime, sizeof(long));
	bread(fp, (char *)&outstanding_taxes, sizeof(long));
	bread(fp, (char *)c, ATTRIBUTE_COUNT * sizeof(long));
	bread(fp, iven, IVENSIZE);
	bread(fp, (char *)ivenarg, IVENSIZE * sizeof(short));
	bread(fp, (char *)potionknown, MAXPOTION * sizeof(int));
	bread(fp, (char *)scrollknown, MAXSCROLL * sizeof(int));
	bread(fp, (char *)spelknow, SPELL_COUNT * sizeof(int));

	/*
	 * Identify the class number from the class string.
	 * identify_class returns the character selection for the class, or 0
	 * if the string matches no valid class.
	 */
	class_num = identify_class(char_class);
	if (class_num != 0)
		class_num -= 'a';
	else
		/* class got corrupt somehow, so reset to the first class */
		strcpy(char_class, InitialClassData[0].ClassName);
}

