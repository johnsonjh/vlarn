/* =============================================================================
 * PROGRAM:  ularn
 * FILENAME: show.h
 *
 * DESCRIPTION:
 * This module contains functions to print item names and show lists of items.
 *
 * =============================================================================
 * EXPORTED VARIABLES
 *
 * None
 *
 * =============================================================================
 * EXPORTED FUNCTIONS
 *
 * show_plusses - Prints the number of plusses as indicated by the input arg
 * qshowstr     - Show the player's inventory without setting up text mode
 * showstr      - Show the player's invertory, setting/exiting text mode
 * showwear     - Show wearable items
 * showwield    - Show wieldable items
 * showread     - Show readable items
 * showeat      - Show edible items
 * showquaff    - Show quaffable items
 * seemagic     - Show magic spells/scrolls/potions discovered
 * show1        - Show an item name
 * show3        - Show an item name with the numebr of plusses
 *
 * =============================================================================
 */

#ifndef __SHOW_H
#define __SHOW_H

/* =============================================================================
 * FUNCTION: show_plusses
 *
 * DESCRIPTION:
 * Prints the number of plusses indicated by the input parameter.
 *
 * PARAMETERS:
 *
 *   plus : The number of plusses to be printed.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void show_plusses(int plus);

/* =============================================================================
 * FUNCTION: qshowstr
 *
 * DESCRIPTION:
 * Prints the player's inventory to the current text output window.
 * It does not set up or exit text mode.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void qshowstr(void);

/* =============================================================================
 * FUNCTION: showstr
 *
 * DESCRIPTION:
 * Displays the player's inventory.
 * THis sets up text mode on entry and resets map mode on exit.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void showstr(void);

/* =============================================================================
 * FUNCTION: showwear
 *
 * DESCRIPTION:
 * This function displays a list of items in the player's inventory that can
 * be worn.
 *
 * PARAMETERS:
 *
 *   None
 *
 * RETURN VALUE:
 *
 *   None.
 *
 */
void showwear(void);

/* =============================================================================
 * FUNCTION: showwield
 *
 * DESCRIPTION:
 * This function displays a list of items in the player's inventory that can
 * be wielded.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void showwield(void);

/* =============================================================================
 * FUNCTION: showread
 *
 * DESCRIPTION:
 * This function displays a list of items in the player's inventory that can
 * be read.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void showread (void);

/* =============================================================================
 * FUNCTION:  showeat
 *
 * DESCRIPTION:
 * This function displays a list of items in the player's inventory that can
 * be eaten.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void showeat (void);

/* =============================================================================
 * FUNCTION: showquaff
 *
 * DESCRIPTION:
 * This function displays a list of items in the player's inventory that can
 * be quaffed.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void showquaff (void);

/* =============================================================================
 * FUNCTION: seemagic
 *
 * DESCRIPTION:
 * Function to show what magic items have been discovered thus far.
 *
 * PARAMETERS:
 *
 *   arg : This value determines what will be displayed
 *           -1 for just spells,
 *           99 gives all spells in game (for when genie asks you what you want)
 *           anything else will give spells, scrolls & potions
 *
 * RETURN VALUE:
 *
 *   None.
 */
void seemagic(int arg);

/* =============================================================================
 * FUNCTION: show1
 *
 * DESCRIPTION:
 * Show an item in the player's inventory.
 *
 * PARAMETERS:
 *
 *   idx    : The index of the item in the player's inventory
 *
 *   str2   : The list of names for potions or scrolls indexed by ivenarg.
 *            For other items, pass NULL.
 *
 *   known  : An array indicating if the scroll/potion is known.
 *            For other items, pass NULL.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void show1 (int idx, char *str2[], int known[]);

/* =============================================================================
 * FUNCTION: show3
 *
 * DESCRIPTION:
 * Show a single item in the player's inventory, including plusses.
 *
 * PARAMETERS:
 *
 *   index : The inventory position of the item to display.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void show3 (int index);

#endif
