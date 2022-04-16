/* =============================================================================
 * PROGRAM:  CODE LIBRARY
 * FILENAME: smart_menu.h
 *
 * DESCRIPTION:
 * This module provides a set of functions to make menu handling for
 * Intuition easier.
 * A much simpler set of data structures for simple text menu are used.
 *
 * Each menu on is specified in an array of SmartMenuItem.
 * The last element of the array should have all fields set to NULL to mark
 * the end of the menu.
 * Each SmartMenuItem specifies the Text, colour and command sequence to use
 * for the menu item. It also specifies the function to be called when the
 * menu item is selected.
 * Note: The same structure is used for menus and submenus.
 *
 * The menus on the menu bar are specified in an array of SmartMenu.
 * This array should also be terminated with an entry with all elements set to
 * NULL.
 *
 * The menu structure specified in the array of SmartMenu is set to a window
 * using the MakeMenuStructure function. This function alocates and sets up the
 * Intuition structures required for the menu and then assigns the menu to the
 * specified window.
 *
 * The window containing the menu should have the IDCMP_MENUPICK flag set so
 * it will receive menu events.
 * When a menu event is received for the window, call DoMenuSelection to
 * with the Code field of the IntuiMessage handle the event. This will call
 * the specified callback function of the event picked.
 *
 * NOTE: This module can currently only handle the menu for one window at a
 *       time.
 *
 * =============================================================================
 * COPYRIGHT:
 *
 * Copyright (c) 1995, 2004, Julian Olds
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   . Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *
 *   . Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * The name of the author may not be used to endorse or promote products
 * derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *
 * =============================================================================
 * EXPORTED VARIABLES
 *
 * Quit : This is set to true for the quit event.
 *
 * =============================================================================
 * EXPORTED FUNCTIONS
 *
 * MenuNil  : The Null action event handler for menu items that do nothing.
 * MenuQuit : The default Quit application menu handler. Sets the Quit flag.
 *            This should also be called when the application exits if it is not
 *            used to handle the Quit menu item.
 * MakeMenuStructure : Makes the Intuition menu structure and assigns the menu
 *                     into a window.
 * DoMenuSelection   : Handles the menu selection events.
 *
 * =============================================================================
 */

#ifndef __SMART_MENU_H
# define __SMART_MENU_H

# include <intuition/intuition.h>

extern int Quit;

struct SmartMenuItem {
  char *Text;
  char ComSeq;
  UBYTE FrontPen;
  UBYTE BackPen;
  void (*SelectFunction)(void);
  struct SmartMenuItem *SubItem;
};

struct SmartMenu {
  char *Text;
  struct SmartMenuItem *FirstItem;
};

/* =============================================================================
 * FUNCTION: MenuNil
 *
 * DESCRIPTION:
 * This is the null callback function for menu events. This is to be used
 * for any items used as separators inthe menu structure.
 * It can also be used as a stub function for any menu item whose behaviour
 * is not yet defined.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void MenuNil(void);

/* =============================================================================
 * FUNCTION: MenuQuit
 *
 * DESCRIPTION:
 * The default Quit application menu handler.
 * This function frees any allcoated memory for menu structures and sets the
 * Quit flag.
 * This should also be called when the application exits if it is not used to
 * handle the Quit menu item.
 *
 * PARAMETERS:
 *
 *   None.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void MenuQuit(void);

/* =============================================================================
 * FUNCTION: MakeMenuStructure
 *
 * DESCRIPTION:
 * This function build the Intuition Menu structures according to the
 * menu layout specified in WindowMenu and assigns the menu created to the
 * Window specified in MenuWindow.
 *
 * PARAMETERS:
 *
 *   MWindow    : The window to use the menu.
 *
 *   WindowMenu : The SmartMenu specification of the menu structure.
 *
 * RETURN VALUE:
 *
 *   int: TRUE if the menu was successfully created.
 */
int MakeMenuStructure(struct Window *MWindow, struct SmartMenu *WindowMenu);

/* =============================================================================
 * FUNCTION: DoMenuSelection
 *
 * DESCRIPTION:
 * This function handles the IDCMP_MENIPICK event, and calls the handler
 * function for the menu item picked.
 *
 * PARAMETERS:
 *
 *   code : The Code field of the IntuiMessage.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void DoMenuSelection(USHORT code);

#endif
