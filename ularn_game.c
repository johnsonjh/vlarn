/* =============================================================================
 * PROGRAM:  vlarn
 * FILENAME: vlarn_game.h
 *
 * DESCRIPTION:
 * Game data used by VLarn.
 * This Contains:
 *   . The names of data files used by vlarn
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
 * libdir         : VLarn library path
 * savedir        : Directroy for save games
 * savefilename   : Filename for saving the game
 * scorefile      : Filename for the scores
 * helpfile       : Filename for vlarn help
 * larnlevels     : Filename for pregenerated levels
 * fortfile       : Filename for fortunes
 * optsfile       : VLarn options file
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
 * read_options : Function to read the vlarn options file
 *
 * =============================================================================
 */

#include <time.h>

#include "header.h"
#include "monster.h"
#include "player.h"
#include "ularn_game.h"

/* =============================================================================
 * Exported variables
 */

/*
 * Game options
 */
char do_fork = 0; /* 1=fork on save, 0=save from main process. NOT SUPPORTED */
char boldon = 1;  /* 1=bold objects,  0=inverse objects */
char mail = 1;    /* 1=mail letters after win game */
char ckpflag = 1; /* 1 if want checkpointing of game, 0 otherwise */
char nobeep = 0;  /* true if program is not to beep*/

char libdir[MAXPATHLEN] = LIBDIR;

char savedir[MAXPATHLEN];

/* the game save filename   */
char savefilename[MAXPATHLEN];

/* the temporary save filename   */
char tempfilename[MAXPATHLEN];

/* the score file       */
char scorefile[MAXPATHLEN];

/* the help text file */
char helpfile[MAXPATHLEN];

/* the maze data file */
char larnlevels[MAXPATHLEN];

/* the fortune data file */
char fortfile[MAXPATHLEN];

/* the options file filename */
char optsfile[MAXPATHLEN] = "vlarn.opt";

/* the checkpoint file filename */
char ckpfile[MAXPATHLEN] = "vlarn.ckp";

/* the diagnostic filename  */
char diagfile[] = "diagfile.txt";

/* the wizard's password */
char *password = "rodney";

int userid;                          /* the players login user id number */
char loginname[USERNAME_LENGTH + 1]; /* players login name */
char logname[LOGNAMESIZE + 1];       /* players name storage for scoring */

char nowelcome = 0;         /* if nowelcome, don't display welcome message */
char nomove = 0;            /* if nomove no count next iteration as move   */
char dropflag = 0;          /* if 1 then don't lookforobject() next round */
char restorflag = 0;        /* 1 means restore has been done */
char enhance_interface = 0; /* 1 means use the enhanced command interface */

char diroffx[] = {0, 0, 1, 0, -1, 1, -1, 1, -1};
char diroffy[] = {0, 1, 0, -1, 0, -1, -1, 1, 1};
int ReverseDir[] = {0, 3, 4, 1, 2, 8, 7, 6, 5};

char *dirname[] = {"None",      "South",     "East",      "North",    "West",
                   "Northeast", "Northwest", "Southeast", "Southwest"};

/* =============================================================================
 * Local variables
 */

#define LINE_LEN 256

typedef enum {
  OPTION_NULL,
  OPTION_NAME,
  OPTION_CLASS,
  OPTION_GENDER,
  OPTION_NAP,
  OPTION_NONAP,
  OPTION_WELCOME,
  OPTION_NOWELCOME,
  OPTION_ENHANCE_INT,
  OPTION_NOENHANCE_INT,
  OPTION_BEEP,
  OPTION_NOBEEP,
  OPTION_COUNT
} OptionType;

static char *OptionString[OPTION_COUNT] = {"",
                                           "name",
                                           "class",
                                           "gender",
                                           "nap",
                                           "nonap",
                                           "welcome",
                                           "nowelcome",
                                           "enhanced_interface",
                                           "noenhanced_interface",
                                           "beep",
                                           "nobeep"};

/* =============================================================================
 * Exported functions
 */

/* =============================================================================
 * FUNCTION: newgame
 */
void newgame(void) {
  time(&initialtime);
  srand((unsigned)initialtime);
}

/* =============================================================================
 * FUNCTION: sethard
 */
