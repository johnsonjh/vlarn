/* =============================================================================
 * PROGRAM:  ularn
 * FILENAME: dungeon_obj.c
 *
 * DESCRIPTION:
 * This module contains functions for handling the effects of static objects
 * in the dungeon.
 * (ie. Objects that cannot be picked up such as stairs, fountains etc.)
 *
 * =============================================================================
 * EXPORTED VARIABLES
 *
 * None
 *
 * =============================================================================
 * EXPORTED FUNCTIONS
 *
 *  oopendoor   - processes the player opening a closed door
 *  oaltar      - processes player stepping onto an alter
 *  othrone     - processes player stepping onto a throne
 *  odeadthrone - processes player stepping onto a dead throne
 *  ofountain   - processes player stepping onto a fountain
 *  ostairs     - processes player stepping onto the stairs (up or down)
 *  oteleport   - processes player stepping onto a teleport trap, and all other
 *                reasons for teleporting the player
 *  opit        - processes player stepping onto a pit
 *  oelevator   - processes player stepping onto an elevator
 *  ostatue     - processes player stepping onto a statue
 *  omirror     - processes player stepping onto a mirror
 *
 * =============================================================================
 */

#include "ularn_win.h"
#include "dungeon_obj.h"
#include "header.h"
#include "player.h"
#include "monster.h"
#include "potion.h"
#include "scores.h"
#include "itm.h"
#include "ularn_game.h"

/* =============================================================================
 * Local functions
 */

/* =============================================================================
 * FUNCTION: ohear
 *
 * DESCRIPTION:
 * Function to cast a +3 protection on the player from an altar.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void ohear(void)
{
	Print("\nYou have been heard!");
	if (c[ALTPRO] == 0)
		c[MOREDEFENSES] += ALTAR_PRO_BOOST;
	c[ALTPRO] += 800; /* protection field */
	recalc();
	UpdateEffects();
}

/* =============================================================================
 * FUNCTION: fch
 *
 * DESCRIPTION:
 * Changes a players ability by 1 for ofountain effects.
 *
 * PARAMETERS:
 *
 *   how : The direction the attribute is to be changed.
 *           x < 0 => down
 *           x > 0 => up
 *
 *   x   : A pointer to the attribute value to change.
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void fch(int how, long *x)
{
	if (how < 0 ) {
		if (*x > 3) {
			Print(" went down by one!");
			--(*x);
		}else
			Print(" remained unchanged!");
	}else {
		Print(" went up by one!");
		(*x)++;
	}
	UpdateStatus();
}

/* =============================================================================
 * FUNCTION: fntchange
 *
 * DESCRIPTION:
 * Function to change player attributes for fountain based effects.
 *
 * PARAMETERS:
 *
 *   how : The size and direction of the change.
 *           how > 0 => raised
 *           how < 0 => lowered
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void fntchange(int how)
{
	long j;

	Printc('\n');
	switch (rnd(9)) {
	case 1:
		Print("Your strength");
		fch(how, &c[STRENGTH]);
		break;
	case 2:
		Print("Your intelligence");
		fch(how, &c[INTELLIGENCE]);
		break;
	case 3:
		Print("Your wisdom");
		fch(how, &c[WISDOM]);
		break;
	case 4:
		Print("Your constitution");
		fch(how, &c[CONSTITUTION]);
		break;
	case 5:
		Print("Your dexterity");
		fch(how, &c[DEXTERITY]);
		break;
	case 6:
		Print("Your charm");
		fch(how, &c[CHARISMA]);
		break;
	case 7:
		j = rnd(level + 1);
		if (how < 0) {
			Printf("You lose %d hit point%s!", (long)j, plural(j));
			losemhp((int)j);
		}else {
			Printf("You gain %d hit point%s!", (long)j, plural(j));
			raisemhp((int)j);
		}
		UpdateStatus();
		break;
	case 8:
		j = rnd(level + 1);
		if (how > 0) {
			Printf("You just gained %d spell%s!", (long)j, plural(j));
			raisemspells((int)j);
		}else {
			Printf("You just lost %d spell%s!", (long)j, plural(j));
			losemspells((int)j);
		}
		UpdateStatus();
		break;
	case 9:
		j = 5 * rnd((level + 1) * (level + 1));
		if (how < 0) {
			Printf("You just lost %d experience point%s!", (long)j, plural(j));
			loseexperience((long)j);
		}else {
			Printf("You just gained %d experience point%s!", (long)j, plural(j));
			raiseexperience((long)j);
		}
		break;
	}
}

/* =============================================================================
 * FUNCTION: obottomless
 *
 * DESCRIPTION:
 * Processes the player falling into a bottomless pit.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void obottomless(void)
{
	Print("\nYou fell into a pit leading straight to HELL!");
	UlarnBeep();

	nap(3000);
	died(DIED_FELL_INTO_BOTTOMLESS_PIT, 0);
}

/* =============================================================================
 * Exported functions
 */

