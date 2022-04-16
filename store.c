/* =============================================================================
 * PROGRAM:  ularn
 * FILENAME: store.c
 *
 * DESCRIPTION:
 * This module contaions functions to handle the locations found on the home
 * level and in the dungeon.
 *
 * =============================================================================
 * EXPORTED VARIABLES
 *
 * None
 *
 * =============================================================================
 * EXPORTED FUNCTIONS
 *
 * dndstore    : Handles entering the DND store
 * oschool     : Handles entering the college of larn
 * obank       : Handles entering the bank of larn
 * obank2      : Handles entering the 8th branch of the bank of larn
 * ointerest   : Calculates interest on bank accounts
 * item_value  : Calculates the value of an item as per the trading post
 * otradepost  : Handles entering the trading post.
 * olrs        : Handles entering the Larn Revenue Service
 * ohome       : Handes the player returning home.
 * write_store : Writes all shop data to the save file
 * read_store  : Reads all shop data from the save file
 *
 * =============================================================================
 */

#include "ularn_game.h"
#include "ularn_win.h"
#include "ularn_ask.h"
#include "saveutils.h"
#include "store.h"
#include "header.h"
#include "player.h"
#include "potion.h"
#include "scroll.h"
#include "dungeon.h"
#include "scores.h"
#include "show.h"
#include "itm.h"

/* =============================================================================
 *
 * ######          ######           #####
 * #     #  #    # #     #         #     #   #####   ####   #####   ######
 * #     #  ##   # #     #         #           #    #    #  #    #  #
 * #     #  # #  # #     #          #####      #    #    #  #    #  #####
 * #     #  #  # # #     #               #     #    #    #  #####   #
 * #     #  #   ## #     #         #     #     #    #    #  #   #   #
 * ######   #    # ######           #####      #     ####   #    #  ######
 *
 */


/* =============================================================================
 * Local variables
 */

/* max # items in the dnd store */
#define DNDSIZE   93

static int dnditm = 0;

struct _itm {
  long price;
  int *mem;
  char obj;
  char arg;
  char qty;
};

/*
 * Store inventory and prices
 */
static struct _itm itm[DNDSIZE] =  {

/*cost    memory   iven name  iven arg      how
   gp   pointer  iven[] ivenarg[]     many */
  {     2,   0,         OLEATHER,     0,     3    },
  {    10,   0,         OSTUDLEATHER, 0,     2    },
  {    40,   0,         ORING,        0,     2    },
  {    85,   0,         OCHAIN,       0,     2    },
  {   220,   0,         OSPLINT,      0,     1    },
  {   400,   0,         OPLATE,       0,     1    },
  {   900,   0,         OPLATEARMOR,  0,     1    },
  {  2600,   0,         OSSPLATE,     0,     1    },
  {   150,   0,         OSHIELD,      0,     1    },
  {  5000,   0,         OELVENCHAIN,  0,     0    },
  {  1000,   0,         OORB,         0,     0    }, /* 10 */
  { 10000,   0,         OSLAYER,      0,     0    },

/*cost    memory  iven name iven arg   how
   gp   pointer    iven[]  ivenarg[]  many */

  {     2,   0,         ODAGGER,           0,     3    },
  {    20,   0,         OSPEAR,            0,     3    },
  {    80,   0,         OFLAIL,            0,     2    },
  {   150,   0,         OBATTLEAXE,        0,     2    },
  {   450,   0,         OLONGSWORD,        0,     2    },
  {  1000,   0,         O2SWORD,           0,     2    },
  {  5000,   0,         OSWORD,            0,     1    },
  { 20000,   0,         OLANCE,            0,     1    },
  {  2000,   0,         OSWORDofSLASHING,  0,     0    }, /* 20 */
  {  7500,   0,         OHAMMER,           0,     0    },

/*cost    memory  iven name iven arg   how
   gp   pointer    iven[]  ivenarg[]  many */
  {  150,   0,         OPROTRING,     1,     1    },
  {   85,   0,         OSTRRING,      1,     1    },
  {  120,   0,         ODEXRING,      1,     1    },
  {  120,   0,         OCLEVERRING,   1,     1    },
  {  180,   0,         OENERGYRING,   0,     1    },
  {  125,   0,         ODAMRING,      0,     1    },
  {  220,   0,         OREGENRING,    0,     1    },
  { 1000,   0,         ORINGOFEXTRA,  0,     1    },

  {  280,   0,         OBELT,         0,     1    }, /* 30 */
  {  400,   0,         OAMULET,       5,     1    },

  {  500,   0,         OCUBEofUNDEAD, 0,     0    },
  {  600,   0,         ONOTHEFT,      0,     0    },

  {  590,   0,         OCHEST,        3,     1    },
  {  200,   0,         OBOOK,         2,     1    },
  {   10,   0,         OCOOKIE,       0,     3    },
  {  666,   0,         OHANDofFEAR,   0,     0    },

/*cost    memory  iven name iven arg   how
   gp   pointer    iven[]  ivenarg[]  many */

  {  20,   potionknown,  OPOTION,      PSLEEP,       6    },
  {  90,   potionknown,  OPOTION,      PHEALING,     5    },
  { 520,   potionknown,  OPOTION,      PRAISELEVEL,  1    }, /* 40 */
  { 100,   potionknown,  OPOTION,      PINCABILITY,  2    },
  {  50,   potionknown,  OPOTION,      PWISDOM,      2    },
  { 150,   potionknown,  OPOTION,      PSTRENGTH,    2    },
  {  70,   potionknown,  OPOTION,      PCHARISMA,    1    },
  {  30,   potionknown,  OPOTION,      PDIZZINESS,   7    },
  { 200,   potionknown,  OPOTION,      PLEARNING,    1    },
  {  50,   potionknown,  OPOTION,      PGOLDDET,     1    },
  {  80,   potionknown,  OPOTION,      PMONSTDET,    1    },

/*cost    memory  iven name iven arg   how
   gp   pointer    iven[]  ivenarg[]  many */

