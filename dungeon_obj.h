/* =============================================================================
 * PROGRAM:  ularn
 * FILENAME: dungeon_obj.h
 *
 * DESCRIPTION:
 * This module contains functions for handling the effects of static objects
 * in the dungeon.
 * (ie. Objects that cannot be picked up such as stairs, fountains etc.)
 *
 * =============================================================================
 * EXPORTED VARIABLES
 *
 * None
 *
 * =============================================================================
 * EXPORTED FUNCTIONS
 *
 *  oopendoor   - processes the player opening a closed door
 *  oaltar      - processes player stepping onto an alter
 *  othrone     - processes player stepping onto a throne
 *  odeadthrone - processes player stepping onto a dead throne
 *  ofountain   - processes player stepping onto a fountain
 *  ostairs     - processes player stepping onto the stairs (up or down)
 *  oteleport   - processes player stepping onto a teleport trap, and all other
 *                reasons for teleporting the player
 *  opit        - processes player stepping onto a pit
 *  oelevator   - processes player stepping onto an elevator
 *  ostatue     - processes player stepping onto a statue
 *  omirror     - processes player stepping onto a mirror
 *
 * =============================================================================
 */

#ifndef __DUNGEON_OBJ_H
#define __DUNGEON_OBJ_H

#define ALTAR_PRO_BOOST 3

/* =============================================================================
 * FUNCTION: oopendoor
 *
 * DESCRIPTION:
 * Process the player opening the door at the specified location.
 *
 * PARAMETERS:
 *
 *   x : The x coordinate of the door to open
 *
 *   y : The y coordinate of the door to open
 *
 * RETURN VALUE:
 *
 *   None.
 */
void oopendoor(int x, int y);

/* =============================================================================
 * FUNCTION: oalter
 *
 * DESCRIPTION:
 * Asks if player wants to pray, desecrate or ignore an alter and
 * performs effects based on the selection.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void oaltar(void);

/* =============================================================================
 * FUNCTION: othrone
 *
 * DESCRIPTION:
 * Asks if the player wants to pry jewels, sit on or ignore a throne and
 * performs effects based on the selection.
 * If a jewel is successfully pried from the throne then the throne is
 * changed to a dead throne.
 * If the gnome king is summoned to defend his throne from vandals or usurpers
 * then the throne is transformed to an OTHRONE2 to indicate that the king
 * of this throne has been summoned.
 *
 * PARAMETERS:
 *
 *   arg : This flag indicates if a gnome king has already been summoned for
 *         the throne.
 *           arg == 0 => king not summoned
 *           arg == 1 => king has already been summoned.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void othrone(int arg);

/* =============================================================================
 * FUNCTION: odeadthrone
 *
 * DESCRIPTION:
 * Asks if the player wants to sit on or ignore a dead throne (already had a
 * jewel removed) and performs effects based on the selection.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void odeadthrone(void);

/* =============================================================================
 * FUNCTION: ofountain
 *
 * DESCRIPTION:
 * Asks if the player wants to drink, wash or ignore the fountain and performs
 * effects based on the selection.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void ofountain(void);

/* =============================================================================
 * FUNCTION: ostairs
 *
 * DESCRIPTION:
 * This functions process the stair cases.
 * Asks if the player wants to stay here, or go up or down as appropriate.
 * If the player changes dungeon level then the level change is processed.
 *
 * PARAMETERS:
 *
 *   dir : Indicates the direction the staircase leads.
 *           dir > 0 => up
 *           dir < 0 => down
 *
 * RETURN VALUE:
 *
 *   None.
 */
void ostairs(int dir);

/* =============================================================================
 * FUNCTION: oteleport
 *
 * DESCRIPTION:
 * Handles teleporting the player for teleport traps, scrolls and innate
 * teleport capability.
 *
 * PARAMETERS:
 *
 *   err : Indicates if the teleport may land the player in an untenable
 *         position.
 *           err == 0 => teleport without error (always safe)
 *           err != 0 => teleport may level player stuck in rock.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void oteleport(int err);

/* =============================================================================
 * FUNCTION: opit
 *
 * DESCRIPTION:
 * Handles the processing of a player stepping onto a pit.
 * If the player changes dunction level the the level change is processed.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void opit(void);

/* =============================================================================
 * FUNCTION: oelevator
 *
 * DESCRIPTION:
 * Performs the processing for a player stepping onto an elevator.
 * If the player changes dungron level the the level change is processed.
 *
 * PARAMETERS:
 *
 *   dir : The direction of the elevator.
 *           dir == 1 => up
 *           dir != 1 => down
 *
 * RETURN VALUE:
 *
 *   None.
 */
void oelevator(int dir);

/* =============================================================================
 * FUNCTION: ostatue
 *
 * DESCRIPTION:
 * Performs the processing for when a player steps onto a statue.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void ostatue(void);

/* =============================================================================
 * FUNCTION: omirror
 *
 * DESCRIPTION:
 * Performs the processing for when a player steps onto a mirror.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void omirror(void);

#endif
