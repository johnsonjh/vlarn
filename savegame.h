/* =============================================================================
 * PROGRAM:  ularn
 * FILENAME: savegame.h
 *
 * DESCRIPTION:
 * This module contains functions for saving and loading the game.
 *
 * =============================================================================
 * EXPORTED VARIABLES
 *
 * None
 *
 * =============================================================================
 * EXPORTED FUNCTIONS
 *
 * savegame    : Function to save the game
 * restoregame : Function to load the game
 *
 * =============================================================================
 */

#ifndef __SAVEGAME_H
# define __SAVEGAME_H

/* =============================================================================
 * FUNCTION: savegame
 *
 * DESCRIPTION:
 * Function to save the current game data to a file.
 *
 * PARAMETERS:
 *
 *   fname : The name of the save file to be written.
 *
 * RETURN VALUE:
 *
 *    0  if successfully written
 *    -1 if an error occurred
 */
int savegame(char *fname);

/* =============================================================================
 * FUNCTION: restoregame
 *
 * DESCRIPTION:
 * Function to restore a game from a file.
 *
 * PARAMETERS:
 *
 *   fname : The name of the file containing the game to be restored.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void restoregame(char *fname);

#endif
