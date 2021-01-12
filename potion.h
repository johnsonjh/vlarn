/* =============================================================================
 * PROGRAM:  ularn
 * FILENAME: potion.h
 *
 * DESCRIPTION:
 * This module contains definitions and functions for handling potions.
 *
 * =============================================================================
 * EXPORTED VARIABLES
 *
 * potionname : The name of each potion
 *
 * =============================================================================
 * EXPORTED FUNCTIONS
 *
 * newpotion   : Function to create a new potion with the required probabilities
 * quaffpotion : Function to process quaffing a potion.
 *
 * =============================================================================
 */

#ifndef __POTION_H
#define __POTION_H

#define MAXPOTION 35  /* maximum number of potions that are possible  */

/*** Potions ***/
#define PSLEEP        0
#define PHEALING      1
#define PRAISELEVEL   2
#define PINCABILITY   3
#define PWISDOM       4
#define PSTRENGTH     5
#define PCHARISMA     6
#define PDIZZINESS    7
#define PLEARNING     8
#define PGOLDDET      9
#define PMONSTDET    10
#define PFORGETFUL   11
#define PWATER       12
#define PBLINDNESS   13
#define PCONFUSION   14
#define PHEROISM     15
#define PSTURDINESS  16
#define PGIANTSTR    17
#define PFIRERESIST  18
#define PTREASURE    19
#define PINSTHEAL    20
#define PCUREDIANTH  21
#define PPOISON      22
#define PSEEINVIS    23
#define P_MAX        23  /* Greatest defined potion number */

/*
 * The amount to boost ability scores for a potion of heroism
 */
#define PHEROISM_BOOST 11

/*
 * The amount to boos strength for apotion of giant strength
 */
#define PGIANTSTR_BOOST 21

/*
 * Names of all potions
 */
extern char *potionname[MAXPOTION];

/* =============================================================================
 * FUNCTION: newpotion
 *
 * DESCRIPTION:
 * Function return a potion # created with probability of occurrence
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   The potion number as defined above.
 */
int newpotion(void);

/* =============================================================================
 * FUNCTION: quaffpotion
 *
 * DESCRIPTION:
 * Function to handle the effects of drinking a potion.
 *
 * PARAMETERS:
 *
 *   pot : The potion number.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void quaffpotion(int pot);

#endif
