/* =============================================================================
 * PROGRAM:  CODE LIBRARY
 * FILENAME: smart_menu.c
 *
 * DESCRIPTION:
 * This module provides a set of functions to make menu handling for
 * Intuition easier.
 * A much simpler set of data structures for simple text menu are used.
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


#include <stdio.h>
#include <string.h>
#include <exec/memory.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/exec.h>
#include <proto/dos.h>

#include "smart_menu.h"

/* =============================================================================
 * Exported variables
 */

int Quit;

/* =============================================================================
 * Local variables
 */

#define MenuLeftIndent 4

static struct Menu *MenuPtr;
static struct SmartMenu *SmartMenuPtr;
static struct Window *MenuWindow;
static struct RastPort *MenuRPort;

static char *SMARTMENU_MEMORY_ERROR = "SmartMenu: AllocMem failed.";

/* =============================================================================
 * Local functions
 */

/* =============================================================================
 * FUNCTION: FreeItems
 *
 * DESCRIPTION:
 * This function frees the items created of an Intuition Menu.
 *
 * PARAMETERS:
 *
 *   WindowMenu : The menuto be freed.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void
FreeItems(
	struct MenuItem *WindowMenu)
{
	struct MenuItem *TmpItemPtr1;
	struct MenuItem *TmpItemPtr2;

	TmpItemPtr1 = WindowMenu;

	while (TmpItemPtr1 != NULL)
	{
      // Free and submenu items for this menu item
		FreeItems(TmpItemPtr1->SubItem);

      // Free the IntuiText and MenuItem for this menu item.
		TmpItemPtr2 = TmpItemPtr1;
		TmpItemPtr1 = TmpItemPtr1->NextItem;
		FreeMem(TmpItemPtr2->ItemFill, sizeof(struct IntuiText));
		FreeMem(TmpItemPtr2, sizeof(struct MenuItem));
	}
}

/* =============================================================================
 * FUNCTION: MenuError
 *
 * DESCRIPTION:
 * This function prints the errot message if there an error is detected by
 * the smart menu module.
 * It then quits the menu handler, freeing all memory used for the menu.
 *
 * PARAMETERS:
 *
 *   ErrorText : The error message to display.
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void
MenuError(
	char *ErrorText)
{
	fprintf(stderr, "%s\n", ErrorText);

	MenuQuit();
}

/* =============================================================================
 * FUNCTION: AddSubItems
 *
 * DESCRIPTION:
 * This function ads subitems to an item in a menu according to the subitems
 * specified in WindowItem.
 *
 * PARAMETERS:
 *
 *   WindowMenu : The menu item to contain the submenu.
 *
 *   WindowItem : The array of SmartMenuItem specifying the submenu.
 *
 *   TextHeight : THe height of text in the menu.
 *
 * RETURN VALUE:
 *
 */
