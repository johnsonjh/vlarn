/* =============================================================================
 * PROGRAM:  ularn
 * FILENAME: fortune.h
 *
 * DESCRIPTION:
 * This module contains functions for displaying fortunes inside fortune
 * cookies.
 *
 * =============================================================================
 * EXPORTED VARIABLES
 *
 * None
 *
 * =============================================================================
 * EXPORTED FUNCTIONS
 *
 * fortune       - Returns a random fortune string.
 * free_fortunes - Frees the fortunes data.
 *
 * =============================================================================
 */

#ifndef __FORTUNE_H
#define __FORTUNE_H

/* =============================================================================
 * FUNCTION: fortune
 *
 * DESCRIPTION:
 * This function returns a random fortune from the ularn fortune file.
 * If the fortune file cannot be read the NULL is returned.
 * On the first call to this function the fortune file is read and the fortunes
 * are stored in memory.
 *
 * PARAMETERS:
 *
 *   file : The name of the file containing the fortunes.
 *
 * RETURN VALUE:
 *
 *   A pointer to the fortune textm or NULL if the file couldn't be read.
 */
char *fortune(char *file);

/* =============================================================================
 * FUNCTION: free_fortunes
 *
 * DESCRIPTION:
 * This function deallocates any memory allocated by fortune().
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void free_fortunes(void);

#endif