/* =============================================================================
 * FUNCTION: oopendoor
 */
void oopendoor(int x, int y)
{
	if (item[x][y] != OCLOSEDDOOR)
		return;

	if (rnd(11) < 7) {
		/*
		 * Failed to open the door
		 * See if something nasty happened instead
		 */
		switch (iarg[x][y]) {
		case 6:
			c[AGGRAVATE] += rnd(400);
			break;

		case 7:
		case 8:
			Print("\nYou are jolted by an electric shock!");

			losehp(DIED_ELECTRIC_SHOCK, rnd(20));
			UpdateStatus();
			break;

		case 9:
			Print("\nYou suddenly feel weaker!");
			if (c[STRENGTH] > 3) c[STRENGTH]--;
			UpdateStatus();
			break;

		default:
			break;
		}

		/* Now the trap has been triggered, clear the trap */
		iarg[x][y] = 0;
	}else {
		item[x][y] = OOPENDOOR;
		show1cell(x, y);
	}
}

/* =============================================================================
 * FUNCTION: oaltar
 */
void oaltar(void)
{
	long k;
	int p;
	int ans;
	int redo;

	do{
		redo = 0;
		ans = get_prompt_input(
			"\nDo you (p) pray  (d) desecrate, or (i) ignore it? ",
			"pdi\033", 1);

		switch (ans) {
		case 'p':
			Print(" pray");
			ans = get_prompt_input(
				"\nDo you (m) give money or (j) just pray? ",
				"mj", 1);

			switch (ans) {
			case 'j':
				p = rund(100);
				if (p < 12) createmonster(makemonst(level + 2));
				else if (p < 17) enchweapon(ENCH_ALTAR);
				else if (p < 22) enchantarmor(ENCH_ALTAR);
				else if (p < 27) ohear();
				else Print("\nNothing happens.");
				break;

			case 'm':
				Print("\nHow much do you donate? ");
				k = get_num_input(c[GOLD]);

				if (k < 0)
					redo = 1;
				else if (c[GOLD] < k) {
					Print(" You don't have that much!");
					nap(1001);
					redo = 1;
				}else {
					/*
					 * Remove gold from player
					 */
					c[GOLD] -= k;

					if ((k < (c[GOLD] / 10)) && (rnd(60) < 30) && !wizard) {
						/*
						 * Player offers < 1/11% of gold insults the gods 50% of the time.
						 * Insulted gods send a demon prince to punish the player.
						 */
						Print(" Cheapskate! The Gods are insulted by such a tiny offering!");
						forget();
						createmonster(DEMONPRINCE);
						c[AGGRAVATE] += 1500;
						/* God takes more gold anyway */
						c[GOLD] -= k;
					}else if (((k < (c[GOLD] + k) / 10) || (k < rnd(50))) && !wizard) {
						/*
						 * Player offers more than 1/11, but less than 1/10 of gold.
						 * God has a chance of sending a monster to encourage the player
						 * to be more generous based on how far below 50 gold the
						 * amount doneted is.
						 */
						createmonster(makemonst(level + 2));
						c[AGGRAVATE] += 500;
						/* God takes more gold anyway */
						c[GOLD] -= k;
					}else {
						/*
						 * Player was reasonably generous, so give a blessing
						 */
						p = rund(16);
						if (p <  4)
							Print(" Thank you.");
						else if (p <  6) {
							enchantarmor(ENCH_ALTAR);
							enchantarmor(ENCH_ALTAR);
						}else if (p < 8) {
							enchweapon(ENCH_ALTAR);
							enchweapon(ENCH_ALTAR);
						}else
							ohear();

					}

					UpdateStatus();
				}
				break;

			default:
				break;

			} /* end while switch : case j or m */
			break;

		case 'd':
			Print(" desecrate");
			if (rnd(100) < 60) {
				createmonster(makemonst(level + 3) + 8);
				c[AGGRAVATE] += 2500;
			}else if (rnd(100) < 5)
				raiselevel();
			else if (rnd(101) < 30) {
				Print("\nThe altar crumbles into a pile of dust before your eyes.");
				forget();
			}else
				Print("\nNothing happens.");
			UpdateStatus();
			break;

		case 'i':
		case ESC:
			Print(" ignore");
			if (rnd(100) < 30) {
				createmonster(makemonst(level + 2));
				c[AGGRAVATE] += rnd(450);
			}else {
				Print("\nNothing happens.");
				nomove = 1; /* XXX trn */
			}
			break;

		} /* end while switch: pray, des, ignore */

	} while (redo);

}

