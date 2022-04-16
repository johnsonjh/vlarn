/* =============================================================================
 * PROGRAM:  ularn
 * FILENAME: ularn.c
 *
 * DESCRIPTION:
 * This is the main module for ularn.
 * It contains the setup and main processing loop.
 *
 * =============================================================================
 * EXPORTED VARIABLES
 *
 * None.
 *
 * =============================================================================
 * EXPORTED FUNCTIONS
 *
 * None.
 *
 * =============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "patchlevel.h"

#include "ularn_game.h"
#include "ularn_win.h"
#include "ularn_ask.h"
#include "getopt.h"
#include "savegame.h"
#include "scores.h"
#include "header.h"
#include "dungeon_obj.h"
#include "dungeon.h"
#include "player.h"
#include "monster.h"
#include "action.h"
#include "object.h"
#include "potion.h"
#include "scroll.h"
#include "spell.h"
#include "show.h"
#include "help.h"
#include "diag.h"
#include "itm.h"

#ifdef WINDOWS
# include <windows.h>
#endif

/* =============================================================================
 * Local variables
 */

#define BUFSZ   256 /* for getlin buffers */
#define MAX_CMDLINE_PARAM 255

static char copyright[] =
  "\nVLarn by Jeffrey H. Johnson <trnsz@pobox.com>\n"
  "  Ularn created by Phil Cordier -- based on Larn by Noah Morgan\n"
  "  Updated by Josh Brandt and David Richerby\n"
  "  Rewrite and Windows32/X11/Amiga graphics conversion by Julian Olds";

static char cmdhelp[] = "\
Cmd line format: vlarn [-sicnh] [-o <optsfile>] [-d #] [-r]\n\
  -s   show the scoreboard\n\
  -i   show the scoreboard with inventories\n\
  -c   create new scoreboard (wizard only)\n\
  -n   suppress welcome message when starting game\n\
  -h   display this help message\n\
  -o <optsfile> specify options file to be used instead of \"~/vlarn.opt\"\n\
  -d # specify level of difficulty (example: vlarn -d 5)\n\
  -r   restore checkpoint (vlarn.ckp) file\n";

static char *optstring = "sicnhro:d:";

static short viewflag;

/* =============================================================================
 * Local functions
 */

#ifdef WINDOWS

/*
 * Windows programs don't use standard command line arguments, so provide
 * a function to convert the windows command line to argc, argv.
 */

/* =============================================================================
 * FUNCTION: _get_cmd_arg
 *
 * DESCRIPTION:
 * Function to help process the windows command line into argc, argv
 * format.
 *
 * PARAMETERS:
 *
 *   pCmdLine : The command line.
 *
 * RETURN VALUE:
 *
 *   The next word on the command line.
 */
static char* _get_cmd_arg(char* pCmdLine)
{
  static char* pArgs = NULL;
  char  *pRetArg;

  if ((pCmdLine == NULL) && (pArgs == NULL)) return NULL;
  if (pArgs == NULL) pArgs = pCmdLine;

  /* skip whitespace */
  for (pRetArg = pArgs; (*pRetArg != 0) && isspace(*pRetArg); pRetArg++);
  {
  	if (*pRetArg == 0) {
  		/* Hit end of string, so return */
  		pArgs = NULL;
  		return NULL;
  	}
  }

  /* check for quote */
  if ( *pRetArg == '"' ) {
  	pRetArg++;
  	pArgs = strchr(pRetArg, '"');
  }else
  	/* skip to whitespace */
  	for (pArgs = pRetArg; (*pArgs != 0) && !isspace(*pArgs); pArgs++);

  if ((pArgs != NULL) && (*pArgs != 0) ) {
  	char* p;
  	p = pArgs;
  	pArgs++;
  	*p = 0;
  }else
  	pArgs = NULL;

  return pRetArg;
}

#endif

