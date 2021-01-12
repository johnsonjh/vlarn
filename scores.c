/* =============================================================================
 * PROGRAM:  ularn
 * FILENAME: scores.c
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

#include "header.h"
#include "ularn_game.h"
#include "ularn_win.h"
#include "ularn_ask.h"
#include "monster.h"
#include "itm.h"
#include "dungeon.h"
#include "player.h"
#include "potion.h"
#include "scroll.h"
#include "store.h"
#include "sphere.h"
#include "show.h"
#include "scores.h"

/* =============================================================================
 * Local variables
 */

/*
 * This is a header written to the scoreboard file for version identification
 * If the scoreboard format doesn't match then make a new score board.
 * If you change the score board format then increase SCORE_VERSION by 1
 * to prevent ularn from  attempting to use old format scoreboards.
 * Energetic coders may provide an automatic score board format upgrade.
 */

#define SCORE_VERSION 1

struct score_header_type
{
  char Id[4];
  int  Format;
};

static struct score_header_type CurrentHeader =
{
  { 'u', 'l', 's', 'b' },
  SCORE_VERSION
};

/* This is the structure for the scoreboard   */
struct score_type
{
  long  score;              /* the score of the player  */
  int   suid;               /* the user id number of the player*/
  DiedReasonType reason;    /* the reason the player died */
  short what;               /* the number of the monster that killedplayer */
  short level;              /* the level player was on when he died */
  short hardlev;            /* the level of difficulty player played at */
  short order;              /* the relative ordering place of this entry */
  char  who[LOGNAMESIZE+1]; /* the name of the character    */
  char  char_class[20];     /* the character class */
  short sciv[IVENSIZE][2];  /* this is the inventory list of the character*/
};

/* This is the structure for the winning scoreboard */
struct win_score_type
{
  long  score;              /* the score of the player  */
  short timeused;           /* the time used in mobuls to win the game*/
  long  taxes;              /* taxes he owes to LRS   */
  int   suid;               /* the user id number of the player*/
  short hardlev;            /* the level of difficulty player played at*/
  short order;              /* the relative ordering place of this entry*/
  char  who[LOGNAMESIZE+1]; /* the name of the character    */
  char  char_class[20];     /* the character class */
};

/* storage for the scoreboard  */
static struct score_type scoreboard[SCORESIZE];

/* storage for the winners scoreboard */
static struct win_score_type winboard[SCORESIZE];

/* Died reason messages */
static char *whydead[DIED_COUNT] =
{
  "killed by a monster",
  "quit",
  "suspended",
  "self - annihilated",
  "shot by an arrow",
  "hit by a dart",
  "fell into a pit",
  "fell into a pit to HELL",
  "a winner",
  "trapped in solid rock",
  "killed by a missing save file",
  "killed by an old save file",
  "caught by the greedy cheater checker trap",
  "killed by a protected save file",
  "killed his family and committed suicide",
  "erased by a wayward finger",
  "fell through a trap door to HELL",
  "fell through a trap door",
  "drank some poisonous water",
  "fried by an electric shock",
  "slipped in a volcano shaft",
  "killed by a stupid act of frustration",
  "attacked by a revolting demon",
  "hit by his own magic",
  "demolished by an unseen attacker",
  "fell into the dreadful sleep",
  "killed by an exploding chest",
  "died of internal complications",
  "annihilated by a sphere",
  "died a post mortem death",
  "wasted by a malloc() failure",
  "wasted by an annoyed genie",
  "took an elevator straight to HELL"
};

/*
 * The last error reading/writing the score file
 */
static int scorerror = 0;

/*
 * The score to highlight when showing the score board
 * This is set to the player's score if it is better that the previous score
 */
static int highlight_pos = -1;
static int highlight_win = 0;

/* =============================================================================
 * Local functions
 */

