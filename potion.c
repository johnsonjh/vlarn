/* =============================================================================
 * PROGRAM:  ularn
 * FILENAME: potion.c
 *
 * DESCRIPTION:
 * This module contains definitions and functions for handling potions.
 *
 * =============================================================================
 * EXPORTED VARIABLES
 *
 * potionname : The name of each potion
 *
 * =============================================================================
 * EXPORTED FUNCTIONS
 *
 * newpotion   : Function to create a new potion with the required probabilities
 * quaffpotion : Function to process quaffing a potion.
 *
 * =============================================================================
 */

#include "header.h"
#include "potion.h"
#include "player.h"
#include "monster.h"
#include "dungeon.h"
#include "itm.h"
#include "ularn_win.h"

/* =============================================================================
 * Exported variables
 */

/*  name array for magic potions  */
char *potionname[MAXPOTION] =
{
  " sleep",
  " healing",
  " raise level",
  " increase ability",
  " wisdom",
  " strength",
  " raise charisma",
  " dizziness",
  " learning",
  " gold detection",
  " monster detection",
  " forgetfulness",
  " water",
  " blindness",
  " confusion",
  " heroism",
  " sturdiness",
  " giant strength",
  " fire resistance",
  " treasure finding",
  " instant healing",
  " cure dianthroritis",
  " poison",
  " see invisible",
  "  ",
  "  ",
  "  ",
  "  ",
  "  ",
  "  ",
  "  ",
  "  ",
  "  ",
  "  ",
  "  "
};

/* =============================================================================
 * Local variables
 */

/*
 * Array to return a potion number created with appropriate probability
 * of occurrence.
 * Each potion appears in this array a number of times proportionate to
 * its probability of occurence.
 */

#define POTION_PROB_SIZE 41

static char potprob[POTION_PROB_SIZE] =
{
  PSLEEP, PSLEEP,
  PHEALING, PHEALING, PHEALING,
  PRAISELEVEL,
  PINCABILITY, PINCABILITY, 
  PWISDOM, PWISDOM,
  PSTRENGTH, PSTRENGTH, 
  PCHARISMA, PCHARISMA,
  PDIZZINESS, PDIZZINESS,
  PLEARNING,
  PGOLDDET, PGOLDDET, PGOLDDET,
  PMONSTDET, PMONSTDET, PMONSTDET,
  PFORGETFUL, PFORGETFUL,
  PWATER, PWATER,
  PBLINDNESS,
  PCONFUSION,
  PHEROISM,
  PSTURDINESS,
  PGIANTSTR,
  PFIRERESIST,
  PTREASURE, PTREASURE,
  PINSTHEAL, PINSTHEAL,
  /* No Cure Dianthroritis */
  PPOISON, PPOISON,
  PSEEINVIS, PSEEINVIS
};

/* =============================================================================
 * Exported functions
 */

/* =============================================================================
 * FUNCTION: newpotion
 */
int newpotion(void)
{
  return (potprob[rund(POTION_PROB_SIZE)]);
}

/* =============================================================================
 * FUNCTION: quaffpotion
 */