/* =============================================================================
 * FUNCTION: ULarnSetup
 *
 * DESCRIPTION:
 * Perform once off initialisation and parse the command parameters.
 *
 * PARAMETERS:
 *
 *   argc : The command line argumant count
 *
 *   argv : The command line arguments.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void ULarnSetup(int argc, char *argv[])
{
  int i;
  int hard;
  int restore_ckp;
  int set_optfile;
  int reset_scores;
  int show_scores;
  char buf[BUFSIZ];
  char *home;

  /* clear the loginname and logname */
  loginname[0] = 0;
  logname[0] = 0;

  set_display(DISPLAY_TEXT);
  ClearText();

  nap(100);

#ifdef UNIX

  home = getenv("HOME");

#else

# ifdef AMIGA_WIN

  home = "";

# else

  home = ".";

# endif

#endif

  /* set the save directory to the home directory by default */
  strcpy(savedir, home);

  /* initialize dungeon storage */

  init_cells();

  /* set the initial clock and initialise the random number generator*/
  newgame();
  hard = -1;
  restore_ckp = 0;
  set_optfile = 0;
  reset_scores = 0;
  show_scores = 0;

  /*
   * now process the command line arguments
   */
  opterr = 0;
  while ((i = ugetopt(argc, argv, optstring)) != -1) {
  	switch (i) {
  	case 's':
  		show_scores = 1;
  		break;

  	case 'i':
  		show_scores = 2;
  		break;

  	case 'c':
  		reset_scores = 1;
  		break;

  	case 'n':
  		nowelcome = 1;
  		break;

  	case 'd':
  		/* specify hardness */
  		hard = atoi(optarg);
  		if (hard > 100) hard = 100;
  		if (hard < 0) {
  			Printf("Difficulty level must be > 0\n");
  			Print(cmdhelp);
  			get_prompt_input("\n\nPress space to exit", " ", 0);

  			endgame();
  		}
  		break;

  	case 'h':
  	case '?':
  		/* print out command line arguments */
  		Printf("%s", copyright);
  		Printf("\n\n  Version %s.%s (%s)", LARN_VERSION, LARN_PATCHLEVEL, LARN_DATE);
  		Printf("\n\n%s", cmdhelp);
  		get_prompt_input("\n\nPress space to exit", " ", 0);

  		endgame();
  		break;

  	case 'o':
  		/* specify a Ularn.opt filename */
  		strcpy(optsfile, optarg);
  		set_optfile = 1;
  		break;

  	case 'r':
  		/* restore checkpointed game */
  		restore_ckp = 1;
  		break;

  	default:
  		if (!opterr)
  			Printf("Unknown option <%s>\n",
  			       argv[optind - 1]);
  		Printf("%s\n", copyright);
  		Printf("Version %s.%s (%s)\n", LARN_VERSION, LARN_PATCHLEVEL, LARN_DATE);
  		Printf("%s", cmdhelp);
  		get_prompt_input("\n\nPress space to exit", " ", 0);

  		endgame();

  		break;

  	}       /* end switch */
  }               /* end while */


  /* Options filename was not specified on the command line, so set default */
  if (!set_optfile) {
  	sprintf(buf, "%s/%s", home, optsfile);
  	strcpy(optsfile, buf);
  }

  /* read the options file if there is one */
  read_options();

  /*
   * Process scorefile initialisation
   */

  /* the Ularn scoreboard filename */
  sprintf(scorefile, "%s/%s", libdir, SCORENAME);

  if (reset_scores) {
  	/* anyone with password can create scoreboard */
  	Print("Preparing to initialize the scoreboard.\n");
  	if (getpassword() != 0) {
  		if (makeboard() == -1) {
  			Print("Scoreboard creation failed!!\n");
  			endgame();
  		}
  		showscores();
  		Print("\nScoreboard initialized.\n");
  	}
  	endgame();
  }

  if (show_scores == 1) {
  	showscores();
  	endgame();
  }else if (show_scores == 2) {
  	showallscores();
  	endgame();
  }

  /* now make scoreboard if it is not there (don't clear) */
  if (access(scorefile, 0) == -1) {
  	/* score file not there */
  	if (makeboard() == -1) {
  		Printf("I can't create the scoreboard.\n");
  		Printf("Check permissions on %s\n", libdir);
  		endgame();
  	}
  }

  /*
   * Get the user name and id.
   * For OS without usernames, use the logname, if it is specified.
   */
  strcpy(loginname, logname);
  GetUser(loginname, &userid);

  if (logname[0] == 0)
  	/* no logname specified, so use loginname */
  	strcpy(logname, loginname);

  /*
   * Set the file names
   */

  /* the Ularn on-line help file */
  sprintf(helpfile, "%s/%s", libdir, HELPNAME);

  /* the pre-made cave level data file */
  sprintf(larnlevels, "%s/%s", libdir, LEVELSNAME);

  /* the fortune data file name */
  sprintf(fortfile, "%s/%s", libdir, FORTSNAME);

  /* save file name in home directory */
  sprintf(savefilename, "%s/vlarn_%s.sav", savedir, loginname);

  /* the checkpoint file */
  sprintf(ckpfile, "%s/vlarn_%s.ckp", home, loginname);

  if (restore_ckp) {
  	if (access(ckpfile, 0) == -1) {
  		Printf("Cannot find checkpoint file %s\n", ckpfile);
  		endgame();
  	}
  	Printf("Restoring...");
  	restorflag = 1;
  	hitflag = 1;
  	restoregame(ckpfile);
  }

  /* restore game if need to, and haven't restored ckpfile */
  if (!restorflag && (access(savefilename, 0) == 0)) {
  	restorflag = 1;
  	hitflag = 1;
  	Print("Restoring...");
  	restoregame(savefilename);
  }

  /* create new game */
  if (restorflag == 0) {
  	/* make the character that will play */
  	/* XXX trn */
  	wizard = 0;
  	makeplayer();
  	/* make the dungeon */
  	newcavelevel(0);

  	if (nowelcome == 0)
  		/* welcome the player to the game */
  		welcome();
  }

  /* set up the desired difficulty  */
  sethard(hard);

  set_display(DISPLAY_MAP);

  showplayer();

  yrepcount = 0;
  hit2flag = 0;

}

