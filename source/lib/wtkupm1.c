/****************************** Module Header ******************************\
*
* Module Name: wtkupm1.c
*
* Source for PM utility functions.
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2000
*
* $Id: wtkupm1.c,v 1.26 2008-10-19 23:43:57 cla Exp $
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

#define COMPILE_HELPDPRINTF         0
#define COMPILE_DUMPNOTEBOOKWINDOWS 0

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
#include "wtkuctl.h"
#include "wpstk.ih"

typedef struct _NOTEBOOKDATA {
         HWND           hwndFrame;
         HWND           hwndNotebook;
         HPOINTER       hptrSysIcon;
         PNOTEBOOKINFO pni;
         BOOL           fHelpInstanceCreated;
         BOOL           fNotebookSpinUsed;
} NOTEBOOKDATA, *PNOTEBOOKDATA;

typedef struct _NOTEBOOKPAGEDATA {
         ULONG          ulPageIndex;
         HWND           hwndFrame;
         HWND           hwndNotebook;
         PNOTEBOOKINFO  pni;
         PNOTEBOOKPAGEINFO pnpi;
} NOTEBOOKPAGEDATA, *PNOTEBOOKPAGEDATA;


#define WM_USER_SETBUTTONFOCUS (WM_USER + 0x5000)
#define WM_USER_SAVEVALUES     (WM_USER + 0x5001)


#define NBPAGECOUNT(h)    (ULONG)WinSendMsg( h, BKM_QUERYPAGECOUNT, 0, MPFROMLONG( BKA_END))
#define NBPAGESTYLE(h,i)    (ULONG)WinSendMsg( h, BKM_QUERYPAGESTYLE,  MPFROMLONG( i), 0)
#define NBCURPAGEID(h)     (ULONG)WinSendMsg( h, BKM_QUERYPAGEID, 0, MPFROM2SHORT( BKA_TOP, 0))
#define NBFIRSTPAGEID(h)   (ULONG)WinSendMsg( h, BKM_QUERYPAGEID, 0, MPFROM2SHORT( BKA_FIRST, 0))
#define NBNPREVPAGEID(h,i)  (ULONG)WinSendMsg( h, BKM_QUERYPAGEID, MPFROMLONG( i), MPFROM2SHORT( BKA_PREV, 0))
#define NBNEXTPAGEID(h,i)  (ULONG)WinSendMsg( h, BKM_QUERYPAGEID, MPFROMLONG( i), MPFROM2SHORT( BKA_NEXT, 0))

#define NBTURNTOPAGE(h,i)  (ULONG)WinSendMsg( h, BKM_TURNTOPAGE, MPFROMLONG( i), 0)

#define NBCURPAGEHWND(h)   (HWND)WinSendMsg( h, BKM_QUERYPAGEWINDOWHWND, MPFROMLONG( NBCURPAGEID( h)), 0)
#define NBCURPAGEDATA(h)   WinQueryWindowPtr( NBCURPAGEHWND( h), QWL_USER)

#define NBPAGEHWND(h,i)   (HWND)WinSendMsg( h, BKM_QUERYPAGEWINDOWHWND, MPFROMLONG( i), 0)
#define NBPAGEDATA(h,i)   WinQueryWindowPtr( NBPAGEHWND( h,i), QWL_USER)

// NOTE:
//  - a page is only a major one, if style bit BKA_MAJOR is set
//  - the style bit BKA_MINOR is always set, so ignore it
#define NBPAGEISMAJOR(h,i) (NBPAGESTYLE(h,i) & BKA_MAJOR)
#define NBPAGEISMINOR(h,i) (!(NBPAGESTYLE(h,i) & BKA_MAJOR))

// -----------------------------------------------------------------------------

#if COMPILE_HELPDPRINTF
#define HELPDPRINTF(p) printf p
#else
#define HELPDPRINTF(p)
#endif

// -----------------------------------------------------------------------------

// old versions of the WtkNotebookDlg specific structs

typedef struct _NOTEBOOKPAGEINFO_1 {
  ULONG          ulDlgResId;        /* resource identifier of notebook page dialog             */
  USHORT         usPageStyle;       /* page style like BKA_MAJOR | BKA_AUTOPAGESIZE            */
  USHORT         usPageOrder;       /* page order like BKA_MAJOR or BKA_MINOR                  */
  PSZ            pszTabText;        /* text for major tab or NULL for using the window title   */
  PSZ            pszMinorTabText;   /* text for minor tab or NULL                              */
  PSZ            pszStatusText;     /* text of status text or NULL                             */
  ULONG          ulHelpPanel;       /* help panel id for the notebook page                     */
  ULONG          ulFocusId;         /* resource identifier of the control to get focus or zero */
  ULONG          ulUndoButtonId;    /* resource identifier of the undo button                  */
  ULONG          ulDefaultButtonId; /* resource identifier of the default button               */
  ULONG          ulOkButtonId;      /* resource identifier of the Ok button                    */
  ULONG          ulCancelButtonId;  /* resource identifier of the Cancel button                    */
  ULONG          ulHelpButtonId;    /* resource identifier of the help button                  */
} NOTEBOOKPAGEINFO_1, *PNOTEBOOKPAGEINFO_1;

typedef struct _NOTEBOOKINFO_1 {
  HWND           hwndParent;        /* handle of the parent window                              */
  HWND           hwndOwner;         /* handle of the owner window                               */
  ULONG          ulDlgResId;        /* resource identifier of notebook frame dialog             */
  ULONG          ulNotebookResId;   /* resource identifier of the notebook control              */
  HMODULE        hmodResource;      /* resource module handle or NULL                           */
  ULONG          ulSysIconId;       /* resource identifier of the system icon                   */
  PSZ            pszHelpTitle;      /* title of the help window                                 */
  PVOID          pvData;            /* address of user data to be passed to callbacks           */
  PSZ            pszHelpLibrary;    /* address of pathname of the help library                  */
  BOOL           fUndoAllPages;     /* flag to undo all pages when undo button is pressed       */
  BOOL           fDefaultAllPages;  /* flag to default all pages when default button is pressed */
  ULONG          ulNbPageCount;     /* number of pages                                          */
  PNOTEBOOKPAGEINFO_1 panpi_1;      /* address to list of notebook page infos                   */
  PFNWINCMD      pfnwcmdInitialize; /* function called on WM_INITDLG or NULL                    */
  PFNWINCNT      pfnwcntControl;    /* function called on WM_CONTROL/WM_COMMAND or NULL         */
  PFNWINCMD      pfnwcmdSave;       /* function called on WM_QUIT or NULL                       */
  PFNWINCMD      pfnwcmdRestore;    /* function called on WM_COMMAND for undo button or NULL    */
  PFNWINCMD      pfnwcmdDefault;    /* function called on WM_COMMAND for default button or NULL */
} NOTEBOOKINFO_1, *PNOTEBOOKINFO_1;

