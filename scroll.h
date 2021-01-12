/* =============================================================================
 * PROGRAM:  ularn
 * FILENAME: scroll.h
 *
 * DESCRIPTION:
 * This module handles the processing for scrolls and books.
 *
 * =============================================================================
 * EXPORTED VARIABLES
 *
 * scrollname : The name of each scroll.
 *
 * =============================================================================
 * EXPORTED FUNCTIONS
 *
 * newscroll   : Function to create a new scroll # with the correct probability
 * read_scroll : Function to process reading a scroll.
 * readbook    : Function to process reading a book.
 *
 * =============================================================================
 */

#ifndef __SCROLL_H
#define __SCROLL_H

#define MAXSCROLL 28    /* maximum number of scrolls that are possible  */

/*** Scrolls ***/
#define SENCHANTARM   0
#define SENCHANTWEAP  1
#define SENLIGHTEN    2
#define SBLANK        3
#define SCREATEMONST  4
#define SCREATEITEM   5
#define SAGGMONST     6
#define STIMEWARP     7
#define STELEPORT     8
#define SAWARENESS    9
#define SHASTEMONST  10
#define SMONSTHEAL   11
#define SSPIRITPROT  12
#define SUNDEADPROT  13
#define SSTEALTH     14
#define SMAGICMAP    15
#define SHOLDMONST   16
#define SGEMPERFECT  17
#define SSPELLEXT    18
#define SIDENTIFY    19
#define SREMCURSE    20
#define SANNIHILATE  21
#define SPULVERIZE   22
#define SLIFEPROT    23
#define S_MAX        23  /* Greatest defined scroll number */

/*
 * Names of all scrolls
 */
extern char *scrollname[MAXSCROLL];

/* =============================================================================
 * FUNCTION: newscroll
 *
 * DESCRIPTION:
 * Function to create scroll #s with the correct probability of occurence.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   The scroll number as defined above.
 */
int newscroll(void);

/* =============================================================================
 * FUNCTION: read_scroll
 *
 * DESCRIPTION:
 * Function to perform the processing of the effect of reading a scroll.
 *
 * PARAMETERS:
 *
 *   typ : The type of scroll to read.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void read_scroll(int typ);

/* =============================================================================
 * FUNCTION: readbook
 *
 * DESCRIPTION:
 * Function to read a book.
 *
 * PARAMETERS:
 *
 *   arg : The dungeon level of the book (stored in itemarg for the book).
 *
 * RETURN VALUE:
 *
 *   None.
 */
void readbook(int arg);

#endif
