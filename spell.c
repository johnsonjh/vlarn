/* =============================================================================
 * PROGRAM:  ularn
 * FILENAME: spell.c
 *
 * DESCRIPTION:
 * This module contains the functions used to process spells and spell like
 * effects.
 *
 * =============================================================================
 * EXPORTED VARIABLES
 *
 * splev        : The maximum spell num to be learnt from a book found on each
 *                dungeon level
 * spelcode     : The three letter codes for the spells
 * spelname     : The text name of each spell
 * speldescript : The text description of each spell.
 *
 * =============================================================================
 * EXPORTED FUNCTIONS
 *
 * godirect       : Function to process ranged spell effects (including scrolls)
 * annihilate     : Function to process the annihilate scroll
 * get_spell_code : Function to get the three letter spell code from the player
 * cast           : Function to cast a spell
 *
 * =============================================================================
 */

#include "header.h"
#include "ularn_game.h"
#include "ularn_win.h"
#include "ularn_ask.h"
#include "spell.h"
#include "sphere.h"
#include "show.h"
#include "dungeon.h"
#include "monster.h"
#include "player.h"
#include "itm.h"

/* =============================================================================
 * Exported variables
 */

char splev[NLEVELS] = {
	1,  4,	7,  11, 15,
	20, 24, 28, 30, 32,
	33, 34, 35, 36, 37,
	38, 38, 38, 38, 38,
	38
};

char *spelcode[SPELL_COUNT] = {
	"pro", "mle",  "dex",  "sle",  "chm",  "ssp",                   /* 0 - 5 */
	"web", "str",  "enl",  "hel",  "cbl",  "cre",  "pha",  "inv",   /*6-13 */
	"bal", "cld",  "ply",  "can",  "has",  "ckl",  "vpr",           /* 14-20 */
	"dry", "lit",  "drl",  "glo",  "flo",  "fgr",                   /* 21 - 26 */
	"sca", "hld",  "stp",  "tel",  "mfi",  "mkw",                   /* 27 - 34 */
	"sph", "gen",  "sum",  "wtw",  "alt",  "per"                    /* 35 - 38 */
};

char *spelname[SPELL_COUNT] = {
	"protection", /* 0 */
	"magic missile",
	"dexterity",
	"sleep",
	"charm monster",
	"sonic spear", /* 5 */
	"web",
	"strength",
	"enlightenment",
	"healing",
	"cure blindness", /* 10 */
	"create monster",
	"phantasmal forces",
	"invisibility",
	"fireball",
	"cold", /* 15 */
	"polymorph",
	"cancellation",
	"haste self",
	"cloud kill",
	"vaporize rock", /* 20 */
	"dehydration",
	"lightning",
	"drain life",
	"invulnerability",
	"flood", /* 25 */
	"finger of death",
	"scare monster",
	"hold monster",
	"time stop",
	"teleport away", /* 30 */
	"magic fire",
	"make a wall",
	"sphere of annihilation",
	"genocide", /* 34 */
	"summon demon",
	"walk through walls",
	"alter reality",
	"permanence" /* 38 */
};

char *speldescript[SPELL_COUNT] = {
	/* 1 */
	"Generates a +2 protection field",
	"Creates and hurls a magic missile equivalent to a +1 magic arrow",
	"Adds +2 to the caster's dexterity",
	"Causes some monsters to go to sleep",
	"Some monsters may be awed at your magnificence",
	"Causes the caster's hands to emit a screeching sound",
	/* 7 */
	"Causes strands of sticky thread to entangle an enemy",
	"Adds +2 to the caster's strength for a time",
	"The caster becomes more aware of things around them",
	"Restores some of the caster's health",
	"Restores sight to one so unfortunate as to be blinded",
	"Creates a monster near to the caster",
	"Creates illusions which, if believed, cause monsters to die",
	"The caster becomes invisible",
	/* 15 */
	"Creates a ball of fire that burns whatever it hits",
	"Sends forth a cone of cold which freezes whatever it touches",
	"You can find out what this does for yourself",
	"Stops a monster from using its special abilities",
	"Speeds up the caster's movements",
	"Creates a fog of poisonous gas which kills all that is within it",
	"Changes rock to air",
	/* 22 */
	"Dries up water in the immediate vicinity",
	"Causes the caster's finger to emit lightning bolts",
	"Subtracts hit points from both you and a monster",
	"This globe helps to protect the player from physical attack",
	"Creates a deluge of water, flooding the immediate chamber",
	"A holy spell calling on your god to back you up",
	/* 28 */
	"Terrifies the monster so that it may not hit the caster",
	"Freezes monsters in their tracks",
	"All movement in the caverns ceases for a limited duration",
	"Teleports a monster",
	"Creates a curtain of fire around the caster",
	/* 33 */
	"Makes a wall in the specified place",
	"Anything caught in this sphere is instantly killed.",
	"Eliminates a species of monster from the game",
	"Summons a demon who may help you out",
	"Allows the player to walk through walls for a short period of time",
	"God only knows what this will do",
	"Makes a character spell permanent, e.g. protection, strength, etc."
};

/* =============================================================================
 * Local variables
 */

/*
 *  spelweird[ monster ] [ spell ] = [ reaction ]
 *
 *  spell =  index into spelldescript[] and spellname[]
 *  reaction = index into spelmes[]
 */