// -----------------------------------------------------------------------------
// this function returns zero, if the data version is uptodate, or a ptr to adapted data

static  PNOTEBOOKINFO _checkNotebookInfoDataVersion( PNOTEBOOKINFO pni)
{
         PNOTEBOOKINFO  pniResult = NULL;
         PNOTEBOOKPAGEINFO pnpiResult;

         ULONG          i;
         ULONG          ulDataSize;

         PNOTEBOOKINFO_1 pni_1;
         PNOTEBOOKPAGEINFO_1 pnpi_1;
do
   {
   // current version used, do nothing
   if (pni->cbFix == sizeof( NOTEBOOKINFO))
      break;

   // from here assume it is version 1 of the data

   // get memory for main info and page info array
   pni_1 = (PNOTEBOOKINFO_1) pni;
   ulDataSize = sizeof( NOTEBOOKINFO) + ( pni_1->ulNbPageCount * sizeof( NOTEBOOKPAGEINFO));
   pniResult = malloc( ulDataSize);
   if (!pniResult)
      break;
   memset( pniResult, 0, ulDataSize);

   // transfer main notebook info
   pniResult->cbFix = sizeof( NOTEBOOKINFO);
   pniResult->hwndParent          = pni_1->hwndParent;
   pniResult->hwndOwner           = pni_1->hwndOwner;
   pniResult->ulDlgResId          = pni_1->ulDlgResId;
   pniResult->ulNotebookResId     = pni_1->ulNotebookResId;
   pniResult->hmodResource        = pni_1->hmodResource;
   pniResult->ulSysIconId         = pni_1->ulSysIconId;
   pniResult->pszHelpTitle        = pni_1->pszHelpTitle;
   pniResult->pvData              = pni_1->pvData;
   pniResult->pszHelpLibrary      = pni_1->pszHelpLibrary;
   pniResult->fUndoAllPages       = pni_1->fUndoAllPages;
   pniResult->fDefaultAllPages    = pni_1->fDefaultAllPages;
   pniResult->ulNbPageCount       = pni_1->ulNbPageCount;
   pniResult->panpi               = (PNOTEBOOKPAGEINFO) ((PBYTE)pniResult + sizeof( NOTEBOOKINFO));
   pniResult->pfnwcmdInitialize   = pni_1->pfnwcmdInitialize;
   pniResult->pfnwcntControl      = pni_1->pfnwcntControl;
   pniResult->pfnwcmdSave         = pni_1->pfnwcmdSave;
   pniResult->pfnwcmdRestore      = pni_1->pfnwcmdRestore;
   pniResult->pfnwcmdDefault      = pni_1->pfnwcmdDefault;

   // transfer notebook page info array
   for (i = 0, pnpiResult = pniResult->panpi, pnpi_1 = pni_1->panpi_1;
        i < pni_1->ulNbPageCount;
        i++, pnpiResult++, pnpi_1++)
      {
      pnpiResult->cbFix             = sizeof( NOTEBOOKPAGEINFO);
      pnpiResult->ulDlgResId        = pnpi_1->ulDlgResId;
      pnpiResult->usPageStyle       = pnpi_1->usPageStyle;
      pnpiResult->usPageOrder       = pnpi_1->usPageOrder;
      pnpiResult->pszTabText        = pnpi_1->pszTabText;
      pnpiResult->pszMinorTabText   = pnpi_1->pszMinorTabText;
      pnpiResult->pszStatusText     = pnpi_1->pszStatusText;
      pnpiResult->ulHelpPanel       = pnpi_1->ulHelpPanel;
      pnpiResult->ulFocusId         = pnpi_1->ulFocusId;
      pnpiResult->ulUndoButtonId    = pnpi_1->ulUndoButtonId;
      pnpiResult->ulDefaultButtonId = pnpi_1->ulDefaultButtonId;
      pnpiResult->ulOkButtonId      = pnpi_1->ulOkButtonId;
      pnpiResult->ulCancelButtonId  = pnpi_1->ulCancelButtonId;
      pnpiResult->ulHelpButtonId    = pnpi_1->ulHelpButtonId;
      }

   } while (FALSE);

return pniResult;
}

// -----------------------------------------------------------------------------

static VOID _setNotebookPageAutoNumbering( HWND hwndNotebook, PULONG paulNumbering,
                                           ULONG ulPageCount, PSZ pszStatusMask)
{

         ULONG          i;
         PULONG         pulPageId;
         CHAR           szStatusText[ 64];

do
   {
   // check parms
   if (!paulNumbering)
      break;

   for (i = 0, pulPageId = paulNumbering;
        i < ulPageCount;
        i++, pulPageId++)
      {
      // determine text and set it
      sprintf( szStatusText, pszStatusMask, i + 1, ulPageCount);
      WinSendMsg( hwndNotebook, BKM_SETSTATUSLINETEXT,
                  MPFROMLONG( *pulPageId), MPFROMP( szStatusText));
      }

   } while (FALSE);

return;
}

// -----------------------------------------------------------------------------

/*
@@WtkCreateHelpInstanceWithTable@SYNTAX
This function creates a help instance using a help table and associates it to
the specified window.
The specified window must meet some
:link reftype=hd viewport dependent refid=RM_WtkCreateHelpInstanceWithTable.requirements:elink..
When a help table is not required, as an alternative
:link reftype=hd refid=WtkCreateHelpInstance.WtkCreateHelpInstance:elink. may be used.

@@WtkCreateHelpInstanceWithTable@PARM@hwnd@in
Handle of the window to be associated with the newly created help instance.

@@WtkCreateHelpInstanceWithTable@PARM@pszWindowTitle@in
Address of the title of the help window.

@@WtkCreateHelpInstanceWithTable@PARM@pszHelpLibrary@in
Address of the pathname of the help library file containing the requested panel.

@@WtkCreateHelpInstanceWithTable@PARM@hmodResource@in
Handle of the module, holding the PM resource for the helptable resource
:p.
The handle may have the following values:
:parml compact.
:pt.NULLHANDLE
:pd.if the parameter pht is set to load a HELPTABLE resource, it
is loaded from the PM resource of the current executable.
:pt.any other value
:pd.if the parameter pht is set to load a HELPTABLE resource,
it is a handle is obtained by a call to DosLoadModule or WinLoadLibrary,
loading the module holding the resource.
:eparml.

@@WtkCreateHelpInstanceWithTable@PARM@pht@in
Address of the HELPTABLE structure to set up the help table.

@@WtkCreateHelpInstanceWithTable@RETURN
Success indicator.
:parml compact.
:pt.TRUE
:pd.Help instance could be created.
:pt.FALSE
:pd.An error occurred.
:eparml.

@@WtkCreateHelpInstanceWithTable@REMARKS
This API creates a help instance, using a help table and associates it with the window
specified in the parameter :hp2.hwnd:ehp2.. This help instance can be destroyed with a call to
:link reftype=hd refid=WtkDestroyHelpInstance.WtkDestroyHelpInstance:elink., specifying the same hwnd
as parameter.
:p.
:note.
:ul compact.
:li. the specified window must have a frame window in the parent window chain and may
not yet have yet a help manager window associated to it.
:eul.

@@
*/