static int
AddSubItems(
	struct MenuItem *WindowMenu,
	struct SmartMenuItem *WindowItem,
	int TextHeight)
{
	int Length;
	int TmpWidth;
	int ItemIndex;
	int ItemLeft;
	int ItemTop;
	int ItemWidth;
	int ItemHeight;
	struct MenuItem *TmpItemPtr1;
	struct MenuItem *TmpItemPtr2;
	struct IntuiText *ItemText;

	ItemLeft = WindowMenu->LeftEdge + WindowMenu->Width;
	ItemTop = WindowMenu->Height;
	ItemWidth = 0;
	ItemHeight = TextHeight;

   // Find the width of the submenu as the maximum  of the sub menu item
   // widths.
	ItemIndex = 0;

	while (WindowItem[ItemIndex].Text != NULL)
	{
		Length = strlen(WindowItem[ItemIndex].Text);
		TmpWidth = TextLength(MenuRPort, WindowItem[ItemIndex].Text, Length);

		if (WindowItem[ItemIndex].ComSeq != 0)
		{
			TmpWidth += COMMWIDTH;
			TmpWidth += TextLength(MenuRPort, "M", 1);
		}

		if (TmpWidth > ItemWidth)
		{
			ItemWidth = TmpWidth;
		}

		ItemIndex++;
	}

   // Allocate the submenu items and add the to the submenu for the menu item.
	TmpItemPtr2 = NULL;
	ItemIndex = 0;

	while (WindowItem[ItemIndex].Text != NULL)
	{
		if ((TmpItemPtr1 = (struct MenuItem *)
			 AllocMem(sizeof(struct MenuItem), MEMF_CLEAR)) == NULL)
		{
			MenuError(SMARTMENU_MEMORY_ERROR);
			return FALSE;
		}

        if ((ItemText = (struct IntuiText *)
			 AllocMem(sizeof(struct IntuiText), MEMF_CLEAR)) == NULL)
		{
			MenuError(SMARTMENU_MEMORY_ERROR);
            return FALSE;
		}

		ItemText->FrontPen = WindowItem->FrontPen;
		ItemText->BackPen = WindowItem->BackPen;
		ItemText->DrawMode = JAM1;
		ItemText->LeftEdge = 0;
		ItemText->TopEdge = 0;
		ItemText->ITextFont = 0;
		ItemText->IText = WindowItem[ItemIndex].Text;
		ItemText->NextText = 0;

		TmpItemPtr1->NextItem = NULL;
		TmpItemPtr1->LeftEdge = ItemLeft;
		TmpItemPtr1->TopEdge = ItemTop;
		TmpItemPtr1->Width = ItemWidth;
		TmpItemPtr1->Height = TextHeight;
		if (WindowItem[ItemIndex].ComSeq != 0)
		{
			TmpItemPtr1->Flags = ITEMTEXT | COMMSEQ | ITEMENABLED | HIGHCOMP;
		}
		else
		{
			TmpItemPtr1->Flags = ITEMTEXT | ITEMENABLED | HIGHCOMP;
		}

		TmpItemPtr1->MutualExclude = 0;
		TmpItemPtr1->ItemFill = ItemText;
		TmpItemPtr1->SelectFill = NULL;
		TmpItemPtr1->Command = WindowItem[ItemIndex].ComSeq;
		TmpItemPtr1->SubItem = NULL;

		if (WindowMenu->SubItem == NULL)
		{
			WindowMenu->SubItem = TmpItemPtr1;
		}
		else
		{
			TmpItemPtr2->NextItem = TmpItemPtr1;
		}
  		TmpItemPtr2 = TmpItemPtr1;

		ItemTop += TextHeight;
		ItemIndex++;
	}

	return TRUE;
}

/* =============================================================================
 * FUNCTION: AddItems
 *
 * DESCRIPTION:
 * This function adds items to an intuition menu  as specified in the
 * SmartMenu structure for the menu.
 *
 * PARAMETERS:
 *
 *   WindowMenu : A pointer to the menu to have items added.
 *
 *   WindowItems: The array of SmartMenuItem to add.
 *
 *   TextHeight : The height of text for the menu.
 *
 * RETURN VALUE:
 *
 *   int : TRUE if the menu items were successfully added.
 */
