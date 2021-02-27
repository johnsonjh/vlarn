/* =============================================================================
 * PROGRAM:  ularn
 * FILENAME: action.c
 *
 * DESCRIPTION:
 * This module contains functions to process the player's actions.
 *
 * =============================================================================
 * EXPORTED VARIABLES
 *
 * None
 *
 * =============================================================================
 * EXPORTED FUNCTIONS
 *
 * run       : Move in a direction until something interesting happens
 * wield     : Wield an item
 * wear      : Wear armour or shield
 * dropobj   : Drop an object
 * readsrc   : Read a scroll ot a book for the inventory
 * eatcookie : Eat a cookie in inventory, and display fortune if possible.
 * quaff     : Drink a potion in inventory
 * closedoor : Close an open door
 * quit      : Asks if really want to quit.
 * do_create : Create an item (wizard only)
 *
 * =============================================================================
 */

#include "dungeon.h"
#include "dungeon_obj.h"
#include "fortune.h"
#include "header.h"
#include "itm.h"
#include "monster.h"
#include "object.h"
#include "player.h"
#include "potion.h"
#include "savegame.h"
#include "scroll.h"
#include "show.h"
#include "ularn_ask.h"
#include "ularn_game.h"
#include "ularn_win.h"

/* =============================================================================
 * Local variables
 */

/*
 * Possible answers to various selections
 */
static char *SelectItemAns = "abcdefghijklmnopqrstuvwxyz*\033";
static char *SelectDropAns = "abcdefghijklmnopqrstuvwxyz*.\033";
static char *SelectWieldAns = "abcdefghijklmnopqrstuvwxyz*-\033";

/*
 * Types of uses of an item
 */
typedef enum UseType {
  USE_WIELD,
  USE_QUAFF,
  USE_READ,
  USE_WEAR,
  USE_EAT,
  USE_DROP,
  USE_COUNT
} UseType;

/*
 * String for each use
 */
static char *UseStrings[USE_COUNT] = {"wield", "quaff", "read",
                                      "wear",  "eat",   "drop"};

/* =============================================================================
 * Local functions
 */

/* =============================================================================
 * FUNCTION: ydhi
 *
 * DESCRIPTION:
 * Function to print the 'you don't have item' message.
 *
 * PARAMETERS:
 *
 *   x : The character index (a .. z) of the inventory slot.
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void ydhi(int x) { Printf("\nYou don't have item %c!", x); }

/* =============================================================================
 * FUNCTION: ycwi
 *
 * DESCRIPTION:
 * Function to print the 'you can't wield item' message.
 *
 * PARAMETERS:
 *
 *   x : The character index (a .. z) of the inventory slot.
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void ycwi(int x) { Printf("\nYou can't wield item %c!", x); }

/* =============================================================================
 * FUNCTION: whatitem
 *
 * DESCRIPTION:
 * Ask the player what item to use for a particular use and gets the
 * player's response.
 *
 * PARAMETERS:
 *
 *   Use : The use to which the item is to be put.
 *
 * RETURN VALUE:
 *
 *   The character input:
 *     the index of the item (a .. z) or
 *     '*' for show list or,
 *     '-' for nothing (wield only) or,
 *     '.' for gold (drop only) or,
 *     ESC for abort action.
 */
static int whatitem(UseType Use) {
  int j;
  int i = 0;
  UseType ItemUse;
  char tmp[IVENSIZE + 2]; // Each item + gold + null terminator
  char prompt[80];
  char Ans;

  if (Use == USE_DROP)
    tmp[i++] = '.';

  for (j = 0; j < IVENSIZE; j++) {
    switch (iven[j]) {
    case ONOTHING:
      ItemUse = USE_COUNT;
      break;
    case OSWORDofSLASHING:
    case OHAMMER:
    case OSWORD:
    case O2SWORD:
    case OSPEAR:
    case ODAGGER:
    case OBATTLEAXE:
    case OLONGSWORD:
    case OFLAIL:
    case OSLAYER:
    case OLANCE:
    case OVORPAL:
      ItemUse = USE_WIELD;
      break;
    case OPOTION:
      ItemUse = USE_QUAFF;
      break;
    case OSCROLL:
    case OBOOK:
      ItemUse = USE_READ;
      break;
    case OPLATE:
    case OCHAIN:
    case OLEATHER:
    case ORING:
    case OSTUDLEATHER:
    case OSPLINT:
    case OPLATEARMOR:
    case OSSPLATE:
    case OSHIELD:
    case OELVENCHAIN:
      ItemUse = USE_WEAR;
      break;
    case OCOOKIE:
      ItemUse = USE_EAT;
      break;
    default:
      ItemUse = USE_DROP;
      break;
    }

    if ((ItemUse != USE_COUNT) && ((Use == USE_DROP) || (ItemUse == Use)))
      tmp[i++] = (char)(j + 'a');
  }

  tmp[i] = '\0';

  sprintf(prompt, "\nWhat do you want to %s [%s * for all%s] ?",
          UseStrings[Use], tmp, (Use == USE_WIELD) ? " - for none" : "");

  if (Use == USE_WIELD)
    Ans = get_prompt_input(prompt, SelectWieldAns, 1);
  else if (Use == USE_DROP)
    Ans = get_prompt_input(prompt, SelectDropAns, 1);
  else
    Ans = get_prompt_input(prompt, SelectItemAns, 1);

  if (Ans == ESC)
    Print(" aborted.");

  return Ans;
}