/* =============================================================================
 * FUNCTION: calc_score
 *
 * DESCRIPTION:
 * This function calculates the score for the player.
 *
 * PARAMETERS:
 *
 *   Winner : Indicates if the player is a winner (0 = non-winner, 1 = winner)
 *
 * RETURN VALUE:
 *
 *   The player's score.
 */
static long calc_score(int Winner)
{
  int gold_value;
  int idx;
  int dlev;
  int deepest;
  int score;
  int stupidity_penalty;

  /*
   * Start by calculating the player's net worth, converting items to their
   * value in gold according to what the trading post would pay.
   * This is a bit mean for gems as the bank pays 5x this.
   */
  gold_value = c[GOLD] + c[BANKACCOUNT];

  for (idx = 0 ; idx < IVENSIZE ; idx++)
  {
    gold_value += item_value(iven[idx], ivenarg[idx]);
  }

  if (!Winner)
  {
    /*
     * If the player isn't a winner, devalue score value of gold by 1/10.
     */

    gold_value -= gold_value / 10;
  }
  else
  {
    /* bonus for winning */
    gold_value += 100000 * c[HARDGAME];
  }

  /*
   * Add score for the deepest level visited
   */

  deepest = 0;
  stupidity_penalty = 1;
  for (dlev = 0 ; dlev < NLEVELS ; dlev++)
  {
    if (beenhere[dlev])
    {
      if ((dlev == MAXLEVEL) && (deepest == 0))
      {
        /* The stupid player went directly into the volcano */
        if (Winner)
        {
          /*
           * If the player actually managed to WIN doing this (pretty unlikely)
           * then give a reward
           */
          gold_value *= 2;
        }
        else
        {
          stupidity_penalty = 1;
        }
      }
      else
      {
        deepest = dlev;
      }
    }
  }

  if (stupidity_penalty)
  {
    deepest = 0;
    gold_value /= 2;
  }

  /* Calculate the final score */
  score = gold_value + c[EXPERIENCE] + deepest * 50;

  return score;
}

/* =============================================================================
 * FUNCTION: readboard
 *
 * DESCRIPTION:
 * Function to read in the scoreboard into a static buffer.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   -1 if unable to read in the scoreboard,
 *    0 if all is OK
 */
static int readboard(void)
{
  FILE *fp;
  int n;
  struct score_header_type header;

  fp = fopen(scorefile, "rb");

  if (fp == (FILE *)NULL)
  {
    Printf("Can't open scorefile '%s' for reading\n", scorefile);
    return(-1);
  }

  n = fread((char *) &header, sizeof(struct score_header_type), 1, fp);
  if (n != 1)
  {
    Print("Can't read score board header\n");
    fclose(fp);
    return (-2);
  }

  if (strncmp(header.Id, CurrentHeader.Id, 4) != 0)
  {
    Print("Not a valid ularn score board\n");
    fclose(fp);
    return (-3);
  }

  if (header.Format != CurrentHeader.Format)
  {
    Print("Incorrect score board version\n");
    fclose(fp);
    return (-4);
  }

  n = fread((char *)scoreboard, sizeof(struct score_type) * SCORESIZE, 1, fp);

  if (n != 1)
  {
    Print("Can't read scoreboard\n");
    fclose(fp);
    return(-5);
  }

  n = fread((char *)winboard, sizeof(struct win_score_type) * SCORESIZE, 1, fp);
  if (n != 1)
  {
    Print("Can't read scoreboard");
    fclose(fp);
    return(-6);
  }

  fclose(fp);

  return(0);
}

/* =============================================================================
 * FUNCTION: writeboard
 *
 * DESCRIPTION:
 * Function to write the scoreboard from readboard()'s buffer.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   -1 if unable to write the scoreboard,
 *    0 if all is OK
 */