/* =============================================================================
 * FUNCTION: othrone
 */
void othrone(int arg)
{
	int i, k;
	int ans;

	ans = get_prompt_input(
		"\nDo you (p) pry off jewels, (s) sit down, or (i) ignore it? ",
		"psi\033", 1);

	switch (ans) {
	case 'p':
		Print(" pry off");
		k = rnd(101);
		if (k < 25) {
			for (i = 0; i < rnd(4); i++) {
				creategem(); /*gems pop off the throne*/
			}
			item[playerx][playery] = ODEADTHRONE;
		}else if ((k < 40) && (arg == 0)) {
			createmonster(GNOMEKING);
			item[playerx][playery] = OTHRONE2;
		}else
			Print("\nNothing happens.");
		break;

	case 's':
		Print(" sit down");
		k = rnd(101);
		if ((k < 30) && (arg == 0)) {
			createmonster(GNOMEKING);
			item[playerx][playery] = OTHRONE2;
		}else if (k < 35) {
			Print("\nZaaaappp!  You've been teleported!\n");
			UlarnBeep();
			oteleport(0);
		}else
			Print("\nNothing happens.");
		break;

	case 'i':
	case ESC:
		Print(" ignore");
		break;;
	}
}

/* =============================================================================
 * FUNCTION: odeadthrone
 */
void odeadthrone(void)
{
	int k;
	int ans;

	ans = get_prompt_input(
		"\nDo you (s) sit down, or (i) ignore it? ",
		"si\033", 1);

	switch (ans) {
	case 's':
		Print(" sit down");
		k = rnd(101);
		if (k < 5)
			raiselevel();
		else if (k < 25) {
			Print("\nZaaaappp!  You've been teleported!\n");
			UlarnBeep();
			oteleport(0);
		}else
			Print("\nNothing happens.");
		break;

	case 'i':
	case ESC:
		Print(" ignore");;
		break;

	}
}

/* =============================================================================
 * FUNCTION: ofountain
 */