/* =============================================================================
 * Exported functions
 */

/* =============================================================================
 * FUNCTION: run
 */
void run(int dir) {
  int i;

  i = 1;

  while (i) {
    i = moveplayer(dir);
    if (i > 0) {
      if (c[HASTEMONST])
        movemonst();

      movemonst();
      randmonst();
      regen();
    }
    if (hitflag)
      i = 0;
    if (i != 0)
      showcell(playerx, playery);
  }
}

/* =============================================================================
 * FUNCTION: wield
 */
void wield(void) {
  int i;
  int it;

  while (1) {
    i = whatitem(USE_WIELD);

    if (i == ESC)
      return;
    else if (i == '*')
      showwield();
    else if (i == '-') {
      c[WIELD] = -1;
      Print("\nYou unwield your weapon.");
      recalc();
      UpdateStatus();
      return;
    } else if ((i >= 'a') && (i <= 'z')) {
      it = i - 'a';
      if (iven[it] == ONOTHING) {
        ydhi(i);
        return;
      } else if (iven[it] == OPOTION) {
        ycwi(i);
        return;
      } else if (iven[it] == OSCROLL) {
        ycwi(i);
        return;
      } else if ((c[SHIELD] != -1) && (iven[it] == O2SWORD)) {
        Print("\nBut one arm is busy with your shield!");
        return;
      } else if ((c[WEAR] == it) || (c[SHIELD] == it)) {
        Printf("\nYou can't wield your %s while you're wearing it!",
               (c[WEAR] == it) ? "armor" : "shield");
        return;
      } else {
        c[WIELD] = it;
        Printf("\nYou wield %s", objectname[(int)iven[it]]);
        show_plusses(ivenarg[it]);
        Printc('.');

        recalc();
        UpdateStatus();
        return;
      }
    }
  }
}

/* =============================================================================
 * FUNCTION: wear
 */
void wear(void) {
  int i;
  int it;

  while (1) {
    i = whatitem(USE_WEAR);
    if (i == ESC)
      return;
    else if (i == '*')
      showwear();
    else if ((i >= 'a') && (i <= 'z')) {
      it = i - 'a';

      switch (iven[it]) {
      case ONOTHING:
        ydhi(i);
        return;
      case OLEATHER:
      case OCHAIN:
      case OPLATE:
      case OSTUDLEATHER:
      case ORING:
      case OSPLINT:
      case OPLATEARMOR:
      case OELVENCHAIN:
      case OSSPLATE:
        if (c[WEAR] != -1) {
          Print("\nYou are already wearing some armor.");
          return;
        }
        c[WEAR] = it;

        if (c[WIELD] == it)
          c[WIELD] = -1;

        Printf("\nYou put on your %s", objectname[(int)iven[it]]);
        show_plusses(ivenarg[it]);
        Printc('.');

        recalc();
        UpdateStatus();
        return;

      case OSHIELD:
        if (c[SHIELD] != -1) {
          Print("\nYou are already wearing a shield.");
          return;
        }
        if (iven[c[WIELD]] == O2SWORD) {
          Print("\nYour hands are busy with the two handed sword!");
          return;
        }
        c[SHIELD] = it;
        if (c[WIELD] == it)
          c[WIELD] = -1;

        Print("\nYou put on your shield");

        show_plusses(ivenarg[it]);
        Printc('.');

        recalc();
        UpdateStatus();
        return;

      default:
        Print("\nYou can't wear that!");
      }
    }
  }
}

/* =============================================================================
 * FUNCTION: dropobj
 */
