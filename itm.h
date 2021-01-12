/* =============================================================================
 * PROGRAM:  ularn
 * FILENAME: itm.h
 *
 * DESCRIPTION:
 * This module contains the item number definitions and the names of all items
 * in the game.
 *
 * =============================================================================
 * EXPORTED VARIABLES
 *
 * objnamelist : The character to display on the map for each object
 * objtilelist : The gfx tile to display on the map for each object
 * objectname  : The text name for each object.
 *
 * =============================================================================
 * EXPORTED FUNCTIONS
 *
 * None
 *
 * =============================================================================
 */

#ifndef __ITM_H
#define __ITM_H

typedef enum
{
  OUNKNOWN,  /* Marker for unknown areas */
  ONOTHING,  /* Nothing                  */

  /* Dungeon features */
  OALTAR,
  OTHRONE,        /* throne */
  OTHRONE2,       /* throne that has summoned a gnome king */
  ODEADTHRONE,    /* throne with jeweles pried off */
  OPIT,
  OSTAIRSUP,
  OELEVATORUP,
  OFOUNTAIN,
  OSTATUE,
  OTELEPORTER,
  OMIRROR,
  OSTAIRSDOWN,
  OELEVATORDOWN,
  ODEADFOUNTAIN,
  OOPENDOOR,
  OCLOSEDDOOR,
  OWALL,

  /* gold piles */
  OGOLDPILE,     /* gold, amount = itemarg */
  ODGOLD,        /* gold, amount = itemarg * 10 */
  OKGOLD,        /* gold, amount = itemarg * 1000 */
  OMAXGOLD,      /* gold, amount = itemarg * 10000 */

  OLARNEYE, /* The Eye of Larn - enables you to see demons */

  /* armor */
  OPLATE,
  OCHAIN,
  OLEATHER,
  ORING,
  OSTUDLEATHER,
  OSPLINT,
  OPLATEARMOR,
  OSSPLATE,
  OSHIELD,
  OELVENCHAIN,      /* elven chain - strong and light, impervious to rust */

  /* weapons */
  OSWORDofSLASHING, /* impervious to rust, light, strong */
  OHAMMER,          /* Bessman's Flailing Hammer */
  OSWORD,
  O2SWORD,          /* 2 handed sword */
  OSPEAR,
  ODAGGER,
  OBATTLEAXE,
  OLONGSWORD,
  OFLAIL,
  OLANCE,
  OVORPAL,          /* 1/20 chance of beheading most monsters */
  OSLAYER,          /* magical sword - increases intelligence by 10,
                       halves damage caused by demons, demon prince
                       and lucifer - strong as lance of death against them*/

  /* rings */
  ORINGOFEXTRA,
  OREGENRING,
  OPROTRING,
  OENERGYRING,
  ODEXRING,
  OSTRRING,
  OCLEVERRING,
  ODAMRING,

  /* Magic items */
  OBELT,          /* belt of striking */
  OSCROLL,        /* a scroll */
  OPOTION,        /* a potion */
  OBOOK,          /* a book */
  OCHEST,         /* a treasure chest */
  OORB,           /* orb of enlightement - gives expanded awareness while held */
  OAMULET,        /* Amulet of invisibility */
  OORBOFDRAGON,   /* Orb or dragon slaying */
  OSPIRITSCARAB,  /* Scarab of spirit protection */
  OCUBEofUNDEAD,  /* Cube of undead control */
  ONOTHEFT,       /* Device of theft prevention */
  OBRASSLAMP,     /* brass lamp - genie pops up and offers spell */
  OHANDofFEAR,    /* hand of fear - scare monster spell lasts
                     twice as long if have this */
  OSPHTALISMAN,   /* talisman of the sphere */
  OWWAND,         /* wand of wonder - cant fall in trap doors/pits */
  OPSTAFF,        /* staff of power - cancels drain life attack */
  OLIFEPRESERVER, /* Amulet of life preservation  - stops n drainlevel attacks */ 

  /* gems */
  ODIAMOND,
  ORUBY,
  OEMERALD,
  OSAPPHIRE,

  /* Buildings/entrances */
  OSCHOOL,        /* College of larn                */
  ODNDSTORE,      /* DnD store                      */
  OBANK2,         /* 8th brange of the bank of larn */
  OBANK,          /* First nation bank of larn      */
  OENTRANCE,      /* dungeon entrance               */
  OVOLDOWN,       /* volcano entrance               */
  OVOLUP,         /* Volcanic tube up               */
  OHOME,          /* Player's home                  */
  OTRADEPOST,     /* Larn trading post              */
  OLRS,           /* Larn Revenue Service           */
  OPAD,           /* Dealer McDope's Pad            */

  /* Traps */
  OTRAPARROWIV,
  OTRAPARROW,
  OIVDARTRAP,
  ODARTRAP,
  OTRAPDOOR,
  OIVTRAPDOOR,
  OIVTELETRAP,

  OANNIHILATION,  /* sphere of annihilation */
  OCOOKIE,
  OURN,           /* golden urn - not implemented */

  /* Drugs */
  OSPEED,
  OACID,
  OHASH,
  OSHROOMS,
  OCOKE,
  OCOUNT
} ObjectIdType;


/*** How enchantment happened ***/
#define ENCH_SCROLL  0  /* Enchantment from reading a scroll */
#define ENCH_ALTAR   1  /* Enchantment from an altar         */

extern char objnamelist[OCOUNT];

extern int objtilelist[OCOUNT];

extern char *objectname[OCOUNT];

#endif