  {  30,   potionknown,  OPOTION,      PFORGETFUL,   3    },
  {  20,   potionknown,  OPOTION,      PWATER,       5    }, /* 50 */
  {  40,   potionknown,  OPOTION,      PBLINDNESS,   3    },
  {  35,   potionknown,  OPOTION,      PCONFUSION,   2    },
  { 520,   potionknown,  OPOTION,      PHEROISM,     1    },
  {  90,   potionknown,  OPOTION,      PSTURDINESS,  2    },
  { 200,   potionknown,  OPOTION,      PGIANTSTR,    2    },
  { 220,   potionknown,  OPOTION,      PFIRERESIST,  4    },
  {  80,   potionknown,  OPOTION,      PTREASURE,    6    },
  { 370,   potionknown,  OPOTION,      PINSTHEAL,    3    },
  {  50,   potionknown,  OPOTION,      PPOISON,      1    },
  { 150,   potionknown,  OPOTION,      PSEEINVIS,    3    }, /* 60 */

/*cost    memory  iven name iven arg   how
   gp   pointer    iven[]  ivenarg[]  many */

  {  850,   0,         OORBOFDRAGON,   0,     0    },
  {  750,   0,         OSPIRITSCARAB,  0,     0    },
  { 8000,   0,         OVORPAL,        0,     0,    },

  {  100,   scrollknown,  OSCROLL,      SENCHANTARM,  2    },
  {  125,   scrollknown,  OSCROLL,      SENCHANTWEAP, 2    },
  {   60,   scrollknown,  OSCROLL,      SENLIGHTEN,   4    },
  {   10,   scrollknown,  OSCROLL,      SBLANK,       4    },
  {  100,   scrollknown,  OSCROLL,      SCREATEMONST, 3    },
  {  200,   scrollknown,  OSCROLL,      SCREATEITEM,  2    },
  {  110,   scrollknown,  OSCROLL,      SAGGMONST,    1    }, /* 70 */
  {  500,   scrollknown,  OSCROLL,      STIMEWARP,    2    },
  {  200,   scrollknown,  OSCROLL,      STELEPORT,    2    },
  {  250,   scrollknown,  OSCROLL,      SAWARENESS,   4    },
  {   20,   scrollknown,  OSCROLL,      SHASTEMONST,  5    },
  {   30,   scrollknown,  OSCROLL,      SMONSTHEAL,   3    },

/*cost    memory  iven name iven arg   how
   gp   pointer    iven[]  ivenarg[]  many */

  {   340,   scrollknown,  OSCROLL,        SSPIRITPROT,  1    },
  {   340,   scrollknown,  OSCROLL,        SUNDEADPROT,  1    },
  {   300,   scrollknown,  OSCROLL,        SSTEALTH,     2    },
  {   400,   scrollknown,  OSCROLL,        SMAGICMAP,    2    },
  {   500,   scrollknown,  OSCROLL,        SHOLDMONST,   2    }, /* 80 */
  {  1000,   scrollknown,  OSCROLL,        SGEMPERFECT,  1    },
  {   500,   scrollknown,  OSCROLL,        SSPELLEXT,    1    },
  {   340,   scrollknown,  OSCROLL,        SIDENTIFY,    2    },
  {   220,   scrollknown,  OSCROLL,        SREMCURSE,    3    },
  {  3900,   scrollknown,  OSCROLL,        SANNIHILATE,  0    },
  {   610,   scrollknown,  OSCROLL,        SPULVERIZE,   1    },
  {  3000,   scrollknown,  OSCROLL,        SLIFEPROT,    0    },
  {   300,   0,            OSPHTALISMAN,   0,            0    },
  {   150,   0,            OWWAND,         0,            0    },
  {    50,   0,            OBRASSLAMP,     0,            0    }, /* 90 */
  {  9500,   0,            OPSTAFF,        0,            0    },
  { 10000,   0,            OLIFEPRESERVER, 0,            0    }
};

/* =============================================================================
 * Local functions
 */

/* =============================================================================
 * FUNCTION: write_dnd_store
 *
 * DESCRIPTION:
 * Writes the DnD store inventory data to the save file.
 *
 * PARAMETERS:
 *
 *   fp : A pointer to the save file being written.
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void write_dnd_store(FILE *fp)
{
  int i;

  /* write the number of each item remaining */
  for (i = 0; i < DNDSIZE; i++)
    bwrite(fp, &(itm[i].qty), sizeof(char));
}

/* =============================================================================
 * FUNCTION: read_dnd_store
 *
 * DESCRIPTION:
 * Reads the DnD Store inventory from the save file.
 *
 * PARAMETERS:
 *
 *   fp : A pointer to the save file being read.
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void read_dnd_store(FILE *fp)
{
  int i;

  /* read the number of each item remaining */
  for (i = 0; i < DNDSIZE; i++)
    bread(fp, &(itm[i].qty), sizeof(char));
}

