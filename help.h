/* =============================================================================
 * PROGRAM:  ularn
 * FILENAME: help.h
 *
 * DESCRIPTION:
 * This module contains functions to display the help file and the welcome
 * screen.
 *
 * format of the Ularn help file:
 *
 * The 1st character of file is the # of pages of help available (ascii digit).
 * The first page (23 lines) of the file is for the introductory message
 * (not counted in above).
 * The pages of help text are 23 lines per page.
 * The help file allows escape sequences for specifying formatting of the
 * text.
 * The currently supported sequences are:
 *   ^[[7m - Set text format to standout  (red)
 *   ^[[8m - Set text format to standout2 (green)
 *   ^[[9m - Set text format to standout3 (blue)
 *   ^[[m  - Set text format to normal.
 *
 * =============================================================================
 * EXPORTED VARIABLES
 *
 * None.
 *
 * =============================================================================
 * EXPORTED FUNCTIONS
 *
 * help    - Display the help file.
 * welcome - Display the welcome message
 *
 * =============================================================================
 */
 
#ifndef __HELP_H
#define __HELP_H

/* =============================================================================
 * FUNCTION: help
 *
 * DESCRIPTION:
 * Function to display the help file.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void help (void);

/* =============================================================================
 * FUNCTION: welcome
 *
 * DESCRIPTION:
 * Function to display the welcome message and background
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void welcome (void);

#endif