void dropobj(void) {
  int i, pitflag = 0;
  char *p;
  long amt;

  p = &item[playerx][playery];

  while (1) {
    if ((i = whatitem(USE_DROP)) == ESC)
      return;

    if (i == '*')
      showstr();
    else {
      /* drop some gold */
      if (i == '.') {
        if (*p == OPIT)
          pitflag = 1;
        if ((*p != ONOTHING) && !pitflag) {
          Print("\nThere's something here already!");
          return;
        }

        Print("\n\n");
        Print("How much gold do you drop? ");
        amt = get_num_input((long)c[GOLD]);

        if (amt <= 0)
          return;

        if (amt > c[GOLD]) {
          Print("\nYou don't have that much!");
          return;
        }

        if (amt <= 32767) {
          *p = OGOLDPILE;
          i = (int)amt;
        } else if (amt <= 327670L) {
          *p = ODGOLD;
          i = (int)(amt / 10);
          amt = 10L * i;
        } else if (amt <= 3276700L) {
          *p = OMAXGOLD;
          i = (int)(amt / 100);
          amt = 100L * i;
        } else if (amt <= 32767000L) {
          *p = OKGOLD;
          i = (int)(amt / 1000);
          amt = 1000L * i;
        } else {
          *p = OKGOLD;
          i = (int)(32767);
          amt = 32767000L;
        }
        c[GOLD] -= amt;
        Printf(" You drop %d gold piece%s.", (long)amt, plural(amt));

        if (pitflag) {
          *p = OPIT;
          Print("\nThe gold disappears down the pit.");
        } else
          iarg[playerx][playery] = (short)i;
        UpdateStatus();
        dropflag = 1;
        return;
      } else {
        drop_object(i - 'a');
        return;
      }
    }
  }
}

/* =============================================================================
 * FUNCTION: readscr
 */
void readscr(void) {
  int i;

  while (1) {
    if ((i = whatitem(USE_READ)) == ESC)
      return;
    if (i != '.') {
      if (i == '*')
        showread();
      else {
        if (iven[i - 'a'] == OSCROLL) {
          read_scroll(ivenarg[i - 'a']);
          iven[i - 'a'] = ONOTHING;
          return;
        }
        if (iven[i - 'a'] == OBOOK) {
          readbook(ivenarg[i - 'a']);
          iven[i - 'a'] = ONOTHING;
          return;
        }
        if (iven[i - 'a'] == ONOTHING) {
          ydhi(i);
          return;
        }
        Print("\nThere's nothing on it to read.");
        return;
      }
    }
  }
}

/* =============================================================================
 * FUNCTION: eatcookie
 */
void eatcookie(void) {
  int i;
  char *p;

  while (1) {
    if ((i = whatitem(USE_EAT)) == ESC)
      return;

    if (i != '.') {
      if (i == '*')
        showeat();
      else if (iven[i - 'a'] == OCOOKIE) {
        Print("\nThe cookie was delicious.");
        iven[i - 'a'] = ONOTHING;
        if (!c[BLINDCOUNT]) {
          p = fortune(fortfile);

          if (p != NULL) {
            Print("  Inside you find a scrap of paper that says:\n");
            Print(p);
          }
        }
        return;
      } else if (iven[i - 'a'] == ONOTHING) {
        ydhi(i);
        return;
      } else {
        Print("\nYou can't eat that!");
        return;
      }
    }
  }
}

/* =============================================================================
 * FUNCTION: quaff
 */
void quaff(void) {
  int i;

  while (1) {
    if ((i = whatitem(USE_QUAFF)) == ESC)
      return;

    if (i != '.') {
      if (i == '*')
        showquaff();
      else if (iven[i - 'a'] == OPOTION) {
        quaffpotion(ivenarg[i - 'a']);
        iven[i - 'a'] = ONOTHING;
        return;
      } else if (iven[i - 'a'] == ONOTHING) {
        ydhi(i);
        return;
      } else {
        Print("\nYou wouldn't want to quaff that, would you? ");
        return;
      }
    }
  }
}

/* =============================================================================
 * FUNCTION: opendoor
 */
void opendoor(void) {
  int dx, dy;

  if (!enhance_interface)
    return;

  /* can't move objects is time is stopped */
  if (c[TIMESTOP]) {
    Printf("\nNothing can be moved while time is stopped!");
    return;
  }

  dx = playerx;
  dy = playery;

  dirsub(&dx, &dy);

  /* don't ask about items at player's location after trying to open a door */
  dropflag = 1;

  if (!checkxy(dx, dy)) {
    if (c[BLINDCOUNT] == 0)
      Print("\nYou see no door there.");
    else
      Print("\nYou cannot feel any door there.");
  } else {
    if (item[dx][dy] == OOPENDOOR)
      Print("\nThat door is already open.");
    else if (item[dx][dy] != OCLOSEDDOOR) {
      if (c[BLINDCOUNT] == 0)
        Print("\nYou see no door there.");
      else
        Print("\nYou cannot feel any door there.");
    } else {
      oopendoor(dx, dy);
      if (item[dx][dy] == OOPENDOOR)
        Print("\nThe door opens.");
      else
        Print("\nThe door resists.");
    }
  }
}

