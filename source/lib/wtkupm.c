/****************************** Module Header ******************************\
*
* Module Name: wtkupm.c
*
* Source for PM utility functions.
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2000
*
* $Id: wtkupm.c,v 1.8 2005-02-15 19:21:19 cla Exp $
*
* ===========================================================================
*
* This file is part of the WPS Toolkit package and is free software.  You can
* redistribute it and/or modify it under the terms of the GNU Library General
* Public License as published by the Free Software Foundation, in version 2
* as it comes in the "COPYING.LIB" file of the WPS Toolkit main distribution.
* This library is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
* License for more details.
*
\***************************************************************************/

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INCL_DOS
#define INCL_WIN
#define INCL_GPI
#define INCL_ERRORS
#include <os2.h>

#include "wtkupm.h"
#include "wtkuerr.h"
#include "wpstk.ih"

// ---------------------------------------------------------------------------

/*
@@WtkCenterWindow@SYNTAX
This function centers a window vertically and/or horizontally
within the owner window.

@@WtkCenterWindow@PARM@hwnd@in
Handle of the window to be centered.

@@WtkCenterWindow@PARM@hwndCenter@in
Handle of the window within which the window
specified by the parameter :hp2.hwnd:ehp2.
has to be centered.
:p.
In order to center a window within the whole desktop, specifiy
:hp2.HWND_DESKTOP:ehp2. here.


@@WtkCenterWindow@PARM@ulFlags@in
A variable specifying how to center the window.
:p.
Sepcify one of the following flags to center the window
horizontally and/or vertically&colon.
:ul compact.
:li.WTK_CENTERWINDOW_BOTH
:li.WTK_CENTERWINDOW_HORIZONTAL
:li.WTK_CENTERWINDOW_VERTICAL
:eul

@@WtkCenterWindow@RETURN
Success indicator.
:parml compact.
:pt.TRUE
:pd.Window could be centered.
:pt.FALSE
:pd.An error occurred.
:eparml.

@@WtkCenterWindow@REMARKS
None

@@
*/

#define WK_CENTERWINDOW_ALL (WTK_CENTERWINDOW_VERTICAL | WTK_CENTERWINDOW_HORIZONTAL)

BOOL APIENTRY WtkCenterWindow( HWND hwnd, HWND hwndCenter, ULONG ulFlags)
{
         BOOL           fResult = FALSE;
         APIRET         rc = NO_ERROR;

         SWP            swp;
         SWP            swpCenter;

do
   {
   // check parms
   if (!hwnd)
      {
      rc = PMERR_INVALID_HWND;
      break;
      }

   if (!ulFlags)
      ulFlags = WK_CENTERWINDOW_ALL;

   if ((ulFlags & ~WK_CENTERWINDOW_ALL) > 0)
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   // query window positions
   if (!WinQueryWindowPos( hwnd, &swp))
      break;
   if (!WinQueryWindowPos( hwndCenter, &swpCenter))
      break;

   // prepare for action
   swp.fl = SWP_MOVE;
   if (ulFlags & ~WTK_CENTERWINDOW_HORIZONTAL)
      swp.x = (swpCenter.cx - swp.cx) / 2;
   if (ulFlags & ~WTK_CENTERWINDOW_VERTICAL)
      swp.y = (swpCenter.cy - swp.cy) / 2;

   fResult = WinSetMultWindowPos( WinQueryAnchorBlock( HWND_DESKTOP), &swp, 1);

   } while (FALSE);

WtkSetErrorInfo( rc);
return fResult;
}

// ---------------------------------------------------------------------------

/*
@@WtkIsFontAvailable@SYNTAX
This function checks if a given font is available.

@@WtkIsFontAvailable@PARM@pszFontname@in
Address of the ASCIIZ name of the font.
:p.
The name may specify the font size (like e.g. :hp2.Helv.8:ehp2.),
but the size will be ignored.

@@WtkIsFontAvailable@RETURN
Success indicator.
:parml compact.
:pt.TRUE
:pd.Font is available.
:pt.FALSE
:pd.Font is not available.
:eparml.

@@WtkIsFontAvailable@REMARKS
This function is intended to check for fonts that either may
or may not be available, like :hp2.WarpSans:ehp2., which is
not available for early :hp2.OS/2 Warp 3:ehp2.,
but has been added to the system with fixpaks.

@@
*/