/* =============================================================================
 * FUNCTION: handsfull
 *
 * DESCRIPTION:
 * Prints the hands full message.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void handsfull(void)
{
  Print("\nYou can't carry anything more!");
  nap(2200);
}

/* =============================================================================
 * FUNCTION: outofstock
 *
 * DESCRIPTION:
 * Printd the out of stock message.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void outofstock(void)
{
  Print("\nSorry, but we are out of that item.");
  nap(2200);
}

/* =============================================================================
 * FUNCTION: nogold
 *
 * DESCRIPTION:
 * Prints the insufficient funds message.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void nogold(void)
{
  Print("\nYou don't have enough gold to pay for that!");
  nap(2200);
}

/* =============================================================================
 * FUNCTION: dnditem
 *
 * DESCRIPTION:
 * Function to print an item from the DnD store inventory at the appropriate
 * screen location for a 2 column output.
 * It is assumed that items will be displayed in pages of IVENSIZE items.
 *
 * PARAMETERS:
 *
 *   i : The index of the item in the DnD Store inventory.
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void dnditem(int i)
{
  int x, y;

  if (i >= DNDSIZE) return;

  x = (i & 1) * 40 + 1;
  y = ((i % IVENSIZE) >> 1) + 5;

  MoveCursor(x, y);
  if (itm[i].qty == 0) {
    Printf("%39s", "");
    return;
  }
  Printf("%c) ", (i % IVENSIZE) + 'a');
  if (itm[i].obj == OPOTION) {
    Print("potion of ");
    Printf("%s", &potionname[(int)itm[i].arg][1]);
  }else if (itm[i].obj == OSCROLL) {
    Print("scroll of ");
    Printf("%s", &scrollname[(int)itm[i].arg][1]);
  }else
    Printf("%s", objectname[(int)itm[i].obj]);
  MoveCursor(x + 31, y);
  Printf("%6ld", ((long)(itm[i].price * 10L)));
}

/* =============================================================================
 * FUNCTION: dnd_2hed
 *
 * DESCRIPTION:
 * Function to print the DnD Store heading text.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void dnd_2hed(void)
{
  Print("Welcome to the VLarn Thrift Shoppe.  We stock many items explorers find useful\n");
  Print("in their adventures.  Feel free to browse to your heart's content.\n");
  Print("Also be advised that if you break 'em, you pay for 'em.");
}

/* =============================================================================
 * FUNCTION: dnd_hed
 *
 * DESCRIPTION:
 * Function to display a page of the DnD Store items.
 * The list of items displayed starts at dnditem.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void dnd_hed(void)
{
  int i;

  for (i = dnditm; i < IVENSIZE + dnditm; i++) dnditem(i);

  MoveCursor(50, 18);
  Print("You have ");
}

/* =============================================================================
 * Exported functions
 */

/* =============================================================================
 * FUNCTION: dndstore
 */
void dndstore(void)
{
  int i;

  dnditm = 0;
  nosignal = 1; /* disable signals */

  set_display(DISPLAY_TEXT);

  ClearText();

  dnd_2hed();
  if (outstanding_taxes > 0) {
    Print("\n\nThe VLarn Revenue Service has ordered us to not do business with tax evaders.\n");
    UlarnBeep();
    Printf("They have also told us that you owe %d gp in back taxes and, as we must\n", (long)outstanding_taxes);
    Print("comply with the law, we cannot serve you at this time.  So Sorry.\n");
    Print("\nPress ");
    Standout("escape");
    Print(" to leave: ");

    get_prompt_input("", "\033", 0);

    set_display(DISPLAY_MAP);

    nosignal = 0; /* enable signals */
    return;
  }

  dnd_hed();
  while (1) {
    MoveCursor(59, 18);
    Printf("%ld gold piece%s", (long)c[GOLD], plural(c[GOLD]));
    ClearToEOL();
    ClearToEOPage(1, 20); /* erase to eod */
    Print("\nEnter your transaction [");
    Standout("space");
    Print(" for more, ");
    Standout("escape");
    Print(" to leave]? ");

    i = get_prompt_input("", "abcdefghijklmnopqrstuvwxyz \033", 1);

    if (i == 12) {
      ClearText();
      dnd_2hed();
      dnd_hed();
    }else if (i == ESC) {
      set_display(DISPLAY_MAP);

      nosignal = 0; /* enable signals */

      return;
    }else if (i == ' ') {
      ClearToEOPage(1, 4);
      if ((dnditm += IVENSIZE) >= DNDSIZE)
        dnditm = 0;
      dnd_hed();
    }else {/* buy something */
      Printc((char)i); /* echo the byte */
      i += dnditm - 'a';
      if (i >= DNDSIZE)
        outofstock();
      else if (itm[i].qty <= 0)
        outofstock();
      else if (pocketfull())
        handsfull();
      else if (c[GOLD] < itm[i].price * 10L)
        nogold();
      else{
        /* Player learns this item */
        if (itm[i].mem != 0)
          itm[i].mem[(int)itm[i].arg] = 1;

        c[GOLD] -= itm[i].price * 10L;
        itm[i].qty--;
        take(itm[i].obj, itm[i].arg);
        if (itm[i].qty == 0)
          dnditem(i);
        nap(1001);
      }
    }

  }
}



/* =============================================================================
 *
 *  #####
 * #     #   ####   #       #       ######   ####   ######
 * #        #    #  #       #       #       #    #  #
 * #        #    #  #       #       #####   #       #####
 * #        #    #  #       #       #       #  ###  #
 * #     #  #    #  #       #       #       #    #  #
 *  #####    ####   ######  ######  ######   ####   ######
 *
 */


/* =============================================================================
 * Local variables
 */

/* the list of courses taken and the time required */
#define MAX_COURSES 8
char course[MAX_COURSES] = { 0, 0, 0, 0, 0, 0, 0, 0 };
char coursetime[MAX_COURSES] = { 10, 15, 10, 20, 10, 10, 10, 5 };