BOOL APIENTRY WtkCreateHelpInstanceWithTable( HWND hwnd, PSZ pszWindowTitle, PSZ pszHelpLibrary,
                                              HMODULE hmodResource, PHELPTABLE pht)
{
         BOOL           fResult = FALSE;
         APIRET         rc = NO_ERROR;
         HELPINIT       hi;
         HWND           hwndHelp = WinQueryHelpInstance( hwnd);

do
   {
   // if help instance already associated, exit
   if (hwndHelp)
      {
      HELPDPRINTF(( "%s: help instance already associated to window 0x%08x\n", __FUNCTION__, hwnd));
      break;
      }

   // create help instance
   memset( &hi, 0, sizeof( hi));
   hi.cb                  = sizeof( hi);
   hi.phtHelpTable        = pht;
   hi.hmodHelpTableModule = hmodResource;
   hi.pszHelpWindowTitle  = pszWindowTitle;
   hi.pszHelpLibraryName  = pszHelpLibrary;
   hwndHelp = WinCreateHelpInstance( WinQueryAnchorBlock( HWND_DESKTOP), &hi);
   HELPDPRINTF(( "%s: created hwndHelp 0x%08x\n", __FUNCTION__, hwndHelp));
   if (!hwndHelp)
      break;

   // attach it to this window
   fResult = WinAssociateHelpInstance( hwndHelp, hwnd);
   HELPDPRINTF(( "%s: associated hwndhwndHelp 0x%08x to hwnd 0x%08x, result %u\n", __FUNCTION__,
                 hwndHelp, hwnd, fResult));

   if (!fResult)
      {
      rc = ERRORIDERROR( WinGetLastError( WinQueryAnchorBlock( HWND_DESKTOP)));
      HELPDPRINTF(( "%s: last error is %u/0x%04x\n", __FUNCTION__, rc, rc));
      WinDestroyHelpInstance( hwndHelp);
      }

   } while (FALSE);

return fResult;
}

// -----------------------------------------------------------------------------

/*
@@WtkCreateHelpInstance@SYNTAX
This function creates a help instance and associates it to the specified window.
The specified window must meet some
:link reftype=hd viewport dependent refid=RM_WtkCreateHelpInstance.requirements:elink..

@@WtkCreateHelpInstance@PARM@hwnd@in
Handle of the window to be associated with the newly created help instance.

@@WtkCreateHelpInstance@PARM@pszWindowTitle@in
Address of the title of the help window.

@@WtkCreateHelpInstance@PARM@pszHelpLibrary@in
Address of the pathname of the help library file containing the requested panel.

@@WtkCreateHelpInstance@RETURN
Success indicator.
:parml compact.
:pt.TRUE
:pd.Help instance could be created.
:pt.FALSE
:pd.An error occurred.
:eparml.

@@WtkCreateHelpInstance@REMARKS
This API creates a help instance and associates it with the window specified in the parameter
:hp2.hwnd:ehp2.. This help instance can be destroyed with a call to
:link reftype=hd refid=WtkDestroyHelpInstance.WtkDestroyHelpInstance:elink., specifying the same hwnd
as parameter.
:p.
:note.
:ul compact.
:li. the specified window must have a frame window in the parent window chain and may
not yet have yet a help manager window associated to it.
:li.the created help instance does not use a help table in order to let the
Help Manager automatically load a panel for the current application window. In
order to let the help instance display a help panel, the application needs to
send an appropriate HM_DISPLAY_HELP message to the help instance or call
:link reftype=hd refid=WtkDisplayHelpPanel.WtkDisplayHelpPanel:elink. when the
Presentation Manager issues a WM_HELP message.
:eul.

@@
*/

BOOL APIENTRY WtkCreateHelpInstance( HWND hwnd, PSZ pszWindowTitle, PSZ pszHelpLibrary)
{
return WtkCreateHelpInstanceWithTable( hwnd, pszWindowTitle, pszHelpLibrary, NULLHANDLE, NULL);
}

// ---------------------------------------------------------------------------

/*
@@WtkDisplayHelpPanel@SYNTAX
This function displays a requested help panel from a previously created help instance.

@@WtkDisplayHelpPanel@PARM@hwnd@in
Handle of the window being previously associated to a help instance.

@@WtkDisplayHelpPanel@PARM@ulHelpPanel@in
Identifier of the panel to be displayed.

@@WtkDisplayHelpPanel@RETURN
Success indicator.
:parml compact.
:pt.TRUE
:pd.Help panel could be displayed
:pt.FALSE
:pd.An error occurred.
:eparml.

@@WtkDisplayHelpPanel@REMARKS
Before launching the requested panel, WtkDisplayHelpPanel checks if
a help instance exists that is associated to one of the parent frame
windows within the window chain. If so, it sends a message to this
help instance closing an open help window. If this is not done,
otherwise a second help windows can be opened.
@@
*/

BOOL APIENTRY WtkDisplayHelpPanel( HWND hwnd, ULONG ulHelpPanel)
{
         BOOL           fResult = FALSE;
         APIRET         rc = NO_ERROR;
         HWND           hwndHelp;

         HWND           hwndParent;
         HWND           hwndHelpParent;
         HWND           hwndDesktop = WinQueryDesktopWindow( WinQueryAnchorBlock( HWND_DESKTOP), 0);

do
   {
   // query the help instance
   hwndHelp = WinQueryHelpInstance( hwnd);
   if (!hwndHelp)
      break;

   // query help instance from parent or owner
   // (this follows the search scheme of WinQueryHelpInstance !)
   // If this exists and is not the same instance,
   // make sure that window of this gets closed
   hwndParent = WinQueryWindow( hwnd, QW_PARENT);
   if ((!hwndParent) || (hwndParent == hwndDesktop))
      hwndParent = WinQueryWindow( hwnd, QW_OWNER);
   if (hwndParent)
      {
      hwndHelpParent = WinQueryHelpInstance( hwndParent);
      if ((hwndHelpParent) && (hwndHelpParent != hwndHelp))
         WinSendMsg( hwndHelpParent, HM_DISMISS_WINDOW, 0, 0);
      }

   // now display the requested panel
   rc = (APIRET) WinSendMsg( hwndHelp, HM_DISPLAY_HELP, MPFROMP( ulHelpPanel), MPFROMSHORT( HM_RESOURCEID));
   fResult = (rc == NO_ERROR);
   if (!fResult)
      WtkSetErrorInfo( rc);

   } while (FALSE);

return fResult;
}

