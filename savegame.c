/* =============================================================================
 * PROGRAM:  ularn
 * FILENAME: savegame.c
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

#include <stdio.h>
#include <fcntl.h>

#include "header.h"
#include "savegame.h"
#include "saveutils.h"
#include "ularn_game.h"
#include "ularn_win.h"
#include "monster.h"
#include "player.h"
#include "spell.h"
#include "dungeon.h"
#include "sphere.h"
#include "store.h"
#include "scores.h"
#include "itm.h"

/* =============================================================================
 * Local functions
 */

/* =============================================================================
 * FUNCTION: greedy
 *
 * DESCRIPTION:
 * Subroutine to not allow greedy cheaters.
 * Displays a cheater message and quits the game.
 *
 * PARAMETERS:
 *
 *   None
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void greedy(void)
{
  if (wizard)
    return;

  Print("\n\nI am so sorry but your character is a little TOO good!  Since this\n");
  Print("cannot normally happen from an honest game, I must assume that you cheated.\n");
  Print("Since you are GREEDY as well as a CHEATER, I cannot allow this game\n");
  Print("to continue.\n");
  nap(5000);
  cheat = 1;
  c[GOLD] = c[BANKACCOUNT] = 0;
  died(DIED_GREEDY_CHEATER, 0);
  return;
}

/* =============================================================================
 * FUNCTION: fsorry
 *
 * DESCRIPTION:
 * Subroutine to not allow altered save files and terminate the attempted
 * restart.
 * Sets the cheat flag to disallow scoring for this game.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void fsorry(void)
{
  if (cheat)
    return;

  Print("\nSorry but your savefile has been altered.\n");
  Print("However, since I am a good sport, I will let you play.\n");
  Print("Be advised, though, that you won't be placed on the scoreboard.");
  cheat = 1;
  nap(4000);
}


/* =============================================================================
 * FUNCTION: fcheat
 *
 * DESCRIPTION:
 * Subroutine to not allow game if save file can't be deleted.
 * Displays a message and quits the game.
 *
 * PARAMETERS:
 *
 *   None
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void fcheat(void)
{
  if (wizard)
    return;
  if (cheat)
    return;

  Print("\nSorry but your savefile can't be deleted.  This can only mean\n");
  Print("that you tried to CHEAT by protecting the directory the savefile\n");
  Print("is in.  Since this is unfair to the rest of the Ularn community, I\n");
  Print("cannot let you play this game.\n");
  nap(5000);
  c[GOLD] = c[BANKACCOUNT] = 0;
  died(DIED_PROTECTED_SAVE_FILE, 0);
  return;
}

/* =============================================================================
 * Exported functions
 */

/* =============================================================================
 * FUNCTION: savegame
 */
int savegame(char *fname)
{
  FILE *fp;

  nosignal = 1;

  /* Save the current level to storage */
  savelevel();

  /* make sure the interest on bank deposits is up to date */
  ointerest();

  /*
   * try and create the save file
   */
  fp = fopen(fname, "wb");

  if (fp == NULL)
  {
    Printf("Can't open file <%s> to save game\n", fname);
    nosignal = 0;
    return(-1);
  }

  FileSum = 0;

  write_player(fp);
  write_levels(fp);
  write_store(fp);
  write_monster_data(fp);
  write_spheres(fp);

  /* file sum */
  bwrite(fp, (char *) &FileSum, sizeof(FileSum));

  fclose(fp);

  nosignal = 0;

  return(0);
}

/* =============================================================================
 * FUNCTION: restoregame
 */
void restoregame(char *fname)
{
  int i;
  unsigned int thesum;
  unsigned int asum;
  int TotalAttr;
  FILE *fp;

  fp = fopen(fname, "rb");

  if (fp == NULL)
  {
    Printf("Can't open file <%s> to restore game\n", fname);
    nap(4000);
    c[GOLD] = c[BANKACCOUNT] = 0;
    died(DIED_MISSING_SAVE_FILE, 0);
    return;
  }

  Printf(" Reading data...");
  init_cells();

  FileSum = 0;

  read_player(fp);
  read_levels(fp);
  read_store(fp);
  read_monster_data(fp);
  read_spheres(fp);

  /* sum of everything so far */
  thesum = FileSum;

  bread(fp, (char *)&asum, sizeof(asum));
  if (asum != thesum)
  {
    fsorry();
  }

  fclose(fp);

  lastpx = 0;
  lastpy = 0;

  if (strcmp(fname, ckpfile) == 0)
  {
    fp = fopen(fname, "ab+");

    if (fp == NULL)
    {
      /*
       * Hmmm. We should be able to write to this file, something fishy
       * is going on.
       */
      fcheat();
    }
    else
    {
      fclose(fp);
    }
  }
  else if (unlink(fname) == -1)
  {
    /* can't unlink save file */
    fcheat();
  }

  /*  for the greedy cheater checker  */
  TotalAttr = 0;
  for (i = ABILITY_FIRST ; i < ABILITY_LAST ; i++)
  {
    TotalAttr += c[i];
    if (c[i] > 300)
    {
      greedy();
    }
  }
  
  if (TotalAttr > 600)
  {
    greedy();
  }

  if ((c[HPMAX] > 999) || (c[SPELLMAX] > 125))
  {
    greedy();
  }

  /* XP has been boosted in the save file, so fix character level */
  if ((c[LEVEL] == 25) && (c[EXPERIENCE] > skill[24]))
  {
    long tmp_xp = c[EXPERIENCE]-skill[24]; /* amount to go up */
    c[EXPERIENCE] = skill[24];
    raiseexperience((long) tmp_xp);
  }

  /* Get the current dungeon level from storage */

  getlevel();

}

