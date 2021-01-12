/* =============================================================================
 * PROGRAM:  ularn
 * FILENAME: help.c
 *
 * DESCRIPTION:
 * This module contains functions to display the help file and the welcome
 * screen.
 *
 * format of the Ularn help file:
 *
 * The 1st character of file is the # of pages of help available (ascii digit).
 * The first page (23 lines) of the file is for the introductory message
 * (not counted in above).
 * The pages of help text are 23 lines per page.
 * The help file allows escape sequences for specifying formatting of the
 * text.
 * The currently supported sequences are:
 *   ^[[7m - Set text format to standout  (red)
 *   ^[[8m - Set text format to standout2 (green)
 *   ^[[9m - Set text format to standout3 (blue)
 *   ^[[m  - Set text format to normal.
 *
 * =============================================================================
 * EXPORTED VARIABLES
 *
 * None.
 *
 * =============================================================================
 * EXPORTED FUNCTIONS
 *
 * help    - Display the help file.
 * welcome - Display the welcome message
 *
 * =============================================================================
 */

#include <stdio.h>

#include "header.h"
#include "ularn_game.h"
#include "ularn_win.h"

#include "player.h"

/* =============================================================================
 * Local variables
 */

/*
 * Help file escape sequence states.
 */
typedef enum
{
  HELP_NORMAL,
  HELP_ESC,
  HELP_COUNT
} HelpStateType;

/*
 * A pointer to the help file handle.
 */
static FILE *help_fp;

/* =============================================================================
 * Local functions
 */

/* =============================================================================
 * FUNCTION: retcont
 *
 * DESCRIPTION:
 * Function to say press return to continue.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void retcont (void)
{
  MoveCursor(1, 24);
  Print("Press ");
  Standout("return");
  Print(" to continue: ");

  get_prompt_input("", "\015\033", 0);
}

/* =============================================================================
 * FUNCTION: openhelp
 *
 * DESCRIPTION:
 * Function to open the help file and return the number of pages in the help
 * file.
 *
 * PARAMETERS:
 *
 *   None
 *
 * RETURN VALUE:
 *
 *   The number of pages in the help file as specified by the first character.
 */
static int openhelp (void)
{
  help_fp = fopen(helpfile, "r");

  if (help_fp == NULL)
  {
    Printf("Can't open help file \"%s\" ", helpfile);

    nap(4000);
    return -1;
  }

  return (fgetc(help_fp) - '0');
}

/* =============================================================================
 * FUNCTION: show_help_page
 *
 * DESCRIPTION:
 * This function prints the next page of help in the help file.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void show_help_page(void)
{
  HelpStateType state;
  int line;
  int line_len;
  int line_pos;
  char tmbuf[128];

  ClearText();

  for (line = 0; line < 23 ; line++)
  {
    fgets(tmbuf, 128, help_fp);

    line_len = strlen(tmbuf);

    state = HELP_NORMAL;

    line_pos = 0;
    while (line_pos < line_len)
    {
      switch (state)
      {
        case HELP_NORMAL:
          if ((tmbuf[line_pos] == '^') &&
              ((line_pos + 3) < line_len))
          {
            if ((tmbuf[line_pos + 1] == '[') &&
                (tmbuf[line_pos + 2] == '['))
            {
              state = HELP_ESC;
              line_pos += 3;
            }
          }

          if (state == HELP_NORMAL)
          {
            Printc(tmbuf[line_pos]);
            line_pos++;
          }
          break;

        case HELP_ESC:
          if (tmbuf[line_pos] == '7')
          {
            SetFormat(FORMAT_STANDOUT);
            line_pos++;
          }
          else if (tmbuf[line_pos] == '8')
          {
            SetFormat(FORMAT_STANDOUT2);
            line_pos++;
          }
          else if (tmbuf[line_pos] == '9')
          {
            SetFormat(FORMAT_STANDOUT3);
            line_pos++;
          }
          else if (tmbuf[line_pos] == 'm')
          {
            SetFormat(FORMAT_NORMAL);
          }

          line_pos++;
          state = HELP_NORMAL;
          break;

        default:
          break;
      }

    }
  }

}

/* =============================================================================
 * Exported functions
 */

/* =============================================================================
 * FUNCTION: help
 */
void help (void)
{
  int num_pages;
  int page;
  int i;
  char tmbuf[128];

  set_display(DISPLAY_TEXT);

  /* open the help file and get # pages */
  num_pages = openhelp();

  if (num_pages < 0)
  {
    set_display(DISPLAY_MAP);
    return;
  }

  /* skip over intro message */
  for (i = 0; i < 23 ; i++)
  {
    fgets(tmbuf, 128, help_fp);
  }

  for (page = num_pages ; page > 0 ; page--)
  {
    show_help_page();

    /* intercept ESC's */
    if (page > 1)
    {
      Print("    ---- Press ");
      Standout("return");
      Print(" to exit, ");
      Standout("space");
      Print(" for more help ---- ");

      i = get_prompt_input("", " \015\033", 0);

      if ((i == '\015') || (i == ESC))
      {
        fclose(help_fp);
        set_display(DISPLAY_MAP);

        return;
      }
    }
  }

  fclose(help_fp);
  retcont();

  set_display(DISPLAY_MAP);

}

/* =============================================================================
 * FUNCTION: welcome
 */
void welcome (void)
{
  int num_pages;

  set_display(DISPLAY_TEXT);

  /* open the help file */
  num_pages = openhelp();
  if (num_pages < 0) return;

  show_help_page();

  fclose(help_fp);

  /* press return to continue */
  retcont();

  set_display(DISPLAY_MAP);


}