static int writeboard(void)
{
  FILE *fp;
  int n;

  fp = fopen(scorefile, "wb");
  if (fp == (FILE *)NULL)
  {
    Print("Can't open scorefile for writing\n");
    return(-1);
  }

  n = fwrite((char *) &CurrentHeader, sizeof(struct score_header_type), 1, fp);
  if (n != 1)
  {
    Print("Can't write scorefile\n");
    fclose(fp);
    return(-1);
  }

  n = fwrite((char *)scoreboard, sizeof(struct score_type) * SCORESIZE, 1, fp);
  if (n != 1)
  {
    Print("Can't write scorefile\n");
    fclose(fp);
    return(-1);
  }

  n = fwrite((char *)winboard, sizeof(struct win_score_type) * SCORESIZE, 1, fp);
  if (n != 1)
  {
    Print("Can't write scorefile\n");
    fclose(fp);
    return(-1);
  }

  fclose(fp);

  return(0);
}

/* =============================================================================
 * FUNCTION: sortboard
 *
 * DESCRIPTION:
 * Function to calculate the position of each score in the scoreboard.
 * NOTE:
 * Unused score entries must be assigned positions in order for the score
 * update function to operate correctly.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   0 if no sorting done, else returns 1
 */
static int sortboard(void)
{
  int i, pos;
  long largest_score;
  int  largest_score_idx;

  /* mark all scores as unsorted */
  for (i = 0; i < SCORESIZE; i++)
  {
    scoreboard[i].order = -1;
    winboard[i].order = -1;
  }

  /* work out the position of each score in the visitor's board */
  for (pos = 0 ; pos < SCORESIZE ; pos++)
  {
    /* find the score in position pos */
    largest_score = -1;
    largest_score_idx = -1;
    for (i = 0 ; i < SCORESIZE ; i++)
    {
      if ((scoreboard[i].order < 0) && (scoreboard[i].score >= largest_score))
      {
        largest_score_idx = i;
        largest_score = scoreboard[i].score;
      }
    }

    if (largest_score_idx >= 0)
    {
      scoreboard[largest_score_idx].order = (short) pos;
    }
  }

  /* work out the position of each score in the winner's board */
  for (pos = 0 ; pos < SCORESIZE ; pos++)
  {
    /* find the score in position pos */
    largest_score = -1;
    largest_score_idx = -1;
    for (i = 0; i < SCORESIZE; i++)
    {
      if ((winboard[i].order < 0) && (winboard[i].score >= largest_score))
      {
        largest_score_idx = i;
        largest_score = winboard[i].score;
      }
    }

    if (largest_score_idx >= 0)
    {
      winboard[largest_score_idx].order = (short) pos;
    }
  }

  return(1);
}

/* =============================================================================
 * FUNCTION: newscore
 *
 * DESCRIPTION:
 * Function to add entry to scoreboard.
 *
 * PARAMETERS:
 *
 *   score       - the player's score
 *
 *   winner      - true if the player is a winner
 *
 *   died_reason - the reason the player died
 *
 *   what        - the monster that killed the player (if any).
 *
 * RETURN VALUE:
 *
 * Returns the new position on the scoreboard if the player made the high
 * score list.
 */