/* =============================================================================
 * Local functions
 */

/* =============================================================================
 * FUNCTION: write_college
 *
 * DESCRIPTION:
 * Write the college data to the save file.
 *
 * PARAMETERS:
 *
 *   fp : A pointer to the save file being written.
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void write_college(FILE *fp)
{
  bwrite(fp, course, MAX_COURSES * sizeof(char));
}

/* =============================================================================
 * FUNCTION: read_college
 *
 * DESCRIPTION:
 * Function to read the college data from the save file.
 *
 * PARAMETERS:
 *
 *   fp : A pointer to the save file being read.
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void read_college(FILE *fp)
{
  bread(fp, course, MAX_COURSES * sizeof(char));
}

/* =============================================================================
 * FUNCTION: sch_hed
 *
 * DESCRIPTION:
 * Function to display the header info for the school.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void sch_hed(void)
{
  ClearText();

  Print("The College of VLarn offers the exciting opportunity of higher education to\n");
  Print("all inhabitants of the caves.  Here is the class schedule:\n\n\n");
  Print("\t\t    Course Name \t       Time Needed\n\n");

  /*line 7 of crt*/
  if (course[0] == 0)
    Print("\t\ta)  Fighter's Training I        10 mobuls");
  Printc('\n');
  if (course[1] == 0)
    Print("\t\tb)  Fighter's Training II       15 mobuls");
  Printc('\n');
  if (course[2] == 0)
    Print("\t\tc)  Introduction to Wizardry    10 mobuls");
  Printc('\n');
  if (course[3] == 0)
    Print("\t\td)  Applied Wizardry            20 mobuls");
  Printc('\n');
  if (course[4] == 0)
    Print("\t\te)  Behavioral Psychology       10 mobuls");
  Printc('\n');
  if (course[5] == 0)
    Print("\t\tf)  Faith for Today             10 mobuls");
  Printc('\n');
  if (course[6] == 0)
    Print("\t\tg)  Contemporary Dance          10 mobuls");
  Printc('\n');
  if (course[7] == 0)
    Print("\t\th)  History of VLarn             5 mobuls");

  Print("\n\n\t\tAll courses cost 250 gold pieces.");
  MoveCursor(30, 18);
  Print("You are presently carrying ");
}


/* =============================================================================
 * Exported functions
 */

/* =============================================================================
 * FUNCTION: oschool
 */
void oschool(void)
{
  int i;
  long time_used;

  nosignal = 1; /* disable signals */

  set_display(DISPLAY_TEXT);

  sch_hed();

  while (1) {
    MoveCursor(57, 18);
    Printf("%d gold piece%s.   ", (long)c[GOLD], plural(c[GOLD]));

    Print("\nWhat is your choice [");
    Standout("escape");
    Print(" to leave] ? ");
    yrepcount = 0;
    i = get_prompt_input("", "abcdefgh\033", 1);

    if (i == 12) {
      /* Pressed ^R, so redraw */
      sch_hed();
      continue;
    }else if (i == ESC) {
      /* exit */
      nosignal = 0;
      set_display(DISPLAY_MAP);

      return;
    }

    Printc((char)i);
    if (c[GOLD] < 250)
      nogold();
    else{
      if (course[i - 'a']) {
        Print("\nSorry but that class is filled.");
        nap(1000);
      }else if (i <= 'h') {
        c[GOLD] -= 250;
        time_used = 0;
        switch (i) {
        case 'a':
          c[STRENGTH] += 2;
          c[CONSTITUTION]++;
          Print("\nYou feel stronger!");
          ClearToEOL();
          MoveCursor(16, 7);
          ClearToEOL();
          break;

        case 'b':
          if (course[0] == 0) {
            Print("\nSorry but this class has a prerequisite of Fighter's Training I");
            ClearToEOL();
            c[GOLD] += 250;
            time_used = -10000;
          }else {
            Print("\nYou feel much stronger!");
            ClearToEOL();
            MoveCursor(16, 8);
            ClearToEOL();
            c[STRENGTH] += 2;
            c[CONSTITUTION] += 2;
          }
          break;

        case 'c':
          c[INTELLIGENCE] += 2;
          Print("\nThe task before you now seems more attainable!");
          ClearToEOL();
          MoveCursor(16, 9);
          ClearToEOL();
          break;

        case 'd':
          if (course[2] == 0) {
            Print("\nSorry but this class has a prerequisite of Introduction to Wizardry");
            ClearToEOL();
            c[GOLD] += 250;
            time_used = -10000;
          }else {
            Print("\nThe task before you now seems very attainable!");
            ClearToEOL();
            MoveCursor(16, 10);
            ClearToEOL();
            c[INTELLIGENCE] += 2;
          }
          break;

        case 'e':
          c[CHARISMA] += 3;
          Print("\nYou now feel like a born leader!");
          ClearToEOL();
          MoveCursor(16, 11);
          ClearToEOL();
          break;

        case 'f':
          c[WISDOM] += 2;
          Print("\nYou now feel more confident that you can find the potion in time!");
          ClearToEOL();
          MoveCursor(16, 12);
          ClearToEOL();
          break;

        case 'g':
          c[DEXTERITY] += 3;
          Print("\nYou feel like dancing!");
          ClearToEOL();
          MoveCursor(16, 13);
          ClearToEOL();
          break;

        case 'h':
          c[INTELLIGENCE]++;
          Print("\nWow! e = mc^2!");
          ClearToEOL();
          MoveCursor(16, 14);
          ClearToEOL();
          break;
        }

        time_used += coursetime[i - 'a'] * 100;

        if (time_used > 0) {
          gtime += time_used;

          /* remember that he has taken that course */
          course[i - 'a']++;

          /* he regenerated */
          c[HP] = c[HPMAX];
          c[SPELLS] = c[SPELLMAX];

          /* cure blindness too!  */
          if (c[BLINDCOUNT])
            c[BLINDCOUNT] = 1;

          /*  end confusion */
          if (c[CONFUSE])
            c[CONFUSE] = 1;

          /* adjust parameters for time change */
          adjusttime((long)time_used);
        }
        nap(1000);
      }
    }
  }
}

