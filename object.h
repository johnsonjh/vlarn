/* =============================================================================
 * PROGRAM:  ularn
 * FILENAME: object.h
 *
 * DESCRIPTION:
 * This module contains function for handling what a player finds when moving
 * onto a new square in the dungeon.
 *
 * =============================================================================
 * EXPORTED VARIABLES
 *
 * None.
 *
 * =============================================================================
 * EXPORTED FUNCTIONS
 *
 * oopenchest    : Process opening a chest at the player's current location
 * lookforobject : Performs processing for the item found at the player's
 *                 current location.
 *
 * =============================================================================
 */

#ifndef __OBJECT_H
#define __OBJECT_H

/* =============================================================================
 * FUNCTION: oopenchest
 *
 * DESCRIPTION:
 * Function to handle opening a chest.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void oopenchest(void);

/* =============================================================================
 * FUNCTION: lookforobject
 *
 * DESCRIPTION:
 * Function to look for an object at the player's current location and give the
 * player his options if an object was found.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void lookforobject(void);

#endif