// ---------------------------------------------------------------------

/*
@@WtkDestroyHelpInstance@SYNTAX
This function destroys a help instance
:link reftype=hd viewport dependent refid=RM_WtkDestroyHelpInstance.associated to the specified window:elink..

@@WtkDestroyHelpInstance@PARM@hwnd@in
Handle of the window that is associated with the help panel to be destroyed.

@@WtkDestroyHelpInstance@RETURN
Success indicator.
:parml compact.
:pt.TRUE
:pd.Help panel could be destroyed
or no help instance was associated to
the window specified in parameter :hp2.hwnd:ehp2..
:pt.FALSE
:pd.An error occurred
:eparml.

@@WtkDestroyHelpInstance@REMARKS
This API is useful to destroy a help instance created with
:link reftype=hd refid=WtkDisplayHelpPanel.WtkDisplayHelpPanel.:elink..

@@
*/

BOOL APIENTRY WtkDestroyHelpInstance( HWND hwnd)
{
         BOOL           fResult = TRUE;
         HWND           hwndHelp = WinQueryHelpInstance( hwnd);

do
   {
   // if no help window given, exit
   if (!hwndHelp)
      {
      HELPDPRINTF(( "%s: could not determine help instance for 0x%08x\n", __FUNCTION__, hwnd));
      break;
      }

   // destroy help incance
   fResult = WinDestroyHelpInstance( hwndHelp);
   HELPDPRINTF(( "%s: destroyed hwndHelp 0x%08x, result %u\n", __FUNCTION__, hwndHelp, fResult));
   if (!fResult)
      break;

   // remove association
   fResult = WinAssociateHelpInstance( NULLHANDLE, hwnd);
   HELPDPRINTF(( "%s: deassociated help instance from hwnd 0x%08x, result %u\n", __FUNCTION__,
                 hwnd, fResult));

   } while (FALSE);

return fResult;
}

// =====================================================================

static VOID _processNotebookPages( HWND hwnd, PFNWINCMD pfnwc)
{
         PNOTEBOOKPAGEDATA  pnpd = WinQueryWindowPtr( hwnd, QWL_USER);
         HAB            hab = WinQueryAnchorBlock( HWND_DESKTOP);
         ULONG          ulPageCount;
         ULONG          ulPageId;
         HWND           hwndPage;

do
   {
   if ((!pnpd) ||
       (!pfnwc))
      break;

   ulPageCount = NBPAGECOUNT( pnpd->hwndNotebook);
   ulPageId    = NBFIRSTPAGEID( pnpd->hwndNotebook);
   if (!ulPageId)
      break;

   while ((ulPageId) && (ulPageId != BOOKERR_INVALID_PARAMETERS))
      {
      // query page handle
      hwndPage = NBPAGEHWND( pnpd->hwndNotebook, ulPageId);

      // call window command function, if window already exists
      if ((hwndPage != BOOKERR_INVALID_PARAMETERS) &&
          (hwndPage != NULLHANDLE)                 &&
          (WinIsWindow( hab, hwndPage)))
         pfnwc( hwndPage, (PVOID) pnpd->pni->pvData);

      // next page
      ulPageId = NBNEXTPAGEID( pnpd->hwndNotebook, ulPageId);

      } // while (ulPageId)

   } while (FALSE);

return;
}

// - - - - - - - - - - - - - - - - - - - - - - -

static VOID _disableAllButtonFocus( HWND hwndNotebook)
{
         HENUM          henum = NULLHANDLE;
         HWND           hwnd;
         ULONG          ulStyle;

do
   {
   henum = WinBeginEnumWindows( hwndNotebook);
   hwnd = WinGetNextWindow( henum);
   while (hwnd)
      {
      if (WtkQueryClassIndex( hwnd) == WC_BUTTON)
         {
         ulStyle = WinQueryWindowULong( hwnd, QWL_STYLE);

         WinSetWindowULong( hwnd, QWL_STYLE, ulStyle & ~BS_DEFAULT);
         }
      hwnd = WinGetNextWindow( henum);
      }
   } while (FALSE);

if (henum) WinEndEnumWindows( henum);
return;
}

// - - - - - - - - - - - - - - - - - - - - - - -


static MRESULT EXPENTRY _NotebookPageDlgPageProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
         BOOL           fResult;
         APIRET         rc;

         PNOTEBOOKPAGEDATA  pnpd = WinQueryWindowPtr( hwnd, QWL_USER);
         PNOTEBOOKINFO      pni  = (pnpd) ? pnpd->pni : NULL;
         PNOTEBOOKPAGEINFO  pnpi = (pnpd) ? pnpd->pnpi : NULL;