/* =============================================================================
 * FUNCTION: parse
 *
 * DESCRIPTION:
 * Execute a command entered by the player.
 *
 * PARAMETERS:
 *
 *   Action : The action command requested.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void parse(ActionType Action)
{
  int i, j;
  int flag;

  switch (Action) {
  case ACTION_DIAG:
  	if (wizard)
  		diag();
  	yrepcount = 0;
  	return;

  case ACTION_MOVE_WEST:
  	moveplayer(4);
  	return;

  case ACTION_RUN_WEST:
  	run(4);
  	return;

  case ACTION_MOVE_EAST:
  	moveplayer(2);
  	return;

  case ACTION_RUN_EAST:
  	run(2);
  	return;

  case ACTION_MOVE_SOUTH:
  	moveplayer(1);
  	return;

  case ACTION_RUN_SOUTH:
  	run(1);
  	return;

  case ACTION_MOVE_NORTH:
  	moveplayer(3);
  	return;

  case ACTION_RUN_NORTH:
  	run(3);
  	return;

  case ACTION_MOVE_NORTHEAST:
  	moveplayer(5);
  	return;

  case ACTION_RUN_NORTHEAST:
  	run(5);
  	return;

  case ACTION_MOVE_NORTHWEST:
  	moveplayer(6);
  	return;

  case ACTION_RUN_NORTHWEST:
  	run(6);
  	return;

  case ACTION_MOVE_SOUTHEAST:
  	moveplayer(7);
  	return;

  case ACTION_RUN_SOUTHEAST:
  	run(7);
  	return;

  case ACTION_MOVE_SOUTHWEST:
  	moveplayer(8);
  	return;

  case ACTION_RUN_SOUTHWEST:
  	run(8);
  	return;

  case ACTION_WAIT:
  	if (yrepcount) viewflag = 1;
  	return;

  case ACTION_NONE:
  	yrepcount = 0;
  	nomove = 1;
  	return;

  case ACTION_WIELD:
  	yrepcount = 0;
  	wield();
  	return;

  case ACTION_WEAR:
  	yrepcount = 0;
  	wear();
  	return;

  case ACTION_READ:
  	yrepcount = 0;
  	if (c[BLINDCOUNT])
  		Print("\nYou can't read anything when you're blind!");
  	else if (c[TIMESTOP] == 0)
  		readscr();
  	return;

  case ACTION_QUAFF:
  	yrepcount = 0;
  	if (c[TIMESTOP] == 0)
  		quaff();
  	return;

  case ACTION_DROP:
  	yrepcount = 0;
  	if (c[TIMESTOP] == 0)
  		dropobj();
  	return;

  case ACTION_CAST_SPELL:
  	yrepcount = 0;
  	cast();
  	return;

  case ACTION_OPEN_DOOR:
  	yrepcount = 0;
  	opendoor();
  	return;

  case ACTION_CLOSE_DOOR:
  	yrepcount = 0;
  	closedoor();
  	return;

  case ACTION_OPEN_CHEST:
  	yrepcount = 0;
  	openchest();
  	return;

  case ACTION_INVENTORY:
  	yrepcount = 0;
  	nomove = 1;
  	showstr();
  	return;

  case ACTION_EAT_COOKIE:
  	yrepcount = 0;
  	if (c[TIMESTOP] == 0)
  		eatcookie();
  	return;

  case ACTION_LIST_SPELLS:
  	yrepcount = 0;
  	seemagic(0);
  	nomove = 1;
  	return;

  case ACTION_HELP:
  	yrepcount = 0;
  	help();
  	nomove = 1;
  	return;

  case ACTION_SAVE:
  	ClearText();
  	Print("Saving . . .");

  	if (savegame(savefilename) == -1)
  		Print("\nSave game failed.\n");
  	wizard = 1; /* so not show scores */
  	died(DIED_SUSPENDED, 0);
  	return;

  case ACTION_TELEPORT:
  	yrepcount = 0;
  	if (wizard) {
  		int t;

  		Print("\nWhich level do you wish to teleport to? ");
  		t = (int)get_num_input(20);
  		if (t > VBOTTOM || t < 0) {
  			Print(" sorry!");
  			return;
  		}

  		playerx = (char)rnd(MAXX - 2);
  		playery = (char)rnd(MAXY - 2);
  		newcavelevel(t);
  		positionplayer();
  		draws(0, MAXX, 0, MAXY);
  		UpdateStatusAndEffects();
  		return;
  	}
  	if (c[LEVEL] >= INNATE_TELEPORT_LEVEL) {
  		oteleport(1);
  		return;
  	}

  	Print("\nYou don't know how to teleport yet.");
  	return;

  case ACTION_IDENTIFY_TRAPS:
  	flag = 0;
  	yrepcount = 0;
  	Printc('\n');
  	for (j = playery - 1; j < playery + 2; j++) {
  		if (j < 0) j = 0;
  		if (j >= MAXY) break;
  		for (i = playerx - 1; i < playerx + 2; i++) {
  			if (i < 0) i = 0;
  			if (i >= MAXX) break;
  			switch (item[i][j]) {
  			case OTRAPDOOR:
  			case ODARTRAP:
  			case OTRAPARROW:
  			case OTELEPORTER:
  			case OELEVATORUP:
  			case OELEVATORDOWN:
  				Print("\nIt's ");
  				Print(objectname[(int)item[i][j]]);
  				flag++;
  			}
  		}
  	}
  	if (flag == 0)
  		Print("\nNo traps are visible.");
  	return;

  case ACTION_BECOME_CREATOR:
  	yrepcount = 0;
  	nomove = 1;
  	if (!wizard)
  		if (getpassword() == 0)
  			return;

  	raiseexperience(370 * 1000000);

  	recalc();
  	UpdateStatus();
  	drawscreen();
  	return;

  case ACTION_CREATE_ITEM:
  	yrepcount = 0;
  	if (wizard)
  		do_create();
  	return;

  case ACTION_TOGGLE_WIZARD:
  	yrepcount = 0;
  	nomove = 1;
  	if (wizard) {
  		Print("\nYou are no longer a wizard.");
  		wizard = 0;
  		return;
  	}
  	if (getpassword()) {
  		Print("\nYou are now a wizard.");
  		wizard = 1;
  	}else
  		Print("Sorry.\n");
  	return;

  case ACTION_DEBUG_MODE:
  	yrepcount = 0;
  	nomove = 1;
  	if (!wizard) {
  		if (getpassword() == 0)
  			return;
  		wizard = 1;
  	}

  	for (i = 0; i < 6; i++)
  		c[i] = 70;
  	iven[0] = ONOTHING;
  	iven[1] = ONOTHING;
  	take(OPROTRING, 50);
  	take(OLANCE, 25);
  	for (i = 0; i < IVENSIZE; i++) {
  		if (iven[i] == OLANCE && ivenarg[i] == 25) {
  			c[WIELD] = i;
  			break;
  		}
  	}
  	c[LANCEDEATH] = 1;
  	c[WEAR] = c[SHIELD] = -1;
  	raiseexperience(370 * 1000000);
  	c[AWARENESS] += 25000;

  	/* learn all spells, scrolls and potions */
  	for (i = 0; i < SPELL_COUNT; i++) spelknow[i] = 1;
  	for (i = 0; i < MAXSCROLL; i++) potionknown[i] = 1;
  	for (i = 0; i < MAXPOTION; i++) potionknown[i] = 1;

  	for (i = 0; i < MAXSCROLL; i++) {
  		if (strlen(scrollname[i]) > 2) {
  			item[i][0] = OSCROLL;
  			iarg[i][0] = (short)i;
  		}
  	}

  	for (i = 0; i < MAXPOTION; i++) {
  		/* no null items */
  		if (strlen(potionname[i]) > 2) {
  			item[(MAXX - 1) - i][0] = OPOTION;
  			iarg[(MAXX - 1) - i][0] = (short)(i);
  		}
  	}

  	j = OALTAR;

  	for (i = 1; i < MAXY; i++) {
  		item[0][i] = (char)j;
  		iarg[0][i] = (short)0;
  		j++;
  	}

  	for (i = 1; i < MAXX; i++) {
  		item[i][MAXY - 1] = (char)j;
  		iarg[i][MAXY - 1] = (short)0;
  		j++;
  	}

  	for (i = 1; i < MAXY - 1; i++) {
  		item[MAXX - 1][i] = (char)j;
  		iarg[MAXX - 1][i] = 0;
  		j++;
  	}

  	for (i = 0; i < MAXY; i++)
  		for (j = 0; j < MAXX; j++)
  			know[j][i] = item[j][i];

  	c[GOLD] += 250000;

  	recalc();
  	UpdateStatus();
  	drawscreen();
  	return;

  case ACTION_REMOVE_ARMOUR:
  	yrepcount = 0;

  	if (c[SHIELD] != -1) {
  		c[SHIELD] = -1;
  		Print("\nYour shield is off.");
  		recalc();
  		UpdateStatus();
  	}else {
  		if (c[WEAR] != -1) {
  			c[WEAR] = -1;
  			Print("\nYour armor is off.");
  			recalc();
  			UpdateStatus();
  		}else
  			Print("\nYou aren't wearing anything.");
  	}
  	return;

  case ACTION_PACK_WEIGHT:
  	Printf("\nThe stuff you are carrying presently weighs %d pound%s.",
  	       (long)packweight(),
  	       plural(packweight()));
  	nomove = 1;
  	yrepcount = 0;
  	return;

  case ACTION_VERSION:
  	yrepcount = 0;
  	Printf("\nVLarn %s.%s (%s) - Difficulty level %d",
  	       LARN_VERSION,
  	       LARN_PATCHLEVEL,
  	       LARN_DATE,
  	       (long)c[HARDGAME]);
  	if (wizard) Print(" (WIZARD)");
  	nomove = 1;
  	if (cheat) Print(" (Cheater)");
  	Print(copyright);
  	return;

  case ACTION_QUIT:
  	yrepcount = 0;
  	quit();
  	nomove = 1;
  	return;

  case ACTION_REDRAW_SCREEN:
  	yrepcount = 0;
  	drawscreen();
  	nomove = 1;
  	return;

  case ACTION_SHOW_TAX:
  	if (outstanding_taxes > 0)
  		Printf("\nYou presently owe %d gp in taxes.", (long)outstanding_taxes);
  	else
  		Print("\nYou do not owe any taxes.");
  	return;

  default:
  	Print("HELP! unknown command\n");
  	break;
  }
}