/* =============================================================================
 * FUNCTION: closedoor
 */
void closedoor(void) {
  int i;
  int dx, dy;

  /* can't move objects is time is stopped */
  if (c[TIMESTOP]) {
    Printf("\nNothing can be moved while time is stopped!");
    return;
  }

  showcell(playerx, playery);

  if (enhance_interface) {
    dx = playerx;
    dy = playery;

    dirsub(&dx, &dy);

    /* don't ask about items at player's location after trying to close a door
     */
    dropflag = 1;

    if (!checkxy(dx, dy)) {
      if (c[BLINDCOUNT] == 0)
        Print("\nYou see no door there.");
      else
        Print("\nYou cannot feel any door there.");
    } else {
      if (item[dx][dy] == OCLOSEDDOOR)
        Print("\nThat door is already closed.");
      else if (item[dx][dy] != OOPENDOOR) {
        if (c[BLINDCOUNT] == 0)
          Print("\nYou see no door there.");
        else
          Print("\nYou cannot feel any door there.");
      } else {
        item[dx][dy] = OCLOSEDDOOR;
        iarg[dx][dy] = 0; /* Clear traps on door */
        Print("\nThe door closes.");
      }
    }

  } else {
    i = item[playerx][playery];
    if (i != OOPENDOOR) {
      Print("\nThere is no open door here.");
      return;
    }

    Print("\nThe door closes.");
    forget();
    item[playerx][playery] = OCLOSEDDOOR;
    iarg[playerx][playery] = 0;

    dropflag = 1; /* So we won't be asked to open it */
  }
}

/* =============================================================================
 * FUNCTION: openchest
 */
void openchest(void) {
  if (item[playerx][playery] != OCHEST)
    Print("\nThere isn't a chest to open here.");
  else {
    oopenchest();
    dropflag = 1; /* so we don't get asked to open it again */
  }
}

/* =============================================================================
 * FUNCTION: quit
 */
void quit(void) {
  int i;

  strcpy(lastmonst, "");
  i = get_prompt_input("\n\nDo you really want to quit? (y)es, (n)o, (s)ave",
                       "yns\033", 1);
  switch (i) {
  case 'y':
    died(DIED_QUICK_QUIT, 0);
    break;

  case 'n':
  case ESC:
    Print(" no.");
    break;

  case 's':
    Print(" save.");
    Print("\nSaving...");
    savegame(savefilename);
    wizard = 1;
    died(DIED_SUSPENDED, 0);
    break;

  default:
    break;
  }
}

/* =============================================================================
 * FUNCTION: do_create
 */
void do_create(void) {
  int t, a;
  int Retry;

  do {
    Retry = 0;

    t = get_prompt_input(
        "\nType of item (Scroll/Potion/Monster/Other) : ", "SsPpMmOo\033", 1);

    switch (t) {
    case ESC:
      break;

    case 's':
    case 'S':
      Print(" Scroll Arg: ");
      a = get_num_input((long)MAXSCROLL);
      Printf("\ncreateitem(OSCROLL, %d)", a);
      createitem(playerx, playery, OSCROLL, a);
      dropflag = 1;
      break;

    case 'p':
    case 'P':
      Print(" Potion Arg: ");
      a = get_num_input((long)MAXPOTION);
      Printf("\ncreateitem(OPOTION, %d)", a);
      createitem(playerx, playery, OPOTION, a);
      dropflag = 1;
      break;

    case 'o':
    case 'O':
      Print("\n\n");
      Print("Item : ");
      t = get_num_input(0);
      Print("Arg : ");
      a = get_num_input(0);
      Printf("\ncreateitem(%d, %d)", t, a);
      createitem(playerx, playery, t, a);
      dropflag = 1;
      break;

    case 'm':
    case 'M':
      Print("\n\n");
      ClearToEOPage(1, 23);
      Print("Monst : ");
      t = get_num_input(0);
      Printf("\ncreatemonster(%d)", t);
      createmonster(t);
      dropflag = 1;
      break;

    default:
      Retry = 1;
      break;
    }
  } while (Retry);
}
