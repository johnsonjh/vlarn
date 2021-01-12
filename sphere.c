/* =============================================================================
 * Program:  ularn
 * FILENAME: sphere.c
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

#include "ularn_game.h"
#include "ularn_win.h"
#include "saveutils.h"
#include "header.h"
#include "monster.h"
#include "player.h"
#include "itm.h"
#include "sphere.h"

/* =============================================================================
 * Local variables
 */

struct sphere
{
  struct sphere *next;  /* pointer to next structure */
  short x,y,lev;    /* location of the sphere */
  short dir;        /* direction sphere is going in */
  short lifetime;   /* duration of the sphere */
};

/*
 * The start of the list of spheres
 */
static struct sphere *spheres = NULL;

/*
 * The next sphere to process when processing the movement for list of spheres.
 * NOTE:
 * This may be changed by rmsphere deleting an unprocessed sphere when
 * two spheres collide.
 */
static struct sphere *sp2;


/* =============================================================================
 * Local functions
 */

/* =============================================================================
 * FUNCTION: sphboom
 *
 * DESCRIPTION:
 * Function to perform the effects of a sphere detonation.
 *
 * PARAMETERS:
 *
 *   x : The x coordinate of the blast
 *
 *   y : The y coordinate of the blast
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void sphboom (int x, int y)
{
  int have_talisman;
  int i;
  int j;
  int xl, xh, yl, yh;

  /* check if the player has the talisman of the sphere */
  have_talisman = player_has_item(OSPHTALISMAN);

  /* sphere explosion ends hold monster and cancellation */
  if (c[HOLDMONST]) c[HOLDMONST] = 1;
  if (c[CANCELLATION]) c[CANCELLATION] = 1;

  xl = x-2;
  yl = y-2;
  xh = x+3;
  yh = y+3;

  if (level == 0)
  {
    /* On the home level the boom can go right to the edge of the map */
    if (xl < 0) xl = 0;
    if (xh > MAXX) xh = MAXX;
    if (yl < 0) yl = 0;
    if (yh > MAXY) yh = MAXY;
  }
  else
  {
    /* In the dungeon the boom stops at the edge wall */
    if (xl < 1) xl = 1;
    if (xh > (MAXX-1)) xh = MAXX-1;
    if (yl < 1) yl = 1;
    if (yh > (MAXY-1)) yh = MAXY-1;
  }

  /* wipe out everything in the range of the sphere */
  for (j = xl ; j < xh ; j++)
  {
    for (i = yl ; i < yh ; i++)
    {
      item[j][i] = ONOTHING;

      if (!mon_has_item(j, i, OSPHTALISMAN))
      {
        /* The monster was caught in the explosion */
        mitem[j][i].mon = MONST_NONE;
      }
      else
      {
        /* The monster has the talisman, and is unaffected */
        Printf("\nThe %s is unaffected by the blast!",
               monster[(int) mitem[j][i].mon].name);
      }

      show1cell(j, i);

      if ((!have_talisman) && (playerx == j) && (playery == i))
      {
        /* player killed in explosion */
        UlarnBeep();
        Print("\nYou were too close to the sphere!");
        nap(3000);
        died(DIED_SPHERE_ANNIHILATION, 0);
      }
    }
  }
  
  /* Analyse wall connections and redisplay */
  AnalyseWalls(xl-1, yl-1, xh, yh);
  
  for (i = xl-1 ; i < xh+1 ; i++)
  {
    for (j = yl-1 ; j < yh+1 ; j++)
    {
      if (checkxy(i, j))
      {
        if ((know[i][j] != OUNKNOWN) && (item[i][j] == OWALL))
        {
          show1cell(i, j);
        }
      }
    }
  }
}

/* =============================================================================
 * FUNCTION: msphere
 *
 * DESCRIPTION:
 * Move 1 sphere of annihiliation.
 *
 * PARAMETERS:
 *
 *   sp     : pointer to the sphere to be moved
 *
 *   create : flag indicating if this is the first creation of the sphere
 *            for creation, don't move the sphere, just place it at the curren
 *            position.
 *
 * RETURN VALUE:
 *
 *   1 if the sphere moved,
 *   0 if the sphere is removed
 */