#ifdef WINDOWS

/* windows uses WinMain instead of main */

/* =============================================================================
 * FUNCTION: WinMain
 *
 * DESCRIPTION:
 * Windows main entry point.
 *
 * PARAMETERS:
 *
 *   hinstance     : This isntance of the application
 *
 *   hprevinstance : The previous instance of the application (if any)
 *
 *   lpcmdline     : The command line
 *
 *   ncmdshow      : The initial state for the application window
 *
 * RETURN VALUE:
 *
 *   Exit status of the application.
 *     0 => normal exit.
 */
int WINAPI WinMain(
  HINSTANCE hinstance,
  HINSTANCE hprevinstance,
  LPSTR lpcmdline,
  int ncmdshow)

#else

/* =============================================================================
 * FUNCTION: main
 *
 * DESCRIPTION:
 * Main entry point.
 *
 * PARAMETERS:
 *
 *   argc : Command line argument count
 *
 *   argv : Command line argument strings
 *
 * RETURN VALUE:
 *
 *   Exit status of the application.
 *     0 => normal exit.
 */
int main(int argc, char *argv[])

#endif

{
  ActionType Action;

#ifdef WINDOWS

  int argc;
  char* argv[MAX_CMDLINE_PARAM];
  TCHAR *p;
  TCHAR wbuf[BUFSZ];
  char buf[BUFSZ];
  size_t len;

#endif

#ifdef AMIGA_WIN
  int StartedFromWB = 0;
  char *fake_argv[1] = { "ularn" };
#endif


#ifdef UNIX_X11

  init_app(getenv("DISPLAY"));

#else

# ifdef WINDOWS

  /*
   * get command line parameters
   */
  p = _get_cmd_arg(GetCommandLine());
  p = _get_cmd_arg(NULL); /* skip first paramter - command name */
  for (argc = 1; p && argc < MAX_CMDLINE_PARAM; argc++ ) {
  	len = strlen(p);
  	if ( len > 0 )
  		argv[argc] = strdup(p);
  	else
  		argv[argc] = "";

  	p = _get_cmd_arg(NULL);
  }
  GetModuleFileName(NULL, wbuf, BUFSZ);
  argv[0] = strdup(strncpy(wbuf, buf, BUFSZ));

  //
  // Setup the display interface
  //

  if (!init_app(hinstance))
  	return 0;

# else

#  ifdef AMIGA_WIN

  /* The Amiga sets argc to 0 if the program was started from Workbench
   * In this case the arg list points to a WBStartup structure instead
   * of and array of arguments.
   */
  if (argc == 0)
  	StartedFromWB = 1;

  if (!init_app()) {
  	close_app();
  	return 0;
  }

#  else

  init_app();

#  endif

# endif

#endif

#ifdef AMIGA_WIN

  if (!StartedFromWB)
  	ULarnSetup(argc, argv);
  else
  	/* fake parameters */
  	ULarnSetup(1, fake_argv);

#else

  ULarnSetup(argc, argv);

#endif

  /* Everything should be common from here on */

  do{
  	if (dropflag == 0) {
  		lookforobject();        /* see if there is an object here*/
  	}else {
  		dropflag = 0;           /* don't show it just dropped an item */
  	}

  	if (hitflag == 0) {
  		if (c[HASTEMONST]) movemonst();

  		movemonst();
  	}

  	if (viewflag == 0)
  		showcell(playerx, playery);
  	else{
  		viewflag = 0; /* show stuff around player */
  	}

  	hitflag = 0;
  	hit3flag = 0;

  	nomove = 1;

  	/*  get commands and make moves */
  	while (nomove) {
  		nomove = 0;
  		Action = get_normal_input();
  		parse(Action); /* may reset nomove=1 */
  	}

  	/* regenerate hp and spells */
  	regen();

  	if (c[TIMESTOP] == 0) {
  		rmst--;
  		if (rmst <= 0) {
  			rmst = (char)(120 - (level << 2));
  			fillmonst(makemonst(level));
  		}
  	}

  } while (Action != ACTION_QUIT);

  /*
   * tidyup and exit
   */
  endgame();

  /* not actually reachable */
  return 0;

}