/* =============================================================================
 *
 * ######                                  #
 * #     #    ##    #    #  #    #         #          ##    #####   #    #
 * #     #   #  #   ##   #  #   #          #         #  #   #    #  ##   #
 * ######   #    #  # #  #  ####           #        #    #  #    #  # #  #
 * #     #  ######  #  # #  #  #           #        ######  #####   #  # #
 * #     #  #    #  #   ##  #   #          #        #    #  #   #   #   ##
 * ######   #    #  #    #  #    #    Of   #######  #    #  #    #  #    #
 *
 */

/* =============================================================================
 * Local variables
 */

/*
 * limit of 1 million gold pieces in bank
 */
#define BANKLIMIT 1000000

/* last time he was in bank */
static long lasttime = 0;

/* the reference to screen location for each gem */
static short gemorder[IVENSIZE] = { 0 };
/* the appraisal of the gems */
static long gemvalue[IVENSIZE] = { 0 };

/* =============================================================================
 * Local functions
 */

/* =============================================================================
 * FUNCTION: write_bank
 *
 * DESCRIPTION:
 * Function to write the bank data to the save file.
 *
 * PARAMETERS:
 *
 *   fp : A pointer to the save file being written.
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void write_bank(FILE *fp)
{
  bwrite(fp, (char *)&lasttime, sizeof(long));
}

/* =============================================================================
 * FUNCTION: read_bank
 *
 * DESCRIPTION:
 * Function to read the bank data from the save file.
 *
 * PARAMETERS:
 *
 *   fp : A pointer to the save file being read.
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void read_bank(FILE *fp)
{
  bread(fp, (char *)&lasttime, sizeof(long));
}

/* =============================================================================
 * FUNCTION: appraise
 *
 * DESCRIPTION:
 * Function to appraise the eye of larn and offer to buy it.
 *
 * PARAMETERS:
 *
 *   eye   : The index of the inventory slot holding the eye of larn
 *
 *   order : The order of the eye oflarn in the gem appraisal display
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void appraise(int eye, int order)
{
  long amt;

  MoveCursor(1, 20);
  Printf("I see you have %s", objectname[OLARNEYE]);
  Print("  I must commend you.  I didn't think\nyou could get it.");
  Print("  Shall I appraise it for you? ");
  yrepcount = 0;
  if (getyn() == 'y') {
    Print("yes.\n  Just one moment please...");
    nap(1000);
    amt = (long)250000L - ((gtime * 7) / 100) * 100;

    if (amt < 50000L) amt = 50000L;

    ClearToEOPage(1, 20);
    MoveCursor(1, 20);
    Printf("\nThis is an excellent stone.\n");
    Printf("It is worth %d gold pieces to us\n", (long)amt);
    Print("Would you like to sell it? ");
    yrepcount = 0;

    if (getyn() == 'y') {
      Print("yes");
      c[GOLD] += amt;
      iven[eye] = ONOTHING;
      c[EYEOFLARN] = 0;

      MoveCursor( (order % 2) * 40 + 1, (order >> 1) + 4 );
      Printf("%39s", "");

      MoveCursor(40, 17);
      Printf("%8d", (long)c[BANKACCOUNT]);
      MoveCursor(49, 18);
      Printf("%8d", (long)c[GOLD]);
    }else {
      Print("no");
      Print("\nIt is, of course, your privilege to keep the stone.");
      nap(500);
    }
  }else {
    Print("no.");
    nap(500);
  }

  ClearToEOPage(1, 20);
  MoveCursor(1, 20);

}

/* =============================================================================
 * FUNCTION: obanksub
 *
 * DESCRIPTION:
 * Function to handle the main processing for the bank.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void obanksub(void)
{
  long amt;
  int i, k, eye = 0;

  ointerest(); /* credit any needed interest */

  if (level == 8) c[TELEFLAG] = 0;

  k = 0;
  for (i = 0; i < IVENSIZE; i++) {
    switch (iven[i]) {
    case OLARNEYE:
    case ODIAMOND:
    case OEMERALD:
    case ORUBY:
    case OSAPPHIRE:
      if (iven[i] == OLARNEYE) {
        eye = i;
        gemvalue[i] = (long)250000L - ((gtime * 7) / 100) * 100;
        if (gemvalue[i] < 50000L)
          gemvalue[i] = 50000L;
      }else
        gemvalue[i] = ivenarg[i] * 100;
      gemorder[i] = (short)k;
      MoveCursor((k % 2) * 40 + 1, (k >> 1) + 4 );
      Printf("%c) %s", i + 'a', objectname[(int)iven[i]]);
      MoveCursor((k % 2) * 40 + 33, (k >> 1) + 4 );
      Printf("%5d", (long)gemvalue[i]);
      k++;
      break;

    default:
      gemvalue[i] = 0;
    }
  }
  MoveCursor(31, 17);
  Printf("You have %8d gold piece%s in the bank.",
         (long)c[BANKACCOUNT],
         plural(c[BANKACCOUNT]));

  MoveCursor(40, 18);
  Printf("You have %8d gold piece%s", (long)c[GOLD], plural(c[GOLD]));
  if (c[BANKACCOUNT] + c[GOLD] >= 500000L)
    Print("\nNote:  Only deposits under 1,000,000gp can earn interest.");

  if (eye)
    appraise(eye, gemorder[eye]);
  while (1) {
    ClearToEOPage(1, 20);
    MoveCursor(1, 20);
    Print("\nYour wish? [(");
    Standout("d");
    Print(") deposit, (");
    Standout("w");
    Print(") withdraw, (");
    Standout("s");
    Print(") sell a stone, or ");
    Standout("escape");
    Print("]  ");
    yrepcount = 0;

    i = get_prompt_input("", "dws\033", 1);

    switch (i) {
    case 'd':
      Print("deposit\nHow much? ");
      amt = get_num_input((long)c[GOLD]);
      if (amt < 0)
        amt = 0;
      else if (amt > c[GOLD]) {
        Print("\n  You don't have that much.");
        amt = 0;
        nap(2000);
      }

      /* Deposit the money */
      c[GOLD] -= amt;
      c[BANKACCOUNT] += amt;
      break;

    case 'w':
      Print("withdraw\nHow much? ");
      amt = get_num_input((long)c[BANKACCOUNT]);
      if (amt < 0)
        amt = 0;
      else if (amt > c[BANKACCOUNT]) {
        Print("\n  You don't have that much in the bank!");
        amt = 0;
        nap(2000);
      }

      /* Withdraw the money */
      c[GOLD] += amt;
      c[BANKACCOUNT] -= amt;
      break;

    case 's':
      i = get_prompt_input("sell\nWhich stone would you like to sell? ",
               "abcdefghijklmnopqrstuvwxys*", 1);

      if (i == '*') {
        /* sell all gems */
        for (i = 0; i < IVENSIZE; i++) {
          if (gemvalue[i]) {
            if (iven[i] == OLARNEYE)
              c[EYEOFLARN] = 0;
            c[GOLD] += gemvalue[i];
            iven[i] = ONOTHING;
            gemvalue[i] = 0;
            k = gemorder[i];
            MoveCursor( (k % 2) * 40 + 1, (k >> 1) + 4 );
            Printf("%39s", "");
          }
        }
      }else {
        if (gemvalue[i = i - 'a'] == 0) {
          Printf("\nItem %c is not a gemstone!", i + 'a');
          nap(2000);
        }else {
          if (iven[i] == OLARNEYE)
            c[EYEOFLARN] = 0;
          c[GOLD] += gemvalue[i];
          iven[i] = ONOTHING;
          gemvalue[i] = 0;
          k = gemorder[i];
          MoveCursor( (k % 2) * 40 + 1, (k >> 1) + 4 );
          Printf("%39s", "");
        }
      }
      break;

    case ESC:
      return;
    }

    MoveCursor(40, 17);
    Printf("%8d", (long)c[BANKACCOUNT]);
    MoveCursor(49, 18);
    Printf("%8d", (long)c[GOLD]);
  }
}