static int
AddItems(
	struct Menu *WindowMenu,
	struct SmartMenuItem *WindowItem,
	int TextHeight)
{
	int Length;
	int TmpWidth;
	int ItemIndex;
	int ItemLeft;
	int ItemTop;
	int ItemWidth;
	int ItemHeight;
	struct MenuItem *TmpItemPtr1;
	struct MenuItem *TmpItemPtr2;
	struct IntuiText *ItemText;

	ItemLeft = 0;
	ItemTop = 0;
	ItemWidth = 0;
	ItemHeight = TextHeight;

   // Calculate the width of the menu as the maximum with of all of the items.
	ItemIndex = 0;

	while (WindowItem[ItemIndex].Text != NULL)
	{
		Length = strlen(WindowItem[ItemIndex].Text);
		TmpWidth = TextLength(MenuRPort, WindowItem[ItemIndex].Text, Length);

		if (WindowItem[ItemIndex].ComSeq != 0)
		{
			TmpWidth += COMMWIDTH;
			TmpWidth += TextLength(MenuRPort, "M", 1);
		}

		if (TmpWidth > ItemWidth)
		{
			ItemWidth = TmpWidth;
		}

		ItemIndex++;
	}

   // Allcoate the menu items ond fill in the data structures in accordance
   // with the SmartMenuItem specifications.
	TmpItemPtr2 = NULL;
	ItemIndex = 0;

	while (WindowItem[ItemIndex].Text != NULL)
	{
		if ((TmpItemPtr1 = (struct MenuItem *)
			 AllocMem(sizeof(struct MenuItem), MEMF_CLEAR)) == NULL)
		{
			MenuError(SMARTMENU_MEMORY_ERROR);
     		return FALSE;
		}
        if ((ItemText = (struct IntuiText *)
			 AllocMem(sizeof(struct IntuiText), MEMF_CLEAR)) == NULL)
		{
			MenuError(SMARTMENU_MEMORY_ERROR);
         	return FALSE;
		}

		ItemText->FrontPen = WindowItem->FrontPen;
		ItemText->BackPen = WindowItem->BackPen;
		ItemText->DrawMode = JAM1;
		ItemText->LeftEdge = 0;
		ItemText->TopEdge = 0;
		ItemText->ITextFont = 0;
		ItemText->IText = WindowItem[ItemIndex].Text;
		ItemText->NextText = 0;

		TmpItemPtr1->NextItem = NULL;
		TmpItemPtr1->LeftEdge = ItemLeft;
		TmpItemPtr1->TopEdge = ItemTop;
		TmpItemPtr1->Width = ItemWidth;
		TmpItemPtr1->Height = TextHeight;
		if (WindowItem[ItemIndex].ComSeq != 0)
		{
			TmpItemPtr1->Flags = ITEMTEXT | COMMSEQ | ITEMENABLED | HIGHCOMP;
		}
		else
		{
			TmpItemPtr1->Flags = ITEMTEXT | ITEMENABLED | HIGHCOMP;
		}

		TmpItemPtr1->MutualExclude = 0;
		TmpItemPtr1->ItemFill = ItemText;
		TmpItemPtr1->SelectFill = NULL;
		TmpItemPtr1->Command = WindowItem[ItemIndex].ComSeq;
		TmpItemPtr1->SubItem = NULL;

		if (WindowMenu->FirstItem == NULL)
		{
			WindowMenu->FirstItem = TmpItemPtr1;
		}
		else
		{
			TmpItemPtr2->NextItem = TmpItemPtr1;
		}
		TmpItemPtr2 = TmpItemPtr1;

      // Add sub items if required.
		if (WindowItem[ItemIndex].SubItem != NULL)
		{
			if (!AddSubItems(
					TmpItemPtr1,
					WindowItem[ItemIndex].SubItem,
					TextHeight))
			{
				return FALSE;
			}
		}

		ItemTop += TextHeight;
		ItemIndex++;
	}

	return TRUE;
}

/* =============================================================================
 * Exported functions
 */

/* =============================================================================
 * FUNCTION: MenuNil
 */
void
MenuNil(void)
{
}

/* =============================================================================
 * FUNCTION: MenuQuit
 */
void
MenuQuit(void)
{
	struct Menu *TmpMenuPtr1;
	struct Menu *TmpMenuPtr2;

   // Free all memory for menu structures
	TmpMenuPtr1 = MenuPtr;

	while (TmpMenuPtr1 != NULL)
	{
		FreeItems(TmpMenuPtr1->FirstItem);
		TmpMenuPtr2 = TmpMenuPtr1;
		TmpMenuPtr1 = TmpMenuPtr1->NextMenu;
		FreeMem(TmpMenuPtr2, sizeof(struct Menu));
	}

   // Remove the menu from the menu strip
	ClearMenuStrip(MenuWindow);

   // Set the quit flag
	Quit = TRUE;
}

