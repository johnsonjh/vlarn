/* =============================================================================
 * PROGRAM:  ularn
 * FILENAME: spell.h
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

#ifndef __SPELL_H
#define __SPELL_H

#include "ularn_win.h"
#include "dungeon.h"

typedef enum
{
  SPELL_PRO, /* Protection             */
  SPELL_MLE, /* Magic missile          */
  SPELL_DEX, /* Dexterity              */
  SPELL_SLE, /* Sleep                  */
  SPELL_CHM, /* Charm monster          */
  SPELL_SSP, /* Sonic spear            */
  SPELL_WEB, /* Web                    */
  SPELL_STR, /* Strength               */
  SPELL_ENL, /* Enlightenment          */
  SPELL_HEL, /* Healing                */
  SPELL_CBL, /* Cure blindness         */
  SPELL_CRE, /* Create monster         */
  SPELL_PHA, /* Phantasmal forces      */
  SPELL_INV, /* Invisibility           */
  SPELL_BAL, /* Fireball               */
  SPELL_CLD, /* Cold                   */
  SPELL_PLY, /* Polymorph              */
  SPELL_CAN, /* Cancellation           */
  SPELL_HAS, /* Haste                  */
  SPELL_CKL, /* Cloud kill             */
  SPELL_VPR, /* Vaporise rock          */
  SPELL_DRY, /* Dehydration            */
  SPELL_LIT, /* Lightning              */
  SPELL_DRL, /* Drain life             */
  SPELL_GLO, /* Invulnerability        */
  SPELL_FLO, /* Flood                  */
  SPELL_FGR, /* Finger of Death        */
  SPELL_SCA, /* Scare monster          */
  SPELL_HLD, /* Hold monster           */
  SPELL_STP, /* Stop time              */
  SPELL_TEL, /* Teleport away          */
  SPELL_MFI, /* Magic fire             */
  SPELL_MKW, /* Make wall              */
  SPELL_SPH, /* Sphere of annihilation */
  SPELL_GEN, /* Genocide               */
  SPELL_SUM, /* Summon demon           */
  SPELL_WTW, /* Walk through walls     */
  SPELL_ALT, /* Alter reality          */
  SPELL_PER, /* Permanence             */
  SPELL_COUNT
} SpellType;

/* The dexterity boost from the DEX spell */
#define SDEXTERITY_BOOST 3
/* The strength boost from the STR spell */
#define SSTRENGTH_BOOST  3
/* The protection boost from the PRO spell */
#define SPELL_PRO_BOOST 2
/* The protection boost from a globe of invulnerability */
#define SPELL_GLOBE_BOOST 10

/*
 * This array defines the highest spell number that may be learnt from a
 * book found on each level of the dungeon
 */
extern char splev[NLEVELS];

/*
 * The spell codes
 */
extern char *spelcode[SPELL_COUNT];

/*
 * The spell names
 */
extern char *spelname[SPELL_COUNT];

/*
 * The spell descriptions
 */
extern char *speldescript[SPELL_COUNT];

/* =============================================================================
 * FUNCTION: godirect
 *
 * DESCRIPTION:
 * Function to hit in a direction from a missile weapon and have it keep on
 * going in that direction until its power is exhausted Enter with the spell
 * number in spnum, the power of the weapon in hp, Printf format string in
 * str, the # of milliseconds to delay between locations in delay, and the
 * character to represent the weapon in cshow. Returns no value.
 *
 * PARAMETERS:
 *
 *   spnum  : The number of the spell being cast
 *
 *   dam    : The initial amount of damage assigned to the spell
 *
 *   delay  : The delay between moving the spell effect in milliseconds
 *
 *   cshow  : The effect to show for this spell.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void godirect(SpellType spnum, int dam, char *str, int delay, DirEffectsType cshow);

/* =============================================================================
 * FUNCTION: annihilate
 *
 * DESCRIPTION:
 * Routine to annihilate all monsters around player (playerx,playery)
 * Gives player experience, but no dropped objects Returns the experience gained
 * from all monsters killed
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void annihilate(void);

/* =============================================================================
 * FUNCTION: get_spell_code
 *
 * DESCRIPTION:
 * Function to get the player to input the three letter spell code.
 *
 * PARAMETERS:
 *
 *   prompt : The prompt to display
 *
 *   code   : A pointer to the string to contain the spell code.
 *            Must be at least 3 characters.
 *            If the spell code entry is aborted (ESC pressed) then the first
 *            character of code is set to 0.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void get_spell_code(char *prompt, char *code);

/* =============================================================================
 * FUNCTION: cast
 *
 * DESCRIPTION:
 * Subroutine called by parse to cast a spell for the player.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void cast(void);

#endif
