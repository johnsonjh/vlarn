/* =============================================================================
 * PROGRAM:  ularn
 * FILENAME: ularn_ask.c
 *
 * DESCRIPTION:
 * This module provides functions for getting answers to common questions
 * in the game from teh player.
 *
 * =============================================================================
 * EXPORTED VARIABLES
 *
 * None
 *
 * =============================================================================
 * EXPORTED FUNCTIONS
 *
 * more        - Asks press space to continue
 * clearpager  - resets pagination calculations for paged text output
 * pager       - Call to perform pagination ofter each line of text is output
 * getyn       - ASks for a yess no answer
 * getpassword - Get the password for wizard
 * dirsub      - Asks for a direction
 *
 * =============================================================================
 */

#include "ularn_game.h"
#include "ularn_win.h"
#include "ularn_ask.h"
#include "header.h"
#include "player.h"
#include "dungeon.h"

/* =============================================================================
 * Local variables
 */

/* the current line number for paginating text */
static int srcount = 0;

/* =============================================================================
 * Exported functions
 */

/* =============================================================================
 * FUNCTION: more
 */
void more(void)
{
  Print("\n  --- press ");
  Standout("space");
  Print(" to continue --- ");
  get_prompt_input("", " ", 0);
}

/* =============================================================================
 * FUNCTION: clearpager
 */
void clearpager(void)
{
  srcount = 0;
}

/* =============================================================================
 * FUNCTION: pager
 */
void pager(void)
{
  if (++srcount >= 22)
  {
    srcount = 0;
    more();
    ClearText();
  }
}

/* =============================================================================
 * FUNCTION: getyn
 */
int getyn (void)
{
  int i;

  i = get_prompt_input("", "yYnN\033 ", 1);

  if (isspace(i)) i = ESC;

  return (i);
}

/* =============================================================================
 * FUNCTION: getpassword
 */
int getpassword (void)
{
  char gpwbuf[9];

  Print("\nEnter Password: ");

  get_password_input(gpwbuf, 8);

  if (strcmp(gpwbuf, password) != 0)
  {
    Print("\nSorry.\n");
    return(0);
  }
  else
  {
    return(1);
  }
}

/* =============================================================================
 * FUNCTION: dirsub
 */
int dirsub(int *x, int *y)
{
  ActionType ans;
  int d;
  ActionType dir_order[9];

  /*
   * Direction keys. Order must match diroff
   */

  dir_order[0] = ACTION_MOVE_SOUTH;
  dir_order[1] = ACTION_MOVE_EAST;
  dir_order[2] = ACTION_MOVE_NORTH;
  dir_order[3] = ACTION_MOVE_WEST;
  dir_order[4] = ACTION_MOVE_NORTHEAST;
  dir_order[5] = ACTION_MOVE_NORTHWEST;
  dir_order[6] = ACTION_MOVE_SOUTHEAST;
  dir_order[7] = ACTION_MOVE_SOUTHWEST;
  dir_order[8] = 0;

  ans = get_dir_input("\nIn What Direction? ", 1);

  d = 0;
  while (dir_order[d] != ans) d++;

  d++;

  Print(dirname[d]);

  *x = playerx + diroffx[d];
  *y = playery + diroffy[d];

  return (d);
}
