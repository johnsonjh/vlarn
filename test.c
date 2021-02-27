
#include <curses.h>
#include <stdio.h>

int UseColor;

static void setup_colour_pairs(void) {

  short i, j;

  for (i = 0; i < 8; i++) {
    for (j = 0; j < 8; j++)
      if ((i > 0) || (j > 0))
        init_pair(i * 8 + j, j, i);
  }

  init_pair(63, COLOR_BLACK, COLOR_BLACK);

  for (i = 0; i < 8; i++) {
    for (j = 0; j < 8; j++) {
      move(i, j);
      attrset(COLOR_PAIR(i * 8 + j));
      addch('*');
    }
  }
  refresh();

  getch();
} // end setup_colour_pairs()

/* =============================================================================
 * FUNCTION: init_app
 */
int init_app2(void) {
  int DefaultBg;

  //  termio_init();

  /* Initialise curses app */

  initscr();
  cbreak();
  noecho();

  nonl();
  intrflush(stdscr, FALSE);

  meta(stdscr, TRUE);
  start_color();
  setup_colour_pairs();

  keypad(stdscr, TRUE);

  /* Create windows */

  //  MapWindow = newwin(17, 67, 0, 0);
  // StatusWindow = newwin(2, 80, 17, 0);
  // EffectsWindow = newwin(17, 13, 0, 67);
  // MessageWindow = newwin(5, 80, 19, 0);

  UseColor = 1; // has_colors();

  /*if (UseColor)
     {
      start_color();

      DefaultBg = COLOR_WHITE;

      init_pair(1, COLOR_BLACK, DefaultBg);
      init_pair(2, COLOR_RED, DefaultBg);
      init_pair(3, COLOR_GREEN, DefaultBg);
      init_pair(4, COLOR_BLUE, DefaultBg);
      init_pair(5, COLOR_YELLOW, DefaultBg);
      init_pair(6, COLOR_CYAN, DefaultBg);


     }
   */
  return 1;
}

main() {
  init_app();
  init_app2();
  endwin();
}