void quaffpotion(int pot)
{
  int i,j;
  int k;

  /* check for within bounds */
  if (pot<0 || pot>=MAXPOTION) return;

  if (potionknown[pot] == 0) potionknown[pot] = 1;
  Printf("\nYou drink a potion of %s.", &potionname[pot][1]);

  /* 
   * In the switch, if no status/attr changes then use return, 
   * else use break to get the UpdateStatusAndEffects call.
   */
  
  switch (pot)
  {
    case PSLEEP:
      Print("\n  You fall asleep...");
      i=rnd(11)-(c[CONSTITUTION]>>2)+2;
      while(--i>0)
      {
        parse2();
        nap(1000);
      }

      Print("\n.. you wake up.");
      return;

    case PHEALING:
      Print("\n  You feel better.");
      if (c[HP] == c[HPMAX])
      {
        /* Already fully healed, so increase max hp */
        raisemhp(1);
      }
      else if ((c[HP] += rnd(20)+20+c[LEVEL]) > c[HPMAX])
      {
        c[HP]=c[HPMAX];
      }
      break;

    case PRAISELEVEL:
      Print("\n  You feel much more skillful!");
      raiselevel();
      raisemhp(1);
      break;

    case PINCABILITY:
      Print("\n  You feel strange for a moment.");
      i = ABILITY_FIRST + rund(ABILITY_COUNT);
      c[i]++;
      break;

    case PWISDOM:
      Print("\n  You feel more self-confident!");
      c[WISDOM] += rnd(2);
      break;

    case PSTRENGTH:
      Print("\n  Wow!  You feel great!");
      if (c[STRENGTH]<12)
      {
        c[STRENGTH]=12;
      }
      else
      {
        c[STRENGTH]++;
      }
      break;

    case PCHARISMA:
      Print("\n  You feel charismatic!");
      c[CHARISMA]++;
      break;

    case PDIZZINESS:
      Print("\n  You become dizzy!");
      if (--c[STRENGTH] < 3) c[STRENGTH]=3;
      break;

    case PLEARNING:
      Print("\n  You feel clever!");
      c[INTELLIGENCE]++;
      break;

    case PGOLDDET:
      Print("\n  You feel greedy...");
      nap(2000);

      for (i=0; i<MAXY; i++)
      {
        for (j=0; j<MAXX; j++)
        {
          if ((item[j][i]==OGOLDPILE) || (item[j][i]==OMAXGOLD))
          {
            show1cell(j, i);
          }
        }
      }
      showplayer();
      return;

    case PMONSTDET:
      for (i = 0; i < MAXY; i++)
      {
        for (j = 0; j < MAXX; j++)
        {
          if (mitem[j][i].mon)
          {
            show1cell(j, i);
          }
        }
      }
      return;

    case PFORGETFUL:
      Print("\n  You stagger for a moment...");

      for (i=0; i<MAXY; i++)
      {
        for (j=0; j<MAXX; j++)
        {
          know[j][i]=OUNKNOWN;
        }
      }
      nap(2000);
      draws(0, MAXX, 0, MAXY);
      return;

    case PWATER:
      return;

    case PBLINDNESS:
      Print("\n  You can't see anything!");
      c[BLINDCOUNT]+=250;  /* dang, that's a long time. */
      /* erase the character, too! */

      showplayer();
      return;

    case PCONFUSION:
      Print("\n  You feel confused.");
      c[CONFUSE]+= 20+rnd(9);
      return;

    case PHEROISM:
      Print("\n  WOW!  You feel fantastic!");
      if (c[HERO]==0)
      {
        for (i = ABILITY_FIRST ; i <= ABILITY_LAST ; i++)
        {
          c[i] += PHEROISM_BOOST;
        }
      }
      c[HERO] += 250;
      break;

    case PSTURDINESS:
      Print("\n  You feel healthier!");
      c[CONSTITUTION]++;
      break;

    case PGIANTSTR:
      Print("\n  You now have incredible bulging muscles!");
      if (c[GIANTSTR]==0) c[STREXTRA] += PGIANTSTR_BOOST;
      c[GIANTSTR] += 700;
      break;

    case PFIRERESIST:
      Print("\n  You feel a chill run up your spine!");
      c[FIRERESISTANCE] += 1000;
      break;

    case PTREASURE:
      Print("\n  You feel greedy...");
      nap(2000);
      for (i=0; i<MAXY; i++)
      {
        for (j=0; j<MAXX; j++)
        {
          k=item[j][i];
          if ((k==ODIAMOND) || (k==ORUBY) || (k==OEMERALD) ||
              (k==OMAXGOLD) || (k==OSAPPHIRE) || (k==OLARNEYE) ||
              (k==OGOLDPILE))
          {
            show1cell(j,i);
          }
        }
      }
      showplayer();
      return;

    case PINSTHEAL:
      c[HP] = c[HPMAX];
      removecurse();
      break;

    case PCUREDIANTH:
      Print("\n  You don't seem to be affected.");
      return;

    case PPOISON:
      Print("\n  You feel a sickness engulf you!");
      c[HALFDAM] += 200 + rnd(200);
      return;

    case PSEEINVIS:
      Print("\n  You feel your vision sharpen.");
      c[SEEINVISIBLE] += rnd(1000)+400;
      monstnamelist[INVISIBLESTALKER] = 'I';
      return;

    default:
      break;
  }

  /*  show new stats    */
  UpdateStatusAndEffects();
  return;
}
