/* =============================================================================
 * PROGRAM:  ularn
 * FILENAME: scroll.c
 *
 * DESCRIPTION:
 * This module handles the processing for scrolls and books.
 *
 * =============================================================================
 * EXPORTED VARIABLES
 *
 * scrollname : The name of each scroll.
 *
 * =============================================================================
 * EXPORTED FUNCTIONS
 *
 * newscroll   : Function to create a new scroll # with the correct probability
 * read_scroll : Function to process reading a scroll.
 * readbook    : Function to process reading a book.
 *
 * =============================================================================
 */

#include "ularn_win.h" 
#include "header.h"
#include "scroll.h"
#include "potion.h"
#include "spell.h"
#include "player.h"
#include "dungeon.h"
#include "dungeon_obj.h"
#include "monster.h"
#include "itm.h"

/* =============================================================================
 * Exported variables
 */

/*  name array for scrolls    */
char *scrollname[MAXSCROLL] =
{
  " enchant armor",
  " enchant weapon",
  " enlightenment",
  " blank paper",
  " create monster",
  " create artifact",
  " aggravate monsters",
  " time warp",
  " teleportation",
  " expanded awareness",
  " haste monsters",
  " monster healing",
  " spirit protection",
  " undead protection",
  " stealth",
  " magic mapping",
  " hold monsters",
  " gem perfection",
  " spell extension",
  " identify",
  " remove curse",
  " annihilation",
  " pulverization",
  " life protection",
  "  ",
  "  ",
  "  ",
  "  "
};

/* =============================================================================
 * Local variables
 */

/*
 * data for the function to read a scroll
 */
static int xh,yh,yl,xl;

/*
 * List of attributes affected by spell extension
 */
#define EXTENSION_COUNT 11
static int exten[EXTENSION_COUNT] =
{
  PROTECTIONTIME,
  DEXCOUNT,
  STRCOUNT,
  CHARMCOUNT,
  INVISIBILITY,
  CANCELLATION,
  HASTESELF,
  GLOBE,
  SCAREMONST,
  HOLDMONST,
  TIMESTOP
};

/*
 *  Array to create scroll numbers with appropriate probability of
 *  occurrence.
 */

#define SCROLL_PROB_SIZE 81

static char scprob[SCROLL_PROB_SIZE] =
{
  SENCHANTARM, SENCHANTARM, SENCHANTARM, SENCHANTARM,
  SENCHANTWEAP, SENCHANTWEAP, SENCHANTWEAP, SENCHANTWEAP, SENCHANTWEAP,
  SENLIGHTEN, SENLIGHTEN, SENLIGHTEN, SENLIGHTEN, SENLIGHTEN, SENLIGHTEN,
  SBLANK, SBLANK, SBLANK, SBLANK, SBLANK,
  SCREATEMONST, SCREATEMONST, SCREATEMONST,
  SCREATEITEM, SCREATEITEM, SCREATEITEM, SCREATEITEM, SCREATEITEM,
  SAGGMONST, SAGGMONST, SAGGMONST, SAGGMONST, SAGGMONST,
  STIMEWARP, STIMEWARP, STIMEWARP, STIMEWARP,
  STELEPORT, STELEPORT, STELEPORT,
  SAWARENESS, SAWARENESS, SAWARENESS, SAWARENESS,
  SHASTEMONST, SHASTEMONST, SHASTEMONST, SHASTEMONST,
  SMONSTHEAL, SMONSTHEAL, SMONSTHEAL,
  SSPIRITPROT, SSPIRITPROT, SSPIRITPROT,
  SUNDEADPROT, SUNDEADPROT, SUNDEADPROT, SUNDEADPROT,
  SSTEALTH, SSTEALTH,
  SMAGICMAP, SMAGICMAP,
  SHOLDMONST, SHOLDMONST, SHOLDMONST,
  SGEMPERFECT, SGEMPERFECT,
  SSPELLEXT, SSPELLEXT,
  SIDENTIFY, SIDENTIFY, SIDENTIFY,
  SREMCURSE, SREMCURSE, SREMCURSE, SREMCURSE,
  SANNIHILATE,
  SPULVERIZE, SPULVERIZE, SPULVERIZE,
  SLIFEPROT
};

/* =============================================================================
 * Exported functions
 */

/* =============================================================================
 * FUNCTION: newscroll
 */
int newscroll(void)
{
  return (scprob[rund(SCROLL_PROB_SIZE)]);
}

/* =============================================================================
 * FUNCTION: read_scroll
 */
