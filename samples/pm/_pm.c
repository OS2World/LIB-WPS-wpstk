/****************************** Module Header ******************************\
*
* Module Name: _pm.c
*
* PM helper functions sample
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2004
*
* $Id: _pm.c,v 1.27 2008-12-22 18:47:01 cla Exp $
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


#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <process.h>

#define INCL_ERRORS
#define INCL_WIN
#include <os2.h>

#define INCL_WTKUTLPM
#define INCL_WTKUTLCONTROL
#define INCL_WTKUTLMODULE
#define INCL_WTKBBH
#include <wtk.h>

#include "_pm.rch"

#include "_pmnb.h"
#include "_pmfddlg.h"
#include "_pmcnr.h"

#define __TITLE__  "PM related helper functions sample"

#define WM_USER_INITSUBCLASS   (WM_USER + 0x1000)

// ---------------------------------------------------------------------

static MRESULT EXPENTRY _MleSubclassProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
         APIRET         rc = NO_ERROR;
         HAB            hab = WinQueryAnchorBlock( HWND_DESKTOP);
static   PFNWP          pfnwpOrgWindowProc = NULL;
static   HWND           hwndMenu = NULLHANDLE;
static   CHAR           szHelpFile[ _MAX_PATH];
static   HWND           hwndBubbleHelp = NULLHANDLE;

if (!pfnwpOrgWindowProc)
   pfnwpOrgWindowProc = WtkGetDefaultWindowProcPtr( hwnd);

switch (msg)
   {
   case WM_USER_INITSUBCLASS:
      {
               ULONG          ulStyle;
               ULONG          ulMleWindowId =  WinQueryWindowUShort( hwnd, QWS_ID);

      // determine helpfile and create help instance
      rc = WtkGetModulePath( (PFN)_MleSubclassProc, szHelpFile, sizeof( szHelpFile));
      if (rc == NO_ERROR)
         strcat( szHelpFile, "\\_pm.hlp");

      // create the help instance
      WtkCreateHelpInstanceWithTable( hwnd, "Help", szHelpFile, NULLHANDLE, MAKEP( 0xFFFF, IDDLG_MAIN));

      // initialize bubble help
      WtkInitializeBubbleHelp( hwnd, NULL, &hwndBubbleHelp);

      // activate and update bubble help
      WtkActivateBubbleHelp( hwndBubbleHelp);
      WtkUpdateBubbleHelp( hwndBubbleHelp, "Press mouse button 2 to\rbring up the context menu.");

      // load basic menu
      hwndMenu = WinLoadMenu( HWND_OBJECT, NULLHANDLE, IDMEN_POPUP);

      // demo usage of WtkInsertMenu
      // insert one menu into existing one
      // (this is helpful when extending the menu of XCenter widgets)
      WtkInsertMenu( hwnd, hwndMenu, IDMEN_DEFFILEDLG, NULLHANDLE, IDMEN_INSERT,
                     WTK_INSERTMENU_TOPSEPARATOR | WTK_INSERTMENU_BOTTOMSEPARATOR);

      // set style of MLE and change font
      WinSendMsg( hwnd, MLM_SETREADONLY, MPFROMLONG( TRUE), 0);
      WinSendMsg( hwnd, MLM_SETSEL, MPFROMLONG( 5), MPFROMLONG( 5));
      WinSendMsg( hwnd, MLM_SETWRAP, MPFROMLONG( TRUE), 0);
      WtkSetWindowFont( hwnd, "9.WarpSans Bold", 0);

      // set MLE content
      WtkAddTextResourceToMLE( WinQueryWindow( hwnd, QW_PARENT),
                               ulMleWindowId, NULLHANDLE, RT_USER_DATAFILE, IDRES_MLETEXT);

      }
      break;

   // -------------------------------------------------------

   case HM_ERROR:
      printf( "HM_ERROR with 0x%08x\n", LONGFROMMP( mp1));
      break;

   // -------------------------------------------------------

   case WM_BUTTON2UP:
      {
               POINTL         ptl;
               ULONG          ulPopupMenuStyle = PU_HCONSTRAIN     | PU_VCONSTRAIN   |
                                                 PU_MOUSEBUTTON1   | PU_MOUSEBUTTON2 |
                                                 PU_MOUSEBUTTON3   | PU_KEYBOARD;

      // popup system menu
      WinQueryPointerPos( HWND_DESKTOP, &ptl);
      WinPopupMenu( HWND_DESKTOP,
                    hwnd,
                    hwndMenu,
                    ptl.x,
                    ptl.y,
                    0,
                    ulPopupMenuStyle);
      }
      break;

   // -------------------------------------------------------

   case WM_INITMENU:
      // don't show bubble help while menu is active
      WtkDeactivateBubbleHelp( hwndBubbleHelp);
      break;

   // -------------------------------------------------------

   case WM_MENUEND:
      // show bubble help again
      WtkActivateBubbleHelp( hwndBubbleHelp);
      break;

   // -------------------------------------------------------

   case WM_COMMAND:
      switch (LONGFROMMP( mp1))
         {
         case IDMEN_DEFNOTEBOOK:
            LaunchNotebook( hwnd, hwndBubbleHelp, szHelpFile, TYPE_DEFNOTEBOOK);
            break;

         case IDMEN_OKNOTEBOOK:
            LaunchNotebook( hwnd, hwndBubbleHelp, szHelpFile, TYPE_OKNOTEBOOK);
            break;

         case IDMEN_DEFDIRDLG:
            LaunchDirDlg( hwnd, hwndBubbleHelp, FALSE, FALSE);
            break;

         case IDMEN_DEFDIRDLGPRELOAD:
            LaunchDirDlg( hwnd, hwndBubbleHelp, TRUE, FALSE);
            break;

         case IDMEN_CUSTDIRDLGPRELOAD:
            LaunchDirDlg( hwnd, hwndBubbleHelp, TRUE, TRUE);
            break;

         case IDMEN_DEFFILEDLG:
            LaunchFileDlg( hwnd, hwndBubbleHelp);
            break;

         case IDMEN_CNRDLG:
            LaunchContainerDialog( hwnd, NULLHANDLE);
            break;

         case IDMEN_EXIT:
            WinPostMsg( hwnd, WM_QUIT, 0, 0);
            break;

         case IDMEN_ITEM1:
         case IDMEN_ITEM2:
         case IDMEN_ITEM3:
            {
                     CHAR           szMessage[ 127];
            sprintf( szMessage, "Inserted text menu item %u was selected!",
                     LONGFROMMP( mp1) - IDMEN_ITEM1 + 1);
            WinMessageBox( HWND_DESKTOP, hwnd, szMessage,
                           __TITLE__, LONGFROMMP( mp1), MB_MOVEABLE | MB_OK | MB_INFORMATION | MB_HELP);
            }
            break;

         case IDMEN_ITEMBITMAP:
            WinMessageBox( HWND_DESKTOP, hwnd, "Inserted bitmap menu item was selected!",
                           __TITLE__, LONGFROMMP( mp1), MB_MOVEABLE | MB_OK | MB_INFORMATION | MB_HELP);
            break;

         }
      return (MRESULT) TRUE;
      break;

   // -------------------------------------------------------

   case WM_DESTROY:
      if (hwndMenu)
         WinDestroyWindow( hwndMenu);

      WtkDestroyHelpInstance( hwnd);

      if (hwndBubbleHelp)
         {
         WtkDeactivateBubbleHelp( hwndBubbleHelp);
         WtkTerminateBubbleHelp( hwndBubbleHelp);
         }
      break;

   } // switch (msg)

return pfnwpOrgWindowProc( hwnd, msg, mp1, mp2);

}

// -----------------------------------------------------------------------------

int main ( int argc, char *argv[])
{

         APIRET         rc = NO_ERROR;
         BOOL           fResult = FALSE;

         HAB            hab = NULLHANDLE;
         HMQ            hmq = NULLHANDLE;
         QMSG           qmsg;

         HWND           hwndFrame;

         HWND           hwndMle;
         ULONG          ulFrameStyles = FCF_SIZEBORDER | FCF_TITLEBAR | FCF_SYSMENU |
                                        FCF_MINMAX | FCF_TASKLIST | FCF_ACCELTABLE;
         ULONG          ulMLEStyle = 0;
do
   {
   // get PM resources
   if ((hab = WinInitialize( 0)) == NULLHANDLE)
      break;
   if ((hmq = WinCreateMsgQueue( hab, 0)) == NULLHANDLE)
      break;

   // create a standard window with MLE as client
   // so we don't have to deal with drawing issues for this sample
   hwndFrame = WinCreateStdWindow( HWND_DESKTOP, 0, &ulFrameStyles,
                                   WC_MLE, __TITLE__, ulMLEStyle,
                                   0, IDDLG_MAIN,
                                   &hwndMle);
   if (!hwndFrame)
      {
      rc = ERRORIDERROR( WinGetLastError( hab));
      break;
      }

   // set window postition, show later
   WinSetWindowPos( hwndFrame, HWND_TOP, 200, 200, 600, 600,
                    SWP_MOVE | SWP_SIZE | SWP_ZORDER | SWP_ACTIVATE);

   // center window on the desktop, then show
   WtkCenterWindow( hwndFrame, HWND_DESKTOP, 0);
   WinShowWindow( hwndFrame, TRUE);

   // subclass MLE window to get further control
   WinSubclassWindow( hwndMle, _MleSubclassProc);

   // run initialization code of our subclass procedure
   WinPostMsg( hwndMle, WM_USER_INITSUBCLASS, 0, 0);

   // now dispatch messages
   while (WinGetMsg( hab, &qmsg, NULLHANDLE, 0, 0))
      {
      WinDispatchMsg(hab, &qmsg);
      }

   } while (FALSE);

// cleanup
if (hmq) WinDestroyMsgQueue( hmq);
if (hab) WinTerminate( hab);

return rc;
}

