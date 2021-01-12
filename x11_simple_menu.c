/* =============================================================================
 * PROGRAM:  CODE LIBRARY
 * FILENAME: x11_simple_menu.c
 *
 * DESCRIPTION:
 * This module provides a simple menu capability for X11 programs.
 * It can currently only handle a single window/menu at a time.
 * To use this library in an application:
 *   1. Create the widow to contain the menu.
 *   2. Call XMENU_SetMenu to set the menu to be displayed in that window.
 *      This must provide a pointer to a function to repaint the windows
 *      that may be obscured by the menu.
 *   3. Call XMENU_GetMenuHeight to get the height of the menu bar.
 *      The application should be acreful not to draw in this area.
 *   4. Add XMENU_EVENT_MASK to the menu window's X event mask.
 *   5. When processing an X event the main event loop should call
 *      XMENU_HandleEvent.
 *      This function will return either:
 *      . -1 if the menu didn't handle this   event, in which case the 
 *        application needs to handle it, or
 *      . The selected menu item id if the event was handled by the menu.
 *        In this case the application shouldn't process this event, but should
 *        act on the returned menu selection id.
 *   6. Call XMENU_Redraw whenever the window containing the menu gets an
 *      expose event.
 *
 * =============================================================================
 * COPYRIGHT:
 *
 * Copyright (c) 2003, Julian Olds
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
 * None.
 *
 * =============================================================================
 * EXPORTED FUNCTIONS
 *
 * XMENU_SetMenu       - Set the menu and menu dislay window
 * XMENU_SetCheck      - Set the checkmark state for a menu item
 * XMENU_GetMenuHeight - Get the menu height in pixels
 * XMENU_Redraw        - Redraw the menu
 * XMENU_HandleEvent   - Process input events for the menu selection
 *
 * =============================================================================
 */

#include <stdio.h>
#include <string.h>

#include "x11_simple_menu.h"
#include "checkmark.bm"

/* =============================================================================
 * Local variables
 */

static char *menu_default_font = "*-helvetica-medium-r-*-12-*";

static Display *MenuDisplay;
static Window  MenuWindow;

static GC menu_gc;
static XGCValues menu_gc_values;
static unsigned long menu_gc_values_mask;

static XFontStruct *menu_font_info;
static char *menu_font_name = NULL;

static Pixmap CheckPixmap = None;

static unsigned long menu_white_pixel;
static unsigned long menu_black_pixel;

static struct XMENU_Menu *The_Menu;
static void (*RepaintWindow)(void);

/* Current popup menu size */
static Window Popup;

static int PopupWidth;
static int PopupHeight;
static int ItemHeight;

/* =============================================================================
 * Local functions
 */

/* =============================================================================
 * FUNCTION: XMENU_PaintPopup
 *
 * DESCRIPTION:
 * Paints the popup menu window with the items of a menu.
 *
 * PARAMETERS:
 *
 *   Popup : The popup window to be painted.
 *
 *   Menu  : The menu for the popup window.
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void XMENU_PaintPopup(Window Popup, struct XMENU_Menu *Menu)
{
  struct XMENU_Item *Item;
  int Top;

  XSetForeground(MenuDisplay, menu_gc, menu_white_pixel);
  XFillRectangle(MenuDisplay, Popup, menu_gc, 
		 0, 0, PopupWidth, PopupHeight);
 
  XSetForeground(MenuDisplay, menu_gc, menu_black_pixel);
  XSetBackground(MenuDisplay, menu_gc, menu_white_pixel);

  Top = 0;
  Item = Menu->ItemList;
 
  while (Item != NULL)
  {
    if (Item->Checked)
      {
	XCopyPlane(MenuDisplay, CheckPixmap, Popup, menu_gc,
		   0, 0,
		   checkmark_width, checkmark_height,
		   menu_font_info->max_bounds.width / 2, 
		   Top + (ItemHeight / 2) - (checkmark_height / 2) , 1);
      }

    XDrawString(MenuDisplay, Popup, menu_gc, 
		checkmark_width + menu_font_info->max_bounds.width,  
		Top + menu_font_info->max_bounds.ascent + 2,
		Item->Text, strlen(Item->Text));
    
    Top += ItemHeight;
    
    Item = Item->Next;
  }
}

/* =============================================================================
 * FUNCTION: XMENU_HighlightItem
 *
 * DESCRIPTION:
 * Highlight a menu item on a popup.
 *
 * PARAMETERS:
 *
 *  Popup     : The popup menu window
 *
 *  ItemId    : The item to be (un)highlighted
 *
 *  Highlight : True if the highlight is to be drawn, 
 *              False if the highlight is to be removed.
 *
 * RETURN VALUE:
 *
 *  None.
 */