static char spelweird[MONST_COUNT][SPELL_COUNT] = {
/*p m d s c s    w s e h c c p i    b c p c h c v    d l d g f f    s h s t m    s g s w a p */
/* None (placeholder) */
	{  0, 0,  0, 0, 0, 0,  0,    0, 0, 0, 0, 0, 0,	0, 0,	0,  0, 0, 0, 0, 0, 0,	0,  0, 0, 0,  0, 0,   0, 0, 0, 0, 0,   0, 0,  0, 0, 0	},
/* lemming */
	{  0, 0,  0, 0, 0, 0,  0,    0, 0, 0, 0, 0, 0,	0, 0,	0,  0, 0, 0, 0, 0, 0,	0,  0, 0, 0,  0, 0,   0, 0, 0, 0, 0,   0, 0,  0, 0, 0	},
/* gnome */
	{  0, 0,  0, 0, 0, 0,  0,    0, 0, 0, 0, 0, 0,	5, 0,	0,  0, 0, 0, 0, 0, 0,	0,  0, 0, 0,  0, 0,   0, 0, 0, 0, 0,   0, 0,  0, 0, 0	},
/* hobgoblin */
	{  0, 0,  0, 0, 0, 0,  0,    0, 0, 0, 0, 0, 0,	0, 0,	0,  0, 0, 0, 0, 0, 0,	0,  0, 0, 0,  0, 0,   0, 0, 0, 0, 0,   0, 0,  0, 0, 0	},
/* jackal */
	{  0, 0,  0, 0, 0, 0,  0,    0, 0, 0, 0, 0, 0,	0, 0,	0,  0, 0, 0, 0, 0, 0,	0,  0, 0, 0,  0, 0,   0, 0, 0, 0, 0,   0, 0,  0, 0, 0	},
/* kobold */
	{  0, 0,  0, 0, 0, 0,  0,    0, 0, 0, 0, 0, 0,	5, 0,	0,  0, 0, 0, 0, 0, 0,	0,  0, 0, 0,  0, 0,   0, 0, 0, 0, 0,   0, 0,  0, 0, 0	},

/*p m d s c s    w s e h c c p i    b c p c h c v    d l d g f f    s h s t m    s g s w a p */
/* orc */
	{  0, 0,  0, 0, 0, 0,  0,    0, 0, 0, 0, 0, 0,	0, 0,	0,  0, 0, 0, 0, 0, 0,	0,  0, 0, 0,  0, 0,   0, 0, 0, 0, 0,   0, 0,  0, 0, 0	},
/* snake */
	{  0, 0,  0, 0, 0, 0,  0,    0, 0, 0, 0, 0, 0,	0, 0,	0,  0, 0, 0, 0, 0, 0,	0,  0, 0, 0,  0, 0,   0, 0, 0, 0, 0,   0, 0,  0, 0, 0	},
/* giant centipede */
	{  0, 0,  0, 0, 0, 0,  0,    0, 0, 0, 0, 0, 0,	0, 0,	0,  0, 0, 0, 0, 0, 0,	0,  0, 0, 0,  0, 0,   0, 0, 0, 0, 0,   0, 0,  0, 0, 0	},
/* jaculi */
	{  0, 0,  0, 0, 0, 0,  0,    0, 0, 0, 0, 0, 0,	0, 0,	0,  0, 0, 0, 0, 0, 0,	0,  0, 0, 0,  0, 0,   0, 0, 0, 0, 0,   0, 0,  0, 0, 0	},
/* troglodyte */
	{  0, 0,  0, 0, 0, 0,  0,    0, 0, 0, 0, 0, 0,	0, 0,	0,  0, 0, 0, 0, 0, 0,	0,  0, 0, 0,  0, 0,   0, 0, 0, 0, 0,   0, 0,  0, 0, 0	},

/*p m d s c s    w s e h c c p i    b c p c h c v    d l d g f f    s h s t m    s g s w a p */
/* giant ant */
	{  0, 0,  0, 0, 0, 0,  0,    0, 0, 0, 0, 0, 0,	0, 0,	0,  0, 0, 0, 0, 0, 0,	0,  0, 0, 0,  0, 0,   0, 0, 0, 0, 0,   0, 0,  0, 0, 0	},
/* floating eye */
	{  0, 0,  0, 8, 0, 10, 0,    0, 0, 0, 0, 0, 0,	0, 0,	0,  0, 0, 0, 0, 0, 0,	0,  0, 0, 0,  4, 0,   0, 0, 0, 0, 0,   0, 0,  0, 0, 0	},
/* leprechaun */
	{  0, 0,  0, 0, 0, 0,  0,    0, 0, 0, 0, 0, 0,	0, 0,	0,  0, 0, 0, 0, 0, 0,	0,  0, 0, 0,  0, 0,   0, 0, 0, 0, 0,   0, 0,  0, 0, 0	},
/* nymph */
	{  0, 0,  0, 0, 0, 0,  0,    0, 0, 0, 0, 0, 0,	0, 0,	0,  0, 0, 0, 0, 0, 0,	0,  0, 0, 0,  0, 0,   0, 0, 0, 0, 0,   0, 0,  0, 0, 0	},
/* quasit */
	{  0, 0,  0, 0, 0, 0,  0,    0, 0, 0, 0, 0, 0,	0, 0,	0,  0, 0, 0, 0, 0, 0,	0,  0, 0, 0,  0, 0,   0, 0, 0, 0, 0,   0, 0,  0, 0, 0	},

/* rust monster */
	{  0, 0,  0, 0, 0, 0,  0,    0, 0, 0, 0, 0, 0,	0, 0,	0,  0, 0, 0, 0, 0, 0,	0,  0, 0, 0,  0, 0,   0, 0, 0, 0, 0,   0, 0,  0, 0, 0	},
/* zombie */
	{  0, 0,  0, 8, 0, 4,  0,    0, 0, 0, 0, 0, 4,	0, 0,	0,  0, 0, 0, 4, 0, 4,	0,  4, 0, 0,  4, 0,   0, 0, 0, 0, 0,   0, 0,  0, 0, 0	},
/* assassin bug */
	{  0, 0,  0, 0, 0, 0,  0,    0, 0, 0, 0, 0, 0,	0, 0,	0,  0, 0, 0, 0, 0, 0,	0,  0, 0, 0,  0, 0,   0, 0, 0, 0, 0,   0, 0,  0, 0, 0	},
/* bugbear */
	{  0, 0,  0, 0, 0, 0,  0,    0, 0, 0, 0, 0, 0,	5, 0,	0,  0, 0, 0, 0, 0, 0,	0,  0, 0, 0,  0, 0,   0, 0, 0, 0, 0,   0, 0,  0, 0, 0	},
/* hell hound */
	{  0, 6,  0, 0, 0, 0,  12,   0, 0, 0, 0, 0, 0,	0, 0,	0,  0, 0, 0, 0, 0, 0,	0,  0, 0, 0,  0, 0,   0, 0, 0, 0, 0,   0, 0,  0, 0, 0	},

/*p m d s c s    w s e h c c p i    b c p c h c v    d l d g f f    s h s t m    s g s w a p */
/* ice lizard */
	{  0, 0,  0, 0, 0, 0,  11,   0, 0, 0, 0, 0, 0,	0, 0,	15, 0, 0, 0, 0, 0, 0,	0,  0, 0, 0,  0, 0,   0, 0, 0, 0, 0,   0, 0,  0, 0, 0	},
/* centaur */
	{  0, 0,  0, 0, 0, 0,  0,    0, 0, 0, 0, 0, 0,	0, 0,	0,  0, 0, 0, 0, 0, 0,	0,  0, 0, 0,  0, 0,   0, 0, 0, 0, 0,   0, 0,  0, 0, 0	},
/* troll */
	{  0, 7,  0, 0, 0, 0,  0,    0, 0, 0, 0, 0, 0,	0, 0,	0,  0, 0, 0, 0, 0, 4,	0,  0, 0, 0,  0, 0,   0, 0, 0, 0, 0,   0, 0,  0, 0, 0	},
/* yeti */
	{  0, 0,  0, 0, 0, 0,  0,    0, 0, 0, 0, 0, 0,	0, 0,	15, 0, 0, 0, 0, 0, 0,	0,  0, 0, 0,  0, 0,   0, 0, 0, 0, 0,   0, 0,  0, 0, 0	},
/* white dragon */
	{  0, 0,  0, 0, 0, 0,  0,    0, 0, 0, 0, 0, 14, 0, 0,	15, 0, 0, 0, 0, 0, 4,	0,  0, 0, 0,  0, 0,   0, 0, 0, 0, 0,   0, 0,  0, 0, 0	},

/* elf */
	{  0, 0,  0, 0, 0, 0,  0,    0, 0, 0, 0, 0, 14, 5, 0,	0,  0, 0, 0, 0, 0, 0,	0,  0, 0, 0,  0, 0,   0, 0, 0, 0, 0,   0, 0,  0, 0, 0	},
/* gelatinous cube */
	{  0, 13, 0, 8, 0, 10, 2,    0, 0, 0, 0, 0, 0,	0, 0,	0,  0, 0, 0, 4, 0, 0,	0,  0, 0, 0,  4, 0,   0, 0, 0, 0, 0,   0, 0,  0, 0, 0	},
/* metamorph */
	{  0, 13, 0, 0, 0, 0,  2,    0, 0, 0, 0, 0, 0,	0, 0,	0,  0, 0, 0, 4, 0, 4,	0,  0, 0, 0,  4, 0,   0, 0, 0, 0, 0,   0, 0,  0, 0, 0	},
/* vortex */
	{  0, 13, 0, 0, 0, 10, 1,    0, 0, 0, 0, 0, 0,	0, 0,	0,  0, 0, 0, 4, 0, 4,	0,  0, 0, 4,  4, 0,   0, 0, 0, 0, 0,   0, 0,  0, 0, 0	},
/* ziller */
	{  0, 0,  0, 0, 0, 0,  0,    0, 0, 0, 0, 0, 0,	0, 0,	0,  0, 0, 0, 0, 0, 0,	0,  0, 0, 0,  0, 0,   0, 0, 0, 0, 0,   0, 0,  0, 0, 0	},

/*p m d s c s    w s e h c c p i     f c p c h c v    d l d g f f    s h s t m    s g s w a p */
/* violet fungi */
	{  0, 0,  0, 8, 0, 0,  0,    0, 0, 0, 0, 0, 0,	0, 0,	0,  0, 0, 0, 0, 0, 0,	0,  0, 0, 0,  4, 0,   0, 0, 0, 0, 0,   0, 0,  0, 0, 0	},
/* wraith */
	{  0, 13, 0, 8, 0, 4,  0,    0, 0, 0, 0, 0, 14, 0, 0,	0,  0, 0, 0, 4, 0, 4,	0,  4, 0, 0,  4, 0,   0, 0, 0, 0, 0,   0, 0,  0, 0, 0	},
/* forvalaka */
	{  0, 0,  0, 0, 0, 0,  0,    0, 0, 0, 0, 0, 0,	5, 0,	0,  0, 0, 0, 0, 0, 0,	0,  0, 0, 0,  0, 0,   0, 0, 0, 0, 0,   0, 0,  0, 0, 0	},
/* lama nobe */
	{  0, 0,  0, 0, 0, 0,  0,    0, 0, 0, 0, 0, 0,	0, 0,	0,  0, 0, 0, 0, 0, 0,	0,  0, 0, 0,  0, 0,   0, 0, 0, 0, 0,   0, 0,  0, 0, 0	},
/* osequip */
	{  0, 0,  0, 0, 0, 0,  0,    0, 0, 0, 0, 0, 0,	0, 0,	0,  0, 0, 0, 0, 0, 0,	0,  0, 0, 0,  0, 0,   0, 0, 0, 0, 0,   0, 0,  0, 0, 0	},
/* rothe */
	{  0, 7,  0, 0, 0, 0,  0,    0, 0, 0, 0, 0, 0,	5, 0,	0,  0, 0, 0, 0, 0, 0,	0,  0, 0, 0,  0, 0,   0, 0, 0, 0, 0,   0, 0,  0, 0, 0	},
/* xorn */
	{  0, 7,  0, 8, 0, 0,  0,    0, 0, 0, 0, 0, 0,	5, 0,	0,  0, 0, 0, 0, 0, 4,	0,  0, 0, 4,  4, 0,   0, 0, 0, 0, 0,   0, 0,  0, 0, 0	},
/* vampire */
	{  0, 0,  0, 0, 0, 4,  2,    0, 0, 0, 0, 0, 14, 0, 0,	0,  0, 0, 0, 4, 0, 0,	0,  4, 0, 0,  4, 0,   0, 0, 0, 0, 0,   0, 0,  0, 0, 0	},
/* invisible staker*/
	{  0, 0,  0, 0, 0, 0,  1,    0, 0, 0, 0, 0, 0,	0, 0,	0,  0, 0, 0, 0, 0, 0,	0,  0, 0, 0,  0, 0,   0, 0, 0, 0, 0,   0, 0,  0, 0, 0	},
/* poltergeist */
	{  0, 13, 0, 8, 0, 4,  1,    0, 0, 0, 0, 0, 0,	0, 0,	4,  0, 0, 0, 4, 0, 4,	0,  4, 0, 4,  4, 0,   0, 0, 0, 0, 0,   0, 0,  0, 0, 0	},

/*p m d s c s    w s e h c c p i    b c p c h c v    d l d g f f    s h s t m    s g s w a p */
/* disenchantress */
	{  0, 0,  0, 8, 0, 0,  0,    0, 0, 0, 0, 0, 0,	0, 0,	0,  0, 0, 0, 0, 0, 0,	0,  0, 0, 0,  0, 0,   0, 0, 0, 0, 0,   0, 0,  0, 0, 0	},
/* shambling mound */
	{  0, 0,  0, 0, 0, 10, 0,    0, 0, 0, 0, 0, 0,	0, 0,	0,  0, 0, 0, 0, 0, 0,	0,  0, 0, 0,  0, 0,   0, 0, 0, 0, 0,   0, 0,  0, 0, 0	},
/* yellow mold */
	{  0, 0,  0, 8, 0, 10, 1,    0, 0, 0, 0, 0, 0,	0, 0,	0,  0, 0, 0, 4, 0, 0,	0,  0, 0, 0,  4, 0,   0, 0, 0, 0, 0,   0, 0,  0, 0, 0	},
/* umber hulk */
	{  0, 7,  0, 0, 0, 0,  0,    0, 0, 0, 0, 0, 0,	5, 0,	0,  0, 0, 0, 0, 0, 0,	0,  0, 0, 0,  0, 0,   0, 0, 0, 0, 0,   0, 0,  0, 0, 0	},
/* gnome king */
	{  0, 7,  0, 0, 3, 0,  0,    0, 0, 0, 0, 0, 0,	5, 0,	0,  9, 0, 0, 0, 0, 0,	0,  0, 0, 0,  0, 0,   0, 0, 0, 0, 0,   0, 0,  0, 0, 0	},

/* mimic */
	{  0, 0,  0, 0, 0, 0,  2,    0, 0, 0, 0, 0, 0,	0, 0,	0,  0, 0, 0, 0, 0, 0,	0,  0, 0, 0,  0, 0,   0, 0, 0, 0, 0,   0, 0,  0, 0, 0	},
/* water lord */
	{  0, 13, 0, 8, 3, 4,  1,    0, 0, 0, 0, 0, 0,	0, 0,	0,  9, 0, 0, 4, 0, 0,	0,  0, 0, 16, 4, 0,   0, 0, 0, 0, 0,   0, 0,  0, 0, 0	},
/* bronze dragon */
	{  0, 7,  0, 0, 0, 0,  0,    0, 0, 0, 0, 0, 0,	0, 0,	0,  0, 0, 0, 0, 0, 0,	0,  0, 0, 0,  0, 0,   0, 0, 0, 0, 0,   0, 0,  0, 0, 0	},
/* green dragon */
	{  0, 7,  0, 0, 0, 0,  11,   0, 0, 0, 0, 0, 0,	0, 0,	0,  0, 0, 0, 0, 0, 0,	0,  0, 0, 0,  0, 0,   0, 0, 0, 0, 0,   0, 0,  0, 0, 0	},
/* purple worm */
	{  0, 0,  0, 0, 0, 0,  0,    0, 0, 0, 0, 0, 0,	0, 0,	0,  0, 0, 0, 0, 0, 0,	0,  0, 0, 0,  0, 0,   0, 0, 0, 0, 0,   0, 0,  0, 0, 0	},

/*p m d s c s    w s e h c c p i    b c p c h c v    d l d g f f    s h s t m    s g s w a p */
/* xvart */
	{  0, 13, 0, 0, 0, 0,  0,    0, 0, 0, 0, 0, 0,	0, 0,	0,  0, 0, 0, 0, 0, 0,	0,  0, 0, 4,  4, 0,   0, 0, 0, 0, 0,   0, 0,  0, 0, 0	},
/* spirit naga */
	{  0, 13, 0, 8, 3, 4,  1,    0, 0, 0, 0, 0, 14, 5, 0,	4,  9, 0, 0, 4, 0, 4,	0,  4, 0, 4,  4, 0,   0, 0, 0, 0, 0,   0, 0,  0, 0, 0	},
/* silver dragon */
	{  0, 6,  0, 9, 0, 0,  12,   0, 0, 0, 0, 0, 0,	0, 0,	0,  9, 0, 0, 0, 0, 0,	0,  0, 0, 0,  0, 0,   0, 0, 0, 0, 0,   0, 0,  0, 0, 0	},
/* platinum dragon */
	{  0, 7,  0, 9, 0, 0,  11,   0, 0, 0, 0, 0, 14, 0, 0,	0,  9, 0, 0, 0, 0, 0,	0,  0, 0, 0,  0, 0,   0, 0, 0, 0, 0,   0, 0,  0, 0, 0	},
/* green urchin */
	{  0, 0,  0, 0, 0, 0,  0,    0, 0, 0, 0, 0, 0,	0, 0,	0,  0, 0, 0, 0, 0, 0,	0,  0, 0, 0,  0, 0,   0, 0, 0, 0, 0,   0, 0,  0, 0, 0	},
/* red dragon */
	{  0, 6,  0, 0, 0, 0,  12,   0, 0, 0, 0, 0, 0,	0, 0,	0,  0, 0, 0, 0, 0, 0,	0,  0, 0, 0,  0, 0,   0, 0, 0, 0, 0,   0, 0,  0, 0, 0	},

/*p m d s c s    w s e h c c p i    b c p c h c v    d l d g f f    s h s t m    s g s w a p */
/* demon lord */
	{ 0,  13, 0, 8, 3, 10, 1,    0, 0, 0, 0, 0, 14, 5, 0,	0,  9, 0, 0, 4, 0, 4,	0,  9, 0, 4,  4, 3,   0, 0, 9, 4, 9,   0, 0,  0, 0, 0	},
/* demon lord */
	{ 0,  13, 0, 8, 3, 10, 1,    0, 0, 0, 0, 0, 14, 5, 0,	0,  9, 0, 0, 4, 0, 4,	0,  9, 0, 4,  4, 3,   0, 0, 9, 4, 9,   0, 0,  0, 0, 0	},
/* demon lord */
	{ 0,  13, 0, 8, 3, 10, 1,    0, 0, 0, 0, 0, 14, 5, 0,	0,  9, 0, 0, 4, 0, 4,	0,  9, 0, 4,  4, 3,   0, 0, 9, 4, 9,   0, 0,  0, 0, 0	},
/* demon lord */
	{ 0,  13, 0, 8, 3, 10, 1,    0, 0, 0, 0, 0, 14, 5, 0,	0,  9, 0, 0, 4, 0, 4,	0,  9, 0, 4,  4, 3,   0, 0, 9, 4, 9,   0, 0,  0, 0, 0	},
/* demon lord */
	{ 0,  13, 0, 8, 3, 10, 1,    0, 0, 0, 0, 0, 14, 5, 0,	0,  9, 0, 0, 4, 0, 4,	0,  9, 0, 4,  4, 3,   0, 0, 9, 4, 9,   0, 0,  0, 0, 0	},
/* demon lord */
	{ 0,  13, 0, 8, 3, 10, 1,    0, 0, 0, 0, 0, 14, 5, 0,	0,  4, 0, 0, 4, 0, 4,	0,  0, 0, 4,  4, 3,   0, 0, 9, 4, 9,   0, 0,  0, 0, 0	},
/* demon lord */
	{ 0,  13, 0, 8, 3, 10, 1,    0, 0, 0, 0, 0, 14, 5, 0,	0,  4, 0, 0, 4, 0, 4,	0,  0, 0, 4,  4, 3,   0, 0, 9, 4, 9,   0, 0,  0, 0, 0	},

/*p m d s c s    w s e h c c p i    b c p c h c v    d l d g f f    s h s t m    s g s w a p */
/* demon prince */
	{ 0,  13, 0, 8, 3, 10, 1,    0, 0, 0, 0, 0, 14, 5, 0,	0,  4, 0, 0, 4, 0, 4,	0,  4, 0, 4,  4, 3,   0, 0, 9, 4, 9,   0, 0,  0, 0, 0	},
/*p m d s c s    w s e h c c p i    b c p c h c v    d l d g f f    s h s t m    s g s w a p */
/* God of Hellfire */
	{  0, 13, 0, 8, 3, 10, 1,    0, 0, 0, 0, 0, 14, 5, 18,	0,  9, 0, 0, 4, 0, 4,	18, 4, 0, 0,  4, 4,   4, 0, 9, 4, 9,   0, 17, 0, 0, 0	}
};