int msphere(struct sphere *sp, int create)
{
  int x, y;
  int i, j;
  MonsterIdType m;
  int it;
  int have_talisman;
  int mon_has_talisman;

  if (level != sp->lev)
  {
    /* Not on the correct level for the sphere, so return */
    return 1;
  }

  /* no movement if direction not found */
  if (sp->dir >= 9) sp->dir = 0;

  x = sp->x;
  y = sp->y;

  if (!create)
  {
    /*
     * The sphere was not just created, so clear the current location
     * and update the sphere position.
     */

    item[x][y] = ONOTHING;
    know[x][y] = item[x][y];

    /* show the now moved sphere */
    show1cell(x, y);

    x += diroffx[sp->dir];
    y += diroffy[sp->dir];
  }

  /* don't go out of bounds */
  if (level == 0)
  {
    /* on the home level the entire map area is valid */
    vxy(x, y);
  }
  else
  {
    /*
     * On dungeon levels don't move onto the edge walls surrounding the
     * dungeon.
     */
    if (x < 1) x = 1;
    if (x >= MAXX-1) x = MAXX-2;
    if (y < 1) y = 1;
    if (y >= MAXY-1) y = MAXY-2;
  }

  /*
   * store the new sphere position back in the sphere record
   */
  sp->x = (short) x;
  sp->y = (short) y;


  /* check if the player has the talisman of the sphere */
  have_talisman = player_has_item(OSPHTALISMAN);

  m = mitem[x][y].mon;
  it = item[x][y];
  if (m != MONST_NONE)
  {  
    mon_has_talisman = mon_has_item(x, y, OSPHTALISMAN);
  }
  else
  {
    mon_has_talisman = 0;
  }

  /* demons dispel spheres if the player doesn't have the talisman */
  if ((!have_talisman) && (m >= DEMONLORD))
  {
    know[x][y] = item[x][y];
    /* show the demon (ha ha) */
    show1cell(x, y);
    Printf("\nThe %s dispels the sphere!", monster[m].name);
    UlarnBeep();

    /* remove any spheres that are here */
    rmsphere(x, y);

    return 0;
  }

  /* disenchantress cancels spheres if the player doesn't have the talisman */
  if ((!have_talisman) && (m == DISENCHANTRESS))
  {
    Printf("\nThe %s cancels the sphere!", monster[m].name);
    UlarnBeep();

    sphboom(x, y);  /* blow up stuff around sphere */
    rmsphere(x, y); /* remove any spheres that are here */

    return 0;
  }

  /* cancellation cancels spheres if the player doesn't have the talisman */
  if ((!have_talisman) && c[CANCELLATION])
  {
    Print("\nAs the cancellation takes effect, you hear a great earth shaking blast!");
    UlarnBeep();

    sphboom(x, y);  /* blow up stuff around sphere */
    rmsphere(x, y); /* remove any spheres that are here */

    return 0;
  }

  /*
   * Collision of sphere and player is not good if the player doesn't have the
   * talisman
   */
  if ((!have_talisman) && (playerx == x) && (playery == y))
  {
    Print("\nYou have been enveloped by the zone of nothingness!\n");
    UlarnBeep();
    rmsphere(x, y); /* remove any spheres that are here */

    nap(4000);
    died(DIED_SELF_ANNIHLATED, 0);
    return 0;
  }

  /* collision of spheres detonates spheres */
  if (item[x][y] == OANNIHILATION)
  {
    Print("\nTwo spheres of annihilation collide! You hear a great earth-shaking blast!");
    UlarnBeep();
    rmsphere(x, y);

    sphboom(x, y);  /* blow up stuff around sphere */
    rmsphere(x, y); /* remove the other sphere */

    return 0;
  }

  /* The sphere still exists, so put it on the map in the new position */
  item[x][y] = OANNIHILATION;
  
  if (it == OWALL)
  {
    /* Destroyed a wall, so analyse wall connections and redisplay */
    
    AnalyseWalls(x-1, y-1, x+1, y+1);
  
    for (i = x-1 ; i <= x+1 ; i++)
    {
      for (j = y-1 ; j <= y+1 ; j++)
      {
        if (checkxy(i, j))
        {
          if ((know[i][j] != OUNKNOWN) && (item[i][j] == OWALL))
          {
            show1cell(i, j);
          }
        }
      }
    }
  }
  
  if (mon_has_talisman)
  {
    Printf("\nThe %s is unaffected by the sphere of annihilation!", monster[m].name);
  }
  else
  {
    mitem[x][y].mon = MONST_NONE;
  }
  
  know[x][y] = item[x][y];

  show1cell(x,y);

  return 1;
}

/* =============================================================================
 * Exported functions
 */

/* =============================================================================
 * FUNCTION: rmsphere
 */
