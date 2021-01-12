/* =============================================================================
 * PROGRAM:  ularn
 * FILENAME: fortune.c
 *
 * DESCRIPTION:
 * This module contains functions for displaying fortunes inside fortune
 * cookies.
 *
 * =============================================================================
 * EXPORTED VARIABLES
 *
 * None
 *
 * =============================================================================
 * EXPORTED FUNCTIONS
 *
 * fortune       - Returns a random fortune string.
 * free_fortunes - Frees the fortunes data.
 *
 * =============================================================================
 */

#include <stdio.h>

#include "header.h"
#include "fortune.h"

/* =============================================================================
 * Local variables
 */

#define MAX_FORTUNE_LEN 80

/*
 * Data structure for a linked list of fortune strings read from the
 * fortune file
 */
struct FortuneType
{
  char Line[MAX_FORTUNE_LEN];
  struct FortuneType *Next;
};

static struct FortuneType *fortunes = NULL;
static int fortune_read = 0;   /* true if we have loaded the fortune info */
static int nlines = 0;         /* # lines in fortune database */

/* =============================================================================
 * Exported functions
 */

/* =============================================================================
 * FUNCTION: fortune
 */

char *fortune(char *file)
{
  FILE *fp;
  char *Line;
  char Buffer[80];
  struct FortuneType *NewFortune;
  int Len;
  int Pos;

  if (fortune_read == 0)
  {
    /* open the file */
    fp = fopen(file, "r");
    if (fp == NULL)
    {
      /* can't find file */
      return(0);
    }

    /* Read the fortune lines from the file */
    while (!feof(fp))
    {
      Line = fgets(Buffer, MAX_FORTUNE_LEN, fp);
      if (Line != NULL)
      {
        /* trim white space and CR/LF fromt he end of the line */
        Len = strlen(Buffer);
        Len--;
        while ((Len > 0) && isspace((int) Buffer[Len]))
        {
          Buffer[Len] = 0;
          Len--;
        }

        /* Allocate a new fortune */
        NewFortune = (struct FortuneType *) malloc(sizeof(struct FortuneType));

        if (NewFortune == NULL)
        {
          return NULL;
        }

        strcpy(NewFortune->Line, Buffer);
        NewFortune->Next = fortunes;
        fortunes = NewFortune;
        nlines++;

      }
    }

    fortune_read = 1;

    fclose(fp);
  }

  if (fortune_read)
  {
    if (nlines > 0)
    {
      Pos = rund(nlines);
      NewFortune = fortunes;

      while (Pos > 0)
      {
        NewFortune = NewFortune->Next;
        Pos--;
      }

      return (NewFortune->Line);
    }
    else
    {
      return (NULL);
    }
  }
  else
  {
    return (NULL);
  }
}

/* =============================================================================
 * FUNCTION: free_fortunes
 */
void free_fortunes(void)
{
  struct FortuneType *Fortune;

  while (fortunes != NULL)
  {
    Fortune = fortunes->Next;
    free(fortunes);
    fortunes = Fortune;
  }
}