static char *spelmes[] = {
	/*  0 */ "", /* spell has no effect on the monster */
	/*  1 */ "the web had no effect on the %s",
	/*  2 */ "the %s changed shape to avoid the web",
	/*  3 */ "the %s isn't afraid of you",
	/*  4 */ "the %s isn't affected",
	/*  5 */ "the %s can see you with his infravision",
	/*  6 */ "the %s vaporizes your missile",
	/*  7 */ "your missile bounces off the %s",
	/*  8 */ "the %s doesn't sleep",
	/*  9 */ "the %s resists",
	/* 10 */ "the %s can't hear the noise",
	/* 11 */ "the %s's tail cuts it free of the web",
	/* 12 */ "the %s burns through the web",
	/* 13 */ "your missiles pass right through the %s",
	/* 14 */ "the %s sees through your illusions",
	/* 15 */ "the %s loves the cold!",
	/* 16 */ "the %s loves the water!",
	/* 17 */ "the demon is terrified of the %s!",
	/* 18 */ "the %s loves fire and lightning!"
};

static char eys[] = "\nEnter your spell [D for list, ESC to abort]: ";

/* used for alter reality spell */
struct isave {
	char type;      /* 0=item,  1=monster */
	short id;       /* item number or monster number */
	short arg;      /* the type of item or hitpoints of monster */
};