void sethard(int hard) {
  int j, k, i;

  if (restorflag == 0) {
    /* don't set c[HARDGAME] if restoring game */
    if (hashewon() == 0) {
      if (hard >= 0)
        c[HARDGAME] = hard;
    } else if (hard > c[HARDGAME] || wizard)
      c[HARDGAME] = hard;
  }

  k = c[HARDGAME];
  if (k != 0) {
    for (j = 0; j <= MAXMONST + 8; j++) {
      i = ((6 + k) * monster[j].hitpoints + 1) / 6;
      monster[j].hitpoints = (short)((i > 32767) ? 32767 : i);

      i = ((6 + k) * monster[j].damage + 1) / 5;
      monster[j].damage = (char)((i > 127) ? 127 : i);

      i = (10 * monster[j].gold) / (10 + k);
      monster[j].gold = (short)((i > 32767) ? 32767 : i);

      i = monster[j].armorclass - k;
      monster[j].armorclass = (char)((i < -127) ? -127 : i);

      i = (int)((7 * monster[j].experience) / (7 + k) + 1);
      monster[j].experience = (i <= 0) ? 1 : i;
    }
  }
}

/* =============================================================================
 * FUNCTION: read_options
 */
void read_options(void) {
  char Line[LINE_LEN + 1];
  char *Str;
  char *tok;
  FILE *fp;
  OptionType OptionId;
  int Found;

  fp = fopen(optsfile, "r");

  if (fp == NULL)
    /*
     * Couldn't open the options file.
     */
    return;

  while (!feof(fp)) {
    Str = fgets(Line, LINE_LEN, fp);

    if (Str == NULL) {
      /* End of file - do nothng */
    } else if (Line[0] == '#') {
      /* Comment line - do nothing */

    } else {
      tok = strtok(Line, " \t\n=");
      if (tok == NULL) {
        /* A blank line */
      } else if (strcmp(tok, "OPTION") == 0) {
        /* Read all options specified on this line. */
        do {
          /* get the option string */
          tok = strtok(NULL, ",:\n");

          /* identify the option */
          if (tok == NULL)
            OptionId = OPTION_NULL;
          else {
            OptionId = OPTION_NAME;
            Found = 0;
            while ((OptionId < OPTION_COUNT) && (!Found)) {
              if (strcmp(OptionString[OptionId], tok) == 0)
                Found = 1;
              else
                OptionId++;
            }
          }

          switch (OptionId) {
          case OPTION_NULL:
            break;

          case OPTION_NAME:
            tok = strtok(NULL, ":,\n");
            strncpy(logname, tok, LOGNAMESIZE);
            break;

          case OPTION_CLASS:
            tok = strtok(NULL, ":,\n");
            char_picked = identify_class(tok);
            break;

          case OPTION_GENDER:
            tok = strtok(NULL, ":,\n");
            if (strcmp(tok, "male") == 0)
              sex = 1;
            else if (strcmp(tok, "female") == 0)
              sex = 0;
            else
              Printf("\nUnknown gender '%s'", tok);
            break;

          case OPTION_NAP:
            nonap = 0;
            break;

          case OPTION_NONAP:
            nonap = 1;
            break;

          case OPTION_WELCOME:
            nowelcome = 0;
            break;

          case OPTION_NOWELCOME:
            nowelcome = 1;
            break;

          case OPTION_ENHANCE_INT:
            enhance_interface = 1;
            break;

          case OPTION_NOENHANCE_INT:
            enhance_interface = 0;
            break;

          case OPTION_BEEP:
            nobeep = 0;
            break;

          case OPTION_NOBEEP:
            nobeep = 1;
            break;

          default:
            Printf("\nUnrecognised option '%s'", tok);
            break;
          }
        } while ((OptionId != OPTION_NULL) && (OptionId < OPTION_COUNT));
      } else if (strcmp(tok, "LIBDIR") == 0) {
        tok = strtok(NULL, ":\n");
        strcpy(libdir, tok);
      } else if (strcmp(tok, "SAVEDIR") == 0) {
        tok = strtok(NULL, ":\n");
        strcpy(savedir, tok);
      } else
        Printf("\nUnrecognised option '%s'", tok);
    }
  }

  fclose(fp);
}
