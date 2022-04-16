/* =============================================================================
 * PROGRAM:  ularn
 * FILENAME: object.c
 *
 * DESCRIPTION:
 * This module contains function for handling what a player finds when moving
 * onto a new square in the dungeon.
 *
 * =============================================================================
 * EXPORTED VARIABLES
 *
 * None.
 *
 * =============================================================================
 * EXPORTED FUNCTIONS
 *
 * lookforobject : Performs processing for the item found at the player's
 *                 current location.
 *
 * =============================================================================
 */

#include "object.h"
#include "dungeon.h"
#include "dungeon_obj.h"
#include "fortune.h"
#include "header.h"
#include "itm.h"
#include "monster.h"
#include "player.h"
#include "potion.h"
#include "scores.h"
#include "scroll.h"
#include "spell.h"
#include "store.h"
#include "ularn_game.h"
#include "ularn_win.h"

/* =============================================================================
 * Local functions
 */

/* =============================================================================
 * FUNCTION: iopts
 *
 * DESCRIPTION:
 * Function to print the ignore option.
 * Originally used to save program space, now probably not required.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void iopts(void) { Print(", or (i) ignore it? "); }

/* =============================================================================
 * FUNCTION: ignore
 *
 * DESCRIPTION:
 * Function to print 'ignore.'.
 * Originally used to save program space, now probably not required.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void ignore(void) { Print("ignore.\n"); }

/* =============================================================================
 * FUNCTION: opotion
 *
 * DESCRIPTION:
 * Function to handle finding a potion.
 *
 * PARAMETERS:
 *
 *   pot : The potion type found.
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void opotion(int pot) {
  int ans;

  ans = get_prompt_input(
      "\nDo you (d) drink it, (t) take it, or (i) ignore it? ", "dit\033", 1);

  switch (ans) {
  case ESC:
  case 'i':
    ignore();
    break;

  case 'd':
    Print("drink.\n");
    forget(); /*  destroy potion  */
    quaffpotion(pot);
    break;

  case 't':
    Print("take.\n");
    if (take(OPOTION, pot) == 0)
      forget();
    break;

  default:
    break;
  }
}

/* =============================================================================
 * FUNCTION: oscroll
 *
 * DESCRIPTION:
 * Function to process finding a mogic scroll.
 *
 * PARAMETERS:
 *
 *   typ : The type of scroll found.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void oscroll(int typ) {
  int ans;

  Print("\nDo you ");
  if (c[BLINDCOUNT] == 0)
    Print("(r) read it, ");
  Print("(t) take it");
  iopts();

  ans = get_prompt_input("", "irt\033", 1);

  switch (ans) {
  case ESC:
  case 'i':
    ignore();
    break;

  case 'r':
    if (c[BLINDCOUNT])
      break;
    Print("read.");
    /* remove the scroll */
    forget();
    /* read the scroll */
    read_scroll(typ);
    break;

  case 't':
    Print("take.");
    if (take(OSCROLL, typ) == 0)
      forget(); /*  destroy it  */
    break;

  default:
    break;
  }
}

/* =============================================================================
 * FUNCTION: obook
 *
 * DESCRIPTION:
 * Function to process finding a book.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void obook(void) {
  int ans;

  Print("\nDo you ");
  if (c[BLINDCOUNT] == 0)
    Print("(r) read it, ");
  Print("(t) take it");
  iopts();

  ans = get_prompt_input("", "irt\033", 1);

  switch (ans) {
  case ESC:
  case 'i':
    ignore();
    return;

  case 'r':
    if (c[BLINDCOUNT])
      break;
    Print("read.");
    readbook(iarg[playerx][playery]);
    /* no more book */
    forget();
    return;

  case 't':
    Print("take.");
    if (take(OBOOK, iarg[playerx][playery]) == 0)
      forget(); /* no more book */
    return;

  default:
    break;
  }
}