/* =============================================================================
 * Local functions
 */

/* =============================================================================
 * FUNCTION: isconfuse
 *
 * DESCRIPTION:
 * Function to check to see if player is confused.
 * Prints a message if the player is confused indicating that magic can't be
 * used.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   0    => not confused
 *   != 0 => confused (returns confused time remaining)
 */
static int isconfuse(void)
{
	if (c[CONFUSE]) {
		Print(" You can't aim your magic!");
		UlarnBeep();
	}
	return c[CONFUSE];
}

/* =============================================================================
 * FUNCTION: nospell
 *
 * DESCRIPTION:
 * Function to check if a spell affects a monster.
 * Prints an appropriate message if the monster is unaffected by the spell.
 *
 * PARAMETERS:
 *
 *   x     : The spell being cast
 *
 *   monst : The mosnter being hit by the spell
 *
 * RETURN VALUE:
 *
 *   0 => monster affected by the spell
 *   1 => monster is immune to the spell
 */
static int nospell(SpellType x, MonsterIdType monst)
{
	int tmp;

	/* bad spell or monst */
	if ((x >= SPELL_COUNT) || (monst >= MONST_COUNT) || ((signed)monst < 0) || ((signed)x < 0))
		return 0;

	if ((tmp = spelweird[monst][x]) == 0)
		return 0;

	Print("\n");
	Printf(spelmes[tmp], monster[monst].name);
	return 1;
}

/* =============================================================================
 * FUNCTION: do_magic_fx
 *
 * DESCRIPTION:
 * Function to display the magic effect on the map.
 *
 * PARAMETERS:
 *
 *   x  : The x location for the effect
 *
 *   y  : The y location for the effect
 *
 *   fx : The effect type to show.
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void do_magic_fx(int x, int y, MagicEffectsType fx)
{
	int frame;
	int frame_count;

	frame_count = magic_effect_frames(fx);

	for (frame = 0; frame < frame_count; frame++) {
		magic_effect(x, y, fx, frame);
		nap(75);
	}
}

/* =============================================================================
 * FUNCTION: vaporize_rock
 *
 * DESCRIPTION:
 * Function to handle the processing for the vaporise rock spell.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void vaporize_rock(void)
{
	int xl, xh;
	int yl, yh;
	char it;
	MonsterIdType pm;
	int i, j;
	int frame;
	int frame_count;
	int show_effect;

	/* calculate the area for vaporize rock */
	xl = max(playerx - 1, 1);
	yl = max(playery - 1, 1);
	xh = min(playerx + 1, MAXX - 2);
	yh = min(playery + 1, MAXY - 2);

	frame_count = magic_effect_frames(MAGIC_VAPORIZE);

	/* Show fx */
	for (frame = 0; frame < frame_count; frame++) {
		for (i = xl; i <= xh; i++) {
			for (j = yl; j <= yh; j++) {
				/* only show effect on objects/monsters that are rock */
				it = item[i][j];
				pm = mitem[i][j].mon;
				show_effect = 0;
				switch (it) {
				case OWALL:
					/* can't vpr below V2 */
					if (level < VBOTTOM - 2)
						show_effect = 1;
					break;
				case OSTATUE:
				case OTHRONE:
				case OALTAR:
					show_effect = 1;
					break;
				default:
					break;
				}

				switch (pm) {
				/* Rock based monsters take damage from vpr */
				case XORN:
				case TROLL:
					show_effect = 1;
					break;
				default:
					break;
				}

				if (show_effect)
					magic_effect(i, j, MAGIC_VAPORIZE, frame);
			}
		}
		nap(75);
	}

	/* process spell effect */
	for (i = xl; i <= xh; i++) {
		for (j = yl; j <= yh; j++) {
			pm = mitem[i][j].mon;
			it = item[i][j];

			switch (it) {
			case OWALL:
				/* can't vpr below V2 */
				if (level < VBOTTOM - 2)
					it = ONOTHING;
				break;
			case OSTATUE:
				if ((c[HARDGAME] > 3) && (rnd(60) < 30)) {
					/* Redisplay statue */
					show1cell(i, j);
					break;
				}
				it = OBOOK;
				iarg[i][j] = (char)level;
				break;
			case OTHRONE:
				pm = GNOMEKING;
				it = OTHRONE2;
				hitp[i][j] = monster[GNOMEKING].hitpoints;
				break;
			case OALTAR:
				pm = DEMONPRINCE;
				hitp[i][j] = monster[DEMONPRINCE].hitpoints;
				createmonster(DEMONPRINCE);
				createmonster(DEMONPRINCE);
				createmonster(DEMONPRINCE);
				createmonster(DEMONPRINCE);
				break;
			default:
				break;
			}

			switch (pm) {
			/* Rock based monsters take damage from vpr */
			case XORN:
				ifblind(i, j);
				hitm(i, j, 200, 1);
				break;
			case TROLL:
				ifblind(i, j);
				hitm(i, j, 200, 1);
				break;
			default:
				break;
			}

			item[i][j] = it;
		}
	}

	/* Work out the new wall tiles for adjacent walls to those vaporised */
	AnalyseWalls(playerx - 2, playery - 2, playerx + 2, playery + 2);

	for (i = playerx - 2; i <= playerx + 2; i++) {
		for (j = playery - 2; j <= playery + 2; j++) {
			if (checkxy(i, j))
				if ((know[i][j] != OUNKNOWN) && (item[i][j] == OWALL))
					show1cell(i, j);
		}
	}

}

