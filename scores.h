/* =============================================================================
 * PROGRAM:  ularn
 * FILENAME: scores.h
 *
 * DESCRIPTION:
 * This modules contains functions to handle the scoreboard display and
 * update at the end of the game.
 *
 * =============================================================================
 * EXPORTED VARIABLES
 *
 * None.
 *
 * =============================================================================
 * EXPORTED FUNCTIONS
 *
 * makeboard     : Create a new scoreboard (deleting the old one)
 * hashewon      : Checks if the player has previously won.
 * paytaxes      : Note the payment of taxes.
 * showscores    : Display the scoreboard
 * showallscores : Show scores including inventories.
 * endgame       : Game tidyup and exit function.
 * died          : Function to handle player dying.
 *
 * =============================================================================
 */

#ifndef __SCORES_H
# define __SCORES_H

/* max number of people on a scoreboard max */
# define SCORESIZE 25

/* Player died reasons */
typedef enum {
  DIED_MONSTER,
  DIED_QUIT,
  DIED_SUSPENDED,
  DIED_SELF_ANNIHLATED,
  DIED_SHOT_BY_ARROW,
  DIED_HIT_BY_DART,
  DIED_FELL_INTO_PIT,
  DIED_FELL_INTO_BOTTOMLESS_PIT,
  DIED_WINNER,
  DIED_TRAPPED_IN_SOLID_ROCK,
  DIED_MISSING_SAVE_FILE,
  DIED_OLD_SAVE_FILE,
  DIED_GREEDY_CHEATER,
  DIED_PROTECTED_SAVE_FILE,
  DIED_KILLED_FAMILY,
  DIED_ERASED_BY_WAYWARD_FINGER,
  DIED_FELL_THROUGH_BOTTOMLESS_TRAPDOOR,
  DIED_FELL_THROUGH_TRAPDOOR,
  DIED_DRANK_POISONOUS_WATER,
  DIED_ELECTRIC_SHOCK,
  DIED_SLIPPED_VOLCANO_SHAFT,
  DIED_STUPIDITY,
  DIED_ATTACKED_BY_DEMON,
  DIED_OWN_MAGIC,
  DIED_UNSEEN_ATTACKER,
  DIED_DREADFUL_SLEEP,
  DIED_EXPLODING_CHEST,
  DIED_INTERNAL_COMPLICATIONS,
  DIED_SPHERE_ANNIHILATION,
  DIED_POST_MORTEM_DEATH,
  DIED_MALLOC_FAILURE,
  DIED_ANNOYED_GENIE,
  DIED_ELEVATOR_TO_HELL,
  DIED_QUICK_QUIT,
  DIED_COUNT
} DiedReasonType;

/* =============================================================================
 * FUNCTION: makeboard
 *
 * DESCRIPTION:
 * Function to create a new scoreboard (wipe out old one).
 *
 * PARAMETERS:
 *
 *   None
 *
 * RETURN VALUE:
 *
 *   -1 if unable to write the scoreboard,
 *    0 if all is OK
 */
int makeboard(void);

/* =============================================================================
 * FUNCTION: hashewon
 *
 * DESCRIPTION:
 * Checks if the player has previously won the game.
 * This function also sets c[HARDGAME] to appropriate value -- 0 if not a
 * winner, otherwise the next level of difficulty listed in the winners
 * scoreboard.  This function also sets outstanding_taxes to the value in
 * the winners scoreboard.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   1 if player has won a game before, else 0
 */
int hashewon(void);

/* =============================================================================
 * FUNCTION: paytaxes
 *
 * DESCRIPTION:
 * Function to pay taxes if any are due.
 *
 * PARAMETERS:
 *
 *   x : The amount of tax to pay.
 *
 * RETURN VALUE:
 *
 *   The amount of tax actually paid.
 */
long paytaxes(long x);

/* =============================================================================
 * FUNCTION: showscores
 *
 * DESCRIPTION:
 * Function to show the scoreboard.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void showscores(void);

/* =============================================================================
 * FUNCTION: showallscores
 *
 * DESCRIPTION:
 * Function to show scores and the iven lists that go with them.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void showallscores(void);

/* =============================================================================
 * FUNCTION: endgame
 *
 * DESCRIPTION:
 * Call all tidyup procedures and exit the program.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void endgame(void);

/* =============================================================================
 * FUNCTION: died
 *
 * DESCRIPTION:
 * Routine to note player death and the reason.
 * Called for all end game conditions, including winning.
 *
 * PARAMETERS:
 *
 *   Reason  : The reason the player died
 *
 *   Monster : The monster involved in the player's death (if any)
 *
 * RETURN VALUE:
 *
 *   None.
 */
void died(DiedReasonType Reason, int Monster);

#endif
