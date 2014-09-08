/****************************** Module Header ******************************\
*
* Module Name: _pmnb.c
*
* PM helper functions sample - notebook control related code
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2004
*
* $Id: _pmnb.c,v 1.3 2008-12-22 18:23:34 cla Exp $
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
#define INCL_WTKFILEBMP
#include <wtk.h>

#include "_pmnb.h"
#include "_pm.rch"

typedef struct _PAGEDATA {
         BOOL           fOkButtonEnabled;
} PAGEDATA, *PPAGEDATA;

static   PSZ            pszBitmapfileNumbers = "numbers.bmp";

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

static VOID _initGuiControls( HWND hwnd, PVOID pvData)
{
         ULONG          ulWindowId =  WinQueryWindowUShort( hwnd, QWS_ID);
         PPAGEDATA      ppd = pvData;

// setup controls
switch (ulWindowId)
   {
   case IDDLG_DEFNOTEBOOK_PAGE1:
      break;

   case IDDLG_DEFNOTEBOOK_PAGE2:
      break;

   case IDDLG_DEFNOTEBOOK_PAGE3:
      break;

   case IDDLG_OKNOTEBOOK_PAGE1:
      WtkEnableNotebookButton( hwnd, IDPBS_OK, ppd->fOkButtonEnabled);
      break;

   case IDDLG_OKNOTEBOOK_PAGE2:
      WtkEnableNotebookButton( hwnd, IDPBS_OK, ppd->fOkButtonEnabled);
      break;

   case IDDLG_OKNOTEBOOK_PAGE3:
      WtkEnableNotebookButton( hwnd, IDPBS_OK, ppd->fOkButtonEnabled);
      break;

   case IDDLG_DEFNOTEBOOK_PAGE4:
   case IDDLG_OKNOTEBOOK_PAGE4:
      {

               ULONG          aulBitmapId[]   = {IDSTC_BITMAP16, IDSTC_BITMAP32, IDSTC_BITMAP64};
               ULONG          aulBitmapSize[] = {16, 32, 64};
#define BITMAP_COUNT (sizeof( aulBitmapId) / sizeof( ULONG))

               APIRET         rc;
               ULONG          i;
               HPS            hps;
               ULONG          ulStyle;
               HWND           hwndStatic;
               HBITMAP        ahbmp[ BITMAP_COUNT];

      // modify static text windows to display the bitmaps of numbers.bmp
      // this file must reside in the current directory
      for (i = 0; i < BITMAP_COUNT; i++)
         {
         // get handle to static window
         hwndStatic = WinWindowFromID( hwnd, aulBitmapId[ i]);

         // load bitmap
         hps = WinGetPS( hwndStatic);
         if (hps)
            {
            rc = WtkLoadBitmapFromFile( hps, pszBitmapfileNumbers, &ahbmp[ i],
                                        aulBitmapSize[ i], aulBitmapSize[ i], TRUE);
            if (rc == NO_ERROR)
               {
               // switch static window style to bitmap
               ulStyle = WinQueryWindowULong( hwndStatic, QWL_STYLE);
               WinSetWindowULong( hwndStatic, QWL_STYLE, (ulStyle & ~SS_TEXT) | SS_BITMAP);

               // set bitmap handle
               WinSendMsg( hwndStatic, SM_SETHANDLE, MPFROMLONG( ahbmp[ i]), 0);

               }

            // cleanup
            WinReleasePS( hps);
            }


         }

      }
      break;

   }

return;
}

// --------------------------------------------------------------------------

static VOID _enableCheckbox( HWND hwnd, ULONG ulCurrentWindowId, ULONG ulPageWindowId,
                             ULONG ulButtonId, BOOL fEnable)
{
if (ulCurrentWindowId != ulPageWindowId)
   WinSendDlgItemMsg( WinWindowFromID( WinQueryWindow( hwnd, QW_PARENT), ulPageWindowId),
                      ulButtonId, BM_SETCHECK,
                      (MPARAM) fEnable, 0L);
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

static VOID _controlGuiValues( HWND hwnd, PVOID pvData, MPARAM mp1, MPARAM mp2)
{
         ULONG          ulWindowId =  WinQueryWindowUShort( hwnd, QWS_ID);
         PPAGEDATA      ppd = pvData;

// control dialog items

switch( SHORT1FROMMP( mp1))
   {

   // same checkbox on all pages of 2nd sample notebook
   case IDCHB_ENABLEOK:
      {
               HWND           hwndOkButton = NULLHANDLE;

      // enable ok button on all pages
      // - for that use a WtkQueryNotebookButton loop and WinEnableWindow
      // - can alse be done using a single call to WtkEnableNotebookButton
      ppd->fOkButtonEnabled = (BOOL) WinSendDlgItemMsg( hwnd, IDCHB_ENABLEOK, BM_QUERYCHECK, 0L, 0L);
      do
         {
         hwndOkButton = WtkQueryNotebookButton( hwnd, IDPBS_OK, hwndOkButton);
         WinEnableWindow( hwndOkButton, ppd->fOkButtonEnabled);
         } while (hwndOkButton);

      // as this checkbox is on every page, set all checkboxes to the same state
      _enableCheckbox( hwnd, ulWindowId, IDDLG_OKNOTEBOOK_PAGE1,  IDCHB_ENABLEOK, ppd->fOkButtonEnabled);
      _enableCheckbox( hwnd, ulWindowId, IDDLG_OKNOTEBOOK_PAGE2,  IDCHB_ENABLEOK, ppd->fOkButtonEnabled);
      _enableCheckbox( hwnd, ulWindowId, IDDLG_OKNOTEBOOK_PAGE21, IDCHB_ENABLEOK, ppd->fOkButtonEnabled);
      _enableCheckbox( hwnd, ulWindowId, IDDLG_OKNOTEBOOK_PAGE22, IDCHB_ENABLEOK, ppd->fOkButtonEnabled);
      _enableCheckbox( hwnd, ulWindowId, IDDLG_OKNOTEBOOK_PAGE3,  IDCHB_ENABLEOK, ppd->fOkButtonEnabled);
      _enableCheckbox( hwnd, ulWindowId, IDDLG_OKNOTEBOOK_PAGE4,  IDCHB_ENABLEOK, ppd->fOkButtonEnabled);
      }
      break;
   }

return;
}

// --------------------------------------------------------------------------

static VOID _saveGuiValues( HWND hwnd, PVOID pvData)
{
         ULONG          ulWindowId =  WinQueryWindowUShort( hwnd, QWS_ID);


// save all values of GUI controls
switch (ulWindowId)
   {
   case IDDLG_DEFNOTEBOOK_PAGE1:
      break;

   case IDDLG_DEFNOTEBOOK_PAGE2:
      break;

   case IDDLG_DEFNOTEBOOK_PAGE21:
      break;

   case IDDLG_DEFNOTEBOOK_PAGE22:
      break;

   case IDDLG_DEFNOTEBOOK_PAGE3:
      break;

   case IDDLG_OKNOTEBOOK_PAGE1:
      break;

   case IDDLG_OKNOTEBOOK_PAGE2:
      break;

   case IDDLG_OKNOTEBOOK_PAGE21:
      break;

   case IDDLG_OKNOTEBOOK_PAGE22:
      break;

   case IDDLG_OKNOTEBOOK_PAGE3:
      break;
   }

return;
}

// --------------------------------------------------------------------------

static VOID _restoreGuiValues( HWND hwnd, PVOID pvData)
{
         ULONG          ulWindowId =  WinQueryWindowUShort( hwnd, QWS_ID);

// restore all values to GUI controls
switch (ulWindowId)
   {
   case IDDLG_DEFNOTEBOOK_PAGE1:
      break;

   case IDDLG_DEFNOTEBOOK_PAGE2:
      break;

   case IDDLG_DEFNOTEBOOK_PAGE21:
      break;

   case IDDLG_DEFNOTEBOOK_PAGE22:
      break;

   case IDDLG_DEFNOTEBOOK_PAGE3:
      break;

   case IDDLG_OKNOTEBOOK_PAGE1:
      break;

   case IDDLG_OKNOTEBOOK_PAGE2:
      break;

   case IDDLG_OKNOTEBOOK_PAGE21:
      break;

   case IDDLG_OKNOTEBOOK_PAGE22:
      break;

   case IDDLG_OKNOTEBOOK_PAGE3:
      break;
   }

return;
}

// -----------------------------------------------------------------------------

static VOID _defaultGuiValues( HWND hwnd, PVOID pvData)
{
         ULONG          ulWindowId =  WinQueryWindowUShort( hwnd, QWS_ID);

// reset controls
switch (ulWindowId)
   {
   case IDDLG_DEFNOTEBOOK_PAGE1:
      break;

   case IDDLG_DEFNOTEBOOK_PAGE2:
      break;

   case IDDLG_DEFNOTEBOOK_PAGE21:
      break;

   case IDDLG_DEFNOTEBOOK_PAGE22:
      break;

   case IDDLG_DEFNOTEBOOK_PAGE3:
      break;

   case IDDLG_OKNOTEBOOK_PAGE1:
      break;

   case IDDLG_OKNOTEBOOK_PAGE2:
      break;

   case IDDLG_OKNOTEBOOK_PAGE21:
      break;

   case IDDLG_OKNOTEBOOK_PAGE22:
      break;

   case IDDLG_OKNOTEBOOK_PAGE3:
      break;
      break;
   }

return;
}

// -----------------------------------------------------------------------------

ULONG LaunchNotebook( HWND hwnd, HWND hwndBubbleHelp, PSZ pszHelpFile, ULONG ulNotebookType)
{
         ULONG          ulResult = MBID_ERROR;
         ULONG          i;

         ULONG          ulFrameId;
         ULONG          ulPageCount;
         PULONG         paulPageId;

         CHAR           szStatusMask[ 64];

static   ULONG          aulDefNotebookPageId[] = { IDDLG_DEFNOTEBOOK_PAGE1,
                                                   IDDLG_DEFNOTEBOOK_PAGE2,
                                                   IDDLG_DEFNOTEBOOK_PAGE21,
                                                   IDDLG_DEFNOTEBOOK_PAGE22,
                                                   IDDLG_DEFNOTEBOOK_PAGE3,
                                                   IDDLG_DEFNOTEBOOK_PAGE4};
static   ULONG          aulOkNotebookPageId[] =  { IDDLG_OKNOTEBOOK_PAGE1,
                                                   IDDLG_OKNOTEBOOK_PAGE2,
                                                   IDDLG_OKNOTEBOOK_PAGE21,
                                                   IDDLG_OKNOTEBOOK_PAGE22,
                                                   IDDLG_OKNOTEBOOK_PAGE3,
                                                   IDDLG_OKNOTEBOOK_PAGE4};
static   ULONG          aulHelpPanelId[]      =  { IDPNL_NOTEBOOKPAGE1,
                                                   IDPNL_NOTEBOOKPAGE2,
                                                   IDPNL_NOTEBOOKPAGE21,
                                                   IDPNL_NOTEBOOKPAGE22,
                                                   IDPNL_NOTEBOOKPAGE3,
                                                   IDPNL_NOTEBOOKPAGE4};

#define DEFNOTEBOOK_PAGECOUNT (sizeof( aulDefNotebookPageId) / sizeof( ULONG))
#define OKNOTEBOOK_PAGECOUNT (sizeof( aulOkNotebookPageId) / sizeof( ULONG))

         NOTEBOOKPAGEINFO anpi[ DEFNOTEBOOK_PAGECOUNT]; // make sure that this number is large enough !
         NOTEBOOKINFO   ni;

         PAGEDATA       pd;

// stop bubble help
WtkDeactivateBubbleHelp( hwndBubbleHelp);

do
   {
   // load subtitle mask
    WinLoadString( WinQueryAnchorBlock( HWND_DESKTOP), NULLHANDLE,
                       IDSTR_PAGESTATUS, sizeof( szStatusMask), szStatusMask);

   // distinct between different notebooks
   switch (ulNotebookType)
      {
      case TYPE_DEFNOTEBOOK:
         ulFrameId   = IDDLG_DEFNOTEBOOK;
         paulPageId  = aulDefNotebookPageId;
         ulPageCount = DEFNOTEBOOK_PAGECOUNT;
         break;

      case TYPE_OKNOTEBOOK:
         ulFrameId   = IDDLG_OKNOTEBOOK;
         paulPageId  = aulOkNotebookPageId;
         ulPageCount = OKNOTEBOOK_PAGECOUNT;
         break;
      }

   // prepare notebook page info
   memset( anpi, 0, sizeof( anpi));
   for (i = 0; i < ulPageCount; i++, paulPageId++)
      {
      // determine notebook style
      switch (*paulPageId)
         {
         case IDDLG_DEFNOTEBOOK_PAGE21:
         case IDDLG_DEFNOTEBOOK_PAGE22:
         case IDDLG_OKNOTEBOOK_PAGE21:
         case IDDLG_OKNOTEBOOK_PAGE22:
            anpi[ i].usPageStyle   = BKA_MINOR;
            break;

         default:
            anpi[ i].usPageStyle = BKA_MAJOR;
            break;
         }

      anpi[ i].ulDlgResId        = *paulPageId;
      anpi[ i].usPageStyle      |= BKA_AUTOPAGESIZE;
      anpi[ i].usPageOrder       = BKA_LAST;
      anpi[ i].pszTabText        = NULL;
      anpi[ i].pszMinorTabText   = NULL;
      anpi[ i].pszStatusText     = NULL;

      if (ulNotebookType == TYPE_OKNOTEBOOK)
         {
         anpi[ i].ulOkButtonId     = IDPBS_OK;
         anpi[ i].ulCancelButtonId = IDPBS_CANCEL;
         anpi[ i].ulFocusId        = IDPBS_CANCEL;
         }
      else
         anpi[ i].ulFocusId        = IDPBS_UNDO;

      anpi[ i].ulHelpPanel       = aulHelpPanelId[ i];
      anpi[ i].ulUndoButtonId    = IDPBS_UNDO;
      anpi[ i].ulDefaultButtonId = IDPBS_DEFAULT;
      anpi[ i].ulHelpButtonId    = IDPBS_HELP;
      }

   // prepare notebook
   memset( &ni, 0, sizeof( ni));
   ni.cbFix             = sizeof( NOTEBOOKINFO);
   ni.hwndParent        = HWND_DESKTOP;
   ni.hwndOwner         = hwnd;
   ni.ulDlgResId        = ulFrameId;
   ni.ulNotebookResId   = IDNBK_NOTEBOOK;
   ni.ulSysIconId       = 0;
   ni.hmodResource      = NULLHANDLE;
   ni.pszHelpTitle      = "Help";
   ni.pszStatusMask     = szStatusMask;
   ni.pvData            = &pd;
   ni.ulNbPageCount     = ulPageCount;
   ni.panpi             = anpi;
   ni.pfnwcmdInitialize = _initGuiControls;
   ni.pfnwcntControl    = _controlGuiValues;
   ni.pfnwcmdSave       = _saveGuiValues;
   ni.pfnwcmdRestore    = _restoreGuiValues;
   ni.pfnwcmdDefault    = _defaultGuiValues;
   ni.pszHelpLibrary    = pszHelpFile;
   ni.fUndoAllPages     = FALSE;
   ni.fDefaultAllPages  = FALSE;

   memset( &pd, 0, sizeof( pd));
   ulResult = WtkNotebookDlg( &ni);

   } while (FALSE);

// restart bubble help
WtkActivateBubbleHelp( hwndBubbleHelp);
return ulResult;
}