/* =============================================================================
 * FUNCTION: direct
 *
 * DESCRIPTION:
 * Routine to ask for a direction to a spell and then hit the monster.
 *
 * PARAMETERS:
 *
 *   spnum : The spell number
 *
 *   dam   : The amount of damage to be done by the spell
 *
 *   str   : The string to print if the spell does damage.
 *           The first format specifier should be '%s' for the monster name
 *           The second format specifier should be '%d' for the arg.
 *           Not all specifiers are required.
 *
 *   arg   : Any special arguments
 *
 *   fx    : The magic effect to display
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void direct(SpellType spnum, int dam, char *str, int arg, MagicEffectsType fx)
{
	int x, y;
	MonsterIdType m;

	/* check for bad arguments */
	if (((signed)spnum < 0) || (spnum >= SPELL_COUNT) || (str == 0))
		return;

	/* check player confusion */
	if (isconfuse()) return;

	/* Ask for direction */
	dirsub(&x, &y);

	if (!checkxy(x, y))
		/* The direction selected is off the map */
		return;

	m = mitem[x][y].mon;
	if (item[x][y] == OMIRROR) {
		if (spnum == SPELL_SLE) {
			/* sleep */
			Print("You fall asleep! ");
			UlarnBeep();

			arg += 2;
			while (arg-- > 0) {
				parse2();
				nap(1000);
			}
			return;
		}else if (spnum == SPELL_WEB) {
			/* web */
			Print("You get stuck in your own web! ");
			UlarnBeep();
			arg += 2;
			while (arg-- > 0) {
				parse2();
				nap(1000);
			}
			return;
		}else {
			Printf(str, "spell caster (that's you)", (long)arg);
			UlarnBeep();
			losehp(DIED_OWN_MAGIC, dam);
			return;
		}
	}

	if (m == MONST_NONE) {
		Print("  There wasn't anything there!");
		return;
	}

	ifblind(x, y);

	/* Do magic fx */
	do_magic_fx(x, y, fx);
	show1cell(x, y);

	if (nospell(spnum, m)) {
		last_monst_hx = (char)x;
		last_monst_hy = (char)y;
		return;
	}

	Printf(str, lastmonst, (long)arg);
	hitm(x, y, dam, 1);
}

/* =============================================================================
 * FUNCTION: tdirect
 *
 * DESCRIPTION:
 * Routine to ask for a direction to a spell and then teleport away monster.
 *
 * PARAMETERS:
 *
 *   spnum : The spell that wants to teleport the monster
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void tdirect(SpellType spnum)
{
	int x, y;
	MonsterIdType m;

	/* check for bad args */
	if (((signed)spnum < 0) || (spnum >= SPELL_COUNT)) return;

	/* check for player confusion */
	if (isconfuse()) return;

	/* Ask for direction */
	dirsub(&x, &y);

	if (!checkxy(x, y))
		/* The direction selected is off the map */
		return;

	m = mitem[x][y].mon;
	if (m == MONST_NONE) {
		Print("  There wasn't anything there!");
		return;
	}

	ifblind(x, y);

	if (nospell(spnum, m)) {
		last_monst_hx = (char)x;
		last_monst_hy = (char)y;
		return;
	}

	do_magic_fx(x, y, MAGIC_TELEPORT);

	teleportmonst(x, y, m);
}

/* =============================================================================
 * FUNCTION: makewall
 *
 * DESCRIPTION:
 * Function to process the makewall spell.
 *
 * PARAMETERS:
 *
 *   spnum ; The spell creating the wall.
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void makewall(SpellType spnum)
{
	int x, y;
	int tx, ty;

	/* check for bad args */
	if (((signed)spnum < 0) || (spnum >= SPELL_COUNT)) return;

	/* check for player confusion */
	if (isconfuse()) return;

	/* Ask for direction */
	dirsub(&x, &y);

	if (checkxy(x, y)) {
		/* within bounds */

		if (item[x][y] != OWALL) {
			/* can't make anything on walls */

			if (item[x][y] == ONOTHING) {
				/* is it free of items? */
				if (mitem[x][y].mon == MONST_NONE) {
					/* is it free of monsters? */
					if ((level != 1) || (x != 33) || (y != MAXY - 1)) {
						do_magic_fx(x, y, MAGIC_WALL);
						item[x][y] = OWALL;
						show1cell(x, y);

						/* Work out the new wall tiles for adjacent walls */
						AnalyseWalls(x - 1, y - 1, x + 1, y + 1);

						for (tx = x - 1; tx <= x + 1; tx++) {
							for (ty = y - 1; ty <= y + 1; ty++) {
								if (checkxy(tx, ty))
									if ((know[tx][ty] != OUNKNOWN) && (item[tx][ty] == OWALL))
										show1cell(tx, ty);
							}
						}

					}else
						Print("\nyou can't make a wall there!");
				}else
					Print("\nthere's a monster there!");
			}else
				Print("\nthere's something there already!");
		}else
			Print("\nthere's a wall there already!");
	}
}

/* =============================================================================
 * FUNCTION: omnidirect
 *
 * DESCRIPTION:
 * Routine to cast a spell and then hit the monster in all directions.
 *
 * PARAMETERS:
 *
 *   spnum : The spell being cast
 *
 *   dam   : The amount of damage
 *
 *   str   : The format string to dislay for monsters hit by the spell.
 *           The string should contains a '%s' for the monster name.
 *
 *   fx    : The magic efect to display.
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void omnidirect(SpellType spnum, int dam, char *str, MagicEffectsType fx)
{
	int xl, yl;
	int xh, yh;
	int x, y;
	MonsterIdType m;
	int frame;
	int frame_count;

	/* check for bad args */
	if ((signed)spnum < 0 || spnum >= SPELL_COUNT || str == 0) return;

	/* get the area affected */
	xl = max(playerx - 1, 0);
	yl = max(playery - 1, 0);
	xh = min(playerx + 1, MAXX - 1);
	yh = min(playery + 1, MAXY - 1);

	/* Show magic effect */
	frame_count = magic_effect_frames(fx);

	for (frame = 0; frame < frame_count; frame++) {
		for (x = xl; x <= xh; x++) {
			for (y = yl; y <= yh; y++)
				if ((x != playerx) || (y != playery))
					magic_effect(x, y, fx, frame);
		}
		nap(75);
	}

	/* Redisplay cell */
	for (x = xl; x <= xh; x++) {
		for (y = yl; y <= yh; y++)
			if ((x != playerx) || (y != playery))
				show1cell(x, y);
	}

	for (x = xl; x <= xh; x++) {
		for (y = yl; y <= yh; y++) {
			m = mitem[x][y].mon;
			if (m != MONST_NONE) {
				if (nospell(spnum, m) == 0) {
					ifblind(x, y);
					Printc('\n');
					Printf(str, lastmonst);
					hitm(x, y, dam, 1);
					nap(800);
				}else {
					last_monst_hx = (char)x;
					last_monst_hy = (char)y;
				}
			}
		}
	}
}

