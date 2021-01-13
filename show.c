/* =============================================================================
 * PROGRAM:  ularn
 * FILENAME: show.c
 *
 * DESCRIPTION:
 * This module contains functions to print item names and show lists of items.
 *
 * =============================================================================
 * EXPORTED VARIABLES
 *
 * None
 *
 * =============================================================================
 * EXPORTED FUNCTIONS
 *
 * show_plusses - Prints the number of plusses as indicated by the input arg
 * qshowstr     - Show the player's inventory without setting up text mode
 * showstr      - Show the player's invertory, setting/exiting text mode
 * showwear     - Show wearable items
 * showwield    - Show wieldable items
 * showread     - Show readable items
 * showeat      - Show edible items
 * showquaff    - Show quaffable items
 * seemagic     - Show magic spells/scrolls/potions discovered
 * show1        - Show an item name
 * show3        - Show an item name with the numebr of plusses
 *
 * =============================================================================
 */

#include "header.h"
#include "ularn_game.h"
#include "ularn_win.h"
#include "ularn_ask.h"
#include "dungeon.h"
#include "player.h"
#include "potion.h"
#include "scroll.h"
#include "spell.h"
#include "itm.h"
#include "show.h"

/* =============================================================================
 * Local variables
 */

/*
 * See magic function line and position counts. Used for paginating see magic.
 */
static int lincount;
static int count;

/* =============================================================================
 * Local functions
 */

/* =============================================================================
 * FUNCTION: t_setup
 *
 * DESCRIPTION:
 * Sets up for text display for the indicated number of lines.
 *
 * PARAMETERS:
 *
 *   count : The number of lines of text to be displayed.
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void t_setup(int count)
{
	(void)count;
	set_display(DISPLAY_TEXT);
	ClearText();
	MoveCursor(1, 1);
}

/* =============================================================================
 * FUNCTION: t_endup
 *
 * DESCRIPTION:
 * Restores normal display screen depending on t_setup()/
 *
 * PARAMETERS:
 *
 *   count : The number of lines of text displayed.
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void t_endup(int count)
{
	(void)count;
	set_display(DISPLAY_MAP);
}

/* =============================================================================
 * FUNCTION: seepage
 *
 * DESCRIPTION:
 * This function is used to paginate the seemagic output.
 * It is called for each spell displayed and assumes that spells are displayed
 * in 3 columns.
 * Whenever the number of calls indicates a full page has been displayed it
 * calls more() to prompt for the next page.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void seepage(void)
{
	if (++count == 3) {
		lincount++;
		count = 0;
		Printc('\n');
		if (lincount > 17) {
			lincount = 0;
			more();
			ClearText();
		}
	}
}

/* =============================================================================
 * Exported functions
 */

/* =============================================================================
 * FUNCTION: show_plusses
 */
void show_plusses(int plus)
{
	if ((plus > 0) || wizard)
		Printf(" +%d", plus);
	else if (plus < 0)
		Printf(" %d", plus);
}

/* =============================================================================
 * FUNCTION: qshowstr
 */
void qshowstr(void)
{
	int k;
	int mobuls_used;
	int mobuls_left;

	clearpager();
	nosignal = 1; /* don't allow ^c etc */

	if (c[GOLD]) {
		Printf(".)   %d gold piece%s.", (long)c[GOLD], plural(c[GOLD]));
		pager();
	}

	for (k = 0; k < IVENSIZE; k++) {
		if (iven[k] != ONOTHING) {
			show3(k);
			pager();
		}
	}

	mobuls_used = (gtime / 100) + 1;
	mobuls_left = (TIMELIMIT / 100) - mobuls_used;
	Printf("\nElapsed time is %d.  You have %d mobul%s left.",
	       mobuls_used, mobuls_left, plural(mobuls_left));

	more();
	nosignal = 0;
}


/* =============================================================================
 * FUNCTION: showstr
 */
void showstr(void)
{
	int i, number;

	number = 3;

	/* count items in inventory */
	for (i = 0; i < IVENSIZE; i++)
		if (iven[i] != ONOTHING) number++;

	t_setup(number);
	qshowstr();
	t_endup(number);
}


/* =============================================================================
 * FUNCTION: showwear
 */