/* =============================================================================
 * FUNCTION: banktitle
 *
 * DESCRIPTION:
 * Function to display the title for this branck of the bank and then
 * perform the bank processing.
 *
 * PARAMETERS:
 *
 *   str : The title string for the branch
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void banktitle(char *str)
{
  nosignal = 1; /* disable signals */
  set_display(DISPLAY_TEXT);
  ClearText();

  Print(str);
  if (outstanding_taxes > 0) {
    Print("\n\nThe VLarn Revenue Service has ordered that your account be frozen until all\n");
    UlarnBeep();
    Printf("levied taxes have been paid.  They have also told us that you owe %d gp in\n", (long)outstanding_taxes);
    Print("taxes and we must comply with them. We cannot serve you at this time.  Sorry.\n");
    Print("We suggest you go to the LRS office and pay your taxes.\n");

    Print("\nPress ");
    Standout("escape");
    Print(" to leave: ");

    get_prompt_input("", "\033", 0);

    set_display(DISPLAY_MAP);

    nosignal = 0; /* enable signals */

    return;
  }
  Print("\n\n\tGemstone\t      Appraisal\t\tGemstone\t      Appraisal");

  obanksub();

  nosignal = 0; /* enable signals */
  set_display(DISPLAY_MAP);

}

/* =============================================================================
 * Exported functions
 */

/* =============================================================================
 * FUNCTION: obank
 */
void obank(void)
{
  banktitle("    Welcome to the First National Bank of VLarn.");
}

/* =============================================================================
 * FUNCTION: obank2
 */
void obank2(void)
{
  banktitle("Welcome to the 8th-level branch office of the First National Bank of VLarn.");
}

/* =============================================================================
 * FUNCTION: ointerest
 */
void ointerest(void)
{
  int i;

  if (c[BANKACCOUNT] < 0)
    c[BANKACCOUNT] = 0;
  else if ((c[BANKACCOUNT] > 0) && (c[BANKACCOUNT] < BANKLIMIT)) {
    i = (gtime - lasttime) / 100; /*# mobuls elapsed since last here*/
    while ((i-- > 0) && (c[BANKACCOUNT] < BANKLIMIT))
      /*
      ** at 1 mobul ~=~ 1 hour, is 10 % a year
      */
      c[BANKACCOUNT] += (long)(c[BANKACCOUNT] / 877);
  }
  lasttime = (gtime / 100) * 100;
}