static int newscore(long score, int winner, DiedReasonType died_reason, int what)
{
  int i;
  int board_idx;
  long gold;
  long taxes;

  gold = c[GOLD] + c[BANKACCOUNT];
  
  if (readboard() < 0)
  {
    return -1;
  }

  if (cheat)
  {
    /* Cheaters can never get on the scoreboard */
    return -1;
  }
  
  if (winner) 
  {
    /* if a winner then delete all non-winning scores */
    for (i = 0 ; i < SCORESIZE ; i++)
    {
      if (scoreboard[i].suid == userid)
      {
        scoreboard[i].score = 0;
      }
    }

    /* Calculate the amount of tax owing */
    taxes = (long) (outstanding_taxes + (gold * TAXRATE));
    outstanding_taxes = taxes;

    /* 
     * Find where in the winner's score board the player's score is to
     * be stored
     */
    board_idx = -1;
    
    /* if he has a slot on the winning scoreboard update it if greater score*/
    for (i = 0 ; i < SCORESIZE ; i++)
    {
      if (winboard[i].suid == userid) 
      {
        board_idx = i;  
      }
    }

    if (board_idx == -1)
    {
      /* he had no entry. look for last entry and see if he has a greater score */
      for (i = 0 ; i < SCORESIZE ; i++)
      {
        if (winboard[i].order == SCORESIZE - 1) 
        { 
          if (score > winboard[i].score)
          {
            board_idx = i;
          }
        }
      }
    }

    if (board_idx >= 0)
    {
      /* Update the taxes owed */
      winboard[board_idx].taxes = taxes;
      
      if ((winboard[board_idx].score < score) || (c[HARDGAME] > winboard[board_idx].hardlev))
      {
        /* Store the new score in this slot */
        strcpy(winboard[board_idx].who, logname);
        strcpy(winboard[board_idx].char_class, char_class);
        winboard[board_idx].score = score;
        winboard[board_idx].hardlev = (short) c[HARDGAME];
        winboard[board_idx].suid = userid;
        winboard[board_idx].timeused = (short) (gtime / 100);        
      }
    }
    
  } 
  else 
  {
    /* Not a winner, so update the visitors score board */

    /* Find where in the visitor's score board the player's score is to 
     * be stores
     */
    
    board_idx = -1;
    
    /* Check if he already has a slot on the scoreboard */
    for (i =  0 ; i < SCORESIZE ; i++)
    {
      if (scoreboard[i].suid == userid) 
      { 
        board_idx = i; 
      }
    }

    /* he had no entry. look for last entry and see if he has a greater score */
    if (board_idx == -1)
    {
      for (i = 0 ; i < SCORESIZE ; i++)
      {
        if (scoreboard[i].order == SCORESIZE - 1) 
        {
          board_idx = i;
        }
      }
    }

    if (board_idx >= 0)
    {
      if ((scoreboard[board_idx].score < score) || 
          (c[HARDGAME] > scoreboard[board_idx].hardlev)) 
      {
        /* This is a better score, so store the new score in this slot */
        strcpy(scoreboard[board_idx].who, logname);
        strcpy(scoreboard[board_idx].char_class, char_class);
        scoreboard[board_idx].score = score;
        scoreboard[board_idx].reason = died_reason;
        scoreboard[board_idx].what = (short) what;
        scoreboard[board_idx].hardlev = (short) c[HARDGAME];
        scoreboard[board_idx].suid = userid;
        scoreboard[board_idx].level = (char) level;
        for (i = 0 ; i < IVENSIZE ; i++) 
        {
          scoreboard[board_idx].sciv[i][0] = iven[i];
          scoreboard[board_idx].sciv[i][1] = ivenarg[i];
        }
      }
      else
      {
        /* The new score isn't better */
        board_idx = -1;
      }
    }
  }

  /* resort the score board */
  sortboard();
  
  if (writeboard() < 0)
  {
    return -1;
  }

  return board_idx;
}

/* =============================================================================
 * FUNCTION: print_died_reason
 *
 * DESCRIPTION:
 * Print the reason a player died.
 *
 * PARAMETERS:
 *
 *   Reason  : The reason the player died
 *
 *   Monster : The monster involved in the player's death (if any)
 *
 *   lev     : The dungeon level
 *
 * RETURN VALUE:
 *
 *   None
 */
static void print_died_reason(DiedReasonType Reason, int Monster, int lev)
{
  char ch;
  char *mod;

  if (Reason == DIED_MONSTER)
  {
    ch = monster[Monster].name[0];

    if ((ch == 'a') || (ch == 'e') || (ch == 'i') || (ch == 'o') || (ch == 'u'))
    {
      mod = "an";
    }
    else
    {
      mod = "a";
    }
    Printf(" killed by %s %s", mod, monster[Monster].name);
  }
  else
  {
    if (!sex && ((Reason == DIED_KILLED_FAMILY) ||
                 (Reason == DIED_OWN_MAGIC)))
    {
      /* fix up died string for female characters */

      switch (Reason)
      {
        case DIED_KILLED_FAMILY:
          Printf(" killed her family and committed suicide");
          break;

        case DIED_OWN_MAGIC:
          Printf(" hit by her own magic");
          break;

        default:
          break;
      }
    }
    else
    {
      Printf(" %s", whydead[Reason]);
    }
  }

  if (Reason != DIED_WINNER)
  {
    Printf(" on %s\n", levelname[lev]);
  }
  else
  {
    Printf("\n");
  }
}