switch (msg)
   {

   case WM_INITDLG:
      {


      // save window data ptr
      pnpd = (PNOTEBOOKPAGEDATA) mp2;
      WinSetWindowPtr( hwnd, QWL_USER, pnpd);
      if (!pnpd)
         break;

      // check for subvalues
      pni  = pnpd->pni;
      pnpi = pnpd->pnpi;
      if ((!pni) || (!pnpi))
         {
         WinDismissDlg( pnpd->hwndFrame, MBID_ERROR);
         break;
         }

      // initilaize controls
      if (pni->pfnwcmdInitialize)
         (pni->pfnwcmdInitialize)( hwnd, pnpd->pni->pvData);

      // setup controls by executing undo
      if (pnpd->pni->pfnwcmdRestore)
         (pnpd->pni->pfnwcmdRestore)( hwnd, pnpd->pni->pvData);

      if (pnpd->pnpi->ulFocusId)
         return (MRESULT) TRUE;
      else
         return (MRESULT) FALSE;
      }
      break; // case WM_INITDLG:

   // -------------------------------

   case WM_CONTROL:
      {
      if (!pnpd)
         break;

      if (pnpd->pni->pfnwcntControl)
         (pnpd->pni->pfnwcntControl)( hwnd, pnpd->pni->pvData, mp1, mp2);
      }
      break; // case WM_CONTROL:

   // -------------------------------

   case WM_COMMAND:
      if (!pnpd)
         break;

      {
               ULONG          ulButtonId = LONGFROMMP( mp1);
               ULONG          hwndControl;
               ULONG          ulWindowStyle;

      if (ulButtonId == pnpd->pnpi->ulUndoButtonId)
         {
         if (pnpd->pni->pfnwcmdRestore)
            {
            if (pnpd->pni->fUndoAllPages)
               _processNotebookPages( hwnd, pnpd->pni->pfnwcmdRestore);
            else
               (pnpd->pni->pfnwcmdRestore)( hwnd, pnpd->pni->pvData);
            }
         }
      else if (ulButtonId == pnpd->pnpi->ulDefaultButtonId)
         {
         if (pnpd->pni->pfnwcmdDefault)
            {
            if (pnpd->pni->fDefaultAllPages)
               _processNotebookPages( hwnd, pnpd->pni->pfnwcmdDefault);
            else
               (pnpd->pni->pfnwcmdDefault)( hwnd, pnpd->pni->pvData);
            }
         }
      else if (ulButtonId == pnpd->pnpi->ulOkButtonId)
         {
         WinSendMsg( pnpd->hwndFrame, WM_USER_SAVEVALUES, 0, 0);
         WinDismissDlg( pnpd->hwndFrame, MBID_OK);
         }
      else if (ulButtonId == pnpd->pnpi->ulCancelButtonId)
         WinDismissDlg( pnpd->hwndFrame, MBID_CANCEL);
      else if (ulButtonId == pnpd->pnpi->ulHelpButtonId)
         WinPostMsg( hwnd, WM_HELP, 0, 0);
      else
         {
         // send BN_CLICKED notification for a pressed button
         // use WtkQueryNotebookButton to also catch non-standard BS_NOTEBOOKBUTTONs
         hwndControl = WtkQueryNotebookButton( hwnd, ulButtonId, NULLHANDLE);

         if ((pnpd->pni->pfnwcntControl) &&
             (WtkQueryClassIndex( hwndControl) == WC_BUTTON))
            (pnpd->pni->pfnwcntControl)( hwnd, pnpd->pni->pvData, MPFROM2SHORT( ulButtonId, BN_CLICKED), MPFROMLONG( hwndControl));
         }
      }
      return (MRESULT)0;

      break; // case WM_COMMAND:


   // -------------------------------

   case WM_CHAR:
      // swallow esc key
      if ((CHARMSG(&msg)->fs & KC_VIRTUALKEY) && (CHARMSG(&msg)->vkey & VK_ESC))
         return (MRESULT) TRUE;
      break; // case WM_CHAR:

   // -------------------------------

   case WM_HELP:
      WtkDisplayHelpPanel( pnpd->hwndFrame, pnpd->pnpi->ulHelpPanel);
      return (MRESULT) 0;
      break;

   } //switch (msg)

return WinDefDlgProc(hwnd,msg,mp1,mp2);
}


// - - - - - - - - - - - - - - - - - - - - - - -

static MRESULT EXPENTRY _NotebookDlgProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
         BOOL           fResult;
         APIRET         rc;

         PNOTEBOOKDATA  pnd = WinQueryWindowPtr( hwnd, QWL_USER);