static void XMENU_HighlightItem(Window Popup, 
				struct XMENU_Menu *Menu, 
				int ItemId,
				int Highlight)
{
  struct XMENU_Item *Item;
  int Count;

  Item = Menu->ItemList;  
  Count = 0;

  while ((Item != NULL) && (Item->ItemId != ItemId))
    {
      Count++;
      Item = Item->Next;
    }

  if (Item == NULL)
    {
      return;
    }

  if (Highlight)
    {
      XSetForeground(MenuDisplay, menu_gc, menu_black_pixel);
    }
  else
    {
      XSetForeground(MenuDisplay, menu_gc, menu_white_pixel);
    }

  XDrawRectangle(MenuDisplay, Popup, menu_gc,
		 0, Count * ItemHeight,
		 PopupWidth - 1, ItemHeight - 1);  

  XFlush(MenuDisplay);
  XSync(MenuDisplay, 0);  

}

/* =============================================================================
 * FUNCTION: XMENU_ItemHit
 *
 * DESCRIPTION:
 * Determine which menu item in the current popup menu is at location x, y
 *
 * PARAMETERS:
 *
 *   Menu : The currently active popup menu
 *
 *   x    : The x coordinate to test.
 *
 *   y    : The y coordinate to test.
 *
 * RETURN VALUE:
 *
 *   The ItemId of the menu item at (x, y) or XMENU_NUMMID if none.
 */
static int XMENU_ItemHit(struct XMENU_Menu *Menu, int x, int y)
{
  struct XMENU_Item *Item;
  int ItemNo;

  if ((x < 0) || (x > PopupWidth) || (y < 0) || (y > PopupHeight))
    {
      return XMENU_NULLID;      
    }

  ItemNo = y / ItemHeight;

  Item = Menu->ItemList;

  while ((Item != NULL) && (ItemNo > 0))
    {
      Item = Item->Next;
      ItemNo--;
    }

  if (Item != NULL)
    {
      return Item->ItemId;
    }
  else
    {
      return XMENU_NULLID;
    }
}

/* =============================================================================
 * FUNCTION: XMENU_MenuHit
 *
 * DESCRIPTION:
 * Identify which menu is at location x, y.
 *
 * PARAMETERS:
 *
 *   Menu : A pointer to the menu structure
 *
 *   x    : The x coordinate to test.
 *
 *   y    : The y coordinate to test.
 *
 * RETURN VALUE:
 *
 *   A pointer to the menu on the menu bar at (x, y) or NULL if none.
 */
static struct XMENU_Menu *XMENU_MenuHit(struct XMENU_Menu *Menu, int x, int y)
{
  struct XMENU_Menu *Pos;
  int FoundHit;

  Pos = Menu;
  FoundHit = 0;

  while ((Pos != NULL) && !FoundHit)
    {
      if ((x >= Pos->HitLeft) && (x < Pos->HitLeft + Pos->HitWidth) &&
	  (y >= Pos->HitTop) && (y < Pos->HitTop + Pos->HitHeight))
	{
	  FoundHit = 1;
	}
      else
	{
	  Pos = Pos->Next;
	}
    }

  return Pos;
}

/* =============================================================================
 * FUNCTION: XMENU_ActivatePopup
 *
 * DESCRIPTION:
 * Create the popup menu window.
 *
 * PARAMETERS:
 *
 *   Menu : The popup menu to activate.
 *
 * RETURN VALUE:
 *
 *   None.
 */
