/****************************** Module Header ******************************\
*
* Module Name: settdlg.c
*
* dialog procedure source of test class of
* settings and details manager WPS sample
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2000
*
* $Id: settdlg.c,v 1.6 2006-12-04 21:28:41 cla Exp $
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

/* C Runtime */
#define EXTERN
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

/* OS/2 Toolkit */
#define INCL_ERRORS
#define INCL_DOS
#define INCL_WIN
#define INCL_PM
#include <os2.h>

/* WPS Toolkit */
#define INCL_WTK
#include <wtk.h>

#include "settdlg.h"
#include "settdlg.rch"
#include "settcls.ih"

// some macros
#define  ENABLECONTROL(hwndDialog,id,flag) (WinEnableWindow( WinWindowFromID( hwndDialog, id), flag))

/* ###################################################################################### */

MRESULT EXPENTRY SettingsPageProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{

         HVALUETABLE   hvt = (HVALUETABLE) WinQueryWindowPtr( hwnd, QWL_USER );

switch( msg )
   {

   // -----------------------------------------------------------------

   case WM_INITDLG:
      {
                CHAR           szData[ 20];
                HWND           hwndButton;

      // save instance data ptr
      WinSetWindowPtr( hwnd, QWL_USER, mp2);
      hvt = LONGFROMMP( mp2);

      // register this page
      WtkRegisterSettingsDialog( hvt, hwnd);

      // WtkRelocateNotebookpageControls makes
      // certain pushbuttons notebookbuttons
      // and relocate all other controls downwards
      //
      // This allows you to use the same dialog template
      // for WARP 3 and WARP 4 !!!!!!!!!!!!!!!!
      //
      // The call does nothing, if
      // - it is not WARP 4 (OS2Version <= 20.30),
      //   as WARP 3 does not know notebook buttons
      // - the dialog does not contain at least one groupbox
      // Also important: pushbuttons not lying below the lowest groupbox
      // are not made notebook buttons !
      //
      // In other words, to make it work properly
      // - place all controls (also pushbuttons) except the
      //   notebook buttons inside or above a groupbox
      //   (BTW you may use several groupboxes)
      // - place the "notebook" buttons below the lowest groupbox
      //  (you would do that anyway, don't you ?)
      WtkRelocateNotebookpageControls( hwnd);

      // fill controls with current values
      WinSendMsg( hwnd, WM_COMMAND, MPFROMLONG( IDDLG_PB_UNDO), 0);

      }
      return (MRESULT) FALSE;
      // break; // end of WM_INITDLG

   // -----------------------------------------------------------------

   case WM_COMMAND:

      switch (SHORT1FROMMP(mp1))

         {
         case IDDLG_PB_HELP:
            {
                     HSETTINGTABLE    hst;
                     M_SettingsClass  *somSelf_M;

                     SettingsClass     *somSelf;
                     SettingsClassData *somThis;

            // just for demo purpose: query the handle to the settingstable
            if (!WtkQuerySettingstable( hvt, &hst))
               break;

            // just for demo purpose: query the pointer to the meta class
            // may be used to call _wpcls* methods
            if (!WtkQueryObjectClass( hst, (PPVOID)&somSelf_M)) 
               break;

            // query ptrs to object instance
            if (!WtkQueryObjectInstance( hvt, (PPVOID)&somSelf, (PPVOID)&somThis))
               break;

            // _wpDisplayHelp( somSelf, ID_HELP_???, MM2_HELPLIBRARY );
            }
            break;

         case IDDLG_PB_UNDO:
            // reset values
            WtkReadObjectValueTable( hvt, hwnd);
            break;
         }

      // do not call WinDefDlgProc in order to not dismiss !
      return (MRESULT) TRUE;
      // beak; // end of WM_COMMAND

   // -----------------------------------------------------------------

   case WM_CONTROL:
      {
                BOOL           fEnableControls = FALSE;

      switch (SHORT1FROMMP(mp1))

         {
         case IDDLG_RB_CFGSTATIC:
            fEnableControls = TRUE;
            // fallthru !!!

         case IDDLG_RB_CFGDYNAMIC:
            ENABLECONTROL( hwnd, IDDLG_ST_LOCALIP, fEnableControls);
            ENABLECONTROL( hwnd, IDDLG_EF_LOCALIP, fEnableControls);
            ENABLECONTROL( hwnd, IDDLG_ST_GATEWAY, fEnableControls);
            ENABLECONTROL( hwnd, IDDLG_EF_GATEWAY, fEnableControls);
            break;

         }
      }
      break; // end of WM_CONTROL

   // -----------------------------------------------------------------

   case WM_DESTROY:
      WtkWriteObjectValueTable( hvt, hwnd);
      WtkDeregisterSettingsDialog( hvt, hwnd);
      break;


   } // end switch

return WinDefDlgProc( hwnd, msg, mp1, mp2);

}