/* =============================================================================
 * FUNCTION: show_winners
 *
 * DESCRIPTION:
 * Subroutine to print out the winning scoreboard.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   The number of entries onthe winner's score board.
 */
static int show_winners(void)
{
  struct win_score_type *p;
  int win_count;
  int i, j;

  /* is there anyone on the scoreboard? */
  win_count = 0;
  for (i = 0; i < SCORESIZE ; i++)
  {
    if (winboard[i].score != 0)
    {
      win_count++;
    }
  }

  if (win_count > 0)
  {
    ClearText();
    clearpager();
    Print("   Score       Diff   Time Needed  Ularn Winners List\n");
    pager();
    
    /* needed to print out the winners in order */

    for (i = 0; i < win_count; i++)
    {
      for (j = 0; j < SCORESIZE; j++)
      {
        /* pointer to the scoreboard entry */
        p = &winboard[j];

        if ((p->order == i)&& (p->score > 0))
        {
          Printf("%-10ld%8d%8d Mobuls  (%s) %s\n",
                 p->score,
                 p->hardlev,
                 p->timeused,
                 p->char_class,
                 p->who);
          pager();
        }

      }
    }
  }

  /* return number of people on scoreboard */
  return (win_count);
}

/* =============================================================================
 * FUNCTION: show_visitors
 *
 * DESCRIPTION:
 * Subroutine to print out the non-winners scoreboard.
 *
 * PARAMETERS:
 *
 *   x : 0 to list the scores, 1 to list inventories too.
 *
 * RETURN VALUE:
 *
 *   Returns the number of players on scoreboard that were shown
 */
static int show_visitors(int show_inv)
{
  int i, j, n;
  int count;
  int oscrollknown[MAXSCROLL];
  int opotionknown[MAXPOTION];

  /*
   * Make all scrolls and potions known for displaying the scoreboard,
   * storing the old known status for restoration later.
   */
  for (i = 0 ; i < MAXSCROLL ; i++)
  {
    oscrollknown[i] = scrollknown[i];
    scrollknown[i] = 1;
  }

  for (i = 0 ; i < MAXPOTION ; i++)
  {
    opotionknown[i] = potionknown[i];
    potionknown[i] = 1;
  }

  c[WEAR] = -1;
  c[WIELD] = -1;
  c[SHIELD] = -1;

  /* is the scoreboard empty? */
  count = 0;
  for (i = 0 ; i < SCORESIZE ; i++)
  {
    if (scoreboard[i].score != 0)
    {
      count++;
    }
  }

  if (count > 0)
  {
    ClearText();
    clearpager();
    Print("  Score Diff  Ularn Visitor Log\n");
    pager();

    for (i = 0 ; i < count ; i++)
    {
      for (j = 0 ; j < SCORESIZE ; j++)
      {
        if ((scoreboard[j].order == i) && (scoreboard[j].score > 0))
        {
          if ((j == highlight_pos) && (!highlight_win))
          {
            SetFormat(FORMAT_STANDOUT);
          }

          Printf("%7ld %3ld   (%s) %s ",
                 scoreboard[j].score,
                 (long) scoreboard[j].hardlev,
                 scoreboard[j].char_class,
                 scoreboard[j].who);

          print_died_reason(scoreboard[j].reason , scoreboard[j].what, scoreboard[j].level);

          if ((j == highlight_pos) && (!highlight_win))
          {
            SetFormat(FORMAT_NORMAL);
          }

          pager();

          if (show_inv)
          {
            for (n = 0 ; n < IVENSIZE ; n++)
            {
              iven[n] = scoreboard[j].sciv[n][0];
              ivenarg[n] = scoreboard[j].sciv[n][1];
            }

            for (n = 0; n < IVENSIZE; n++)
            {
              if (iven[n] != ONOTHING)
              {
                show3(n);
                pager();
              }
            }
            pager();
          }
        }
      } /* for each score in the score board */
    } /* for each score position */

  } /* if any scores in the scoreboard */

  /* Restore the known scrolls and potions */
  for (i = 0 ; i < MAXSCROLL ; i++)
  {
    scrollknown[i] = oscrollknown[i];
  }

  for (i = 0 ; i < MAXPOTION ; i++)
  {
    potionknown[i] = opotionknown[i];
  }

  return (count); /* return the number of players just shown */
}

