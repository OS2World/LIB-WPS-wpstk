/****************************** Module Header ******************************\
*
* Module Name: _ctl.c
*
* PM control related functions sample
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2000
*
* $Id: _ctl.c,v 1.5 2006-08-13 19:49:55 cla Exp $
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
#define INCL_DOS
#define INCL_WIN
#include <os2.h>

#define INCL_WTKUTLPM
#define INCL_WTKUTLCONTROL
#define INCL_WTKUTLMODULE
#include <wtk.h>

/* define a handle type */
typedef LHANDLE HCON;
typedef HCON *PHCON;

typedef struct _CONDATA
   {
         APIRET         rc;
         PSZ            pszTitle;
         TID            tid;
         HEV            hevStartup;
         HWND           hwndFrame;
         HWND           hwndMle;
   } CONDATA, *PCONDATA;


static   HCON           hcon = NULLHANDLE;

#define LASTERROR ERRORIDERROR( WinGetLastError( hab))

// Definitions for private class
// NOTE: the bits for window style must be in
// the bit range of BS_PRIMARYSTYLES (0x000fL)

#define WC_USER_MYCLASS          ((PSZ) 0xffff5123L)
#define CS_USER_MYCLASS_MYSTYLE  0x0002

// ---------------------------------------------------------------------

static MRESULT EXPENTRY _MleSubclassProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
         APIRET         rc = NO_ERROR;
static   PFNWP          pfnwpOrgWindowProc = NULL;

if (!pfnwpOrgWindowProc)
   pfnwpOrgWindowProc = WtkGetDefaultWindowProcPtr( hwnd);

switch (msg)
   {
   case WM_HELP:
      // display help panel
      WtkDisplayHelpPanel( hwnd, 1);
      break;

   } // switch (msg)

return pfnwpOrgWindowProc( hwnd, msg, mp1, mp2);

}

// -----------------------------------------------------------------------------