void showwear(void)
{
	int count;
	int i;
	int j;

	nosignal = 1; /* don't allow ^c etc */
	clearpager();

	/* count number of items we will display */
	count = 2;
	for (j = 0; j < IVENSIZE; j++) {
		i = iven[j];

		if (i != ONOTHING) {
			switch (i) {
			case OLEATHER:
			case OPLATE:
			case OCHAIN:
			case ORING:
			case OSTUDLEATHER:
			case OSPLINT:
			case OPLATEARMOR:
			case OSSPLATE:
			case OSHIELD:
			case OELVENCHAIN:
				count++;

			default:
				break;
			}
		}
	}

	t_setup(count);

	/* Display the wearable items in item index order */
	for (i = 22; i < 93; i++) {
		for (j = 0; j < IVENSIZE; j++) {
			if (iven[j] == i) {
				switch (i) {
				case OLEATHER:
				case OPLATE:
				case OCHAIN:
				case ORING:
				case OSTUDLEATHER:
				case OSPLINT:
				case OPLATEARMOR:
				case OSSPLATE:
				case OSHIELD:
				case OELVENCHAIN:
					show3(j);
					pager();
					break;

				default:
					break;
				}
			}
		}
	}

	more();

	nosignal = 0;
	t_endup(count);
}

/* =============================================================================
 * FUNCTION: showwield
 */
void showwield(void)
{
	int i;
	int j;
	int count;

	nosignal = 1; /* don't allow ^c etc */
	clearpager();

	/* count how many items */
	count = 2;
	for (j = 0; j < IVENSIZE; j++) {
		i = iven[j];
		if (i != ONOTHING) {
			switch (i) {
			case ODIAMOND:
			case ORUBY:
			case OEMERALD:
			case OSAPPHIRE:
			case OBOOK:
			case OCHEST:
			case OLARNEYE:
			case ONOTHEFT:
			case OSPIRITSCARAB:
			case OCUBEofUNDEAD:
			case OPOTION:
			case OORB:
			case OHANDofFEAR:
			case OBRASSLAMP:
			case OURN:
			case OWWAND:
			case OSPHTALISMAN:
			case OSCROLL:
				break;

			default:
				count++;
				break;
			}
		}
	}

	t_setup(count);

	/* display the wieldable items in item order */
	for (i = 22; i < 93; i++) {
		for (j = 0; j < IVENSIZE; j++) {
			if (iven[j] == i) {
				switch (i) {
				case ODIAMOND:
				case ORUBY:
				case OEMERALD:
				case OSAPPHIRE:
				case OBOOK:
				case OCHEST:
				case OLARNEYE:
				case ONOTHEFT:
				case OSPIRITSCARAB:
				case OCUBEofUNDEAD:
				case OPOTION:
				case OHANDofFEAR:
				case OBRASSLAMP:
				case OURN:
				case OSPHTALISMAN:
				case OWWAND:
				case OORB:
				case OSCROLL:
					break;

				default:
					show3(j);
					pager();
					break;
				}
			}
		}
	}

	more();
	nosignal = 0;
	t_endup(count);
}

/* =============================================================================
 * FUNCTION: showread
 */
void showread(void)
{
	int i;
	int j;
	int count;

	nosignal = 1; /* don't allow ^c etc */
	clearpager();

	count = 2;
	for (j = 0; j < IVENSIZE; j++) {
		switch (iven[j]) {
		case OBOOK:
		case OSCROLL:
			count++;
			break;

		default:
			break;
		}
	}

	t_setup(count);


	/* display the readable items in item order */
	for (i = 22; i < 84; i++) {
		for (j = 0; j < IVENSIZE; j++) {

			if (iven[j] == i) {
				switch (i) {
				case OBOOK:
				case OSCROLL:
					show3(j);
					pager();
					break;

				default:
					break;
				}
			}
		}
	}

	more();
	nosignal = 0;
	t_endup(count);
}

/* =============================================================================
 * FUNCTION: showeat
 */
void showeat(void)
{
	int i;
	int j;
	int count;

	nosignal = 1; /* don't allow ^c etc */
	clearpager();

	count = 2;
	for (j = 0; j < IVENSIZE; j++) {
		switch (iven[j]) {
		case OCOOKIE:
			count++;
			break;

		default:
			break;

		}
	}

	t_setup(count);

	/* show the edible items in item order */
	for (i = 22; i < 84; i++) {
		for (j = 0; j < IVENSIZE; j++) {

			if (iven[j] == i) {
				switch (i) {
				case OCOOKIE:
					show3(j);
					pager();
					break;

				default:
					break;
				}
			}
		}
	}

	more();
	nosignal = 0;
	t_endup(count);
}

/* =============================================================================
 * FUNCTION: showquaff
 */
void showquaff(void)
{
	int i, j, count;

	nosignal = 1; /* don't allow ^c etc */
	clearpager();

	count = 2;
	for (j = 0; j < IVENSIZE; j++) {
		switch (iven[j]) {
		case OPOTION:
			count++;
			break;

		default:
			break;
		}
	}
	t_setup(count);

	/* show the quaffable items in item order */
	for (i = 22; i < 84; i++) {
		for (j = 0; j < IVENSIZE; j++) {
			if (iven[j] == i) {
				switch (i) {
				case OPOTION:
					show3(j);
					pager();
					break;

				default:
					break;
				}
			}
		}
	}

	more();
	nosignal = 0;
	t_endup(count);
}