switch (msg)
   {

   case WM_INITDLG:
      {
                     APIRET         rc;
                     ULONG          i;

                     USHORT         usPageStyle;
                     BOOL           fAutoNumbering = FALSE;

                     CHAR           szTabText[ 128];
                     CHAR           szStatusText[ 128];

                     HWND           hwndNotebookPage;
                     ULONG          ulPageId;
                     PSZ            pszTabText;
                     BOOKPAGEINFO   bpi;
                     PAGEINFO       pi;

                     PNOTEBOOKPAGEDATA pnpd;
                     PNOTEBOOKPAGEINFO pnpi;

                     PULONG         paulNumbering = NULL;
                     ULONG          ulNumberIndex = 0;

      // save window data ptr
      pnd = (PNOTEBOOKDATA) mp2;
      WinSetWindowPtr( hwnd, QWL_USER, pnd);
      if (!pnd)
         {
         WinDismissDlg( hwnd, MBID_ERROR);
         break;
         }
      if (!pnd->pni)
         {
         WinDismissDlg( hwnd, MBID_ERROR);
         break;
         }

      // set sysmenu icon
      if (pnd->pni->ulSysIconId)
         {
         pnd->hptrSysIcon = WinLoadPointer( HWND_DESKTOP, pnd->pni->hmodResource, pnd->pni->ulSysIconId);
         WinSendMsg( hwnd, WM_SETICON, (MPARAM) pnd->hptrSysIcon, 0L);
         }

      // autonumbering requested ?
      fAutoNumbering = ((pnd->pni->pszStatusMask) && (*pnd->pni->pszStatusMask));
      if (fAutoNumbering)
         paulNumbering = calloc( pnd->pni->ulNbPageCount, sizeof( ULONG));

      // store handle of frame and notebook
      pnd->hwndFrame = hwnd;
      pnd->hwndNotebook = WinWindowFromID( hwnd, pnd->pni->ulNotebookResId);

      if (!pnd->hwndNotebook)
         {
         WinDismissDlg( hwnd, MBID_ERROR);
         break;
         }

#if COMPILE_DUMPNOTEBOOKWINDOWS
      printf( "WtkNotebookDlg: frame hwnd 0x%08x\n", hwnd);
      printf( "WtkNotebookDlg: notebook hwnd 0x%08x\n", pnd->hwndNotebook);
#endif

      // process all pages
      for (i = 0, pnpi = pnd->pni->panpi; i < pnd->pni->ulNbPageCount; i++, pnpi++)
         {
         // allocate page memory
         pnpd = malloc( sizeof( NOTEBOOKPAGEDATA));
         if (!pnpd)
            break;
         memset( pnpd, 0, sizeof( NOTEBOOKPAGEDATA));
         pnpd->ulPageIndex  = i;
         pnpd->hwndNotebook = pnd->hwndNotebook;
         pnpd->hwndFrame    = hwnd;
         pnpd->pni          = pnd->pni;
         pnpd->pnpi         = pnpi;

         // determine page style
         usPageStyle = pnpi->usPageStyle;
         if (fAutoNumbering)
            usPageStyle |= BKA_STATUSTEXTON;

         // load page
         hwndNotebookPage = WinLoadDlg( pnd->hwndNotebook,
                                        pnd->hwndNotebook,
                                        _NotebookPageDlgPageProc,
                                        pnd->pni->hmodResource,
                                        pnpi->ulDlgResId,
                                        pnpd);
         rc = (hwndNotebookPage) ?
                 NO_ERROR :
                 ERRORIDERROR( WinGetLastError( WinQueryAnchorBlock( HWND_DESKTOP)));

         // insert page
         ulPageId = (ULONG) WinSendMsg( pnd->hwndNotebook, BKM_INSERTPAGE,
                                        (MPARAM) NULL,
                                        MPFROM2SHORT( usPageStyle, pnpi->usPageOrder));

         // associate dialog to notebook page
         fResult = (BOOL) WinSendMsg( pnd->hwndNotebook, BKM_SETPAGEWINDOWHWND,
                                      MPFROMLONG( ulPageId),
                                      MPFROMHWND( hwndNotebookPage));

         // get page name from dialog
         szTabText[ 0] = 0;
         WinQueryWindowText( hwndNotebookPage, sizeof( szTabText), szTabText);
         pszTabText = szTabText;

#if COMPILE_DUMPNOTEBOOKWINDOWS
      printf( "WtkNotebookDlg: page hwnd 0x%08x resid 0x%08x pageid 0x%08x %s \n",
               hwndNotebookPage, pnpi->ulDlgResId, ulPageId, szTabText);
#endif

         // override with major tab text, if given
         if ((pnpi->usPageStyle & BKA_MAJOR)  &&
            (pnpi->pszTabText)                &&
            (*pnpi->pszTabText))
            pszTabText = pnpi->pszTabText;

         // override with minor tab text, if given
         if ((pnpi->usPageStyle & BKA_MINOR) &&
             (pnpi->pszMinorTabText)         &&
             (*pnpi->pszMinorTabText))
            pszTabText = pnpi->pszMinorTabText;

         // set tab text only for
         // - major tab
         // - minor tab with tab text explicitely provided
         if ((pnpi->usPageStyle & BKA_MAJOR) ||
             ((pnpi->pszMinorTabText) &&
              (*pnpi->pszMinorTabText)))
            WinSendMsg( pnd->hwndNotebook, BKM_SETTABTEXT,
                        MPFROMLONG( ulPageId), (MPARAM) pszTabText);

         // set status text
         if ((pnpi->pszStatusText) &&
             (*pnpi->pszStatusText))
            WinSendMsg( pnd->hwndNotebook, BKM_SETSTATUSLINETEXT,
                        MPFROMLONG( ulPageId), MPFROMP( pnpi->pszStatusText));

         // always try to destroy titlebar of page
         WinDestroyWindow( WinWindowFromID( hwndNotebookPage, FID_TITLEBAR));

         // set status text if set
         if (pnpi->pszStatusText)
            WinSendMsg( pnd->hwndNotebook, BKM_SETSTATUSLINETEXT,
                        (MPARAM)ulPageId, MPFROMP( pnpi->pszStatusText));

         // check for auto numbering
         if ((fAutoNumbering) && (paulNumbering))
            {
            if (pnpi->usPageStyle & BKA_MAJOR)
               {
               if (ulNumberIndex)
                  // process auto numbering sequence finished
                  _setNotebookPageAutoNumbering( pnd->hwndNotebook, paulNumbering,
                                                 ulNumberIndex + 1, pnd->pni->pszStatusMask);

               // start it again
               ulNumberIndex = 0;
               *(paulNumbering + ulNumberIndex) = ulPageId;

               }
            else if (pnpi->usPageStyle & BKA_MINOR)
               {
               ulNumberIndex++;
               *(paulNumbering + ulNumberIndex) = ulPageId;
               }

            } // if ((fAutoNumbering) && (paulNumbering))

         } // for (i = 0, pnpi = pnd->pni->panpi; i < pnd->pni->ulNbPageCount; i++, pnpi++)

      // finish autonumbering
      if ((fAutoNumbering) && (paulNumbering) && (ulNumberIndex))
         // process last auto numbering sequence
         _setNotebookPageAutoNumbering( pnd->hwndNotebook, paulNumbering,
                                        ulNumberIndex + 1, pnd->pni->pszStatusMask);

      // cleanup
      if (paulNumbering)
         free( paulNumbering);

      // create help instance
      if (pnd->pni->pszHelpLibrary)
         pnd->fHelpInstanceCreated = WtkCreateHelpInstance( hwnd, pnd->pni->pszHelpTitle, pnd->pni->pszHelpLibrary);

      _disableAllButtonFocus( pnd->hwndNotebook);

      return (MRESULT) FALSE;
      }
      break; // case WM_INITDLG:

   // -------------------------------

   case WM_SYSCOMMAND:

      // save on exit, if no ok button
      // defined for the active page
      if (LONGFROMMP( mp1) == (ULONG)SC_CLOSE)
         {

#if SAVE_WITHOUT_OKBUTTON_ONLY
                  PNOTEBOOKPAGEDATA  pnpd;

         pnpd = NBCURPAGEDATA( pnd->hwndNotebook);

         if ((pnpd) && (!pnpd->pnpi->ulOkButtonId))
            WinSendMsg( hwnd, WM_USER_SAVEVALUES, 0, 0);
#else
         WinSendMsg( hwnd, WM_USER_SAVEVALUES, 0, 0);
#endif
         }

      break; // case WM_SYSCOMMAND:

   // -------------------------------

   case WM_USER_SAVEVALUES:
      {

            HWND           hwndPage;

      // determine hwnd of current page
      hwndPage = NBCURPAGEHWND( pnd->hwndNotebook);

      if (pnd->pni->pfnwcmdSave)
         _processNotebookPages( hwndPage, pnd->pni->pfnwcmdSave);
      }
      break;

   // -------------------------------

   case WM_USER_SETBUTTONFOCUS:
      {
               HWND           hwndPage;
               HWND           hwndControl;
               HWND           hwndButton;
               PNOTEBOOKPAGEDATA  pnpd;

      // set default button
      hwndPage = NBCURPAGEHWND( pnd->hwndNotebook);
      pnpd = NBCURPAGEDATA( pnd->hwndNotebook);
      if (pnpd)
         {
         if (pnpd->pnpi->ulFocusId)
            {
            // is control part of the page ?
            hwndControl = WinWindowFromID( hwndPage, pnpd->pnpi->ulFocusId);
            if (hwndControl)
               {
               WinSetFocus( HWND_DESKTOP, hwndControl);
               break;
               }

            // otherwise it may be a button
            hwndButton = WtkQueryNotebookButton( hwndPage, pnpd->pnpi->ulFocusId, NULLHANDLE);
            while (hwndButton)
               {
               if (WinIsWindowVisible( hwndButton))
                  WinSetFocus( HWND_DESKTOP, hwndButton);
               hwndButton = WtkQueryNotebookButton( hwndPage, pnpd->pnpi->ulFocusId, hwndButton);
               }
            }

         } // if (pnpd)

      }

      break;

   // -------------------------------

   case WM_FOCUSCHANGE:
      {
                  HWND           hwndFocus = LONGFROMMP( mp1);
                  PSZ            pszClassIndex;
                  BOOL           fLooseFocus = SHORT1FROMMP( mp2);

      // examine for internal class id of +/- spinbutton control
      pszClassIndex = WtkQueryClassIndex( hwndFocus);
      if (pszClassIndex == ((PSZ)0xffff0037L))
         pnd->fNotebookSpinUsed =  !fLooseFocus;
      }
      break; // case WM_FOCUSCHANGE:

   // -------------------------------

   case WM_CONTROL:
      {
      if ((pnd) && (pnd->pni))
         {
         if (SHORT1FROMMP( mp1) == pnd->pni->ulNotebookResId)

            switch (SHORT2FROMMP( mp1))
               {
               case BKN_PAGESELECTEDPENDING:
                  {
                           PPAGESELECTNOTIFY ppsn = (PPAGESELECTNOTIFY)mp2;
                           ULONG             ulPageToTurn = 0;

                           ULONG             ulPageIdNext;
                           ULONG             ulPageIdPrev;

                  // if notebook spinbutton has been used to turn page
                  // don't modify behaviour
                  if (pnd->fNotebookSpinUsed)
                     break;

                  // determine id of next page
                  ulPageIdNext = NBNEXTPAGEID( pnd->hwndNotebook, ppsn->ulPageIdCur);


                  // is the same tab selected again ?
                  if ((ppsn) && (ppsn->ulPageIdCur == ppsn->ulPageIdNew))
                     {
                     // if next page is minor, turn to it
                     if (NBPAGEISMINOR( pnd->hwndNotebook, ulPageIdNext))
                        ulPageToTurn = ulPageIdNext;
                     }
                  else
                     {

                     // act only if
                     if (NBPAGEISMINOR( pnd->hwndNotebook, ppsn->ulPageIdCur) &&
                         NBPAGEISMAJOR( pnd->hwndNotebook, ppsn->ulPageIdNew) &&
                         NBPAGEISMINOR( pnd->hwndNotebook, ulPageIdNext))
                        {
                        // determine the major page for the current minor page
                        ulPageIdPrev = NBNPREVPAGEID( pnd->hwndNotebook, ppsn->ulPageIdCur);
                        while (ulPageIdPrev)
                           {
                           // if major page is found, exit loop
                           if (NBPAGEISMAJOR( pnd->hwndNotebook, ulPageIdPrev))
                              break;

                           // get previous page again
                           ulPageIdPrev = NBNPREVPAGEID( pnd->hwndNotebook, ulPageIdPrev);
                           }

                        // if the the major page of the current one
                        // is the requested one, select the next minor page instead
                        if (ulPageIdPrev == ppsn->ulPageIdNew)
                           ulPageToTurn = ulPageIdNext;

                        } // if (NBPAGEISMINOR( pnd->hwndNotebook, ppsn->ulPageIdCur) ...

                     } // else if ((ppsn) && (ppsn->ulPageIdCur == ppsn->ulPageIdNew))

                  if (ulPageToTurn)
                     {
                     // interrupt turn to the requested page
                     ppsn->ulPageIdNew = 0;

                     // turn to the intended page instead
                     NBTURNTOPAGE( pnd->hwndNotebook, ulPageIdNext);
                     }

                  // disable default border
                  _disableAllButtonFocus( pnd->hwndNotebook);
                  }
                  break;

               case BKN_PAGESELECTED:
                  WinPostMsg( hwnd, WM_USER_SETBUTTONFOCUS, 0, 0);
                  break; // case BKN_PAGESELECTED:

               } // switch (SHORT2FROMMP( mp1))

         } // if ((pnd) && (pnd->pni))
      }
      break; // case WM_CONTROL:

   // -------------------------------

   case WM_DESTROY:
      if (pnd->fHelpInstanceCreated)
         WtkDestroyHelpInstance( hwnd);
      if (pnd->hptrSysIcon)
         WinDestroyPointer( pnd->hptrSysIcon);
      break;

   // -------------------------------

   case WM_HELP:
      {
               BOOL           fHelPanelFound = FALSE;
               ULONG          i;
               PNOTEBOOKPAGEINFO pnpi;

               ULONG          ulPageId;
               HWND           hwndPage;
               USHORT         ulDlgResId;

      // query window id of selected page
      hwndPage   = NBCURPAGEHWND( pnd->hwndNotebook);
      ulDlgResId = WinQueryWindowUShort( hwndPage, QWS_ID);

      // check all notebook pages for help panel id
      for (i = 0, pnpi = pnd->pni->panpi; i < pnd->pni->ulNbPageCount; i++, pnpi++)
         {
         if (pnpi->ulDlgResId == ulDlgResId)
            {
            fHelPanelFound = TRUE;
            break;
            }
         }

      if (fHelPanelFound)
         // launch page specific page
         WtkDisplayHelpPanel( hwnd, pnpi->ulHelpPanel);
      }
      return (MRESULT) 0;
      break;

   } // switch (msg)

