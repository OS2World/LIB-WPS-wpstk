/****************************** Module Header ******************************\
*
* Module Name: _pmfddlg.c
*
* PM helper functions sample - file/directory dialog related code
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2004
*
* $Id: _pmfddlg.c,v 1.1 2008-12-22 18:12:51 cla Exp $
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
#define INCL_WTKBBH
#include <wtk.h>

#include "_pm.rch"
#include "_pmfddlg.h"

// ---------------------------------------------------------------------

static MRESULT EXPENTRY _DirDlgSubclassProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
         PDIRDLG        pdd = WinQueryWindowPtr( hwnd, QWL_USER);

switch (msg)
   {
   case WM_COMMAND:
      switch (LONGFROMMP( mp1))
         {
         case DID_CANCEL:
            WinAlarm( HWND_DESKTOP, WA_ERROR);
            break;
         }
      break;
   }

return WtkDefDirDlgProc( hwnd, msg, mp1, mp2);
}


// -----------------------------------------------------------------------------

ULONG LaunchDirDlg( HWND hwnd, HWND hwndBubbleHelp,
                    BOOL fPreload, BOOL fLoadCustomDialog)

{
         BOOL           ulResult = MBID_ERROR;
         BOOL           fResult;
         DIRDLG         dd;

         CHAR           szMessage[ _MAX_PATH];
         PSZ            pszButtonCode;

// stop bubble help
WtkDeactivateBubbleHelp( hwndBubbleHelp);

do
   {
   // setup data
   memset( &dd, 0, sizeof( dd));
   dd.cbSize     = sizeof( dd);
   dd.fl         = WTK_DDS_CENTER | WTK_DDS_HELPBUTTON | WTK_DDS_FORCEDIALOGID;
   dd.pszTitle   = "Directory test dialog";
   dd.pfnDlgProc = _DirDlgSubclassProc;
   strcpy( dd.szFullDir, "?:\\OS2");

   // select help panel and store it in user reserved field
   if (fPreload)
      {
      dd.ulUser  = (fLoadCustomDialog) ?
                      IDPNL_CUSTDIRDLGPRELOAD :
                      IDPNL_DEFDIRDLGPRELOAD;
      }
   else
      dd.ulUser  = IDPNL_DEFDIRDLG;

   // preload volume info
   if (fPreload)
      dd.fl |= WTK_DDS_PRELOAD_VOLINFO;

   // use custom dialog
   if (fLoadCustomDialog)
      {
      dd.fl      |= WTK_DDS_CUSTOM;
      dd.usDlgId  = IDDLG_CUSTDIRDLGPRELOAD;
      }
   else if (fPreload)
      dd.usDlgId  = IDDLG_DEFDIRDLGPRELOAD;
   else
      dd.usDlgId  = IDDLG_DEFDIRDLG;

   // launch dialog
   fResult = WtkDirDlg( HWND_DESKTOP, hwnd, &dd);

   if (!fResult)
      sprintf( szMessage, "error launching WtkDirDlg\n");
   else if (dd.lReturn == DID_OK)
      sprintf( szMessage, "directory is: %s\n", dd.szFullDir);
   else
      sprintf( szMessage, "error occurred!\n");

   switch (dd.lReturn)
      {
      case DID_OK:     pszButtonCode = "(DID_OK)";     break;
      case DID_CANCEL: pszButtonCode = "(DID_CANCEL)"; break;
      case DID_ERROR:  pszButtonCode = "(DID_ERROR)";  break;
      default:         pszButtonCode = "";             break;
      }

   sprintf( &szMessage[ strlen( szMessage)],
            "\015\015"
            "return code is: %u %s\015"
            "OS/2 reason code is: %u\015",
            dd.lReturn, pszButtonCode, dd.rc);

   WinMessageBox( HWND_DESKTOP, hwnd, szMessage, "Directory Dialog Result",
                  IDDLG_MSGBOX_RESULTDIRDLG, MB_MOVEABLE | MB_OK | MB_HELP);

   // on error return error code
   if (!fResult)
      break;

   // hand over result
   ulResult = dd.lReturn;

   } while (FALSE);


// restart bubble help
WtkActivateBubbleHelp( hwndBubbleHelp);
return ulResult;
}

// -----------------------------------------------------------------------------

ULONG LaunchFileDlg( HWND hwnd, HWND hwndBubbleHelp)

{
         BOOL           ulResult = MBID_ERROR;
         BOOL           fResult;
         FILEDLG        fd;

         CHAR           szMessage[ _MAX_PATH];
         PSZ            pszButtonCode;

// stop bubble help
WtkDeactivateBubbleHelp( hwndBubbleHelp);

do
   {
   // setup data
   memset( &fd, 0, sizeof( fd));
   fd.cbSize     = sizeof( fd);
   fd.fl         = FDS_CENTER | FDS_HELPBUTTON | FDS_OPEN_DIALOG;
   fd.pszTitle   = "System File test dialog";
   strcpy( fd.szFullFile, "C:\\*");

   // launch dialog
   fResult = WinFileDlg( HWND_DESKTOP, hwnd, &fd);

   if (!fResult)
      sprintf( szMessage, "error launching WinFileDlg\n");
   else if (fd.lReturn == DID_OK)
      sprintf( szMessage, "file is: %s\n", fd.szFullFile);
   else
      sprintf( szMessage, "error occurred!\n");

   switch (fd.lReturn)
      {
      case DID_OK:     pszButtonCode = "(DID_OK)";     break;
      case DID_CANCEL: pszButtonCode = "(DID_CANCEL)"; break;
      case DID_ERROR:  pszButtonCode = "(DID_ERROR)";  break;
      default:         pszButtonCode = "";             break;
      }

   sprintf( &szMessage[ strlen( szMessage)],
            "\015\015"
            "return code is: %u %s\015"
            "WinFileDlg error source code is: %u\015",
            fd.lReturn, pszButtonCode, fd.lSRC);

   WinMessageBox( HWND_DESKTOP, hwnd, szMessage, "File Dialog Result",
                  IDDLG_MSGBOX_RESULTFILEDLG, MB_MOVEABLE | MB_OK | MB_HELP);

   // on error return error code
   if (!fResult)
      break;

   // hand over result
   ulResult = fd.lReturn;

   } while (FALSE);


// restart bubble help
WtkActivateBubbleHelp( hwndBubbleHelp);
return ulResult;
}