/* =============================================================================
 * FUNCTION: show_player_score
 *
 * DESCRIPTION:
 * Subroutine to print out the line showing the player when he is killed.
 *
 * PARAMETERS:
 *
 *   Reason  : The reason the player died
 *
 *   Monster : The monster involved in the player's death.
 *
 *   score   : The player's score.
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void show_player_score(DiedReasonType Reason, int Monster, long score)
{
  char *cls;

  cls = class[c[LEVEL]];

  if (char_class[0] == 0)
  {
    strcpy(char_class, "<unknown>");
  }

  Print("---------------------------------------------------------------\n");
  Print("                 U L A R N    S C O R E S\n");
  Print("---------------------------------------------------------------\n");
  Printf("Score: %ld  Diff: %d  ", score, c[HARDGAME]);
  Printf("Level: %s  Char: %s\n", cls, char_class);
  Printf("\t%s", logname);

  print_died_reason(Reason, Monster, level);

  if (wizard)
  {
    Print(" (wizard)");
  }

  if (cheat)
  {
    Print(" (cheater)");
  }

  Print("\n");
  Print("---------------------------------------------------------------");

}

/* =============================================================================
 * Exported functions
 */

/* =============================================================================
 * FUNCTION: makeboard
 */
int makeboard(void)
{
  int i;

  for (i = 0; i < SCORESIZE; i++)
  {
    winboard[i].taxes = 0;
    winboard[i].score = 0;
    winboard[i].hardlev = 0;
    winboard[i].order = (short) i;

    scoreboard[i].score = 0;
    scoreboard[i].hardlev = 0;
    scoreboard[i].order = (short) i;
  }

  if (writeboard())
  {
    return(-1);
  }

#ifdef UNIX
  chmod(scorefile, 0666);
#endif

  return(0);
}

/* =============================================================================
 * FUNCTION: hashewon
 */
int hashewon(void)
{
  int i;
  int rt;

  c[HARDGAME] = 0;

  rt = readboard(); 

  if (rt < 0)
  {
    return(0);
  }
  
  /* search through winners scoreboard */
  for (i = 0; i < SCORESIZE; i++)
  {
    if ((winboard[i].suid == userid) && (winboard[i].score > 0))
    {
      c[HARDGAME] = winboard[i].hardlev + 1;
      outstanding_taxes = winboard[i].taxes;
      return(1);
    }
  } 

  return(0);
}

/* =============================================================================
 * FUNCTION: paytaxes
 */
long paytaxes(long x)
{
  int i;
  long  amt;

  if (x <= 0)
  {
    return(0L);
  }

  if (readboard() < 0)
  {
    return(0L);
  }

  /* look for players winning entry */
  for (i = 0; i < SCORESIZE; i++)
  {
    if ((winboard[i].suid == userid)&& (winboard[i].score > 0))
    {
      /* calculate the amount to pay (making sure the player doesn't overpay */
      amt = winboard[i].taxes;
      if (x < amt) amt = x;

      winboard[i].taxes -= amt;
      outstanding_taxes -= amt;

      if (writeboard() < 0)
      {
        return(0);
      }

      return(amt);
    }
  }

  return(0L); /* couldn't find user on winning scoreboard */
}