static void XMENU_ActivatePopup(struct XMENU_Menu *Menu)
{
  int Direction, Ascent, Descent;
  XCharStruct Extents;

  XSetWindowAttributes Attributes;

  struct XMENU_Item *Item;
  int rc;
  Window Root;
  Window Child;
  int winx, winy;
  int winwidth, winheight;
  int winborder, windepth;
  
  /* Calculate the popup menu size */

  PopupWidth = 0;
  PopupHeight = 0;

  Item = Menu->ItemList;
  while (Item != NULL)
    {
      XTextExtents(menu_font_info, Item->Text, strlen(Item->Text),
		   &Direction, &Ascent, &Descent, &Extents);
      
      if (Extents.width > PopupWidth)
	{
	  PopupWidth = Extents.width;
	}

      PopupHeight += ItemHeight;

      Item = Item->Next;
    }

  PopupWidth += checkmark_width + menu_font_info->max_bounds.width * 2;

  /* Find the popup menu position */

  rc = XGetGeometry(MenuDisplay, MenuWindow, &Root, &winx, &winy, 
		    &winwidth, &winheight, &winborder, &windepth);

  winx = 0;
  winy = 0;

  XTranslateCoordinates(MenuDisplay, 
			MenuWindow, Root, 
			winx, winy, &winx, &winy, &Child);

  /* Create the popup menu window */
  
  Attributes.border_pixel = menu_black_pixel;
  Attributes.backing_pixel = menu_white_pixel;
  Attributes.backing_store = WhenMapped;
  Attributes.save_under = 1;
  Attributes.override_redirect = 1;

  Popup = XCreateWindow
    (MenuDisplay,
     RootWindow(MenuDisplay, DefaultScreen(MenuDisplay)),
     winx + Menu->HitLeft, 
     winy + Menu->HitTop + Menu->HitHeight,
     PopupWidth, PopupHeight,
     1,              /* Border width */
     CopyFromParent, /* Depth */ 
     InputOutput,    /* Class */
     DefaultVisual(MenuDisplay, DefaultScreen(MenuDisplay)),
     CWBorderPixel | CWBackingStore | CWSaveUnder | CWOverrideRedirect,
     &Attributes);

  XSelectInput(MenuDisplay, 
	       Popup, 
	       ExposureMask | 
	       ButtonPressMask | ButtonReleaseMask | 
	       Button1MotionMask);


  XMapRaised(MenuDisplay, Popup);
  XFlush(MenuDisplay);
  XSync(MenuDisplay, 0);  

}

/* =============================================================================
 * FUNCTION: XMENU_HandleSelection
 *
 * DESCRIPTION:
 * Handle the selection from the menu.
 * This actiivates the initially selected menu and processes all x events until
 * either a menu item is selected or the menu is deactivated.
 *
 * PARAMETERS:
 *
 *   Menu : A pointer to the menu structure for the menus.
 *
 *   x    : The x coordinate of the initial selection
 *
 *   y    : The y coordinate of the initial selection
 *
 * RETURN VALUE:
 *
 *   The ItemId of the menu item selected or XMENU_NULLID if none.
 */