BOOL APIENTRY WtkIsFontAvailable( PSZ pszFontname)
{
         BOOL           fResult = FALSE;
         APIRET         rc = NO_ERROR;

         PSZ            pszCopy = NULL;
         HPS            hps = NULLHANDLE;

         PSZ            p;
         FONTMETRICS    fm;
         LONG           lMaxFonts;
         LONG           lrc;

do
   {
   // check parms
   if ((!pszFontname) || (!*pszFontname))
      break;

   // create copy and cut off size indicator
   pszCopy = strdup( pszFontname);
   if (!pszCopy)
      break;
   pszFontname = pszCopy;
   p = strchr( pszFontname, '.');
   if (p) pszFontname = p + 1;

   // check font
   hps  = WinBeginPaint( HWND_DESKTOP, 0, 0);
   if (!hps)
      break;

   lMaxFonts = 1;
   memset( &fm, 0, sizeof( fm));
   lrc = GpiQueryFonts( hps, QF_PUBLIC, pszFontname,
                        &lMaxFonts, sizeof( FONTMETRICS), &fm);
   if (lrc == GPI_ALTERROR)
      break;

   // facename returned ?
   if (strcmp( pszFontname, fm.szFacename))
      break;

   // we're done
   fResult = TRUE;

   } while (FALSE);

// cleanup
if (hps) WinEndPaint( hps);
if (pszCopy) free( pszCopy);

return fResult;
}

// ---------------------------------------------------------------------------

/*
@@WtkSetWindowFont@SYNTAX
This function explicitely sets the specified font for a given window as presentation
parameter and optionally for all of its child windows.

@@WtkSetWindowFont@PARM@hwnd@in
Handle of the window to be modified.

@@WtkSetWindowFont@PARM@pszFontname@in
Address of the ASCIIZ name of the font.
:p.
Specify a font name including the size like e.g. "WarpSans.9".

@@WtkSetWindowFont@PARM@fIncludeChildren@in
A flag to set the new font for all child windows.
:p.
This is useful if any child window has already a presentation
parameter for the font set and so would not inherit the font
setting from the window specified with :hp2.hwnd:ehp2..

@@WtkSetWindowFont@RETURN
Success indicator.
:parml compact.
:pt.TRUE
:pd.Font could be set as presentation parameter.
:pt.FALSE
:pd.Font could not be set as presentation parameter.
:eparml.

@@WtkSetWindowFont@REMARKS
Note that this function does not return an error, if the specified
font is not available. This will only result in that no font is set
for the window.
@@
*/

BOOL APIENTRY WtkSetWindowFont( HWND hwnd, PSZ pszFontname, BOOL fIncludeChildren)
{
         BOOL           fResult = FALSE;
         BOOL           fErrorOccurred = FALSE;
         BOOL           fFontSet;
         HENUM          henum;
         HWND           hwndItem;
do
   {
   // check parms
   if ((!pszFontname) || (!*pszFontname))
      break;

   // remove before
   fFontSet = WinSetPresParam( hwnd, PP_FONTNAMESIZE, strlen( pszFontname) + 1, pszFontname);
   if (!fFontSet)
      fErrorOccurred = TRUE;

   if (fIncludeChildren)
      {
      henum = WinBeginEnumWindows( hwnd);
      hwndItem = WinGetNextWindow( henum);
      while (hwndItem)
         {
         // set font
         fFontSet = WinSetPresParam( hwndItem, PP_FONTNAMESIZE, strlen( pszFontname) + 1, pszFontname);
         if (!fFontSet)
            fErrorOccurred = TRUE;

         // next window
         hwndItem = WinGetNextWindow( henum);
         }

      WinEndEnumWindows( henum);
      }

   if (!fErrorOccurred)
      fResult = TRUE;

   } while (FALSE);

return fResult;
}

// ---------------------------------------------------------------------------