/* =============================================================================
 *
 * #######
 *    #     #####     ##    #####      #    #    #   ####
 *    #     #    #   #  #   #    #     #    ##   #  #    #
 *    #     #    #  #    #  #    #     #    # #  #  #
 *    #     #####   ######  #    #     #    #  # #  #  ###
 *    #     #   #   #    #  #    #     #    #   ##  #    #
 *    #     #    #  #    #  #####      #    #    #   ####
 */

/* =============================================================================
 * Local functions
 */

/* =============================================================================
 * FUNCTION: otradhead
 *
 * DESCRIPTION:
 * Function to print the trading post heading text.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void otradhead(void)
{
  ClearText();
  Print("Welcome to the VLarn Trading Post.  We buy items that explorers no longer find\n");
  Print("useful.  Since the condition of the items you bring in is not certain,\n");
  Print("and we incur great expense in reconditioning the items, we usually pay\n");
  Print("only 20% of their value were they to be new.  If the items are badly\n");
  Print("damaged, we will pay only 10% of their new value.\n\n");
}


/* =============================================================================
 * FUNCTION: cnsitm
 *
 * DESCRIPTION:
 * Prints the 'can't sell unidentified items' message.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void cnsitm(void)
{
  Print("\nSorry, we can't accept unidentified objects.");
}

/* =============================================================================
 * Exported functions
 */

/* =============================================================================
 * FUNCTION: item_value
 */
int item_value(int it, int itarg)
{
  int arg;
  int value;
  int found_item;
  int j;

  value = -1;

  if ((it == ODIAMOND) ||
      (it == ORUBY) ||
      (it == OEMERALD) ||
      (it == OSAPPHIRE))
    value = (long)20 * itarg;
  else{
    found_item = 0;
    j = 0;
    while ((j < DNDSIZE) && (!found_item)) {
      if (itm[j].obj == it) {
        if ((itm[j].obj == OSCROLL) || (itm[j].obj == OPOTION))
          value = (long)2 * itm[j + itarg].price;
        else{
          value = (long)itm[j].price;

          arg = itarg;

          if (arg >= 0) value *= 2;

          while ((arg != 0) && (value < 500000L)) {
            if (arg > 0) {
              /* appreciate if a +n object */
              value = (14 * (value + 67)) / 10;
              arg--;
            }else {
              /* depreciate -n object */
              value = (value * 10) / 14;
              arg++;
            }
          }
        }

        /* always offer at least 1 gp */
        if (value == 0) value = 1;

        found_item = 1;
      }

      j++;
    }
  }

  return value;
}


/* =============================================================================
 * FUNCTION: otradepost
 */
void otradepost(void)
{
  int ans;
  int no_sell_flag;       /* set to true if the selected item can't be sold */
  int idx;                /* inventory index of the item to sell            */
  int it, itarg;          /* item number and argument of the item to sell   */
  long value;             /* value of the item in gold                      */

  set_display(DISPLAY_TEXT);
  ClearText();

  yrepcount = 0;
  dnditm = 0;
  otradhead();

  while (1) {
    Print("\nWhat item do you want to sell to us [");
    Standout("*");
    Print(" for list, or ");
    Standout("escape");
    Print("] ? ");
    ans = get_prompt_input("", "abcdefghijklmnopqrstuvwxyz.*\033", 1);

    if (ans == ESC) {
      recalc();
      set_display(DISPLAY_MAP);

      return;
    }else if (ans == '*') {
      ClearText();
      qshowstr();
      otradhead();
    }else if ((ans >= 'a') && (ans <= 'z')) {
      no_sell_flag = 0;

      idx = ans - 'a';

      it = iven[idx];
      itarg = ivenarg[idx];

      if (it == OSCROLL) {
        if (!scrollknown[itarg]) {
          /* can't sell unidentified item */
          no_sell_flag = 1;
          cnsitm();
        }
      }else if (it == OPOTION) {
        if (!potionknown[itarg]) {
          /* can't sell unidentified item */
          no_sell_flag = 1;
          cnsitm();
        }
      }else if ((it == OLANCE) && ramboflag) {
        no_sell_flag = 1;
        Print("\nYou don't *really* want to sell that, now do you?");
      }

      if (!no_sell_flag) {
        if (it == ONOTHING)
          Printf("\nYou don't have item %c!", ans);
        else{

          value = item_value(it, itarg);

          if (value >= 0) {
            /* show what the item was */
            show3(idx);

            Printf("\nItem (%c) is worth %d gold piece%s to us.  Do you want to sell it? ", ans, (long)value, plural(value));
            if (getyn() == 'y') {
              Print("yes.\n");
              c[GOLD] += value;
              if (c[WEAR] == idx) c[WEAR] = -1;
              if (c[WIELD] == idx) c[WIELD] = -1;
              if (c[SHIELD] == idx) c[SHIELD] = -1;
              adjustcvalues(it, itarg);
              iven[idx] = ONOTHING;
            }else
              Print("no thanks.\n");
          }else
            /*
             * negative value indicates that the item is not in the price list
             */
            Print("\nSo sorry but we are not authorized to accept that item.");
        }
      }       /* if not no_sell_flag */
    }               /* if item selected */
  }
}

/* =============================================================================
 *
 * #       ######   #####
 * #       #     # #     #
 * #       #     # #
 * #       ######   #####
 * #       #   #         #
 * #       #    #  #     #
 * ####### #     #  #####
 *
 */

/* =============================================================================
 * Exported functions
 */

/* =============================================================================
 * FUNCTION: olrs
 */