void ofountain(void)
{
	int x;
	int ans;

	ans = get_prompt_input(
		"\nDo you (d) drink, (w) wash yourself, or (i) ignore it? ",
		"dwi\033", 1);

	switch (ans) {
	case 'd':
		Print("drink");
		if (rnd(1501) < 4) {
			Print("\nOH MY GOD!! You have caught the *dreadful sleep*!");
			UlarnBeep();
			nap(3000);
			died(DIED_DREADFUL_SLEEP, 0);
			return;
		}else {
			x = rnd(100);
			if (x == 1)
				raiselevel();
			else if (x < 11) {
				x = rnd((level << 2) + 2);
				Printf("\nBleah! The water tasted like stale gatorade!  You lose %d hit point%s!",
				       (long)x, plural(x));

				losehp(DIED_DRANK_POISONOUS_WATER, x);
				UpdateStatus();
			}else if (x < 14) {
				c[HALFDAM] += 200 + rnd(200);
				Print("\nThe water makes you vomit.");
			}else if (x < 17) {
				/* Same effect as giant strength */
				Print("\n  You now have incredible bulging muscles!");
				if (c[GIANTSTR] == 0) c[STREXTRA] += PGIANTSTR_BOOST;
				c[GIANTSTR] += 700;
				UpdateEffects();
			}else if (x < 45)
				Print("\nNothing seems to have happened.");
			else if (rnd(3) != 2) {
				fntchange(1);   /*change char levels upward*/
			}else {
				fntchange(-1);  /*change char levels downward*/
			}

			if (rnd(12) < 3) {
				Print("\nThe fountains bubbling slowly quietens.");
				/* dead fountain */
				item[playerx][playery] = ODEADFOUNTAIN;
			}
		}
		break;

	case 'i':
	case ESC:
		Print(" ignore");
		break;

	case 'w':
		Print("wash yourself.");
		if (rnd(100) < 11) {
			x = rnd((level << 2) + 2);
			Printf("\nThe water burns like acid!  You lose %d hit point%s!",
			       (long)x, plural(x));

			losehp(DIED_DRANK_POISONOUS_WATER, x);
			UpdateStatus();
		}else if (rnd(100) < 29) {
			Print("\nYou are now clean.");
			if (c[ITCHING])
				/*
				 * Managed to get rid of the itching powder, so set it so the
				 * next call to regen will cancel the effect.
				 */
				c[ITCHING] = 1;
		}else if (rnd(100) < 31)
			Print("\nThis water needs soap -- the dirt didn't come off.");
		else if (rnd(100) < 34)
			createmonster(WATERLORD);
		else
			Print("\nNothing seems to have happened.");
		break;
	}
}

/* =============================================================================
 * FUNCTION: ostairs
 */
void ostairs(int dir)
{
	int x, y;
	int ans;

	Print("\nDo you (s) stay here  or  ");
	if (dir > 0)
		Print("(u) go up?  ");
	else
		Print("(d) go down?  ");

	ans = get_prompt_input("", "sudi\033", 1);

	switch (ans) {
	case ESC:
	case 's':
	case 'i':
		Print("stay here.");
		return;

	case 'u':
		Print("go up.");
		if (dir < 0)
			Print("\nThe stairs don't go up!");
		else{
			/* not on V1 */
			if ((level >= 2) && (level != (DBOTTOM + 1))) {
				newcavelevel(level - 1);
				for (x = 0; x < MAXX; x++) {
					for (y = 0; y < MAXY; y++) {
						if (item[x][y] == OSTAIRSDOWN) {
							playerx = (char)x;
							playery = (char)y;
							x = MAXX;
							y = MAXY;
						}
					}
				}

				if (mitem[playerx][playery].mon != MONST_NONE)
					/*
					 * A monster is on the stairs, so find an empty position for the
					 * player.
					 */
					positionplayer();

				draws(0, MAXX, 0, MAXY);
				UpdateStatusAndEffects();
			}else
				Print("\nThe stairs lead to a dead end!");
		}
		break;

	case 'd':
		Print("go down.");
		if (dir > 0)
			Print("\nThe stairs don't go down!");
		else{
			/* not on dungeon bottom or V5 */
			if ((level != 0) && (level != DBOTTOM) && (level != VBOTTOM)) {
				newcavelevel(level + 1);
				for (x = 0; x < MAXX; x++) {
					for (y = 0; y < MAXY; y++) {
						if (item[x][y] == OSTAIRSUP) {
							playerx = (char)x;
							playery = (char)y;
							x = MAXX;
							y = MAXY;
						}
					}
				}

				if (mitem[playerx][playery].mon != MONST_NONE)
					/*
					 * A monster is on the stairs, so find an empty position for the
					 * player.
					 */
					positionplayer();

				draws(0, MAXX, 0, MAXY);
				UpdateStatusAndEffects();
			}else
				Print("\nThe stairs lead to a dead end!");
		}
		break;

	default:
		break;

	}
}