return WinDefDlgProc(hwnd,msg,mp1,mp2);
}

// - - - - - - - - - - - - - - - - - - - - - - -

/*
@@WtkNotebookDlg@SYNTAX
This function launches a notebook, where the frame dialog as well
as the notebook pages are loaded from a PM resource.

@@WtkNotebookDlg@PARM@pni@in
Address of the NOTEBOOKINFO structure.
:p.
The field NOTEBOOKINFO.cbFix must be set to the size of the NOTEBOOKINFO structure.
.br
All NOTEBOOKPAGEINFO records pointed to by the NOTEBOOKINFO.panpi
pointer must as well have set their field cbFix to the size of the
structure NOTEBOOKPAGEINFO.

@@WtkNotebookDlg@RETURN
Reply value.
:parml compact.
:pt.MBID_OK
:pd.Notebook could be processed successfully
:pt.MID_ERROR
:pd.An error occurred.
:eparml.

@@WtkNotebookDlg@REMARKS
If a WM_COMMAND event is triggered by a pushbutton other than the
undo, default, ok, cancel or help button defined in the NOTEEBOOKPAGEINFO,
the pfnwcntControl callback is called with the notification BN_CLICKED
for the resource identifier of the pressed pushbutton.
:p.
In order to let WtkNotebookDlg automatically set the notebook page status
lines to page numbers for major and minor pages,
set NOTEBOOKINFO.pszStatusMask to a sprintf compatible string like
"Page %u of %u". Then WtkNotebookDlg will add page numbering to a
major page and its minor pages, overriding any status text possibly being
specified in NOTEBOOKPAGEINFO.pszStatusText. Major pages not being
followed by a minor page are not numbered.
:note.
Don't use the field NOTEBOOKPAGEINFO.pszMinorTabText if autonumbering is used,
else the context menu of the notebook tabs will include the minor tab text in the
sub menu instead of the status line text!

@@
*/