void rmsphere (int x, int y)
{
  struct sphere *sp;
  struct sphere *prev_sp;
  struct sphere *tmp;

  sp = spheres;
  prev_sp = NULL;

  while (sp != NULL)
  {
    if ((level == sp->lev) && (x == sp->x) && (y == sp->y))
    {
      /*
       * The sphere is on this level and at the location of the sphere to be
       * deleted
       */

      item[x][y] = ONOTHING;
      know[x][y] = item[x][y];

      /* show the now missing sphere */
      show1cell(x,y);
      c[SPHCAST]--;

      tmp = sp;      /* store the ponter to the sphere to be deleted */
      sp = sp->next; /* get a pointer to the next sphere */

      if (tmp == spheres)
      {
        /* remove from the head of the list */
        spheres = sp;
      }

      if (prev_sp != NULL)
      {
        /* link over the sphere to be deleted */
        prev_sp->next = sp;
      }

      if (tmp == sp2)
      {
        /*
         * The sphere being deleted is the next sphere to move, so
         * update the next sphere to move
         */
        sp2 = sp;
      }

      free((char*) tmp);

      /* Found the sphere to be removed, so return */
      return;

    }
    else
    {
      /* advance to the next sphere */
      prev_sp = sp;
      sp = sp->next;
    }
  }
}

/* =============================================================================
 * FUNCTION: newsphere
 */
void newsphere (int x, int y, int dir, int life)
{
  struct sphere *sp;

  /* Allocate the new sphere */
  sp = (struct sphere *) malloc(sizeof(struct sphere));

  if (sp == (struct sphere *) NULL)
  {
    died(DIED_INTERNAL_COMPLICATIONS, 0);
  }

  sp->x = (short) x;
  sp->y = (short) y;
  sp->lev = (short) level;
  sp->dir = (short) dir;
  sp->lifetime = (short) life;
  sp->next = spheres;
  spheres = sp;

  c[SPHCAST]++;

  msphere(sp, 1);

  /*
   * The new sphere is about to move in movemonst , so give the player
   * a chance to see it in the first position it is cast.
   */
  nap(500);

}

/* =============================================================================
 * FUNCTION: movsphere
 */
void movsphere(void)
{
  struct sphere *sp;

  sp = spheres;

  /* look through sphere list */
  while (sp != NULL)
  {
    /*
     * Set the next sphere to process
     * Note: This may be changed by msphere if two spheres collide.
     */
    sp2 = sp->next;

    /* has sphere run out of gas? */

    if (sp->lev == level)
    {
      /*
       * Only move spheres on this level.
       * All other spheres are in stasis.
       */
      sp->lifetime--;
      if (sp->lifetime < 0)
      {
        /* delete sphere */
        rmsphere(sp->x, sp->y);
      }
      else
      {
        /* time to move the sphere */
        if (rnd((int) max(7, c[INTELLIGENCE]>>1)) <= 2)
        {
          /* sphere changes direction */
          sp->dir = (short) rnd(8);
        }

        msphere(sp, 0);
      }
    }

    /*
     * advance to the next sphere
     * Do it this way as the moved sphere may have been deleted, and we don't
     * want to access through unallocated memory do we.
     */
    sp = sp2;
  }
}

/* =============================================================================
 * FUNCTION: free_spheres
 */
void free_spheres(void)
{
  struct sphere *sp;
  struct sphere *next_sp;

  sp = spheres;

  /* look through sphere list */
  while (sp != NULL)
  {
    next_sp = sp->next;

    free((char*) sp);

    sp = next_sp;
  }

  spheres = NULL;

}

/* =============================================================================
 * FUNCTION: write_spheres
 */
void write_spheres(FILE *fp)
{
  int count;
  struct sphere *sp;

  /* count the number of spheres */
  sp = spheres;
  count = 0;
  while (sp != NULL)
  {
    count++;
    sp = sp->next;
  }

  /* write the count */
  bwrite(fp, (char *) &count, sizeof(int));

  sp = spheres;
  while (sp != NULL)
  {
    bwrite(fp, (char *) sp, sizeof(struct sphere));
    sp = sp->next;
  }
}

/* =============================================================================
 * FUNCTION: read_spheres
 */
void read_spheres(FILE *fp)
{
  int count;
  struct sphere *sp;

  spheres = NULL;

  bread(fp, (char *) &count, sizeof(int));

  while (count > 0)
  {
    sp = (struct sphere *) malloc(sizeof(struct sphere));

    bread(fp, (char *) sp, sizeof(struct sphere));

    sp->next = spheres;
    spheres = sp;

    count--;
  }
}