/* =============================================================================
 * FUNCTION: oteleport
 */
void oteleport(int err)
{
	int tmp;

	if (err) {
		if (rnd(151) < 3) {
			/* stuck in a rock if the player can't walk through walls */
			if (c[WTW] == 0) {
				died(DIED_TRAPPED_IN_SOLID_ROCK, 0);
				return;
			}
		}
	}

	/* show ?? on bottomline if been teleported */
	if (!wizard) c[TELEFLAG] = 1;

	if (level == 0)
		tmp = 0;
	else if (level <= DBOTTOM) {
		/* in dungeon */
		tmp = rnd(5) + level - 3;
		if (tmp > DBOTTOM)
			tmp = DBOTTOM;
		if (tmp < 0)
			tmp = 0;
	}else {
		/* in volcano */
		tmp = rnd(4) + level - 2;

		if (tmp >= VBOTTOM)
			tmp = VBOTTOM;
		if (tmp < DBOTTOM + 1)
			/* back to surface */
			tmp = 0;
	}
	playerx = (char)rnd(MAXX - 2);
	playery = (char)rnd(MAXY - 2);
	if (level != tmp)
		newcavelevel(tmp);
	else
		positionplayer();
	draws(0, MAXX, 0, MAXY);
	UpdateStatusAndEffects();
}


/* =============================================================================
 * FUNCTION: opit
 */
void opit(void)
{
	int i;

	if (rnd(101) > 81) return;

	if ((rnd(70) > (9 * c[DEXTERITY] - packweight())) || (rnd(101) < 5)) {
		/* Never fall into a pit if the player has a wand of wonder */
		if (player_has_item(OWWAND)) {
			Print("\nYou float right over the pit.");
			return;
		}

		if (level == DBOTTOM || level == VBOTTOM)
			/* Pits on the bottom of the dungeon or volcano are bottomless */
			obottomless();
		else{
			if (rnd(101) < 20) {
				i = 0;
				Print("\nYou fell ino a pit!  A poor monster cushions your fall!\n");
			}else {
				i = rnd(level * 3 + 3);
				if (i > c[HP]) i = c[HP];

				Printf("\nYou fell into a pit!  You suffer %d hit point%s damage.",
				       (long)i, plural(i));

			}

			losehp(DIED_FELL_INTO_PIT, i);
			nap(2000);
			newcavelevel(level + 1);
			draws(0, MAXX, 0, MAXY);
		}
	}
}

/* =============================================================================
 * FUNCTION: oelevator
 */
void oelevator(int dir)
{
	int new_level;

	if (dir == 1) {
		/* going up */
		if (level == 0) {
			Print(",\nunfortunately, it is out of order.");
			return;
		}

		playerx = (char)rnd(MAXX - 2);
		playery = (char)rnd(MAXY - 2);
		nap(2000);
		if (level <= DBOTTOM)
			/* In dungeon */
			newcavelevel(rund(level));
		else{
			/* In volcano */
			new_level = DBOTTOM + rund(level - DBOTTOM);
			if (new_level == DBOTTOM)
				new_level = 0;
			newcavelevel(new_level);
		}
	}else {
		/* going down */
		if ((level == DBOTTOM) || (level == VBOTTOM)) {
			nap(2000);
			Print("\nand it leads straight to HELL!");
			UlarnBeep();
			nap(3000);
			died(DIED_ELEVATOR_TO_HELL, 0);
			return;
		}
		playerx = (char)rnd(MAXX - 2);
		playery = (char)rnd(MAXY - 2);
		nap(2000);

		if (level < DBOTTOM)
			/* In dungeon */
			newcavelevel(level + rnd(DBOTTOM - level));
		else
			/* in volcano */
			newcavelevel(level + rnd(VBOTTOM - level));
	}

	draws(0, MAXX, 0, MAXY);
	UpdateStatusAndEffects();
}

/* =============================================================================
 * FUNCTION: ostatue
 */
void ostatue(void)
{
}   /* nothing happens when you move on a statue */


/* =============================================================================
 * FUNCTION: omirror
 */
void omirror(void)
{
}   /* nothing happens when you move on a mirror */