static int XMENU_HandleSelection(struct XMENU_Menu *Menu,  int x, int y)
{
  struct XMENU_Menu *Current;
  struct XMENU_Menu *NewMenu;
  int Done;
  int Dragging;
  Window Child;
  int winx, winy;
  XEvent xevent;
  int SelectId;
  int OldId;

  /* Activate the Popup menu for the initially selected menu */
  Current = XMENU_MenuHit(Menu, x, y);

  if (Current != NULL)
    {
      XMENU_ActivatePopup(Current);
    }

  /* Prepare for event handling */

  Done = 0;
  Dragging = 0;
  SelectId = XMENU_NULLID;

  /* 
   * Handle events until either a menu item is selected of the menu is
   * Deactivated.
   */

  while (!Done)
    {
      XNextEvent(MenuDisplay, &xevent);
      
      switch (xevent.type)
	{
	case Expose:
	  if (xevent.xexpose.window == Popup)
	    {
	      XMENU_PaintPopup(Popup, Current);
	    }
	  else if (xevent.xexpose.window == MenuWindow)
	    {
	      RepaintWindow();
	    }
	  break;
	
	case ButtonPress:
	  winx = xevent.xbutton.x;
	  winy = xevent.xbutton.y;

	  if (xevent.xbutton.window != Popup)
	    {
	      XTranslateCoordinates(MenuDisplay, 
				    xevent.xbutton.window, Popup, 
				    winx, winy, &winx, &winy, &Child);
	    }

	  SelectId = XMENU_ItemHit(Current, winx, winy);
	  if (SelectId != XMENU_NULLID)
	    {
	      XMENU_HighlightItem(Popup, Current, SelectId, 1);
	    }
	  else
	    {
	      winx = xevent.xmotion.x;
	      winy = xevent.xmotion.y;
	      
	      if (xevent.xmotion.window != MenuWindow)
		{
		  XTranslateCoordinates(MenuDisplay, 
					xevent.xmotion.window, MenuWindow, 
					winx, winy, &winx, &winy, &Child);
		}
	      
	      NewMenu = XMENU_MenuHit(The_Menu, winx, winy);
	      
	      if ((NewMenu != Current) && (NewMenu != NULL))
		{
		  XUnmapWindow(MenuDisplay, Popup);
		  XDestroyWindow(MenuDisplay, Popup);
		  
		  Current = NewMenu;
		  XMENU_ActivatePopup(Current);
		}	
	      else
		{
		  Done = 1;
		}
	    }
	  break;

	case ButtonRelease:
	  
	  if (Dragging)
	    {
	      Done = 1;
	    }
	  else
	    {
	      if (SelectId != XMENU_NULLID)
		{
		  Done = 1;
		}
	    }
	  break;

	case MotionNotify:
	  Dragging = 1;

	  winx = xevent.xmotion.x;
	  winy = xevent.xmotion.y;

	  if (xevent.xmotion.window != Popup)
	    {
	      XTranslateCoordinates(MenuDisplay, 
				    xevent.xmotion.window, Popup, 
				    winx, winy, &winx, &winy, &Child);
	    }

	  OldId = SelectId;
	  SelectId = XMENU_ItemHit(Current, winx, winy);
	  if (SelectId != OldId)
	    {
	      XMENU_HighlightItem(Popup, Current, OldId, 0);
	      
	      if (SelectId != XMENU_NULLID)
		{
		  XMENU_HighlightItem(Popup, Current, SelectId, 1);
		}
	    }
	  
	  if (SelectId == XMENU_NULLID)
	    {
	      /* Pointer not over a menu item, so check for menu change */
	  
	      winx = xevent.xmotion.x;
	      winy = xevent.xmotion.y;
	      
	      if (xevent.xmotion.window != MenuWindow)
		{
		  XTranslateCoordinates(MenuDisplay, 
					xevent.xmotion.window, MenuWindow, 
					winx, winy, &winx, &winy, &Child);
		}

	      NewMenu = XMENU_MenuHit(The_Menu, winx, winy);
	      
	      if ((NewMenu != Current) && (NewMenu != NULL))
		{
		  XUnmapWindow(MenuDisplay, Popup);
		  XDestroyWindow(MenuDisplay, Popup);
		  
		  Current = NewMenu;
		  XMENU_ActivatePopup(Current);
		}	      

	    }

	  break;

	default:
	  break;
	}

      
    }

  XDestroyWindow(MenuDisplay, Popup);

  return SelectId;
}

/* =============================================================================
 * Exported functions
 */

/* =============================================================================
 * FUNCTION: XMENU_SetMenu
 */
void XMENU_SetMenu(Display *dpy, 
		   Window win, 
		   struct XMENU_Menu *Menu, 
		   char *FontSpec,
		   void (*Repaint)(void))
{
  int screen_num;

  MenuDisplay = dpy;
  MenuWindow = win;

  The_Menu = Menu;
  RepaintWindow = Repaint;

  menu_gc_values.cap_style = CapButt;
  menu_gc_values.join_style = JoinBevel;
  menu_gc_values_mask = GCCapStyle | GCJoinStyle;
  menu_gc = XCreateGC(MenuDisplay, MenuWindow, 
		      menu_gc_values_mask, &menu_gc_values);

  if (CheckPixmap == None)
    {
      CheckPixmap = XCreateBitmapFromData
	(MenuDisplay, MenuWindow, 
	 checkmark_bits, checkmark_width, checkmark_height);
    }

  screen_num = DefaultScreen(MenuDisplay);
  menu_white_pixel = WhitePixel(MenuDisplay, screen_num);
  menu_black_pixel = BlackPixel(MenuDisplay, screen_num);

  if (FontSpec != NULL)
    {
      menu_font_name = FontSpec;
    }
  else
    {
      menu_font_name = menu_default_font;
    }

  menu_font_info = XLoadQueryFont(MenuDisplay, menu_font_name);
  if (!menu_font_info)
    {
      fprintf(stderr, 
	      "Error: XLoadQueryFont: failed loading font '%s'\n", 
	      menu_font_name);
      return;
    }

  XSetFont(MenuDisplay, menu_gc, menu_font_info->fid);

  ItemHeight = menu_font_info->max_bounds.ascent +
    menu_font_info->max_bounds.descent + 4;

  /* Draw the menu */

  XMENU_Redraw();
}