/* =============================================================================
 * FUNCTION: dirpoly
 *
 * DESCRIPTION:
 * Routine to ask for a direction and polymorph a monst
 *
 * PARAMETERS:
 *
 *   spnum : The spell causing the polymorph.
 *
 * RETURN VALUE:
 *
 *   None.
 */

static void dirpoly(SpellType spnum)
{
	int x, y;
	MonsterIdType m;

	/* check for bad args */
	if ((signed)spnum < 0 || spnum >= SPELL_COUNT) return;

	/* check for player confusion */
	if (isconfuse()) return;

	/* Ask for direction */
	dirsub(&x, &y);

	if (!checkxy(x, y))
		/* The direction selected is off the map */
		return;

	if (mitem[x][y].mon == MONST_NONE) {
		Print("  There wasn't anything there!");
		return;
	}

	ifblind(x, y);

	if (nospell(spnum, mitem[x][y].mon)) {
		last_monst_hx = (char)x;
		last_monst_hy = (char)y;
		return;
	}

	/* show some magic animation */
	do_magic_fx(x, y, MAGIC_SPARKLE);

	/* Transform into a non-genocided monster */
	do{
		m = rnd(MAXMONST + 7);
		mitem[x][y].mon = (char)m;
	} while ((monster[m].flags & FL_GENOCIDED) != 0);

	hitp[x][y] = monster[m].hitpoints;
	show1cell(x, y); /* show the new monster */
}

/* =============================================================================
 * FUNCTION: genmonst
 *
 * DESCRIPTION:
 * Function to ask for monster and genocide from game
 * This is done by setting a flag in the monster[] structure
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void genmonst(void)
{
	int i, j;
	int done;
	int textmode;
	int column;
	int row;

	textmode = 0;
	done = 0;

	while (!done) {
		Print("\nEnter the number of the monster to genocide [* for list]: ");
		i = get_num_input(-1);

		if (i == -1) {
			set_display(DISPLAY_TEXT);
			ClearText();
			textmode = 1;

			Print("Monster numbers");

			column = 0;
			row = 3;
			for (j = 1; j < DEMONLORD; j++) {
				MoveCursor(column * 28 + 1, row);
				if ((monster[i].flags & FL_GENOCIDED) == 0)
					Printf("%-2d) %s", j, monster[j].name);
				else
					Printf("%-2d) Genocided", j);

				column++;
				if (column > 2) {
					column = 0;
					row++;
				}
			}
			Print("\n");
		}else if ((i > 0) && (i < DEMONLORD)) {
			if (((monster[i].flags & FL_GENOCIDED) == 0) || wizard) {
				if (textmode) {
					set_display(DISPLAY_MAP);
					textmode = 0;
				}

				/* genocided from game */
				monster[i].flags |= FL_GENOCIDED;
				Printf("\n  There will be no more %ss.", monster[i].name);

				/* now wipe out monsters on this level */
				newcavelevel(level);
				draws(0, MAXX, 0, MAXY);
				UpdateStatusAndEffects();

				done = 1;
			}
		}else {
			if (textmode) {
				set_display(DISPLAY_MAP);
				textmode = 0;
			}

			Print("\n  You sense failure!");
			done = 1;
		}

	}

	/*
	 * Make sure the map is displayed on exit
	 */

	if (textmode)
		set_display(DISPLAY_MAP);

}