/* =============================================================================
 * FUNCTION: ocookie
 *
 * DESCRIPTION:
 * Function to process finding a cookie.
 *
 * PARAMETERS:
 *
 *   None
 *
 * RETURN VALUE:
 *
 *   None
 */
void ocookie(void) {
  int ans;
  char *p;

  Print("\nDo you (e) eat it, (t) take it");
  iopts();

  ans = get_prompt_input("", "eit\033", 1);

  switch (ans) {
  case ESC:
  case 'i':
    ignore();
    return;

  case 'e':
    Print("eat.\nThe cookie tasted good.");
    forget(); /* no more cookie */
    if (c[BLINDCOUNT])
      return;

    p = fortune(fortfile);
    if (p == (char *)NULL)
      return;

    Print("  A message inside the cookie reads:\n");
    Print(p);
    return;

  case 't':
    Print("take.");
    if (take(OCOOKIE, 0) == 0)
      forget();
    return;

  default:
    break;
  }
}

/* =============================================================================
 * FUNCTION: ogold
 *
 * DESCRIPTION:
 * Function to pick up some gold.
 *
 * PARAMETERS:
 *
 *   arg : The size of the gold pile
 *         if arg ==
 *           ODGOLD   then the pile is worth 10x  the argument
 *           OMAXGOLD then the pile is worth 100x the argument
 *           OKGOLD   then the pile is worth 1000x the argument
 *
 * RETURN VALUE:
 *
 *   None.
 */
void ogold(int arg) {
  long i;

  i = iarg[playerx][playery];

  if (arg == ODGOLD)
    i *= 10;
  else if (arg == OMAXGOLD)
    i *= 100;
  else if (arg == OKGOLD)
    i *= 1000;

  Printf("\nYou find %d gold piece%s.", i, plural(i));
  c[GOLD] += i;
  UpdateStatus();

  /* destroy gold */
  item[playerx][playery] = ONOTHING;
}

/* =============================================================================
 * FUNCTION: ochest
 *
 * DESCRIPTION:
 * Function to handle finding a chest.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void ochest(void) {
  int ans;

  ans = get_prompt_input(
      "\nDo you (t) take it, (o) try to open it, or (i) ignore it? ", "oti\033",
      1);
  switch (ans) {
  case 'o':
    Print(" open it.");
    oopenchest();
    break;

  case 't':
    Print(" take");
    if (take(OCHEST, iarg[playerx][playery]) == 0)
      item[playerx][playery] = ONOTHING;
    break;

  case 'i':
  case ESC:
    ignore();
    break;

  default:
    break;
  }
}

/* =============================================================================
 * FUNCTION: finditem
 *
 * DESCRIPTION:
 * Function to say what object we found and ask if player wants to take it
 *
 * PARAMETERS:
 *
 *   itm : The item found
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void finditem(int itm) {
  int tmp, i;

  Printf("\n\nYou find %s", objectname[itm]);

  tmp = iarg[playerx][playery];
  switch (itm) {
  case ODIAMOND:
  case ORUBY:
  case OEMERALD:
  case OSAPPHIRE:
  case OSPIRITSCARAB:
  case OORBOFDRAGON:
  case OORB:
  case OHANDofFEAR:
  case OWWAND:
  case OCUBEofUNDEAD:
  case ONOTHEFT:
    Print(".");
    break;

  default:
    if (tmp > 0)
      Printf(" + %d", (long)tmp);
    else if (tmp < 0)
      Printf(" %d", (long)tmp);
  }

  Print("\nDo you want to (t) take it");
  iopts();

  i = get_prompt_input("", "it\033", 1);

  if (i == 't') {
    Print("take.");
    if (take(itm, tmp) == 0)
      forget();
  } else
    ignore();
}

/* =============================================================================
 * Exported functions
 */

/* =============================================================================
 * FUNCTION: oopenchest
 */
