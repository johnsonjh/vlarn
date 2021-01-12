/* =============================================================================
 * PROGRAM:  ularn
 * FILENAME: diag.c
 *
 * DESCRIPTION:
 * Diagnostic dump module.
 *
 * =============================================================================
 * EXPORTED VARIABLES
 *
 * None
 *
 * =============================================================================
 * EXPORTED FUNCTIONS
 *
 * diag - Print diagnostic information
 *
 * =============================================================================
 */

#include "header.h"
#include "diag.h"
#include "ularn_game.h"
#include "itm.h"
#include "dungeon.h"
#include "monster.h"
#include "player.h"
#include "potion.h"
#include "scroll.h"
#include "spell.h"
#include "ularn_win.h"

/* =============================================================================
 * Local variables
 */

/*
 * Character attribute strings.
 * The strings must match the order of the AttributeType enumeration
 */
static char *cdef[] = {
  "STRENGTH", "INTELLIGENCE", "WISDOM", "CONSTITUTION", "DEXTERITY",
  "CHARISMA", "HPMAX", "HP", "GOLD", "EXPERIENCE",
  "LEVEL", "REGEN", "WCLASS", "AC", "BANKACCOUNT",
  "SPELLMAX", "SPELLS", "ENERGY", "ECOUNTER", "MOREDEFENSES",
  "WEAR", "PROTECTIONTIME", "WIELD", "AMULET", "REGENCOUNTER",
  "MOREDAM", "DEXCOUNT", "STRCOUNT", "BLINDCOUNT", "CAVELEVEL",
  "CONFUSE", "ALTPRO", "HERO", "CHARMCOUNT", "INVISIBILITY",
  "CANCELLATION", "HASTESELF", "EYEOFLARN", "AGGRAVATE ", "GLOBE",
  "TELEFLAG", "SLAYING", "NEGATESPIRIT", "SCAREMONST ", "AWARENESS",
  "HOLDMONST", "TIMESTOP", "HASTEMONST", "CUBEofUNDEAD", "GIANTSTR",
  "FIRERESISTANCE", "BESSMANN", "NOTHEFT", "HARDGAME", "",
  "", "", "", "", "VORPAL",
  "LANCEDEATH", "SPIRITPRO", "UNDEADPRO", "SHIELD ", "STEALTH",
  "ITCHING", "LAUGHING", "DRAINSTRENGTH", "CLUMSINESS", "INFEEBLEMENT",
  "HALFDAM", "SEEINVISIBLE", "FILLROOM", "", "SPHCAST",
  "WTW", "STREXTRA", "TMP", "LIFEPROT", "ORB",
  "ELVUP", "ELVDOWN", "HAND", "CUBEUNDEAD", "DRAGSLAY",
  "NEGATE", "URN", "LAMP", "TALISMAN", "WAND",
  "STAFF", "DEVICE", "SLASH", "ELVEN", "VORP",
  "SLAY", "PAD", "COKED", "", "" };

/*
 * Names for inventory items
 */
static char *ivendef[] = {
  "",
  "OALTAR", "OTHRONE", "OORB", "OPIT", "OSTAIRSUP",
  "OELEVATORUP", "OFOUNTAIN", "OSTATUE", "OTELEPORTER", "OSCHOOL",
  "OMIRROR", "ODNDSTORE", "OSTAIRSDOWN", "OELEVATORDOWN", "OBANK2",
  "OBANK", "ODEADFOUNTAIN", "OGOLDPILE", "OOPENDOOR", "OCLOSEDDOOR",
  "OWALL", "OTRAPARROW", "OTRAPARROWIV", "OLARNEYE", "OPLATE",
  "OCHAIN", "OLEATHER", "ORING", "OSTUDLEATHER", "OSPLINT",
  "OPLATEARMOR", "OSSPLATE", "OSHIELD", "OSWORDofSLASHING", "OHAMMER",
  "OSWORD", "O2SWORD", "OSPEAR", "ODAGGER", "OBATTLEAXE",
  "OLONGSWORD", "OFLAIL", "OLANCE", "ORINGOFEXTRA", "OREGENRING",
  "OPROTRING", "OENERGYRING", "ODEXRING", "OSTRRING", "OCLEVERRING",
  "ODAMRING", "OBELT", "OSCROLL", "OPOTION", "OBOOK",
  "OCHEST", "OAMULET", "OORBOFDRAGON", "OSPIRITSCARAB", "OCUBEofUNDEAD",
  "ONOTHEFT", "ODIAMOND", "ORUBY", "OEMERALD", "OSAPPHIRE",
  "OENTRANCE", "OVOLDOWN", "OVOLUP", "OHOME", "OMAXGOLD",
  "OKGOLD", "ODGOLD", "OIVDARTRAP", "ODARTRAP", "OTRAPDOOR",
  "OIVTRAPDOOR", "OTRADEPOST", "OIVTELETRAP", "ODEADTHRONE", "OANNIHILATION",
  "OTHRONE2", "OLRS", "OCOOKIE", "OURN", "OBRASSLAMP",
  "OHANDofFEAR", "OSPHTALISMAN", "OWWAND", "OPSTAFF", "OVORPAL",
  "OSLAYER", "OELVENCHAIN", "OSPEED", "OACID", "OHASH",
  "OSHROOMS", "OCOKE", "OPAD", "" };

