/* =============================================================================
 * PROGRAM:  ularn
 * FILENAME: action.h
 *
 * DESCRIPTION:
 * This module contains functions to process the player's actions.
 *
 * =============================================================================
 * EXPORTED VARIABLES
 *
 * None
 *
 * =============================================================================
 * EXPORTED FUNCTIONS
 *
 * run       : Move in a direction until something interesting happens
 * wield     : Wield an item
 * wear      : Wear armour or shield
 * dropobj   : Drop an object
 * readsrc   : Read a scroll ot a book for the inventory
 * eatcookie : Eat a cookie in inventory, and display fortune if possible.
 * quaff     : Drink a potion in inventory
 * opendoor  : Open a closed door (enhanced interface only)
 * closedoor : Close an open door
 * openchest : Open a chest at the current location (enhanced interface only)
 * quit      : Asks if really want to quit.
 * do_create : Create an item (wizard only)
 *
 * =============================================================================
 */

#ifndef __ACTION_H
#define __ACTION_H

/* =============================================================================
 * FUNCTION: run
 *
 * DESCRIPTION:
 * Move the player in the input direction until something interesting is
 * reached.
 *
 * PARAMETERS:
 *
 *   dir : The direction to move the player.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void run (int dir);

/* =============================================================================
 * FUNCTION: wield
 *
 * DESCRIPTION:
 * Function to wield a weapon.
 * Asks the player what to wield and changes the that item.
 *
 * PARAMETERS:
 *
 *   None
 *
 * RETURN VALUE:
 *
 *   None.
 */
void wield (void);

/* =============================================================================
 * FUNCTION: wear
 *
 * DESCRIPTION:
 * Function to wear armour or a shield.
 *
 * PARAMETERS:
 *
 *   None
 *
 * RETURN VALUE:
 *
 *   None.
 */
void wear (void);

/* =============================================================================
 * FUNCTION: dropobj
 *
 * DESCRIPTION:
 * Function to drop an object from the player's inventory.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void dropobj (void);

/* =============================================================================
 * FUNCTION: readscr
 *
 * DESCRIPTION:
 * Read a scroll or book being carried.
 * ASks for what to read then performs appropriate processing for the item
 * selected.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void readscr (void);

/* =============================================================================
 * FUNCTION: eatcookie
 *
 * DESCRIPTION:
 * Function to eat a cookie one is carrying.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void eatcookie (void);

/* =============================================================================
 * FUNCTION: quaff
 *
 * DESCRIPTION:
 * Function to quaff a potion being carried.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void quaff (void);

/* =============================================================================
 * FUNCTION: opendoor
 *
 * DESCRIPTION:
 * Function to open a closed door near the player's current location.
 * Asks for the direction to the door and opens the door at that location
 * (if any).
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void opendoor(void);

/* =============================================================================
 * FUNCTION: closedoor
 *
 * DESCRIPTION:
 * Function to close an open door at the player's current location.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void closedoor(void);

/* =============================================================================
 * FUNCTION: openchest
 *
 * DESCRIPTION:
 * Function to open a chest at the player's current location.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void openchest(void);

/* =============================================================================
 * FUNCTION: quit
 *
 * DESCRIPTION:
 * Function to ask if the player really wants to quit.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void quit (void);

/* =============================================================================
 * FUNCTION: do_create
 *
 * DESCRIPTION:
 * Function to create an item by the player (wizard only)).
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void do_create(void);

#endif