void oopenchest(void) {
  int i;
  int k;

  if (item[playerx][playery] != OCHEST)
    return;

  k = rnd(101);
  if (k < 40) {
    Print("\nThe chest explodes as you open it.");
    UlarnBeep();
    i = rnd(10);
    if (i > c[HP])
      i = c[HP];

    Printf("\nYou suffer %d hit point%s damage!", (long)i, plural(i));
    losehp(DIED_EXPLODING_CHEST, i);
    UpdateStatus();

    switch (rnd(10)) {
    case 1:
      c[ITCHING] += rnd(1000) + 100;
      Print("\nYou feel an irritation spread over your skin!");
      UlarnBeep();
      break;

    case 2:
      c[CLUMSINESS] += rnd(1600) + 200;
      Print("\nYou begin to lose hand-eye co-ordination!");
      UlarnBeep();
      break;

    case 3:
      c[HALFDAM] += rnd(1600) + 200;
      Print("\nYou suddenly feel sick and BARF all over your shoes!");
      UlarnBeep();
      break;
    }

    /* Remove the chest */
    item[playerx][playery] = ONOTHING;

    /* create the items in the chest */
    if (rnd(100) < 69)
      /* gems from the chest */
      creategem();

    dropgold(rnd(110 * iarg[playerx][playery] + 200));

    for (i = 0; i < rnd(4); i++)
      something(playerx, playery, iarg[playerx][playery] + 2);
  } else
    Print("\nNothing happens.");
}

/* =============================================================================
 * FUNCTION: lookforobject
 */