/* =============================================================================
 * FUNCTION: showscores
 */
void showscores(void)
{
  int i, j;

  if (readboard() < 0)
  {
    get_prompt_input("\nPress return to continue", "\015", 0);
    return;
  }
  
  i = show_winners();

  if (i > 0)
  {
    get_prompt_input("\nPress return to continue", "\015", 0);
  }
  
  j = show_visitors(0);

  if ((i + j) == 0)
    Print("\nThe scoreboard is empty.\n");
  else
    Print("\n");

  get_prompt_input("\nPress return to continue", "\015", 0);

}

/* =============================================================================
 * FUNCTION: showallscores
 */
void showallscores(void)
{
  int i, j;

  if (readboard() < 0)
  {
    return;
  }

  i = show_winners();
  j = show_visitors(1);

  if ((i + j) == 0)
  {
    Print("\nThe scoreboard is empty.\n");
  }
  else
  {
    Print("\n");
  }

  get_prompt_input("\nPress return to continue", "\015", 0);

}

/* =============================================================================
 * FUNCTION: endgame
 */
void endgame(void)
{
  /* deallocate any allocated memory */

  free_cells();
  free_spheres();

  /* close the application */
  close_app();

  /* and exit */
  exit(0);
}

/* =============================================================================
 * FUNCTION: died
 */
void died(DiedReasonType Reason, int Monster)
{
  int win;
  int can_revive;
  long score;

  can_revive = 1;

  if (c[LIFEPROT] > 0)
  {
    /* if life protection */
    switch (Reason)
    {
      case DIED_QUIT:
      case DIED_SUSPENDED:
      case DIED_FELL_INTO_BOTTOMLESS_PIT:
      case DIED_WINNER:
      case DIED_MISSING_SAVE_FILE:
      case DIED_OLD_SAVE_FILE:
      case DIED_GREEDY_CHEATER:
      case DIED_PROTECTED_SAVE_FILE:
      case DIED_KILLED_FAMILY:
      case DIED_FELL_THROUGH_BOTTOMLESS_TRAPDOOR:
      case DIED_INTERNAL_COMPLICATIONS:
      case DIED_POST_MORTEM_DEATH:
      case DIED_MALLOC_FAILURE:
      case DIED_ELEVATOR_TO_HELL:
      case DIED_QUICK_QUIT:
        can_revive = 0;
        break;

      default:
        break;
    }

    if (can_revive)
    {
      --c[LIFEPROT];
      c[HP] = c[HPMAX];
      --c[CONSTITUTION];
      Print("\nYou feel wiiieeeeerrrrrd all over! ");
      UlarnBeep();
      nap(4000);

      /* only case where died() returns */
      return;
    }
  }

  /* remove checkpoint file if used */
  if (ckpflag)
    unlink(ckpfile);

  /* if we are not to display the scores */
  if ((Reason == DIED_QUICK_QUIT) || (Reason == DIED_SUSPENDED))
  {
    /* for quick exit or saved game */
    ClearText();
    Printf("Tidying up...\n");
    
    endgame();
  }

  win = (Reason == DIED_WINNER);

  /* Now calculate the player's final score */
  score = calc_score(win);

  set_display(DISPLAY_TEXT);
  ClearText();

  /* Show the players final score */
  show_player_score(Reason, Monster, score);

  get_prompt_input("\nPress return to continue", "\015", 0);


  if (!(wizard || cheat))
  {
    /* wizards and cheaters can't get on the score boards */

    highlight_pos = newscore(score, win, Reason, Monster);
    highlight_win = win;
  }

  if (scorerror == 0)
  {
    /* if we updated the scoreboard*/
    showscores();
  }

  /* if (win && mail) mailbill(); */

  ClearText();
  Print("\n");

  /* exit the game */
  endgame();
}