/*
@@WtkInsertMenu@SYNTAX
This function inserts a new menu, loaded from a PM resource, into an existing menu.

@@WtkInsertMenu@PARM@hwnd@in
Handle of the frame window to load the new menu with.

@@WtkInsertMenu@PARM@hwndMenu@in
Handle of the existing menu, where the new menu has to be inserted.

@@WtkInsertMenu@PARM@ulInsertAfter@in
Identifier of the menu item within the existing window, after which the
new menu should be inserted.
:p.
The identifier may have the following values:
:parml compact.
:pt.MIT_FIRST or 0
:pd.include menu at the beginning of the existing menu
:pt.MIT_END or MIT_LAST
:pd.include menu at the end of the existing menu
:pt.any other value
:pd.identifier of a menu item, after which the new menu is to be included
:eparml.

@@WtkInsertMenu@PARM@hmodResource@in
Handle of the module, holding the PM resource with the new menu.
:p.
The handle may have the following values:
:parml compact.
:pt.NULLHANDLE
:pd.the new menu is assumed to be hold in the PM resource of the current executable.
:pt.any other value
:pd.a handle is obtained by a call to DosLoadModule or WinLoadLibrary,
loading the module holding the PM resource including the new menu
:eparml.

@@WtkInsertMenu@PARM@ulMenuId@in
Resource indentifier of the new menu.

@@WtkInsertMenu@PARM@ulOptions@in
A variable specifying how to include the new menu,
:p.
Sepcify one of the following flags to add separators
before and/or after the menu&colon.
:ul compact.
:li.WTK_INSERTMENU_TOPSEPARATOR
:li.WTK_INSERTMENU_BOTTOMSEPARATOR
:eul

@@WtkInsertMenu@RETURN
Success indicator.
:parml compact.
:pt.TRUE
:pd.The menu has ben added
:pt.FALSE
:pd.The manu could not be added.
:eparml.

@@WtkInsertMenu@REMARKS
This function was mainly intended to extend context menus of XCenter widgets without
having to code extensions to the existing menu directly. For that, it is recommended
to specify the following values for the parameter :hp2.ulInsertAfter:ehp2.&colon.
:parml compact.
:pt.:hp2.ID_CRMI_PROPERTIES:ehp2.
:pd.insert the new menu after the XCenter properties item, if that is used
:pt.:hp2.MIT_FIRST:ehp2. or zero
:pd.insert the new menu at the beginning of the existing menu, if
the XCenter properties item is not used by the widget.
:eparml.

:note.
:ul compact.
:li.the value :hp2.ID_CRMI_PROPERTIES:ehp2. is defined in the file
includes\dlgids.h of the :hp2.XWorkplace:ehp2. sourcecode. If you have no access
to this file, you may want to try to define the value to :hp2.0x7f08:ehp2., but this,
of course, may be subject to changes.
:eul.

@@
*/