/* =============================================================================
 * FUNCTION: XMENU_SetCheck
 */
void XMENU_SetCheck(int ItemId, XMENU_CheckState CheckState)
{
  struct XMENU_Menu *Menu;
  struct XMENU_Item *Item;
  int Found;

  Found = False;
  Menu = The_Menu;

  while ((Menu != NULL) && !Found)
    {
      Item = Menu->ItemList;

      while ((Item != NULL) && !Found)
	{
	  if (Item->ItemId == ItemId)
	    {
	      Item->Checked = CheckState;
	      Found = 1;
	    }

	  Item = Item->Next;
	}

      Menu = Menu->Next;
    }
}

/* =============================================================================
 * FUNCTION: XMENU_GetMenuHeight
 */
int XMENU_GetMenuHeight(void)
{
  int CharHeight;

  CharHeight = 
    menu_font_info->max_bounds.ascent + menu_font_info->max_bounds.descent;

  return CharHeight + 10;

}

/* =============================================================================
 * FUNCTION: XMENU_Redraw
 */
void XMENU_Redraw(void)
{
  struct XMENU_Menu *Pos;
  XWindowAttributes win_attr;
  XCharStruct Extents;
  int MenuWidth;
  int MenuHeight;
  int MenuLeft;
  int MenuTop;
  int Direction;
  int Ascent;
  int Descent;
  Status rc;

  rc = XGetWindowAttributes(MenuDisplay, MenuWindow, &win_attr);
  
  MenuWidth = win_attr.width;
  MenuHeight = XMENU_GetMenuHeight();

  /* Clear the menu area */

  XSetForeground(MenuDisplay, menu_gc, menu_white_pixel);
  XFillRectangle(MenuDisplay, MenuWindow, menu_gc,
		 0, 0,
		 MenuWidth, MenuHeight);
  

  /* Draw the menu bottom line */

  XSetForeground(MenuDisplay, menu_gc, menu_black_pixel);
  XFillRectangle(MenuDisplay, MenuWindow, menu_gc,
		 0, MenuHeight -2,
		 MenuWidth, 2);  


  Pos = The_Menu;
  MenuLeft = 8;
  MenuTop = 4;

  while (Pos != NULL)
    {
           
      XDrawString(MenuDisplay, MenuWindow, menu_gc, 
		  MenuLeft,  MenuTop + menu_font_info->max_bounds.ascent,
		  Pos->Text, strlen(Pos->Text));

      XTextExtents(menu_font_info, Pos->Text, strlen(Pos->Text),
		   &Direction, &Ascent, &Descent, &Extents);

      Pos->HitLeft = MenuLeft;
      Pos->HitTop = MenuTop;

      Pos->HitWidth = Extents.width;
      Pos->HitHeight = 
	menu_font_info->max_bounds.ascent +
	menu_font_info->max_bounds.descent;

      MenuLeft += Extents.width + 8;
      Pos = Pos->Next;
 
    }
}

/* =============================================================================
 * FUNCTION: XMENU_HandleEvent
 */
int XMENU_HandleEvent(XEvent *Event)
{
  int Selected;
  int x, y;

  Selected = XMENU_NULLID;

  switch (Event->type)
    {
    case ButtonPress:
      x = Event->xbutton.x;
      y = Event->xbutton.y;

      if (Event->xbutton.button != Button1)
	{
	  return 0;
	}
      
      if (XMENU_MenuHit(The_Menu, x, y) != NULL)
	{
	  Selected = XMENU_HandleSelection(The_Menu, x, y);
	}

      
      return Selected;

    default:
      break;
    }

  return -1;
}
