/* =============================================================================
 * PROGRAM:  ularn
 * FILENAME: ularn_game.h
 *
 * DESCRIPTION:
 * Game data used by Ularn.
 * This Contains:
 *   . The names of data files used by ularn
 *   . The player's name
 *   . Current game options
 *
 * =============================================================================
 * EXPORTED VARIABLES
 *
 * do_fork        : True if fork on save (now unsupported)
 * boldon         : True if objects are to be dislayed in bold (tty only)
 * mail           : True if mail bills when game is won
 * ckpflag        : True if checkpoint files are to be used.
 * nobeep         : True if beep is off.
 * libdir         : Ularn library path
 * savedir        : Directroy for save games
 * savefilename   : Filename for saving the game
 * scorefile      : Filename for the scores
 * helpfile       : Filename for ularn help
 * larnlevels     : Filename for pregenerated levels
 * fortfile       : Filename for fortunes
 * optsfile       : Ularn options file
 * ckpfile        : Checkpoint file name
 * diagfile       : Diagnostic dump file name
 * userid         : User Id of the player
 * password       : Wizard password
 * loginname      : The login name of the player
 * logname        : The name to appear on the score board
 * nowelcome      : True if no welcome message is to be displayed
 * nomove         : True if player action resulted in no move.
 * dropflag       : True if the player just dropped the item
 * restoreflag    : True if the game is to be restored from a file
 * diroffx        : Direction offsets for x coordinate
 * diroffy        : Direction offsets for y coordinate
 * ReverseDir     : Lookup for the index of the reverse direction
 * dirname        : The name of each direction.
 *
 * =============================================================================
 * EXPORTED FUNCTIONS
 *
 * newgame      : Funtion to initialise a new game.
 * sethard      : Function to set the game difficulty
 * read_options : Function to read the ularn options file
 *
 * =============================================================================
 */

#ifndef __ULARN_GAME_H
# define __ULARN_GAME_H

/* =============================================================================
 * Exported variables
 */

/*
 * Game options
 */
extern char do_fork;
extern char boldon;  /* 1=bold objects,  0=inverse objects */
extern char mail;    /* 1=mail letters after win game */
extern char ckpflag; /* 1 if want checkpointing of game, 0 otherwise */
extern char nobeep;  /* true if program is not to beep*/

/* *************** File Names *************** */

# define SCORENAME "Vscore"
# define HELPNAME "Vhelp"
# define LEVELSNAME "Vmaps"
# define FORTSNAME "Vfortune"

/* maximum number moves before the game is called*/
# define TIMELIMIT 90000

/* create a checkpoint file every CKCOUNT moves */
# define CKCOUNT 150

/* max size of the players name */
# define LOGNAMESIZE 80
# define USERNAME_LENGTH 80

# ifndef MAXPATHLEN
#  define MAXPATHLEN 1024
# endif

/* The library files directory */
extern char libdir[MAXPATHLEN];

/* The directory for saved games */

extern char savedir[MAXPATHLEN];

/* the game save filename   */
extern char savefilename[MAXPATHLEN + 255];

/* the score file       */
extern char scorefile[MAXPATHLEN + 16];

/* the help text file */
extern char helpfile[MAXPATHLEN + 16];

/* the maze data file */
extern char larnlevels[MAXPATHLEN + 16];

/* the fortune data file */
extern char fortfile[MAXPATHLEN + 16];

/* the options file filename */
extern char optsfile[MAXPATHLEN + 16];

/* the checkpoint file filename */
extern char ckpfile[MAXPATHLEN + 16];

/* the diagnostic filename  */
extern char diagfile[MAXPATHLEN];

/* the wizard's password */
extern char *password;

extern int userid; /* the players login user id number */
extern char loginname[USERNAME_LENGTH + 1]; /* players login name */
extern char logname[LOGNAMESIZE + 1]; /* players name storage for scoring */

extern char nowelcome;         /* if nowelcome, don't display welcome message */
extern char nomove;            /* if nomove no count next iteration as move   */
extern char dropflag;          /* if 1 then don't lookforobject() next round */
extern char restorflag;        /* 1 means restore has been done */
extern char enhance_interface; /* 1 means use the enhanced command interface */

/*
 * Direction deltas
 */
extern char diroffx[];
extern char diroffy[];
extern int ReverseDir[];
extern char *dirname[];

/* =============================================================================
 * Exported functions
 */

/* =============================================================================
 * FUNCTION: newgame
 *
 * DESCRIPTION:
 * Function to perform initialisatin for a new game.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void newgame(void);

/* =============================================================================
 * FUNCTION: sethard
 *
 * DESCRIPTION:
 * Function to set the game difficulty level.
 *
 * PARAMETERS:
 *
 *   hard : The difficulty level to set.
 *            -1 => Default hardness
 *            any other value is the desired hardness
 *
 * RETURN VALUE:
 *
 *   None.
 */
void sethard(int hard);

/* =============================================================================
 * FUNCTION: read_options
 *
 * DESCRIPTION:
 * Function to read the ularn options file.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void read_options(void);

#endif