void lookforobject(void) {
  int i, j;
  int dx, dy;
  char code[3];

  /* can't find objects is time is stopped */
  if (c[TIMESTOP])
    return;

  i = item[playerx][playery];
  if (i == ONOTHING)
    return;

  showcell(playerx, playery);
  yrepcount = 0;

  switch (i) {
  case OGOLDPILE:
  case OMAXGOLD:
  case OKGOLD:
  case ODGOLD:
    ogold(i);
    break;

  case OPOTION:
    Print("\n\nYou find a magic potion");
    i = iarg[playerx][playery];
    if (potionknown[i])
      Printf(" of %s", &potionname[i][1]);
    Print(".");
    opotion(i);
    break;

  case OSCROLL:
    Print("\n\nYou find a magic scroll");
    i = iarg[playerx][playery];
    if (scrollknown[i])
      Printf(" of %s", &scrollname[i][1]);
    Print(".");
    oscroll(i);
    break;

  case OALTAR:
    if (nearbymonst())
      return;
    Print("\n\nThere is a holy altar here.");
    oaltar();
    break;

  case OBOOK:
    Print("\n\nYou find a book.");
    obook();
    break;

  case OCOOKIE:
    Print("\n\nYou find a fortune cookie.");
    ocookie();
    break;

  case OTHRONE:
    if (nearbymonst())
      return;
    Printf("\n\nThere is %s here.", objectname[i]);
    othrone(0);
    break;

  case OTHRONE2:
    if (nearbymonst())
      return;
    Printf("\n\nThere is %s here.", objectname[i]);
    othrone(1);
    break;

  case ODEADTHRONE:
    Printf("\n\nThere is %s here.", objectname[i]);
    odeadthrone();
    break;

  case OORB:
    if (nearbymonst())
      return;
    finditem(i);
    break;

  case OBRASSLAMP:
    Print("\nYou find a brass lamp.");

    i = get_prompt_input(
        "\nDo you want to (r) rub it, (t) take it, or (i) ignore it? ",
        "rit\033", 1);

    if (i == 'r') {
      i = rnd(100);
      if (i > 90) {
        Print("\nThe magic genie was very upset at being disturbed!");

        losehp(DIED_ANNOYED_GENIE, (int)c[HP] / 2 + 1);
        UlarnBeep();
      }
      /* higher level, better chance of spell */
      else if ((rnd(100) + c[LEVEL] / 2) > 80) {
        Print("\nA magic genie appears!");
        get_spell_code("\n  What spell would you like? : ", code);

        Printc('\n');
        for (i = 0; i < SPELL_COUNT; i++) {
          if ((spelcode[i][0] == code[0]) && (spelcode[i][1] == code[1]) &&
              (spelcode[i][2] == code[2])) {
            spelknow[i]++;
            Printf("\nSpell \"%s\":  %s\n%s", spelcode[i], spelname[i],
                   speldescript[i]);
            Print("\nThe genie prefers not to be disturbed again.");
            forget();

            recalc();
            UpdateStatus();
            return;
          }
        }

        Print("\nThe genie has never heard of such a spell!");
        Print("\nThe genie prefers not to be disturbed again.");
        forget();

        recalc();
        UpdateStatus();
        return;
      } else
        Print("\nnothing happened.");

      if (rnd(100) < 15) {
        Print("\nThe genie prefers not to be disturbed again!");
        forget();
        c[LAMP] = 0; /* chance of finding lamp again */
      }

      UpdateStatus();
    } else if (i == 't') {
      Print("take.");
      if (take(OBRASSLAMP, 0) == 0)
        forget();
    } else
      Print("ignore.");
    return;

  case OWWAND:
    if (nearbymonst())
      return;
    finditem(i);
    break;

  case OHANDofFEAR:
    if (nearbymonst())
      return;
    finditem(i);
    break;

  case OPIT:
    Print("\n\nYou're standing at the top of a pit.");
    opit();
    break;

  case OSTAIRSUP:
    Print("\n\nThere is a circular staircase here.");
    ostairs(1); /* up */
    break;

  case OELEVATORUP:
    Print("\n\nYou have found an express elevator going up.");
    oelevator(1); /*  up  */
    break;

  case OELEVATORDOWN:
    Print("\n\nYou have found an express elevator going down.");
    oelevator(-1); /*  down  */
    break;

  case OFOUNTAIN:
    if (nearbymonst())
      return;
    Print("\n\nThere is a fountain here.");
    ofountain();
    break;

  case OSTATUE:
    if (nearbymonst())
      return;
    Print("\n\nYou stand before a statue.");
    ostatue();
    break;

  case OCHEST:
    Print("\n\nThere is a chest here.");
    ochest();
    break;

  case OIVTELETRAP:
    if (rnd(11) < 6)
      return;
    item[playerx][playery] = OTELEPORTER;

  case OTELEPORTER:
    /*
     * The player is being teleported, so obviously the player gets
     * to know that a teleport trap is here.
     * oteleport forces a screen redraw, so don't bother display anything
     * here.
     */
    know[playerx][playery] = item[playerx][playery];
    Print("\nZaaaappp!  You've been teleported!\n");
    UlarnBeep();
    nap(3000);
    oteleport(0);
    break;

  case OSCHOOL:
    if (nearbymonst())
      return;
    Print("\n\nYou have found the College of VLarn.");

    i = get_prompt_input("\nDo you (g) go inside, or (i) stay here? ", "gi\033",
                         1);

    if (i == 'g') {
      oschool(); /*  the college of larn */
    } else
      Print(" stay here.");
    break;

  case OMIRROR:
    if (nearbymonst())
      return;
    Print("\n\nThere is a mirror here.");
    omirror();
    break;

  case OBANK2:
  case OBANK:
    if (nearbymonst())
      return;
    if (i == OBANK)
      Print("\n\nYou have found the bank of VLarn.");
    else
      Print("\n\nYou have found a branch office of the bank of VLarn.");

    j = get_prompt_input("\nDo you (g) go inside, or (i) stay here? ", "gi\033",
                         1);

    if (j == 'g') {
      if (i == OBANK)
        obank();
      else
        obank2(); /*  the bank of larn  */

    } else
      Print(" stay here.");
    break;

  case ODEADFOUNTAIN:
    if (nearbymonst())
      return;
    Print("\n\nThere is a dead fountain here.");
    break;

  case ODNDSTORE:
    if (nearbymonst())
      return;
    Print("\n\nThere is a DND store here.");
    i = get_prompt_input("\nDo you (g) go inside, or (i) stay here? ", "gi\033",
                         1);

    if (i == 'g')
      dndstore(); /*  the dnd adventurers store  */
    else
      Print(" stay here.");
    break;

  case OSTAIRSDOWN:
    Print("\n\nThere is a circular staircase here.");
    ostairs(-1); /* down */
    break;

  case OOPENDOOR:
    Print("\nThere is an open door here.");
    break;

  case OCLOSEDDOOR:

    /* can't move objects if time is stopped */
    if (c[TIMESTOP])
      return;

    dx = playerx;
    dy = playery;

    if (dropflag)
      return;
    Printf("\n\nYou find %s", objectname[i]);
    Print("\nDo you (o) try to open it");
    iopts();

    i = get_prompt_input("", "oi\033", 1);

    if ((i == ESC) || (i == 'i')) {
      ignore();
      playerx = lastpx;
      playery = lastpy;
      lastpx = (char)dx;
      lastpy = (char)dy;
      break;
    } else {
      Print("open.");

      /* Try and open the door that is here */
      oopendoor(playerx, playery);

      if (item[playerx][playery] == OCLOSEDDOOR) {
        /*
         * Door didn't open.
         * Move the player back where they came from.
         */

        playerx = lastpx;
        playery = lastpy;
        lastpx = (char)dx;
        lastpy = (char)dy;
      }
    }
    break;

  case OENTRANCE:
    Print("\nYou have found ");
    Print(objectname[OENTRANCE]);
    Print("\nDo you (g) go inside");
    iopts();
    i = get_prompt_input("", "gi\033", 1);

    if (i == 'g') {
      newcavelevel(1);
      playerx = 33;
      playery = MAXY - 2;

      /* Make sure the entrance to the dungeon is clear */
      item[33][MAXY - 1] = ONOTHING;
      mitem[33][MAXY - 1].mon = MONST_NONE;

      draws(0, MAXX, 0, MAXY);
      UpdateStatusAndEffects();
      return;
    } else
      ignore();
    break;

  case OVOLDOWN:
    Print("\nYou have found ");
    Print(objectname[OVOLDOWN]);
    Print("\nDo you (c) climb down");
    iopts();
    i = get_prompt_input("", "ci\033", 1);

    if ((i == ESC) || (i == 'i')) {
      ignore();
      break;
    }
    if (level != 0) {
      Print("\nThe shaft only extends 5 feet downward!");
      return;
    }
    if (packweight() > 45 + 3 * (c[STRENGTH] + c[STREXTRA])) {
      Print("\nYou slip and fall down the shaft.");
      UlarnBeep();

      losehp(DIED_SLIPPED_VOLCANO_SHAFT, 30 + rnd(20));
      UpdateStatus();
    } else
      Print("climb down.");
    nap(3000);
    newcavelevel(DBOTTOM + 1); /* down to V1 */
    playerx = (char)rnd(MAXX - 2);
    playery = (char)rnd(MAXY - 2);
    positionplayer();
    draws(0, MAXX, 0, MAXY);
    UpdateStatusAndEffects();
    return;

  case OVOLUP:
    Print("\nYou have found ");
    Print(objectname[OVOLUP]);
    Print("\nDo you (c) climb up");
    iopts();
    i = get_prompt_input("", "ci\033", 1);

    if ((i == ESC) || (i == 'i')) {
      ignore();
      break;
    }
    if (packweight() > 40 + 5 * (c[DEXTERITY] + c[STRENGTH] + c[STREXTRA])) {
      Print("\nYou slip and fall down the shaft.");
      UlarnBeep();

      losehp(DIED_SLIPPED_VOLCANO_SHAFT, 15 + rnd(20));
      UpdateStatus();
      return;
    }
    Print("climb up.");

    nap(3000);
    newcavelevel(0);
    for (i = 0; i < MAXY; i++)
      for (j = 0; j < MAXX; j++) {
        /* put player near volcano shaft */
        if (item[j][i] == OVOLDOWN) {
          playerx = (char)j;
          playery = (char)i;
          j = MAXX;
          i = MAXY;
          positionplayer();
        }
      }
    draws(0, MAXX, 0, MAXY);
    UpdateStatusAndEffects();
    return;

  case OTRAPARROWIV:
    if (rnd(17) < 13)
      return; /* for an arrow trap */
    item[playerx][playery] = OTRAPARROW;
  case OTRAPARROW:
    Print("\nYou are hit by an arrow!");
    UlarnBeep(); /* for an arrow trap */

    losehp(DIED_SHOT_BY_ARROW, rnd(10) + level);
    UpdateStatus();
    return;

  case OIVDARTRAP:
    if (rnd(17) < 13)
      return; /* for a dart trap */
    item[playerx][playery] = ODARTRAP;
  case ODARTRAP:
    Print("\nYou are hit by a dart!");
    UlarnBeep(); /* for a dart trap */

    losehp(DIED_HIT_BY_DART, rnd(5));
    if ((--c[STRENGTH]) < 3)
      c[STRENGTH] = 3;
    UpdateStatus();
    return;

  case OIVTRAPDOOR:
    if (rnd(17) < 13)
      return; /* for a trap door */
    item[playerx][playery] = OTRAPDOOR;
  case OTRAPDOOR:
    for (i = 0; i < IVENSIZE; i++) {
      if (iven[i] == OWWAND) {
        Print("\nYou escape a trap door.");
        return;
      }
    }
    if ((level == DBOTTOM) || (level == VBOTTOM)) {
      Print("\nYou fall through a trap door leading straight to HELL!");
      UlarnBeep();
      nap(3000);
      died(DIED_FELL_THROUGH_BOTTOMLESS_TRAPDOOR, 0);
    }
    Print("\nYou fall through a trap door!");
    UlarnBeep();
    losehp(DIED_FELL_THROUGH_TRAPDOOR, rnd(5 + level));
    nap(2000);
    newcavelevel(level + 1);
    draws(0, MAXX, 0, MAXY);
    UpdateStatusAndEffects();
    return;

  case OTRADEPOST:
    if (nearbymonst())
      return;
    Print("\nYou have found the VLarn trading Post.");
    i = get_prompt_input("\nDo you (g) go inside, or (i) stay here? ", "gi\033",
                         1);

    if (i == 'g')
      otradepost();
    else
      Print("stay here.");
    return;

  case OHOME:
    if (nearbymonst())
      return;
    Print("\nYou have found your way home.");
    i = get_prompt_input("\nDo you (g) go inside, or (i) stay here? ", "gi\033",
                         1);

    if (i == 'g')
      ohome();
    else
      Print("stay here.");
    return;

  case OWALL:
    break;

  case OANNIHILATION:
    if (player_has_item(OSPHTALISMAN)) {
      Print("\nThe Talisman of the Sphere protects you from annihilation!");
      return;
    }

    /* annihilated by sphere of annihilation */
    died(DIED_SPHERE_ANNIHILATION, 0);
    return;

  case OLRS:
    if (nearbymonst())
      return;
    Print("\n\nThere is an LRS office here.");
    i = get_prompt_input("\nDo you (g) go inside, or (i) stay here? ", "gi\033",
                         1);

    if (i == 'g') {
      olrs(); /*  the larn revenue service */
    } else
      Print(" stay here.");
    break;

  default:
    finditem(i);
    break;
  }
}