/* =============================================================================
 * FUNCTION: speldamage
 *
 * DESCRIPTION:
 * Function to perform spell actions cast by the player.
 * Please insure that there are 2 spaces before all messages here.
 *
 * PARAMETERS:
 *
 *   Spell : The spell being cast.
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void speldamage(SpellType Spell)
{
	int i, j, clev;
	int xl, yl;
	int xh, yh;
	char *s;

	/* no such spell */
	if (Spell >= SPELL_COUNT) return;

	if (c[TIMESTOP]) {
		/* not if time stopped */
		Print("\n  It didn't seem to work.");
		return;
	}

	if ((rnd(23) == 7) || (rnd(18) > c[INTELLIGENCE])) {
		Print("\n  It didn't work!");
		return;
	}

	clev = c[LEVEL];
	if ((clev * 3 + 2) < (int)Spell) {
		Print("\n  Nothing happens.  You seem inexperienced.");
		return;
	}

	switch (Spell) {
	/* ----- LEVEL 1 SPELLS ----- */

	case SPELL_PRO:
		if (c[PROTECTIONTIME] == 0) {
			c[MOREDEFENSES] += SPELL_PRO_BOOST; /* protection field +2 */
		}
		c[PROTECTIONTIME] += 250;
		do_magic_fx(playerx, playery, MAGIC_SPARKLE);
		return;

	case SPELL_MLE:
		/* magic missile */
		i = rnd(((clev + 1) << 1)) + clev + 3;
		godirect(Spell, i, (clev >= 2) ?
			 "  Your missiles hit the %s." :
			 "  Your missile hit the %s.", 100, EFFECT_MLE);
		return;

	case SPELL_DEX:
		/* dexterity   */
		if (c[DEXCOUNT] == 0)
			adjust_ability(DEXTERITY, SDEXTERITY_BOOST);
		c[DEXCOUNT] += 400;
		do_magic_fx(playerx, playery, MAGIC_SPARKLE);
		return;

	case SPELL_SLE:
		/* sleep   */
		i = rnd(3) + 1;
		s = "  While the %s slept, you smashed it %d times.";
		direct(Spell, fullhit(i), s, i, MAGIC_SLEEP);
		return;

	case SPELL_CHM:
		/* charm monster */
		c[CHARMCOUNT] += c[CHARISMA] << 1;
		do_magic_fx(playerx, playery, MAGIC_SPARKLE);
		return;

	case SPELL_SSP:
		/* sonic spear */
		godirect(Spell, rnd(10) + 15 + clev,
			 "  The sound damages the %s.", 70, EFFECT_SSP);
		return;

	/* ----- LEVEL 2 SPELLS ----- */
	case SPELL_WEB:
		/* web */
		i = rnd(3) + 2;
		s = "  While the %s is entangled, you hit it %d times.";
		direct(Spell, fullhit(i), s, i, MAGIC_WEB);
		return;

	case SPELL_STR:
		/* strength */
		if (c[STRCOUNT] == 0)
			c[STREXTRA] += SSTRENGTH_BOOST;
		c[STRCOUNT] += 150 + rnd(100);
		do_magic_fx(playerx, playery, MAGIC_SPARKLE);
		return;

	case SPELL_ENL:
		/* enlightenment */
		yl = playery - 5;
		yh = playery + 6;
		xl = playerx - 15;
		xh = playerx + 16;
		vxy(xl, yl);
		vxy(xh, yh);                    /* check bounds */
		for (i = yl; i <= yh; i++)      /* enlightenment   */
			for (j = xl; j <= xh; j++)
				know[j][i] = item[j][i];
		draws(xl, xh + 1, yl, yh + 1);
		return;

	case SPELL_HEL:
		/* healing */
		raisehp(20 + (clev << 1));
		do_magic_fx(playerx, playery, MAGIC_SPARKLE);
		return;

	case SPELL_CBL:
		/* cure blindness  */
		c[BLINDCOUNT] = 0;
		do_magic_fx(playerx, playery, MAGIC_SPARKLE);
		return;

	case SPELL_CRE:
		if (wizard) {
			Printf("Number: ");
			if ((i = get_num_input((long)-1)) != -1) {
				createmonster(i);
				return;
			}
		}
		createmonster(makemonst(level + 1) + 8);
		return;

	case SPELL_PHA:
		/* illusion */
		if (rnd(11) + 7 <= c[WISDOM])
			direct(Spell, rnd(20) + 20 + clev, "  The %s believed!", 0, MAGIC_PHANTASMAL);
		else
			Print("  It didn't believe the illusions!");
		return;

	case SPELL_INV:
		/* if he has the amulet of invisibility then add more time */

		j = 0;
		for (i = 0; i < IVENSIZE; i++)
			if (iven[i] == OAMULET)
				j += 1 + ivenarg[i];
		c[INVISIBILITY] += (j << 7) + 12;
		do_magic_fx(playerx, playery, MAGIC_SPARKLE);
		return;

	/* ----- LEVEL 3 SPELLS ----- */
	case SPELL_BAL:
		/* fireball */
		godirect(Spell, rnd(25 + clev) + 25 + clev,
			 "  The fireball hits the %s.", 40, EFFECT_BAL);
		return;

	case SPELL_CLD:
		/* cold */
		godirect(Spell, rnd(25) + 20 + clev,
			 "  The cone of cold strikes the %s.", 60, EFFECT_CLD);
		return;

	case SPELL_PLY:
		/* polymorph */
		dirpoly(Spell);
		return;

	case SPELL_CAN:
		/* cancellation  */
		c[CANCELLATION] += 5 + clev;
		return;

	case SPELL_HAS:
		/* haste self  */
		c[HASTESELF] += 7 + clev;
		do_magic_fx(playerx, playery, MAGIC_SPARKLE);
		return;

	case SPELL_CKL:
		/* cloud kill */
		omnidirect(Spell, 30 + rnd(10), "  The %s gasps for air!", MAGIC_CLOUD);
		return;

	case SPELL_VPR:
		vaporize_rock();
		return;

	/* ----- LEVEL 4 SPELLS ----- */
	case SPELL_DRY:
		/* dehydration */
		direct(Spell, 100 + clev, "  The %s shrivels up.", 0, MAGIC_DEHYDRATE);
		return;

	case SPELL_LIT:
		/* lightning */
		godirect(Spell, rnd(25) + 20 + (clev << 1),
			 "  A lightning bolt hits the %s.", 10, EFFECT_LIT);
		return;

	case SPELL_DRL:
		/* drain life */
		i = min(c[HP] - 1, c[HPMAX] / 2);
		direct(Spell, i + i, "", 0, MAGIC_DRAIN);
		c[HP] -= i;
		return;

	case SPELL_GLO:
		/* globe of invulnerability */
		if (c[GLOBE] == 0) c[MOREDEFENSES] += SPELL_GLOBE_BOOST;
		c[GLOBE] += 200;
		adjust_ability(INTELLIGENCE, -1);
		do_magic_fx(playerx, playery, MAGIC_SPARKLE);
		return;

	case SPELL_FLO:
		/* flood */
		omnidirect(Spell, 32 + clev,
			   "  The %s struggles for air in your flood!",
			   MAGIC_FLOOD);
		return;

	case SPELL_FGR:
		/* finger of death */
		if (rnd(151) == 63) {
			UlarnBeep();
			Print("\nYour heart stopped!\n");
			nap(4000);
			died(DIED_ERASED_BY_WAYWARD_FINGER, 0);
			return;
		}

		if (c[WISDOM] > rnd(10) + 10)
			direct(Spell, 2000, "  The %s's heart stopped.", 0, MAGIC_FINGER);
		else
			Print("  It didn't work.");
		return;

	/* ----- LEVEL 5 SPELLS ----- */
	case SPELL_SCA:
		/* scare monster */
		c[SCAREMONST] += rnd(10) + clev;

		/* if have HANDofFEAR make last longer */
		for (i = 0; i < IVENSIZE; i++) {
			if (iven[i] == OHANDofFEAR) {
				c[SCAREMONST] *= 3;
				break;
			}
		}
		do_magic_fx(playerx, playery, MAGIC_SPARKLE);
		return;

	case SPELL_HLD:
		/* hold monster */
		c[HOLDMONST] += rnd(10) + clev;
		return;

	case SPELL_STP:
		/* time stop */
		c[TIMESTOP] += rnd(20) + (clev << 1);
		return;

	case SPELL_TEL:
		/* teleport */
		tdirect(Spell);
		return;

	case SPELL_MFI:
		/* magic fire */
		omnidirect(Spell, 35 + rnd(10) + clev, "  The %s cringes from the flame.", MAGIC_FIRE);
		return;

	/* ----- LEVEL 6 SPELLS ----- */
	case SPELL_MKW:
		/* make wall */
		makewall(Spell);
		return;

	case SPELL_SPH:
		/* sphere of annihilation */
		if ((rnd(23) == 5) && (wizard == 0)) {
			if (!player_has_item(OSPHTALISMAN)) {
				UlarnBeep();
				Print("\nYou have been enveloped by the zone of nothingness!\n");
				nap(4000);
				died(DIED_SELF_ANNIHLATED, 0);
				return;
			}
		}
		xl = playerx;
		yl = playery;
		adjust_ability(INTELLIGENCE, -1);
		i = dirsub(&xl, &yl);                   /* get direction of sphere */
		newsphere(xl, yl, i, rnd(20) + 11);     /* make a sphere */
		return;

	case SPELL_GEN:
		/* genocide */
		genmonst();
		spelknow[SPELL_GEN]--;
		adjust_ability(INTELLIGENCE, -1);
		return;

	case SPELL_SUM:
		/* summon demon */
		if (rnd(100) > 30) {
			direct(Spell, 150, "  The demon strikes at the %s.", 0, MAGIC_DEMON);
			return;
		}
		if (rnd(100) > 15) {
			Print("  Nothing seems to have happened.");
			return;
		}

		Print("  The demon turned on you and then vanished!");
		UlarnBeep();
		i = rnd(40) + 30;
		losehp(DIED_ATTACKED_BY_DEMON, i); /* must say killed by a demon */
		return;

	case SPELL_WTW:
		/* walk through walls */
		c[WTW] += rnd(10) + 5;
		do_magic_fx(playerx, playery, MAGIC_SPARKLE);
		return;

	case SPELL_ALT:
	{
		/* alter reality */

		struct isave   *save;
		/* pointer to item save structure */
		int sc;
		sc = 0; /* # items saved */
		save = (struct isave *)malloc(sizeof(struct isave) * MAXX * MAXY * 2);
		if (save == (struct isave *)NULL)
			died(DIED_MALLOC_FAILURE, 0);

		/* save all items and monsters */
		for (j = 0; j < MAXY; j++) {
			for (i = 0; i < MAXX; i++) {
				xl = item[i][j];
				if ((xl != ONOTHING) && (xl != OWALL) && (xl != OANNIHILATION)) {
					save[sc].type = 0;
					save[sc].id = item[i][j];
					save[sc++].arg = iarg[i][j];
				}

				if (mitem[i][j].mon) {
					save[sc].type = 1;
					save[sc].id = mitem[i][j].mon;
					save[sc++].arg = hitp[i][j];
				}

				item[i][j] = OWALL;
				mitem[i][j].mon = MONST_NONE;

			}
		}

		eat(1, 1);

		if (level == 1) item[33][MAXY - 1] = ONOTHING;

		for (j = rnd(MAXY - 2), i = 1; i < MAXX - 1; i++)
			item[i][j] = ONOTHING;

		/* put objects back in level */
		while (sc > 0) {
			--sc;
			if (save[sc].type == 0) {
				int trys;

				trys = 100;
				do{
					trys--;
					i = rnd(MAXX - 1);
					j = rnd(MAXY - 1);
				} while ((trys > 0) && (item[i][j] != ONOTHING));

				if (trys) {
					item[i][j] = save[sc].id;
					iarg[i][j] = save[sc].arg;
				}
			}else {
				/* put monsters back in */
				int trys;

				trys = 100;
				do{
					trys--;
					i = rnd(MAXX - 1);
					j = rnd(MAXY - 1);
				} while ((trys > 0) && ((item[i][j] == OWALL) || mitem[i][j].mon));

				if (trys) {
					mitem[i][j].mon = save[sc].id;
					hitp[i][j] = save[sc].arg;
				}
			}
		}

		adjust_ability(INTELLIGENCE, -1);

		AnalyseWalls(0, 0, MAXX, MAXY);

		for (i = 0; i < MAXX; i++) {
			for (j = 0; j < MAXY; j++) {
				if (wizard)
					know[i][j] = item[i][j];
				else
					know[i][j] = OUNKNOWN;
			}
		}
		draws(0, MAXX, 0, MAXY);
		if (wizard == 0) spelknow[SPELL_ALT]--;
		if (save) free((char *)save);
		positionplayer();
		return;
	}

	case SPELL_PER:
		/* permanence */
		adjusttime(-99999L);
		spelknow[SPELL_PER]--; /* forget */
		adjust_ability(INTELLIGENCE, -1);
		do_magic_fx(playerx, playery, MAGIC_SPARKLE);
		return;

	default:
		Printf("  spell %d not available!", (long)Spell);
		UlarnBeep();
		return;
	}
}