/*
 * Pointer to the debug output file
 */
static FILE *dfile;


/* =============================================================================
 * Local functions
 */

/* =============================================================================
 * FUNCTION: drawdiagscreen
 *
 * DESCRIPTION:
 * Draw the ASCII map to the diag file.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void diagdrawscreen(void)
{
  int i, j;
  MonsterIdType Monst;

  /* east west walls of this line */
  for (i = 0; i < MAXY; i++)
  {
    for (j = 0; j < MAXX; j++)
    {
      Monst = mitem[j][i].mon;
      if (Monst != MONST_NONE)
      {
        fprintf(dfile, "%c", monstnamelist[Monst]);
      }
      else
      {
        fprintf(dfile, "%c", objnamelist[(int) item[j][i]]);
      }
    }
    fprintf(dfile, "\n");
  }
}

/* =============================================================================
 * Exported functions
 */


/* =============================================================================
 * FUNCTION: diag
 */
void diag(void)
{
  int i, j;
  MonsterIdType Monst;

  if ((dfile = fopen(diagfile, "w")) == (FILE *)NULL) return;

  Print("\nDiagnosing . . .\n");

  fprintf(dfile, "\n-------- Beginning of DIAG diagnostics ---------\n\n");

  fprintf(dfile, "Hit points: %2ld(%2ld)\n", c[HP], c[HPMAX]);

  fprintf(dfile, "gold: %ld  Experience: %ld  Character level: %ld  Level in caverns: %d\n",
    (long) c[GOLD],
    (long) c[EXPERIENCE],
    (long) c[LEVEL],
    level);

  fprintf(dfile, "\nFor the c[] array:\n");
  fflush(dfile);

  for( j=0; j<100; j++)
  {
    fprintf(dfile, "c[%d]\t%-20s\t= %ld\n", j, cdef[j], c[j]);
  }
  fprintf(dfile, "\n\n");
  fflush(dfile);

  fprintf(dfile, "Inventory\n");
  for (j=0; j<IVENSIZE; j++)
  {
    fprintf (dfile, "iven[%d] %-12s = %d",
    j, ivendef[(int) iven[j]], iven[j] );
    fprintf (dfile, "\t%s", objectname[(int) iven[j]] );
    fprintf (dfile, "\t+ %d\n", ivenarg[j] );
  }

  fprintf(dfile, "\nHere are the maps:\n\n");
 
  i = level;
  for (j = 0; j < NLEVELS; j++)
  {
    newcavelevel(j);
    fprintf(dfile, "\n-------------------------------------------------------------------\n");
    fprintf(dfile, "Map %s    level %d\n", levelname[level], level);
    fprintf(dfile, "-------------------------------------------------------------------\n");
    diagdrawscreen();
    fflush(dfile);
  }
  level = (char) i;
  getlevel();

  fprintf(dfile, "\n\nNow for the monster data:\n\n");
  fprintf(dfile, "\nTotal types of monsters: %d\n\n", MAXMONST + 8);
  fprintf(dfile, "   Monster Name      LEV  AC   DAM  ATT  GOLD   HP     EXP\n");
  fprintf(dfile, "-----------------------------------------------------------------\n");
  fflush(dfile);

  for (Monst = MONST_NONE ; Monst < MONST_COUNT ; Monst++)
  {
    fprintf(dfile, "%19s  %2d  %3d ",
      monster[Monst].name,
      monster[Monst].level,
      monster[Monst].armorclass);
    fprintf(dfile, " %3d  %3d ",
      monster[Monst].damage,
      monster[Monst].attack);
    fprintf(dfile, "%6d  %3d   %6ld\n",
      monster[Monst].gold,
      monster[Monst].hitpoints,
     (long) monster[Monst].experience);

    fflush(dfile);
  }

  fprintf(dfile, "\nAvailable potions:\n\n");
  for (i = 0; i < MAXPOTION; i++)
  {
    fprintf(dfile, "%20s\n", &potionname[i][1]);
  }
  fflush(dfile);

  fprintf(dfile, "\nAvailable scrolls:\n\n");
  for (i = 0; i < MAXSCROLL; i++)
  {
    fprintf(dfile, "%20s\n", &scrollname[i][1]);
  }
  fflush(dfile);

  fprintf(dfile, "\nSpell list:\n\n");
  fprintf(dfile, "spell#  name           description\n");
  fprintf(dfile, "-------------------------------------------------\n\n");

  for (j = 0 ; j < SPELL_COUNT ; j++)
  {
    fprintf(dfile, "%-10s", spelcode[j]);
    fprintf(dfile, " %21s\n", spelname[j]);
    fprintf(dfile, "%s\n", speldescript[j]);
  }
  fflush(dfile);

  fprintf(dfile, "\nObject list\n\n");
  fprintf(dfile, "\nj \tObject \tName\n");
  fprintf(dfile, "---------------------------------\n");
  for (j = 0; j < OCOUNT; j++)
  {
    fprintf(dfile, "%d \t%c \t%s\n",
      j,
      objnamelist[j],
      objectname[j] );
  }
  fflush(dfile);

  fprintf(dfile,"\n-------- End of DIAG diagnostics ---------\n");
  fflush(dfile);
  fclose(dfile);

  Print("\nDone Diagnosing.\n");
}