/* =============================================================================
 * FUNCTION: seemagic
 */
void seemagic(int arg)
{
	int i, number;

	count = 0;
	lincount = 0;
	nosignal = 1;

	set_display(DISPLAY_TEXT);

	if (arg == 99) {
		number = (SPELL_COUNT + 2) / 3 + 4; /* # lines needed to display */
		ClearText();
		MoveCursor(1, 1);
		Print("Availible spells are:\n\n");
		for (i = 0; i < SPELL_COUNT; i++) {
			Printf("%s %-20s ", spelcode[i], spelname[i]);
			seepage();
		}
		seepage();
		more();

		nosignal = 0;
		draws(0, MAXX, 0, number);
		return;
	}else if (arg == -1) {
		/* if display spells while casting one */
		number = 0;
		for (i = 0; i < SPELL_COUNT; i++)
			if (spelknow[i]) number++;

		number = (number + 2) / 3 + 4; /* # lines needed to display */
		ClearText();
		MoveCursor(1, 1);
	}else
		ClearText();

	Print("The magic spells you have discovered to date are:\n\n");
	for (i = 0; i < SPELL_COUNT; i++) {
		if (spelknow[i]) {
			Printf("%s %-20s ", spelcode[i], spelname[i]);
			seepage();
		}
	}

	if (arg == -1) {
		seepage();
		more();
		nosignal = 0;
		set_display(DISPLAY_MAP);

		return;
	}

	lincount += 3;
	if (count != 0) {
		count = 2;
		seepage();
	}

	Print("\nThe magic scrolls you have found to date are:\n\n");
	count = 0;
	for (i = 0; i < MAXSCROLL; i++) {
		if (scrollknown[i]) {
			Printf("%-26s", &scrollname[i][1]);
			seepage();
		}
	}

	lincount += 3;
	if (count != 0) {
		count = 2;
		seepage();
	}

	Print("\nThe magic potions you have found to date are:\n\n");
	count = 0;
	for (i = 0; i < MAXPOTION; i++) {
		if (potionknown[i]) {
			Printf("%-26s", &potionname[i][1]);
			seepage();
		}
	}

	if (lincount != 0) more();

	nosignal = 0;
	set_display(DISPLAY_MAP);

}

/* =============================================================================
 * FUNCTION: show1
 */
void show1(int idx, char *str2[], int known[])
{
	/* standard */
	if (known == 0) {
		if (str2 == 0)
			Printf("\n%c)   %s", idx + 'a', objectname[(int)iven[idx]]);
		else if (*str2[ivenarg[idx]] == 0)
			Printf("\n%c)   %s", idx + 'a', objectname[(int)iven[idx]]);
		else
			Printf("\n%c)   %s of%s", idx + 'a', objectname[(int)iven[idx]], str2[ivenarg[idx]]);
	}else {
		/* scroll or potion or something with a known array */
		if (str2 == 0)
			Printf("\n%c)   %s", idx + 'a', objectname[(int)iven[idx]]);
		else if (*str2[ivenarg[idx]] == 0)
			Printf("\n%c)   %s", idx + 'a', objectname[(int)iven[idx]]);
		else if (known[ivenarg[idx]] == 0)
			Printf("\n%c)   %s", idx + 'a', objectname[(int)iven[idx]]);
		else
			Printf("\n%c)   %s of%s", idx + 'a', objectname[(int)iven[idx]], str2[ivenarg[idx]]);
	}

	if (wizard)
		Printf(" [ %d ]", ivenarg[idx]);
}

/* =============================================================================
 * FUNCTION: show3
 */
void show3(int index)
{
	switch (iven[index]) {
	case OPOTION:
		show1(index, potionname, potionknown);
		break;

	case OSCROLL:
		show1(index, scrollname, scrollknown);
		break;

	case OLARNEYE:
	case OBOOK:
	case OSPIRITSCARAB:
	case ODIAMOND:
	case ORUBY:
	case OCUBEofUNDEAD:
	case OEMERALD:
	case OCHEST:
	case OCOOKIE:
	case OSAPPHIRE:
	case OORB:
	case OHANDofFEAR:
	case OBRASSLAMP:
	case OURN:
	case OWWAND:
	case OSPHTALISMAN:
	case ONOTHEFT:
		show1(index, (char **)0, (int *)0);
		break;

	default:
		Printf("\n%c)   %s", index + 'a', objectname[(int)iven[index]]);
		show_plusses(ivenarg[index]);
		break;
	}

	if (c[WIELD] == index)
		Print(" (weapon in hand)");

	if ((c[WEAR] == index) || (c[SHIELD] == index))
		Print(" (being worn)");

}
