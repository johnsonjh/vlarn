/* =============================================================================
 * PROGRAM:  ularn
 * FILENAME: sphere.h
 *
 * DESCRIPTION:
 * This module contains functions for maintaining & moving the spheres of
 * annihilation.
 *
 * =============================================================================
 * EXPORTED VARIABLES
 *
 * None
 *
 * =============================================================================
 * EXPORTED FUNCTIONS
 *
 * rmsphere      : Remove a sphere from a location on the map
 * newsphere     : Create a new sphere
 * movsphere     : Move a sphere
 * free_spheres  : Free all allocated spheres
 * write_spheres : Write the spheres to the save file
 * read_spheres  : Read the spheres from the save file
 *
 * =============================================================================
 */

#ifndef __SPHERE_H
#define __SPHERE_H

#include <stdio.h>

/* =============================================================================
 * FUNCTION: rmsphere
 *
 * DESCRIPTION:
 * Function to delete a sphere of annihilation from list.
 *
 * PARAMETERS:
 *
 *   x : The map x coordinate from which the sphere is to be deleted
 *
 *   y : The map y coordinate from which the sphere is to be deleted
 *
 * RETURN VALUE:
 *
 *   None.
 */
void rmsphere (int x, int y);

/* =============================================================================
 * FUNCTION: newsphere
 *
 * DESCRIPTION:
 * Function to create a new sphere of annihilation.
 *
 * PARAMETERS:
 *
 *   x        : The x coordinate of the new sphere
 *
 *   y        : The y coordinate of the new sphere
 *
 *   dir      : The initial direction of travel for the sphere in diroff format
 *
 *   lifetime : The number of turns the sphere is last.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void newsphere (int x, int y, int dir, int life);

/* =============================================================================
 * FUNCTION: movsphere
 *
 * DESCRIPTION:
 * Function to look for and move spheres of annihilation.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void movsphere(void);

/* =============================================================================
 * FUNCTION: free_spheres
 *
 * DESCRIPTION:
 * Function to free all spheres of annihilation.
 * This needs to be called before exitting the game.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void free_spheres(void);

/* =============================================================================
 * FUNCTION: write_spheres
 *
 * DESCRIPTION:
 * Function to write the spheres of annihilation to the save file.
 *
 * PARAMETERS:
 *
 *   fp : A pointer to the save file being written.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void write_spheres(FILE *fp);

/* =============================================================================
 * FUNCTION: read_spheres
 *
 * DESCRIPTION:
 * Function to read the spheres of annihilation from the save file.
 *
 * PARAMETERS:
 *
 *   fp : A pointer to the save file currently being read.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void read_spheres(FILE *fp);

#endif