void olrs(void)
{
  int i;
  long amt;

  nosignal = 1; /* disable signals */
  set_display(DISPLAY_TEXT);
  ClearText();

  MoveCursor(1, 4);

  if (outstanding_taxes) {
    if (cheat) {
      Print("Sorry but it seems you are trying to pay off your taxes by cheating!");

      Print("\n\n\tpress [escape] to exit the office.");
      get_prompt_input("", "\033", 0);

      nosignal = 0; /* enable signals */
      set_display(DISPLAY_MAP);

      return;

    }

  }

  Print("Welcome to the VLarn Revenue Service district office.  How can we help you?");
  while (1) {
    MoveCursor(1, 6);
    if (outstanding_taxes > 0)
      Printf("You presently owe %d gp in taxes.  ", (long)outstanding_taxes);
    else
      Print("You do not owe us any taxes.           ");

    MoveCursor(1, 8);
    if (c[GOLD] > 0)
      Printf("You have %6d gp.    ", (long)c[GOLD]);
    else
      Print("You have no gold pieces.  ");

    MoveCursor(1, 20);
    Print("\n\nYour wish? [(");
    Standout("p");
    Print(") pay taxes, or ");
    Standout("escape");
    Print("]  ");

    ClearToEOPage(40, 22);

    yrepcount = 0;
    i = get_prompt_input("", "p\033", 1);

    switch (i) {
    case 'p':
      Print("pay taxes.\nHow much? ");
      amt = (long)get_num_input((long)c[GOLD]);
      if (amt < 0)
        amt = 0;
      else if (amt > c[GOLD]) {
        Print("  You don't have that much.");
        amt = 0;
      }

      c[GOLD] -= paytaxes((long)amt);

      nap(500);

      break;

    case ESC:
      nosignal = 0; /* enable signals */
      set_display(DISPLAY_MAP);

      return;
    }

  }
}


/* =============================================================================
 *
 * #     #
 * #     #   ####   #    #  ######
 * #     #  #    #  ##  ##  #
 * #######  #    #  # ## #  #####
 * #     #  #    #  #    #  #
 * #     #  #    #  #    #  #
 * #     #   ####   #    #  ######
 *
 */

/* =============================================================================
 * Exported functions
 */

/* =============================================================================
 * FUNCTION: ohome
 */
void ohome(void)
{
  int i;

  set_display(DISPLAY_TEXT);

  nosignal = 1; /* disable signals */
  for (i = 0; i < IVENSIZE; i++)
    /* remove the potion of cure dianthroritis from inventory */
    if (iven[i] == OPOTION)
      if (ivenarg[i] == PCUREDIANTH) {
        iven[i] = ONOTHING;
        ClearText();

        Print("Congratulations.  You found the potion of cure "
              "dianthroritis!\n");
        Print("\nFrankly, No one thought you could do it.");
        Print("  Boy!  Did you surprise them!\n");
        nap(1000);
        if (gtime > TIMELIMIT) {
          Print("\nHowever... the doctor has the sad duty to "
                "inform you that your daughter has\n");
          Print("died! You didn't make it in time.  In your agony, "
                "you kill the doctor,\nyour ");
          if (sex == 1)
            Print("wife");
          else
            Print("husband");
          Print(" and yourself!  Too bad...\n");
          nap(5000);
          died(DIED_KILLED_FAMILY, 0);
        }else {
          Print("\nThe doctor is now administering the potion and, "
                "in a few moments,\n");
          Print("your daughter should be well on her way to "
                "recovery.\n");
          nap(6000);
          Print("\nThe potion is.");
          nap(1000);
          Print(".");
          nap(1000);
          Print(".");
          nap(1000);
          Print(" working!  The doctor thinks that\n");
          Print("your daughter will recover in a few days.  "
                "Congratulations!");
          UlarnBeep();
          nap(5000);
          died(DIED_WINNER, 0);
        }
      }

  while (1) {
    ClearText();
    Printf("Welcome home %s.", logname);
    Print("  Latest word from the doctor is not good.\n");

    if (gtime > TIMELIMIT) {
      Print("\nThe doctor has the sad duty to inform you that your "
            "daughter has died!\n");
      Print("You didn't make it in time.");
      Print("In your agony, you kill the doctor,\nyour ");
      if (sex == 1)
        Print("wife");
      else
        Print("husband");
      Print(" and yourself!  Too bad...");
      nap(5000);
      died(DIED_KILLED_FAMILY, 0);
    }

    Print("\nThe diagnosis is confirmed as dianthroritis.  "
          "He guesses that\n");
    Printf("your daughter has only %d mobuls left in this world.  "
           "It's up to you,\n", (long)((TIMELIMIT - gtime + 99) / 100));
    Printf("%s, to find the only hope for your daughter, the very rare\n",
           logname);
    Print("potion of cure dianthroritis.  It is rumored that only deep "
          "in the\n");
    Print("depths of the caves can this potion be found.\n\n\n");
    Print("\n     ----- press ");
    Standout("return");
    Print(" to continue, ");
    Standout("escape");
    Print(" to leave ----- ");

    i = get_prompt_input("", "\033\015", 0);

    if (i == ESC) {
      set_display(DISPLAY_MAP);

      nosignal = 0; /* enable signals */

      return;
    }
  }
}


/* =============================================================================
 * FUNCTION: write_store
 */
void write_store(FILE *fp)
{
  write_dnd_store(fp);
  write_college(fp);
  write_bank(fp);
}

/* =============================================================================
 * FUNCTION: read_store
 */
void read_store(FILE *fp)
{
  read_dnd_store(fp);
  read_college(fp);
  read_bank(fp);
}