static BOOL _duplicateMenuItems( HWND hwndMenu, HWND hwndNewMenu, ULONG usInsertItemPos)
{
         BOOL           fResult = FALSE;
         ULONG          i;

         HWND           hwndSubMenu;

         HAB            hab = WinQueryAnchorBlock( HWND_DESKTOP);
         ULONG          ulItemCount;
         ULONG          ulItemId;
         MENUITEM       mi;
         CHAR           szMenuText[ 128];
         ULONG          ulItemPos;
         SWP            swp;

do
   {

   // cycle though all items of menu to insert
   ulItemCount = (ULONG)WinSendMsg( hwndNewMenu, MM_QUERYITEMCOUNT, 0, 0);
   for (i = 0; i < ulItemCount; i++)
      {
      // check item attributes and text, last item first
      ulItemId = (ULONG)WinSendMsg( hwndNewMenu, MM_ITEMIDFROMPOSITION, MPFROMP( ulItemCount - i - 1), 0);
      if (!WinSendMsg( hwndNewMenu, MM_QUERYITEM, MPFROM2SHORT( ulItemId, FALSE), MPFROMP( &mi)))
         continue;

      // take care for the item text
      memset( szMenuText, 0, sizeof( szMenuText));
      WinSendMsg( hwndNewMenu, MM_QUERYITEMTEXT, MPFROM2SHORT( ulItemId, sizeof( szMenuText)), MPFROMP( szMenuText));

      // release a bitmap from the source menu item,
      // so that it does not get destroyed below
      if (mi.hItem)
         {
                  MENUITEM       miTmp;

         memcpy( &miTmp, &mi, sizeof( mi));
         WinSendMsg( hwndNewMenu, MM_SETITEM, MPFROM2SHORT( 0, FALSE), MPFROMP( &miTmp));
         }

      // if it is a sumbenu, it needs to be copied
      if (mi.hwndSubMenu)
         {
         hwndSubMenu = WinCreateWindow( HWND_OBJECT, 
                                        WC_MENU, 
                                        NULL, 
                                        WS_VISIBLE, 
                                        0, 0, 0, 0,
                                        hwndMenu,
                                        HWND_TOP,
                                        0, 
                                        NULL, 
                                        NULL);
         if (hwndSubMenu)
            {
            if (_duplicateMenuItems( hwndSubMenu, mi.hwndSubMenu, 0))
               mi.hwndSubMenu = hwndSubMenu;
            else
               WinDestroyWindow( hwndSubMenu);
            }
         }

      // add to menu
      mi.iPosition = usInsertItemPos;
      ulItemPos = (ULONG) WinSendMsg( hwndMenu, MM_INSERTITEM, MPFROMP( &mi), MPFROMP( szMenuText));

      }

   // done
   fResult = TRUE;

   } while (FALSE);

return fResult;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

BOOL APIENTRY WtkInsertMenu( HWND hwnd, HWND hwndMenu, ULONG ulInsertAfter,
                             HMODULE hmodResource, ULONG ulMenuId, ULONG ulOptions)
{
         BOOL           fResult = FALSE;
         ULONG          i;
         HAB            hab = WinQueryAnchorBlock( HWND_DESKTOP);

         ULONG          usInsertItemPos = 0;

         HWND           hwndNewMenu = NULLHANDLE;
         ULONG          ulItemPos;
         MENUITEM       mi;

do
   {
   // check parms
   if (!WinIsWindow( hab, hwnd))
      break;
   if (!WinIsWindow( hab, hwndMenu))
      break;

   // determine position where items are to be inserted
   switch (ulInsertAfter)
      {
      case MIT_FIRST:
         usInsertItemPos = 0;
         break;

      case MIT_END:
      case MIT_LAST:
         usInsertItemPos = (ULONG)WinSendMsg( hwndMenu, MM_QUERYITEMCOUNT, 0, 0);
         break;

      default:
         usInsertItemPos = (ULONG) WinSendMsg( hwndMenu, MM_ITEMPOSITIONFROMID, MPFROM2SHORT( ulInsertAfter,  FALSE), 0);
         usInsertItemPos++;
         break;
      }

   if (usInsertItemPos == MIT_NONE)
      break;

   // load menu to extend
   hwndNewMenu = WinLoadMenu( HWND_DESKTOP, hmodResource, ulMenuId);
   if (!hwndNewMenu)
      break;

   // add last separator, if to be added at first position
   if (ulOptions & WTK_INSERTMENU_BOTTOMSEPARATOR)
      {
      memset( &mi, 0, sizeof( mi));
      mi.iPosition = usInsertItemPos;
      mi.afStyle   = MIS_SEPARATOR;
      ulItemPos = (ULONG) WinSendMsg( hwndMenu, MM_INSERTITEM, MPFROMP( &mi), 0);
      }

   // duplicate all menu items and submenus
   if (!_duplicateMenuItems( hwndMenu, hwndNewMenu, usInsertItemPos))
      break;

   // add first separator, if to be added at second position
   if (ulOptions & WTK_INSERTMENU_TOPSEPARATOR)
      {
      memset( &mi, 0, sizeof( mi));
      mi.iPosition = usInsertItemPos;
      mi.afStyle   = MIS_SEPARATOR;
      ulItemPos = (ULONG) WinSendMsg( hwndMenu, MM_INSERTITEM, MPFROMP( &mi), 0);
      }

   // done
   fResult = TRUE;

   } while (FALSE);

// cleanup
if (hwndNewMenu) WinDestroyWindow( hwndNewMenu);
return fResult;
}

// ---------------------------------------------------------------------------

/*
@@WtkGetStringDrawLen@SYNTAX
This function determines the length of a string in pixels,
if it would be drawn with the current attributes of the 
specified presentation space.

@@WtkGetStringDrawLen@PARM@hps@in
Handle of the presentation space.

@@WtkGetStringDrawLen@PARM@pszString@in
Address of the ASCIIZ name of the string.

@@WtkGetStringDrawLen@RETURN
String length in pixel.

@@WtkGetStringDrawLen@REMARKS
None.
@@
*/

ULONG APIENTRY WtkGetStringDrawLen( HPS hps, PSZ pszString)
{
         ULONG          ulStrLen;
         PPOINTL        paptl = NULL;
         PPOINTL        pptlLast;

do
   {
   // get memory
   paptl = malloc( sizeof( POINTL) * (strlen( pszString) + 1));
   if (!paptl)
      break;

   // test draw the string to get max width
   if (!GpiQueryCharStringPos( hps, 0L, strlen( pszString), pszString, NULL, paptl))
      break;
   pptlLast  = paptl + strlen( pszString);


   ulStrLen = pptlLast->x;

   } while (FALSE);


// cleanup
if (paptl) free( paptl);
return ulStrLen;
}