void read_scroll(int typ)
{
  int i, j;

  if (typ<0 || typ>=MAXSCROLL) return;  /* be sure we are within bounds */

  if (scrollknown[typ] == 0)
    scrollknown[typ] = 1;
  Printf("\nYou read a scroll of %s.", &(scrollname[typ][1]));

  switch(typ)
  {
    case SENCHANTARM:
      enchantarmor(ENCH_SCROLL);
      return;

    case SENCHANTWEAP:
      enchweapon(ENCH_SCROLL);
      return;

    case SENLIGHTEN:
      yh = min(playery+7,MAXY);
      xh = min(playerx+25,MAXX);
      yl = max(playery-7,0);
      xl = max(playerx-25,0);
      for (i=yl; i<yh; i++)
      {
        for (j=xl; j<xh; j++)
        {
          know[j][i] = item[j][i];
          if (mitem[j][i].mon != MONST_NONE)
          {
            stealth[j][i] |= STEALTH_SEEN;
          }
        }
      }
      nap(2000);
      draws(xl, xh, yl, yh);
      return;

    case SBLANK:
      return;

    case SCREATEMONST:
      createmonster(makemonst(level+1));
      return;

    case SCREATEITEM:
      something(playerx, playery, level);
      return;

    case SAGGMONST:
      c[AGGRAVATE]+=800;
      return;

    case STIMEWARP:
      /*
       *  This code is slightly wrong in that, if gtime is small and
       *  we can't go back by the required number of mobuls, it's
       *  still reported that we did.  I don't think this is
       *  critical -- dmr
       */
      i = (rnd(1000) - 850)/100;
      if (i==0) i=1;

      gtime += 100*i;
      if (gtime < 0) gtime = 0;

      Printf("\nYou go %sward in time by %d mobul%s", (i<0)?"back":"for",
        abs(i), plural(i));

      adjusttime((long)(i*100));/* adjust time for time warping */
      return;

    case STELEPORT:
      oteleport(0);
      return;

    case SAWARENESS:
      c[AWARENESS] += 1800;
      return;

    case SHASTEMONST:
      c[HASTEMONST] += rnd(55)+12;
      Printf("  You feel nervous.");
      return;

    case SMONSTHEAL:
      for (i = 0; i < MAXY ; i++)
      {
        for (j = 0 ; j < MAXX ; j++)
        {
          if (mitem[j][i].mon)
          {
            hitp[j][i] = monster[(int) mitem[j][i].mon].hitpoints;
          }
        }
      }
      Printf("  You feel uneasy.");
      return;

    case SSPIRITPROT:
      c[SPIRITPRO] += 300 + rnd(200);
      UpdateEffects();
      return;

    case SUNDEADPROT:
      c[UNDEADPRO] += 300 + rnd(200);
      UpdateEffects();
      return;

    case SSTEALTH:
      c[STEALTH] += 250 + rnd(250);
      UpdateEffects();
      return;

    case SMAGICMAP:
      for (i=0; i<MAXY; i++)
      {
        for (j=0; j<MAXX; j++)
        {
          know[j][i] = item[j][i];
          if (mitem[j][i].mon != MONST_NONE)
          {
            stealth[j][i] |= STEALTH_SEEN;
          }
        }
      }
      nap(2000);
      draws(0, MAXX, 0, MAXY);
      return;

    case SHOLDMONST:
      c[HOLDMONST] += 30;
      UpdateEffects();
      return;

    case SGEMPERFECT:
      for (i=0; i<IVENSIZE; i++)
      {
        switch(iven[i])
        {
          case ODIAMOND:
          case ORUBY:
          case OEMERALD:
          case OSAPPHIRE:
            j = ivenarg[i];
            j *= 2;
            if ((j <= 0) && ivenarg[i])
            {
              j = 2550;
            }
            ivenarg[i] = (short) j;
            break;

          default:
            break;
        }
      }
      break;

    case SSPELLEXT:
      for (i = 0 ; i < EXTENSION_COUNT ; i++)
      {
        c[exten[i]] <<= 1;
      }
      break;

    case SIDENTIFY:
      for (i=0; i<IVENSIZE; i++)
      {
        if (iven[i]==OPOTION)
        {
          potionknown[ivenarg[i]] = 1;
        }
        if (iven[i]==OSCROLL)
        {
          scrollknown[ivenarg[i]] = 1;
        }
      }
      break;

    case SREMCURSE:
      removecurse();
      break;

    case SANNIHILATE:
      annihilate();
      break;

    case SPULVERIZE:
      godirect(SPELL_LIT, 150, "The ray hits the %s.", 10, EFFECT_LIT);
      break;

    case SLIFEPROT:
      c[LIFEPROT]++;
      break;
  }
}

/* =============================================================================
 * FUNCTION: readbook
 */
void readbook(int arg)
{
  int i, tmp;

  if (arg <= 3)
  {
    tmp = max(splev[arg], 1);

    i = rund(tmp+1);
  }
  else
  {
    if (arg >= NLEVELS)
    {
      arg = NLEVELS-1;
    }

    /* Stop finding lower level spells deeper in the dungeon */

    tmp = max(splev[arg] - 9, 1);
    i = rnd(tmp) + 9;
  }

  /* Learn this spell more */
  spelknow[i]++;

  Printf("\nSpell \"%s\":  %s\n%s.",
    spelcode[i], spelname[i], speldescript[i]);

  /*
   * Boost int 40% of the time
   */
  if (rnd(10)==4)
  {
    Print("\nYou feel clever!");
    c[INTELLIGENCE]++;
    UpdateStatus();
  }
}