/* =============================================================================
 * Exported functions
 */

/* =============================================================================
 * FUNCTION: godirect
 */
void godirect(SpellType spnum, int dam, char *str, int delay, DirEffectsType cshow)
{
	char *it;
	int x, y;
	MonsterIdType m;
	int dir;
	int dx, dy;
	int tx, ty;

	/* check for bad args */
	if (((signed)spnum < 0) || (spnum >= SPELL_COUNT) || (str == 0) || (delay < 0))
		return;

	/* Check player confusion */
	if (isconfuse()) return;

	/* Ask for direction */
	dir = dirsub(&x, &y);

	dx = x - playerx;
	dy = y - playery;
	x = playerx;
	y = playery;

	while (dam > 0) {
		x += dx;
		y += dy;
		if (!checkxy(x, y))
			/* out of bounds */
			break;

		/* if energy hits player */
		if ((x == playerx) && (y == playery)) {
			Print("\nYou are hit by your own magic!");
			UlarnBeep();
			losehp(DIED_OWN_MAGIC, dam);
			return;
		}

		/* if not blind show effect */
		if (c[BLINDCOUNT] == 0) {
			mapeffect(x, y, cshow, dir);
			nap(delay);
			show1cell(x, y);
		}

		/* is there a monster there? */
		if ((m = mitem[x][y].mon) != MONST_NONE) {
			ifblind(x, y);
			/* cannot cast a missile spell at lucifer!! */
			if ((m == LUCIFER) || (m >= DEMONLORD && rnd(100) < 10)) {
				dx *= -1;
				dy *= -1;
				Print("\n");
				Printf("\nthe %s returns your puny missile!",
				       monster[m].name);
			}else {
				if (nospell(spnum, m)) {
					last_monst_hx = (char)x;
					last_monst_hy = (char)y;
					return;
				}
				Printc('\n');
				Printf(str, lastmonst);
				dam -= hitm(x, y, dam, 1);
				show1cell(x, y);
				nap(1000);
				x -= dx;
				y -= dy;
			}
		}else {
			it = &item[x][y];
			switch (*it) {
			case OWALL:
				Printc('\n');
				Printf(str, "wall");
				if (dam >= 50 + c[HARDGAME]) {
					/*enough damage?*/
					if (level < VBOTTOM - 2) {
						/* can't break wall below V2 */
						if ((x < MAXX - 1) && (y < MAXY - 1) && (x) && (y)) {
							Print("  The wall crumbles.");
							*it = ONOTHING;
							show1cell(x, y);

							/* Work out the new wall tiles for adjacent walls */
							AnalyseWalls(x - 1, y - 1, x + 1, y + 1);

							for (tx = x - 1; tx <= x + 1; tx++) {
								for (ty = y - 1; ty <= y + 1; ty++) {
									if (checkxy(tx, ty))
										if ((know[tx][ty] != OUNKNOWN) && (item[tx][ty] == OWALL))
											show1cell(tx, ty);
								}
							}

						}
					}
				}
				dam = 0;
				break;

			case OCLOSEDDOOR:
				Printc('\n');
				Printf(str, "door");
				if (dam >= 40) {
					Print("  The door is blasted apart.");
					*it = ONOTHING;
					show1cell(x, y);
				}
				dam = 0;
				break;

			case OSTATUE:
				Printc('\n');
				Printf(str, "statue");
				if (dam > 44) {
					if ((c[HARDGAME] <= 3) && (rnd(60) < 30)) {
						/*
						 * 50% chance of revealing a book if difficulty <= 3
						 */
						Print("  The statue crumbles.");
						*it = OBOOK;
						iarg[x][y] = (char)level;
						show1cell(x, y);
					}
				}
				dam = 0;
				break;

			case OTHRONE:
				Printc('\n');
				Printf(str, "throne");
				if (dam > 33) {
					/*
					 * If destroying a throne, a gnome king appears
					 */
					mitem[x][y].mon = GNOMEKING;
					hitp[x][y] = monster[GNOMEKING].hitpoints;
					*it = OTHRONE2;
					show1cell(x, y);
				}
				dam = 0;
				break;

			case OMIRROR:
				dx *= -1;
				dy *= -1;
				dir = ReverseDir[dir];
				break;

			default:
				break;
			}
		}

		dam -= 3 + (int)(c[HARDGAME] >> 1);
	}
}

/* =============================================================================
 * FUNCTION: annihilate
 */
void annihilate(void)
{
	int frame_count;
	int frame;
	int xl, yl;
	int xh, yh;
	int x, y;
	long xp;
	MonsterIdType monst;

	/* get the area affected */
	xl = max(playerx - 1, 0);
	yl = max(playery - 1, 0);
	xh = min(playerx + 1, MAXX - 1);
	yh = min(playery + 1, MAXY - 1);

	frame_count = magic_effect_frames(MAGIC_ANNIHILATE);

	for (frame = 0; frame < frame_count; frame++) {
		for (x = xl; x <= xh; x++) {
			for (y = yl; y <= yh; y++)
				if ((x != playerx) || (y != playery))
					magic_effect(x, y, MAGIC_ANNIHILATE, frame);
		}
		nap(75);
	}

	xp = 0;
	for (x = xl; x <= xh; x++) {
		for (y = yl; y <= yh; y++) {
			monst = mitem[x][y].mon;
			if (monst != MONST_NONE) {
				/* if a monster there */
				if (monst < DEMONLORD) {
					xp += monster[monst].experience;
					mitem[x][y].mon = MONST_NONE;
				}else {
					Printf("\nThe %s barely escapes being annihilated!",
					       monster[monst].name);
					/* lose half hit points */
					hitp[x][y] = (short)((hitp[x][y] >> 1) + 1);
				}
			}
		}
	}

	if (xp > 0) {
		Print("\nYou hear loud screams of agony!");
		raiseexperience(xp);
	}

	/* redisplay cells */
	for (x = xl; x <= xh; x++) {
		for (y = yl; y <= yh; y++)
			if ((x != playerx) || (y != playery))
				show1cell(x, y);
	}


}

/* =============================================================================
 * FUNCTION: get_spell_code
 */
void get_spell_code(char *prompt, char *code)
{
	int Pos;
	char a;

	Print(prompt);
	Pos = 0;
	do{
		a = get_prompt_input("", "abcdefghijklmnopqrstuvwxyzD\033", 1);

		if ((Pos == 0) && (a == 'D')) {
			seemagic(-1);
			Print(prompt);
		}else if (a != ESC) {
			Printc((char)a);
			code[Pos] = (char)a;
			Pos++;
		}

	} while ((a != ESC) && (Pos < 3));

	if (a == ESC) {
		/* to escape casting a spell   */
		Print("aborted.");
		code[0] = 0;
		return;

	}
}

/* =============================================================================
 * FUNCTION: cast
 */
void cast(void)
{
	int i;
	int Found;
	char code[3];

	if (c[SPELLS] <= 0) {
		Print("\nYou don't have any spells!");
		return;
	}

	do{
		get_spell_code(eys, code);
		if (code[0] == 0)
			return;

		/* seq search for his spell */
		Found = 0;
		i = 0;
		while ((i < SPELL_COUNT) && !Found) {
			if ((spelcode[i][0] == code[0]) &&
			    (spelcode[i][1] == code[1]) &&
			    (spelcode[i][2] == code[2])) {
				if (spelknow[i]) {
					c[SPELLS]--;
					Printf(" (%s)", spelname[i]);
					speldamage(i);
					Found = 1;
				}
			}

			i++;
		}

		if (!Found)
			Print(" Unknown spell! Try again.");

		recalc();
		UpdateStatusAndEffects();

	} while (!Found);
}