/* =============================================================================
 * FUNCTION: MakeMenuStructure
 */
int
MakeMenuStructure(
	struct Window *MWindow,
	struct SmartMenu *WindowMenu)
{
	int Length;
	int MenuIndex;
	int MenuLeft;
	int MenuTop;
	int MenuWidth;
	int MenuHeight;
	struct Menu *TmpMenuPtr1;
	struct Menu *TmpMenuPtr2;
	struct Screen *WindowScreen;
	int TextHeight;

   // Store the window for the menu.
	MenuWindow = MWindow;
	MenuRPort = MenuWindow->RPort;

	MenuPtr = NULL;
	SmartMenuPtr = WindowMenu;

   // Get the font hight for the screen font.
	WindowScreen = MenuWindow->WScreen;
	TextHeight = WindowScreen->Font->ta_YSize;

   // Initialise sizes and positions/
	TmpMenuPtr2 = NULL;
	MenuIndex = 0;
	MenuLeft = TextLength(MenuRPort, " ", 1);;
	MenuTop = 0;
	MenuHeight = TextHeight;

   // Create the menu structures
	while (WindowMenu[MenuIndex].Text != NULL)
	{
		Length = strlen(WindowMenu[MenuIndex].Text);
		MenuWidth = TextLength(MenuRPort, WindowMenu[MenuIndex].Text, Length);
		MenuWidth += TextLength(MenuRPort, " ", 1);

		if ((TmpMenuPtr1 = (struct Menu *)
		 	 AllocMem(sizeof(struct Menu), MEMF_CLEAR)) == NULL)
		{
			MenuError(SMARTMENU_MEMORY_ERROR);
			return FALSE;
		}

		TmpMenuPtr1->NextMenu = NULL;
		TmpMenuPtr1->LeftEdge = MenuLeft;
		TmpMenuPtr1->TopEdge = MenuTop;
		TmpMenuPtr1->Width = MenuWidth;
		TmpMenuPtr1->Height = MenuHeight;
		TmpMenuPtr1->Flags = MENUENABLED;
		TmpMenuPtr1->MenuName = WindowMenu[MenuIndex].Text;
		TmpMenuPtr1->FirstItem = NULL;

		if (MenuPtr == NULL)
		{
			MenuPtr = TmpMenuPtr1;
		}
		else
		{
			TmpMenuPtr2->NextMenu = TmpMenuPtr1;
		}
		TmpMenuPtr2 = TmpMenuPtr1;

		if (WindowMenu[MenuIndex].FirstItem != NULL)
		{
			if (!AddItems(
					TmpMenuPtr1,
					WindowMenu[MenuIndex].FirstItem,
					TextHeight))
			{
				return FALSE;
			}
      }

		MenuLeft += MenuWidth;;
		MenuIndex++;
	}

   // Set the menu
	SetMenuStrip(MenuWindow, MenuPtr);

	return TRUE;

}

/* =============================================================================
 * FUNCTION: DoMenuSelection
 */
void
DoMenuSelection(
	USHORT code)
{
	USHORT Number;
	int MenuNumber;
	int ItemNumber;
	int SubNumber;
	struct MenuItem *Item;

	Number = code;
	while (Number != MENUNULL)
	{
		Item = ItemAddress(MenuPtr, Number);

		MenuNumber = MENUNUM(Number);
		ItemNumber = ITEMNUM(Number);
		SubNumber = SUBNUM(Number);

		if (SubNumber == NOSUB)
		{
			SmartMenuPtr[MenuNumber].FirstItem[ItemNumber].SelectFunction();
		}
		else
		{
			SmartMenuPtr[MenuNumber].FirstItem[ItemNumber].
				SubItem[SubNumber].SelectFunction();
		}

		Number = Item->NextSelect;
	}
}