static VOID _sampleConsoleThread( PVOID pvParm)
{

         APIRET         rc = NO_ERROR;

         PCONDATA       pcd = pvParm;

         HAB            hab = NULLHANDLE;
         HMQ            hmq = NULLHANDLE;
         QMSG           qmsg;

         ULONG          ulFrameStyles = FCF_SIZEBORDER | FCF_TITLEBAR | FCF_SYSMENU | FCF_MINBUTTON | FCF_TASKLIST;
         ULONG          ulMLEStyle = 0;

         CHAR           szHelpFile[ _MAX_PATH];

do
   {
   // get PM resources
   if ((hab = WinInitialize( 0)) == NULLHANDLE)
      break;
   if ((hmq = WinCreateMsgQueue( hab, 0)) == NULLHANDLE)
      break;

   // check parms
   if (!pcd)
      {
      // show message box here
      break;
      }

   // init vars
   pcd->rc         = NO_ERROR;
   pcd->hwndFrame  = NULLHANDLE;
   pcd->hwndMle    = NULLHANDLE;


   // create a standard window with MLE as client
   pcd->hwndFrame = WinCreateStdWindow( HWND_DESKTOP, WS_VISIBLE, &ulFrameStyles,
                                        WC_MLE, pcd->pszTitle, ulMLEStyle,
                                        0, 0xffff,
                                        &pcd->hwndMle);
   if (!pcd->hwndFrame)
      {
      pcd->rc = LASTERROR;
      break;
      }

   // subclass MLE window to get further control
   WinSubclassWindow( pcd->hwndMle, _MleSubclassProc);

   // create help instance
   rc = WtkGetModulePath( (PFN)_MleSubclassProc, szHelpFile, sizeof( szHelpFile));
   if (rc == NO_ERROR)
      {
      strcat( szHelpFile, "\\_ctl.hlp");
      // create the help instance
      WtkCreateHelpInstance( pcd->hwndMle, "Help", szHelpFile);
      }

   // set some attributes
   WinSetWindowPos( pcd->hwndFrame, HWND_TOP, 200, 200, 640, 500, SWP_MOVE | SWP_SIZE | SWP_SHOW | SWP_ZORDER | SWP_ACTIVATE);

   // tell main thread that console is ready
   DosPostEventSem( pcd->hevStartup);


   // now dispatch messages
   while (WinGetMsg( hab, &qmsg, NULLHANDLE, 0, 0))
      {
      WinDispatchMsg(hab, &qmsg);
      }

   } while (FALSE);

// cleanup
if (hmq) WinDestroyMsgQueue( hmq);
if (hab) WinTerminate( hab);
if (pcd)
   {
   if (pcd->hwndMle)    WinDestroyWindow( pcd->hwndMle);
   if (pcd->hwndFrame)  WinDestroyWindow( pcd->hwndFrame);
   if (pcd->hevStartup) DosClose( pcd->hevStartup);
   free( pcd);
   }

_endthread();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

APIRET WtkCreateSampleConsole( PSZ pszTitle)
{

         APIRET         rc = NO_ERROR;
         PCONDATA       pcd = NULL;
         BOOL           fDontCleanup = FALSE;

do
   {
   // already created ?
   if (hcon)
      break;

   // allocate memory
   pcd = malloc( sizeof( CONDATA));
   if (!pcd)
      {
      rc = ERROR_NOT_ENOUGH_MEMORY;
      break;
      }
   memset( pcd, 0, sizeof( CONDATA));
   pcd->pszTitle = pszTitle;

   // create startup semaphore
   rc = DosCreateEventSem( NULL, &pcd->hevStartup, 0, FALSE);
   if (rc != NO_ERROR)
      break;

   // from here don't cleanup memory myself, let thread do it
   // otherwise we could free memory that the thread is still using
   fDontCleanup = TRUE;

   // start the thread here
   pcd->tid = _beginthread( &_sampleConsoleThread, NULL, 16384, pcd);
   if (pcd->tid == -1)
      {
      rc = ERROR_INVALID_FUNCTION;
      break;
      }

   // wait for window to be created
   rc = DosWaitEventSem( pcd->hevStartup, 5000);
   if (rc != NO_ERROR)
      break;

   // report pointer as handle
   hcon = (HCON) pcd;

   } while (FALSE);

// cleanup
if ((rc != NO_ERROR) && (!fDontCleanup))
   if (pcd) free( pcd);

return rc;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

APIRET WtkWriteSampleConsole( PSZ pszMessage, ...)
{
         APIRET         rc = NO_ERROR;
         PCONDATA       pcd = (PCONDATA) hcon;
         va_list        arg_ptr;
         CHAR           szBuffer[ 4096];

do
   {
   // already created ?
   if (!hcon)
      {
      rc = ERROR_INVALID_HANDLE;
      break;
      }

   // determine message
   va_start (arg_ptr, pszMessage);
   vsprintf( szBuffer, pszMessage, arg_ptr);
   va_end (arg_ptr);

   // append message to MLE
   WinSendMsg( pcd->hwndMle,  MLM_INSERT, MPFROMP( szBuffer), 0);

   } while (FALSE);

return rc;
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

APIRET WtkTerminateSampleConsole( BOOL fWait)
{
         APIRET         rc = NO_ERROR;
         PCONDATA       pcd = (PCONDATA) hcon;
         TID            tid = -1;

do
   {
   // already created ?
   if (!hcon)
      {
      rc = ERROR_INVALID_HANDLE;
      break;
      }

  if (!fWait)
     {
     // kill thread here by sending a quit to the window
     WinPostMsg( pcd->hwndFrame, WM_QUIT, 0, 0);
     }

   // wait for proper thread ending
   rc = DosWaitThread( &pcd->tid, DCWW_WAIT);

   } while (FALSE);

return rc;
}

// -----------------------------------------------------------------------------

#define PRINTSEPARATOR printf("\n------------------------------------------\n")
#define printf WtkWriteSampleConsole

int main ( int argc, char *argv[])
{

         APIRET         rc = NO_ERROR;
         BOOL           fResult = FALSE;
         HAB            hab = NULLHANDLE;
         HMQ            hmq = NULLHANDLE;

         HWND           hwndCtrl;

         PCONDATA       pcd;

do
   {
   // create the console
   rc = WtkCreateSampleConsole( "Workplace Shell Toolkit - PM control related functions sample");
   if (rc != NO_ERROR)
      {
      rc = ERROR_INVALID_FUNCTION;
      break;
      }

   // get PM resources
   if ((hab = WinInitialize( 0)) == NULLHANDLE)
      break;
   if ((hmq = WinCreateMsgQueue( hab, 0)) == NULLHANDLE)
      break;

   // -----------------------------------------------------------------

   printf( "\n"
           "- registering private class with WC_* constant name\n");
   fResult  = WinRegisterClass( hab, WC_USER_MYCLASS, WinDefWindowProc, 0, 0);

   if (!fResult)
      {
      printf( "-- error: WinRegisterClass: cannot register private PM class. rc=%u\n", LASTERROR);
      break;
      }

   printf( "- creating object window of private class\n");
   hwndCtrl = WinCreateWindow( HWND_OBJECT, WC_USER_MYCLASS, NULL, 0, 0, 0, 0,
                               CS_USER_MYCLASS_MYSTYLE, HWND_OBJECT,
                               HWND_TOP, 0, NULL, NULL);
   if (!hwndCtrl)
      {
      printf( "-- error: WinCreateWindow: cannot create object window. rc=%u\n", LASTERROR);
      break;
      }

   // explicitely set window style (due to unknown reason the flStyle
   // parameter of WinCreateWindow does not suffice)
   WinSetWindowULong( hwndCtrl, QWL_STYLE, CS_USER_MYCLASS_MYSTYLE);

   PRINTSEPARATOR;

   // -----------------------------------------------------------------

   // --- test with public classs

   pcd = (PCONDATA) hcon;
   if (pcd)
      {
      printf( "\n"
              "- testing WtkIsOfPublicPmClass with public class\n");
      fResult = WtkIsOfPublicPmClass( pcd->hwndFrame, WC_FRAME, 0);
      if (!fResult)
         printf( "-- error: WtkIsOfPublicPmClass: frame window is reported\n"
                 "       not to be of specified class.\n");
      else
         printf( "-- WtkIsOfPublicPmClass: frame window is properly reported\n"
                 "       being of specified class.\n");
      }

   // --- test with private class, having a WC_ constant class name

   printf( "\n"
           "- testing WtkIsOfPublicPmClass with private class\n"
           "   having a WC_* classname\n");
   fResult = WtkIsOfPublicPmClass( hwndCtrl, WC_USER_MYCLASS, CS_USER_MYCLASS_MYSTYLE);
   WinDestroyWindow( hwndCtrl);
   if (!fResult)
      printf( "-- error: WtkIsOfPublicPmClass: object window is reported\n"
                 "       not to be of specified class.\n");
   else
      printf( "-- WtkIsOfPublicPmClass: object window is properly reported\n"
                 "       being of specified class.\n");

   // -----------------------------------------------------------------


   } while (FALSE);

// cleanup
WtkTerminateSampleConsole( TRUE);
if (hmq) WinDestroyMsgQueue( hmq);
if (hab) WinTerminate( hab);

return rc;
}


