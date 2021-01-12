/* =============================================================================
 * PROGRAM:  ularn
 * FILENAME: store.h
 *
 * DESCRIPTION:
 * This module contaions functions to handle the locations found on the home
 * level and in the dungeon.
 *
 * =============================================================================
 * EXPORTED VARIABLES
 *
 * None
 *
 * =============================================================================
 * EXPORTED FUNCTIONS
 *
 * dndstore    : Handles entering the DND store
 * oschool     : Handles entering the college of larn
 * obank       : Handles entering the bank of larn
 * obank2      : Handles entering the 8th branch of the bank of larn
 * ointerest   : Calculates interest on bank accounts
 * item_value  : Calculates the value of an item as per the trading post
 * otradepost  : Handles entering the trading post.
 * olrs        : Handles entering the Larn Revenue Service
 * opad        : Handles enetering dealer McDope's pad.
 * ohome       : Handes the player returning home.
 * write_store : Writes all shop data to the save file
 * read_store  : Reads all shop data from the save file
 *
 * =============================================================================
 */

#ifndef __STORE_H
#define __STORE_H

#include "itm.h"

/* the tax rate for the LRS = 5%  */
#define TAXRATE (0.05)

/* =============================================================================
 * FUNCTION: dndstore
 *
 * DESCRIPTION:
 * Function for handling entering the DND store.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void dndstore(void);

/* =============================================================================
 * FUNCTION: oschool
 *
 * DESCRIPTION:
 * Function for handling entering the college of larn.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void oschool(void);

/* =============================================================================
 * FUNCTION: obank
 *
 * DESCRIPTION:
 * Function for handling the bank of larn.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void obank(void);

/* =============================================================================
 * FUNCTION: obanks
 *
 * DESCRIPTION:
 * Function for handling the 8th branch of the bank of larn.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void obank2(void);

/* =============================================================================
 * FUNCTION: ointerest
 *
 * DESCRIPTION:
 * Function for calculating bank interest.
 * Updates the player's bank account for interest earned since the last call.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void ointerest(void);

/* =============================================================================
 * FUNCTION: item_value
 *
 * DESCRIPTION:
 * Function for calculating the calue of an item according to the trading post.
 *
 * PARAMETERS:
 *
 *   it    : The item to be valued
 *
 *   itarg : The items arg (# plusses etc).
 *
 * RETURN VALUE:
 *
 *  The value of the item in gold.
 */
int item_value(int it, int itarg);

/* =============================================================================
 * FUNCTION: otradepost
 *
 * DESCRIPTION:
 * Function for handling entering the trading post.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void otradepost(void);

/* =============================================================================
 * FUNCTION: olrs
 *
 * DESCRIPTION:
 * Function for handling entering the larn revenue service.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void olrs(void);

/* =============================================================================
 * FUNCTION: opad
 *
 * DESCRIPTION:
 * Function to handle entering dealer McDope's pad.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void opad(void);

/* =============================================================================
 * FUNCTION: ohome
 *
 * DESCRIPTION:
 * Function to handle the player returning home.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void ohome(void);


/* =============================================================================
 * FUNCTION: write_store
 *
 * DESCRIPTION:
 * Function to write the store data to the save file.
 *
 * PARAMETERS:
 *
 *   fp : File pointer to the save file being written.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void write_store(FILE *fp);

/* =============================================================================
 * FUNCTION: read_store
 *
 * DESCRIPTION:
 * Function to read the store data from the save file.
 *
 * PARAMETERS:
 *
 *   fp : Pointer to the save file being read.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void read_store(FILE *fp);

#endif