ULONG APIENTRY WtkNotebookDlg( PNOTEBOOKINFO pni)
{
         ULONG          ulResult = MBID_ERROR;
         ULONG          i;

         NOTEBOOKDATA   nd;
         PVOID          pvNewData = NULL;

do
   {
   // check parms
   if (!pni)
      break;

   // check for newer
   pvNewData = _checkNotebookInfoDataVersion( pni );
   if (pvNewData)
      pni = pvNewData;

   // check sizes
   if (pni->cbFix != sizeof( NOTEBOOKINFO))
      break;

   memset( &nd, 0, sizeof( nd));
   nd.pni = pni;
   ulResult = WinDlgBox( pni->hwndParent, pni->hwndOwner, _NotebookDlgProc, pni->hmodResource, pni->ulDlgResId, &nd);

   } while (FALSE);

// cleanup
if (pvNewData) free( pvNewData);
return ulResult;
}

// ---------------------------------------------------------------------

/*
@@WtkQueryNotebookButton@SYNTAX
This function searches a button of a notebook control. This is
usful for buttons with the BS_NOTEBOOKBUTTON style set, as those
are not anylonger a direct child of a notebook page after WM_INITDLG.

@@WtkQueryNotebookButton@PARM@hwndPage@in
Handle of the notebook page.

@@WtkQueryNotebookButton@PARM@ulButtonId@in
Resource identifier of the button to search.

@@WtkQueryNotebookButton@PARM@hwndBefore@in
Handle of the button window handle returned by a pevious call to WtkQueryNotebookButton.
:parml compact.
:pt.NULLHANDLE
:pd.perform search from scratch
:pt.any other value
:pd.search from specified window on
:eparml.

@@WtkQueryNotebookButton@RETURN
Window handle of notebook button
:parml compact.
:pt.NULLHANDLE
:pd.button could not be found
:pt.any other value
:pd.window handle of button
:eparml.

@@WtkQueryNotebookButton@REMARKS
None.

@@
*/

HWND APIENTRY WtkQueryNotebookButton( HWND hwndPage, ULONG ulButtonId, HWND hwndBefore)
{
         HWND           hwndButton = NULLHANDLE;
         PSZ            pszClassName;
         HWND           hwndParent;

         HENUM          henum = NULLHANDLE;
         HWND           hwnd;
         USHORT         usResId;

do
   {
   // first check if button is part of the dialog
   if (!hwndBefore)
      {
      hwndButton = WinWindowFromID( hwndPage, ulButtonId);
      if ((hwndButton) && (WtkQueryClassIndex( hwndButton) == WC_BUTTON))
         break;
      else
         hwndButton = NULLHANDLE;
      }

   // search notebook hwnd
   hwndParent = WinQueryWindow( hwndPage, QW_PARENT);
   pszClassName = WtkQueryClassIndex( hwndParent);
   while (pszClassName != WC_NOTEBOOK)
      {
      // try next one
      hwndParent = WinQueryWindow( hwndParent, QW_PARENT);
      if (!hwndParent)
         break;
      pszClassName = WtkQueryClassIndex( hwndParent);
      }

   // now check if the button exists here
   if (pszClassName == WC_NOTEBOOK)
      {
      if (hwndBefore)
         {

         // search window that was found before
         henum = WinBeginEnumWindows( hwndParent);
         hwnd = WinGetNextWindow( henum);
         while ((hwnd) && (hwnd != hwndBefore))
            {
            hwnd = WinGetNextWindow( henum);
            }

         // search next button
         if (hwnd)
            {
            hwnd = WinGetNextWindow( henum);
            while (hwnd)
               {
               pszClassName = WtkQueryClassIndex( hwnd);
               usResId = WinQueryWindowUShort( hwnd, QWS_ID);
               if ((pszClassName == WC_BUTTON) && (usResId == ulButtonId))
                  {
                  hwndButton = hwnd;
                  break;
                  }

               hwnd = WinGetNextWindow( henum);
               }
            }
         }
      else
         {
         // simply let PM report the first window with this id
         hwndButton = WinWindowFromID( hwndParent, ulButtonId);
         pszClassName = WtkQueryClassIndex( hwndButton);
         if (pszClassName != WC_BUTTON)
            hwndButton = NULLHANDLE;
         }
      }

   } while (FALSE);

if (henum) WinEndEnumWindows( henum);
return hwndButton;
}

// ---------------------------------------------------------------------

/*
@@WtkEnableNotebookButton@SYNTAX
This function enables a button of a notebook control. This is
usful for buttons with the BS_NOTEBOOKBUTTON style set, as those
are not anylonger a direct child of a notebook page after WM_INITDLG.
Moreover, all buttons with the same resource identifier are processed.

@@WtkEnableNotebookButton@PARM@hwndPage@in
Handle of the notebook page.

@@WtkEnableNotebookButton@PARM@ulButtonId@in
Resource identifier of the button to enable.

@@WtkEnableNotebookButton@PARM@fEnable@in
New enabled state.
:parml compact.
:pt.TRUE
:pd.Set button state to enabled
:pt.FALSE
:pd.Set window state to disabled.
:eparml.

@@WtkEnableNotebookButton@RETURN
Success indicator.
:parml compact.
:pt.TRUE
:pd.The update status could be changed successfully.
:pt.FALSE
:pd.An error occurred.
:eparml.

@@WtkEnableNotebookButton@REMARKS
None.

@@
*/

BOOL APIENTRY WtkEnableNotebookButton( HWND hwndPage, ULONG ulButtonId, BOOL fEnable)
{
         BOOL           fResult = FALSE;
         HWND           hwndButton;

hwndButton = WtkQueryNotebookButton( hwndPage, ulButtonId, NULLHANDLE);
while (hwndButton)
   {
   if (WinEnableWindow( hwndButton, fEnable))
      fResult = TRUE;

   hwndButton = WtkQueryNotebookButton( hwndPage, ulButtonId, hwndButton);
   }

return fResult;
}

