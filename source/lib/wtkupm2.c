/****************************** Module Header ******************************\
*
* Module Name: wtkupm2.c
*
* Source for PM utility functions.
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2000
*
* $Id: wtkupm2.c,v 1.8 2006-08-16 14:56:13 cla Exp $
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

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INCL_DOSDEVIOCTL
#define INCL_DOS
#define INCL_WIN
#define INCL_GPI
#define INCL_ERRORS
#include <os2.h>

#include "wtkupm.h"
#include "wtkuioc.h"
#include "wtkufil.h"
#include "wtkuctl.h"
#include "wpstk.ih"


#define DIALOG_EXTRAWIDTH 60
#define DIRLISTBOX_IDENTATION_PIXEL 4
#define DIRLISTBOX_MARGIN_PIXEL 2

#define  WTK_DDS_VALID_FLAGS (WTK_DDS_CENTER |\
                              WTK_DDS_CUSTOM |\
                              WTK_DDS_HELPBUTTON |\
                              WTK_DDS_PRELOAD_VOLINFO |\
                              WTK_DDS_FORCEDIALOGID)

static   PSZ            pszModuleName = "PMSDMRI";

#define IDSYSDLG_FILEDIALOG            256
#define IDSYSBMP_OPENFOLDER            301
#define IDSYSBMP_CLOSEDFOLDER          300

#define IDSYSMSG_DRIVE_NOT_AVAILABLE   1106
#define IDSYSMSG_DRIVE_NOT_READY       1122
#define IDSYSMSG_DISK_UNREADABLE       1124
#define IDSYSMSG_INSERT_DISKETTE       1127
#define IDSYSMSG_SELECTOK              1128

// -----------------------------------------------------------------------------

static ULONG _getParmCount( PSZ pszMessage)
{
         ULONG          ulParmCount = 0;
         ULONG          ulThisParmNum;

// scan message for paramaters (%1 to %9)
while ((pszMessage) && (*pszMessage))
   {
   if (*pszMessage == '%')
      {
      pszMessage++;
      if (isdigit( *pszMessage))
         {
         ulThisParmNum = *pszMessage - '0';
         ulParmCount = MAX( ulParmCount, ulThisParmNum);
         }
      }
   pszMessage++;
   }

return ulParmCount;
}

// -----------------------------------------------------------------------------

// NOTE: pass string parameters only !!!

static ULONG _showMessage( HWND hwnd, ULONG ulMsgStyle, ULONG ulMessageId, ...)
{
         ULONG          ulResult = MBID_ERROR;
         APIRET         rc = NO_ERROR;

         va_list        arg_ptr;
         ULONG          ulParmCount;

         PDIRDLG        pdd = WinQueryWindowPtr( hwnd, QWL_USER);
         HMODULE        hmodResource;
         CHAR           szMessageStr[ _MAX_PATH];
         CHAR           szMessage[ _MAX_PATH];
         ULONG          ulResultLen;

         ULONG          ul2ndMessageId;

do
   {
   // get module handle and string
   rc = DosQueryModuleHandle( pszModuleName, &hmodResource);
   if (rc != NO_ERROR)
      break;

   if (!WinLoadString( WinQueryAnchorBlock( hwnd), hmodResource,
                       ulMessageId, sizeof( szMessageStr), szMessageStr))
      break;

   // determine number of
   ulParmCount = _getParmCount( szMessageStr);

   // insert parameters
   memset( szMessage, 0, sizeof( szMessage));
   va_start( arg_ptr, ulMessageId);
   DosInsertMessage( (PSZ*)arg_ptr, ulParmCount,
                     szMessageStr, strlen( szMessageStr),
                     szMessage, sizeof( szMessage),
                     &ulResultLen);
   va_end (arg_ptr);

   // for special messages, append another one
   // without parameters
   switch (ulMessageId)
      {
      case IDSYSMSG_INSERT_DISKETTE:
         ul2ndMessageId = IDSYSMSG_SELECTOK;
         break;

      default:
         ul2ndMessageId = 0;
         break;
      }
   if (ul2ndMessageId)
      {
      strcat( szMessage, "\r\r");
      WinLoadString( WinQueryAnchorBlock( hwnd), hmodResource, ul2ndMessageId,
                     sizeof( szMessage) - strlen( szMessage),
                     &szMessage[ strlen( szMessage)]);
      strcat( szMessage, "\r ");
      }


   // execute message box
   ulResult = WinMessageBox( HWND_DESKTOP, hwnd, szMessage, pdd->pszTitle, -1, MB_MOVEABLE | ulMsgStyle);

   } while (FALSE);

return ulResult;
}

// -----------------------------------------------------------------------------

static APIRET _checkDrive( HWND hwnd, PSZ pszPath, BOOL fCheckAccess)
{
         APIRET         rc = NO_ERROR;
         ULONG          ulMsgId;
         ULONG          ulResult;

         // subject ot change if ever drive identifiers come true !
         CHAR           szDrive[ 32];
         CHAR           szCurDrive[ 32];

         BYTE           abParm[ 2];
         BYTE           bData;
         ULONG          ulParmLen = sizeof( abParm);
         ULONG          ulDataLen = sizeof( bData);
         BOOL           fLoop = FALSE;

do
   {
   // check parms
   if ((!pszPath)  ||
       (!*pszPath) ||
       (!strchr( pszPath, ':')))
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   // initialize
   fLoop = FALSE;

   // check drive with a call not accessing the media
   // may be required later
   strcpy( szDrive, pszPath);
   strcpy( strchr( szDrive, ':') + 1, "");
   strupr( szDrive);
   memset( abParm, 0, sizeof( abParm));
   rc = WtkDevIOCtl( szDrive,
                     WTK_OPENDEVICE_SHARED,
                     IOCTL_DISK,
                     DSK_BLOCKREMOVABLE,
                     &abParm,
                     &ulParmLen,
                     &bData,
                     &ulDataLen);


   // show error message, if requested
   if (hwnd)
      switch (rc)
         {
         case NO_ERROR:

            // check for accessible drive, when requested
            if (fCheckAccess)
               {
                        FSINFO         fsi;

               // query information
               memset( &fsi, 0, sizeof( fsi));
               rc = DosQueryFSInfo( szDrive[ 0] - 'A' + 1, FSIL_VOLSER, &fsi, sizeof( fsi));
               }
            if (rc != NO_ERROR)
               {
               // show prompt
               ulResult = _showMessage( hwnd, MB_RETRYCANCEL | MB_ERROR, IDSYSMSG_DISK_UNREADABLE, szDrive);

               // on retry one more check
               fLoop = (ulResult == MBID_RETRY);
               }
            break;

         // ---------------------------------------

         // virtual network drive is ok !
         case ERROR_NOT_SUPPORTED:
            rc = NO_ERROR;
            break;

         // ---------------------------------------

         case ERROR_INVALID_DRIVE:
            _showMessage( hwnd, MB_OK | MB_INFORMATION, IDSYSMSG_DRIVE_NOT_AVAILABLE);
            break;

         // ---------------------------------------

         case ERROR_DISK_CHANGE:

            // show the diskette change
            ulResult = _showMessage( hwnd, MB_OKCANCEL | MB_INFORMATION, IDSYSMSG_INSERT_DISKETTE, szDrive);

            if (ulResult == MBID_OK)
               {
               // query current drive letter for the requested drive
               // NOTE: the drive letter passed as first parm must be any
               //       valid file handle, so we use the boot drive here
               bData = szDrive[ 0] - 'A' + 1;
               rc = WtkDevIOCtl( "?:",
                                 WTK_OPENDEVICE_SHARED,
                                 IOCTL_DISK,
                                 DSK_GETLOGICALMAP,
                                 &abParm,
                                 &ulParmLen,
                                 &bData,
                                 &ulDataLen);
               sprintf( szCurDrive, "%c:", bData + 'A' - 1);

               // select new drive letter
               bData = szDrive[ 0] - 'A' + 1;
               rc = WtkDevIOCtl( szCurDrive,
                                 WTK_OPENDEVICE_SHARED,
                                 IOCTL_DISK,
                                 DSK_SETLOGICALMAP,
                                 &abParm,
                                 &ulParmLen,
                                 &bData,
                                 &ulDataLen);

               // one more check
               fLoop = TRUE;
               }
            break;

         // ---------------------------------------

         case ERROR_NOT_READY:
            switch (szDrive[ 0])
               {
               case (USHORT)'A':
               case (USHORT)'B':
                  ulResult = _showMessage( hwnd, MB_RETRYCANCEL | MB_INFORMATION, IDSYSMSG_INSERT_DISKETTE, szDrive);

                  // on retry one more check
                  fLoop = (ulResult == MBID_RETRY);
                  break;

               default:
                  _showMessage( hwnd, MB_OK | MB_INFORMATION, IDSYSMSG_DRIVE_NOT_READY, szDrive);
                  break;
               }
            break;
         }

   } while (fLoop);

return rc;
}

// -----------------------------------------------------------------------------

static ULONG _getDirectoryLevel( PSZ pszDirectory)
{
         ULONG          ulLevel = 0;
         PSZ            p;
do
   {
   // check parms
   if ((!pszDirectory) ||
       (!*pszDirectory))
      break;

   // for root directories return fixed value
   if ((strlen( pszDirectory) > 2) &&
       (!strcmp( pszDirectory + strlen( pszDirectory) - 2, ":\\")))
      {
      ulLevel = 1;
      break;
      }

   // get max level of current path
   // to determine identation
   p = strchr( pszDirectory, '\\');
   while (p)
      {
      ulLevel++;
      if (p)
         p = strchr( p + 1, '\\');
      }
    ulLevel++;

   } while (FALSE);

return ulLevel;
}

// -----------------------------------------------------------------------------

static BOOL _determinePathOfSelectedDrive( HWND hwnd)
{
         BOOL           fResult = FALSE;
         APIRET         rc = NO_ERROR;
         PDIRDLG        pdd = WinQueryWindowPtr( hwnd, QWL_USER);
         HWND           hwndDriveComboBox = WinWindowFromID( hwnd, DID_DRIVE_CB);

         ULONG          ulItem;
         CHAR           szPathOfSelectedDrive[ _MAX_PATH];
         ULONG          ulDirLen;

         PSZ            pszPathOfDrive;

do
   {
   // check pointer
   if (!pdd)
      break;

   // get selected item
   ulItem = WinQueryLboxSelectedItem( hwndDriveComboBox);

   // get text and isolate drive letter
   if (!WinQueryLboxItemText( hwndDriveComboBox, ulItem,
                              szPathOfSelectedDrive, sizeof( szPathOfSelectedDrive)))
      break;
   strcpy( strchr( szPathOfSelectedDrive, ':') + 1, "");
   strcat( szPathOfSelectedDrive, "\\");


   // check if drive is valid
   rc = _checkDrive( hwnd, szPathOfSelectedDrive, TRUE);
   if (rc != NO_ERROR)
      break;

   // if already selected, the handle is set to a path
   pszPathOfDrive = (PSZ) WinSendMsg( hwndDriveComboBox, LM_QUERYITEMHANDLE, MPFROMLONG( ulItem), 0);
   if (pszPathOfDrive)
      strcpy( szPathOfSelectedDrive, pszPathOfDrive);
   else
      {
      // append current directory
      ulDirLen = sizeof( szPathOfSelectedDrive) - (ULONG) strlen( szPathOfSelectedDrive);
      rc = DosQueryCurrentDir( toupper( szPathOfSelectedDrive[ 0]) - 'A' + 1,
                               &szPathOfSelectedDrive[ strlen( szPathOfSelectedDrive)],
                               &ulDirLen);
      if (rc != NO_ERROR)
         break;

      // add a duplicate as item handle
      WinSendMsg( hwndDriveComboBox, LM_SETITEMHANDLE,
                  MPFROMLONG( ulItem),
                  MPFROMP( strdup( szPathOfSelectedDrive)));
      }

   // replace path in structure
   strcpy( pdd->szFullDir, szPathOfSelectedDrive);



   // done
   fResult = TRUE;

   } while (FALSE);

return fResult;
}

// -----------------------------------------------------------------------------

static VOID _determineSelectedPath( HWND hwnd)
{
         PDIRDLG        pdd = WinQueryWindowPtr( hwnd, QWL_USER);
         HWND           hwndListbox = WinWindowFromID( hwnd, DID_DIRECTORY_LB);

         ULONG          i;
         ULONG          ulItem;

         CHAR           szSelectedPath[ _MAX_PATH];
         ULONG          ulLevel;

         PSZ            pszLastLevelPart;

do
   {
   // check pointer
   if (!pdd)
      break;

   // get directory level
   ulLevel = _getDirectoryLevel( pdd->szFullDir);

   // get current item
   ulItem = WinQueryLboxSelectedItem( hwndListbox);

   // assemble current path
   szSelectedPath[ 0] = 0;
   pszLastLevelPart = szSelectedPath;
   for (i = 0; i < ulItem + 1; i++)
      {
      if (i <= ulLevel)
         {
         // note where this item was placed
         pszLastLevelPart = &szSelectedPath[ strlen( szSelectedPath)];

         // prepend backslash from second item on
         if (i > 1)
            {
            strcpy( pszLastLevelPart, "\\");
            pszLastLevelPart++;
            }
         }

      // append item or replace last level part
      WinQueryLboxItemText( hwndListbox, i,
                            pszLastLevelPart,
                            sizeof( szSelectedPath) - ( pszLastLevelPart - szSelectedPath));
      }

   // replace path in structure
   strcpy( pdd->szFullDir, szSelectedPath);

   } while (FALSE);

return;
}

// -----------------------------------------------------------------------------

static ULONG _updateDriveItem( HWND hwndListbox, ULONG ulItem, CHAR chDrive, BOOL fReadDriveInfo, PSZ pszPath)
{
         ULONG          ulResultItem = LIT_ERROR;
         APIRET         rc = NO_ERROR;
         FSINFO         fsi;
         CHAR           szItem[ _MAX_PATH];
         PSZ            pszPathOfDrive;

do
   {
   // check parms
   if ((chDrive < 'A') ||
       (chDrive > 'Z'))
      break;

   // read requested drive info
   if (fReadDriveInfo)
      {

      // NOTE: here _checkDrive (with GUI error handling) is either not necessary
      // or has already been called by _determinePathOfSelectedDrive()

      // query information
      memset( &fsi, 0, sizeof( fsi));
      rc = DosQueryFSInfo( chDrive - 'A' + 1, FSIL_VOLSER, &fsi, sizeof( fsi));

      if (rc == ERROR_INVALID_DRIVE)
         break;
      }
   else if (chDrive > 'B')
      {
      // only check if drive is valid (don't really access it)
      sprintf( szItem, "%c:", chDrive);
      rc = _checkDrive( NULLHANDLE, szItem, FALSE);
      if (rc == ERROR_INVALID_DRIVE)
         break;
      }

   // create item text
   if ((fReadDriveInfo) && (fsi.vol.szVolLabel[ 0]))
      sprintf( szItem, "%c:  [%s]", chDrive, fsi.vol.szVolLabel);
   else
      sprintf( szItem, "%c:", chDrive);

   // add or update item
   switch (ulItem)
      {
      case LIT_END:
         ulResultItem = WinInsertLboxItem( hwndListbox, ulItem, szItem);
         break;

      default:
         // update listbox item
         WinSetLboxItemText( hwndListbox, ulItem, szItem);
         WinSetWindowText( hwndListbox, szItem);

         // set current path as item handle
         if ((pszPath) && (*pszPath))
            {
            // remove old handle if given
            pszPathOfDrive = (PSZ) WinSendMsg( hwndListbox,
                                               LM_QUERYITEMHANDLE,
                                               MPFROMLONG( ulItem), 0);
            if (pszPathOfDrive)
               free( pszPathOfDrive);

            // add a duplicate as current item handle
            WinSendMsg( hwndListbox, LM_SETITEMHANDLE,
                        MPFROMLONG( ulItem),
                        MPFROMP( strdup( pszPath)));
            }

         ulResultItem = ulItem;
         break;
      }

   } while (FALSE);

return ulResultItem;
}


// -----------------------------------------------------------------------------

static VOID _updateDriveCombobox( HWND hwnd)
{
         APIRET         rc = NO_ERROR;
         PDIRDLG        pdd = WinQueryWindowPtr( hwnd, QWL_USER);
         HWND           hwndDriveComboBox = WinWindowFromID( hwnd, DID_DRIVE_CB);

         ULONG          i;
         CHAR           szItem[ _MAX_PATH];
         ULONG          ulSelectItem;
         BOOL           fSelectedDrive;

do
   {
   // check pointer
   if (!pdd)
      break;

   // avoid flickering
   WinEnableWindowUpdate( hwndDriveComboBox, FALSE);

   if (WinQueryLboxCount( hwndDriveComboBox))
      {
      // set the combobox to the drive of the currently selected path

      // isolate drive from selecte path
      strcpy( szItem, pdd->szFullDir);
      strcpy( strchr( szItem, ':') + 1, "");

      // search item and select
      ulSelectItem = (ULONG) WinSendMsg( hwndDriveComboBox, LM_SEARCHSTRING,
                                         MPFROM2SHORT( LSS_PREFIX, LIT_FIRST), szItem);

      if ((ulSelectItem != LIT_NONE) &&
          (ulSelectItem != LIT_ERROR))
         {
         // update the item with current info (always update !)
         _updateDriveItem( hwndDriveComboBox, ulSelectItem, szItem[ 0], TRUE, pdd->szFullDir);

         // select it (this will use the updated text already)
         WinSendMsg( hwndDriveComboBox,
                     LM_SELECTITEM,
                     MPFROMLONG( ulSelectItem),
                     MPFROMLONG( TRUE));
         }
      }
   else
      {
      // initialize
      for (i = 'A'; i <= 'Z'; i++)
         {
         sprintf( szItem, "%c:", i);
         fSelectedDrive = !strnicmp( szItem, pdd->szFullDir, strlen( szItem));

         // add item to box, add volinfo either to all or only selected items
         ulSelectItem = _updateDriveItem( hwndDriveComboBox, LIT_END,i,
                                          ((fSelectedDrive) || (pdd->fl & WTK_DDS_PRELOAD_VOLINFO)), NULL);
         if (ulSelectItem == LIT_ERROR)
            continue;

         // compare drive with selected path
         // on match select
         if (fSelectedDrive)
            WinSendMsg( hwndDriveComboBox,
                        LM_SELECTITEM,
                        MPFROMLONG( ulSelectItem),
                        MPFROMLONG( TRUE));

         } // for (i = 1; i <= 26; i++)

      } // if (!WinQueryLboxCount())

   } while (FALSE);

// reenable drawing
WinEnableWindowUpdate( hwndDriveComboBox, TRUE);
return;
}

// -----------------------------------------------------------------------------

static VOID _updateDirListbox( HWND hwnd)
{
         APIRET         rc = NO_ERROR;
         PDIRDLG        pdd = WinQueryWindowPtr( hwnd, QWL_USER);
         HWND           hwndDirListbox = WinWindowFromID( hwnd, DID_DIRECTORY_LB);

         ULONG          i;

         CHAR           szPath[ _MAX_PATH];
         CHAR           szItem[ _MAX_PATH];

         PSZ            p;
         PSZ            pszItem;
         ULONG          ulSelectItem;

         ULONG          ulItemCount;
         CHAR           szBasePath[ _MAX_PATH];
         CHAR           szNextDir[ _MAX_PATH];
         ULONG          ulDirOffset;
         HDIR           hdir = HDIR_CREATE;

do
   {
   // check pointer
   if (!pdd)
      break;

   // avoid flickering
   WinEnableWindowUpdate( hwndDirListbox, FALSE);

   // remove all listbox items
   WinSendMsg( hwndDirListbox, LM_DELETEALL, 0, 0);

   // if path does not exist, keep listbox empty
   // and drive setting untouched
   if (!WtkDirExists( pdd->szFullDir))
      break;

   // add entries of path to dir listbox
   // last one selected
   strcpy( szPath, pdd->szFullDir);
   pszItem = szPath;
   p = strchr( pszItem, '\\');
   while (p)
      {
      // cut of remaining path
      *p = 0;

      // on first entry (drive) append backslash
      if (pszItem == szPath)
         {
         strcpy( szItem, pszItem);
         strcat( szItem, "\\");
         pszItem = szItem;
         }

      // insert item
      ulSelectItem = WinInsertLboxItem( hwndDirListbox, LIT_END, pszItem);

      // advance to next item
      pszItem = p + 1;
      p = strchr( pszItem, '\\');
      }

   // append last entry if any left
   if (*pszItem)
      ulSelectItem = WinInsertLboxItem( hwndDirListbox, LIT_END, pszItem);

   // select item
   WinSendMsg( hwndDirListbox,
               LM_SELECTITEM,
               MPFROMLONG( ulSelectItem),
               MPFROMLONG( TRUE));

   // add all directories below selected one
   strcpy( szBasePath, pdd->szFullDir);
   if ( szBasePath[ strlen( szBasePath) - 1] != '\\')
      strcat( szBasePath, "\\");
   ulDirOffset = strlen( szBasePath);
   strcat( szBasePath, "*");
   while ((rc = WtkGetNextDirectory( szBasePath,
                                     &hdir,
                                     szNextDir,
                                     sizeof( szNextDir))) == NO_ERROR)
      {
      // insert this item
      WinInsertLboxItem( hwndDirListbox, LIT_END, &szNextDir[ ulDirOffset]);
      }

   // --------------------------------

   // put full path to entryfield
   WinSetDlgItemText( hwnd, DID_FILENAME_ED, pdd->szFullDir);

   } while (FALSE);

// reenable drawing
WinEnableWindowUpdate( hwndDirListbox, TRUE);
return;
}

// --------------------------------------------------------------------------


static VOID _drawListboxItem( HWND hwnd, POWNERITEM poi)
{
         PDIRDLG        pdd = WinQueryWindowPtr( hwnd, QWL_USER);
         CHAR           szItemText[ _MAX_PATH];

         ULONG          ulBgColor;
         ULONG          ulFgColor;

         BOOL           fSelected = FALSE;
         ULONG          ulLevel = 0;
         BOOL           fOpenedDirectory = FALSE;

         ULONG          ulPointerSize = WinQuerySysValue( HWND_DESKTOP, SV_CXPOINTER) / 2;
         HMODULE        hmodResource;
         HPOINTER       hpointer;

         PSZ            p;

do
   {
   // check parms
   if (!poi)
      break;

   // check if state changed
   fSelected = (poi->fsState);

   // get directory level
   ulLevel = _getDirectoryLevel( pdd->szFullDir);

   // max identation for all directories
   // below selected one
   if (poi->idItem < ulLevel)
      {
      ulLevel = poi->idItem ;
      fOpenedDirectory = FALSE;
      }
   else
      fOpenedDirectory = TRUE;

   // use RGB colors
   GpiCreateLogColorTable( poi->hps, 0, LCOLF_RGB, 0, 0, NULL);

   // get colors to use
   ulBgColor = WinQuerySysColor( HWND_DESKTOP,
                                 (fSelected) ?
                                    SYSCLR_HILITEBACKGROUND :
                                    SYSCLR_WINDOW,
                                 0);

   if (!WinQueryPresParam( poi->hwnd,
                           (fSelected) ?
                              PP_HILITEFOREGROUNDCOLOR :
                              PP_FOREGROUNDCOLOR,
                           (fSelected) ?
                              PP_HILITEFOREGROUNDCOLORINDEX :
                              PP_FOREGROUNDCOLORINDEX,
                           NULL,
                           sizeof( ulFgColor),
                           &ulFgColor,
                           QPF_ID2COLORINDEX))
      ulFgColor = WinQuerySysColor( HWND_DESKTOP,
                                    (fSelected) ?
                                       SYSCLR_HILITEFOREGROUND :
                                       SYSCLR_WINDOWTEXT,
                                    0);


   // draw selection and cursor according to attributes
   WinFillRect( poi->hps, &poi->rclItem, ulBgColor);

   // determine identation
   poi->rclItem.xLeft += (ulLevel * DIRLISTBOX_IDENTATION_PIXEL) + DIRLISTBOX_MARGIN_PIXEL;

   // draw bitmap
   // make sure to take font metrics into account
   DosQueryModuleHandle( pszModuleName, &hmodResource);
   hpointer = WinLoadPointer( HWND_DESKTOP,
                              hmodResource,
                              (fOpenedDirectory) ?
                                 IDSYSBMP_OPENFOLDER :
                                 IDSYSBMP_CLOSEDFOLDER);
   if (hpointer)
      {
      FONTMETRICS fm;
      GpiQueryFontMetrics( poi->hps, sizeof( fm), &fm);

      WinDrawPointer( poi->hps,
                      poi->rclItem.xLeft,
                      poi->rclItem.yBottom + fm.lMaxDescender,
                      hpointer,
                      DP_NORMAL);
      WinDestroyPointer( hpointer);
      poi->rclItem.xLeft += ulPointerSize + (2 * DIRLISTBOX_MARGIN_PIXEL);
      }


   // draw text
   WinSendMsg( poi->hwnd,
               LM_QUERYITEMTEXT,
               MPFROM2SHORT( poi->idItem, sizeof( szItemText)),
               szItemText);

   WinDrawText( poi->hps, strlen( szItemText), szItemText, &poi->rclItem, ulFgColor, ulBgColor,
                DT_LEFT | DT_VCENTER);


   // tell that we have taken care for highlighing
   poi->fsState = poi->fsStateOld = 0;

   } while (FALSE);

return;
}

// -----------------------------------------------------------------------------

#define REMOVECONTROL(h,f) WinDestroyWindow( h)

static VOID _initializeControls( HWND hwnd)
{
         PDIRDLG        pdd = WinQueryWindowPtr( hwnd, QWL_USER);
         APIRET         rc;
         CHAR           szText[ 128];

         ULONG          ulDlgMinWidth;

         HWND           hwndEntryText    = WinWindowFromID( hwnd, DID_FILENAME_TXT);
         HWND           hwndEntryField   = WinWindowFromID( hwnd, DID_FILENAME_ED);
         HWND           hwndDriveText    = WinWindowFromID( hwnd, DID_DRIVE_TXT);
         HWND           hwndDriveBox     = WinWindowFromID( hwnd, DID_DRIVE_CB);
         HWND           hwndDirText      = WinWindowFromID( hwnd, DID_DIRECTORY_TXT);
         HWND           hwndDirBox       = WinWindowFromID( hwnd, DID_DIRECTORY_LB);
         HWND           hwndOkButton     = WinWindowFromID( hwnd, DID_OK_PB);
         HWND           hwndApplyButton  = WinWindowFromID( hwnd, DID_APPLY_PB);
         HWND           hwndCancelButton = WinWindowFromID( hwnd, DID_CANCEL);
         HWND           hwndHelpButton   = WinWindowFromID( hwnd, DID_HELP_PB);

         SWP            swpFrame;
         SWP            swpEntryText;
         SWP            swpEntryField;
         SWP            swpDriveText;
         SWP            swpDriveBox;
         SWP            swpDirText;
         SWP            swpDirBox;
         SWP            swpOkButton;
         SWP            swpApplyButton;
         SWP            swpCancelButton;
         SWP            swpHelpButton;

         ULONG          ulGap;
do
   {

   // get dimensions of frame and certain controls
   WinQueryWindowPos( hwnd,              &swpFrame);
   WinQueryWindowPos( hwndEntryText,     &swpEntryText);
   WinQueryWindowPos( hwndEntryField,    &swpEntryField);
   WinQueryWindowPos( hwndDriveText,     &swpDriveText);
   WinQueryWindowPos( hwndDriveBox,      &swpDriveBox);
   WinQueryWindowPos( hwndDirText,       &swpDirText);
   WinQueryWindowPos( hwndDirBox,        &swpDirBox);
   WinQueryWindowPos( hwndOkButton   ,   &swpOkButton);
   WinQueryWindowPos( hwndApplyButton,   &swpApplyButton);
   WinQueryWindowPos( hwndCancelButton,  &swpCancelButton);
   WinQueryWindowPos( hwndHelpButton,    &swpHelpButton);

   // --------------------------------

   // adjust pushbutton positions
   swpCancelButton.fl = SWP_MOVE;
   swpCancelButton.x  = (swpOkButton.x * 2) + swpOkButton.cx;
   WinSetMultWindowPos( hwndCancelButton, &swpCancelButton, 1);

   swpHelpButton.fl = SWP_MOVE;
   swpHelpButton.x  = swpOkButton.x + swpCancelButton.x  + swpCancelButton.cx;
   WinSetMultWindowPos( hwndHelpButton, &swpHelpButton, 1);

   // --------------------------------

   // calculate minimum dialog width according to buttons
   if (pdd->fl & WTK_DDS_HELPBUTTON)
      ulDlgMinWidth = swpHelpButton.x + swpHelpButton.cx + swpOkButton.x;
   else
      ulDlgMinWidth = swpCancelButton.x + swpCancelButton.cx + swpOkButton.x;

   // adjust frame
   swpFrame.fl = SWP_SIZE;
   swpFrame.cx = MAX( (swpDirBox.x * 2) + swpDirBox.cx, ulDlgMinWidth);
   WinSetMultWindowPos( hwnd, &swpFrame, 1);


   // --------------------------------

   // make controls wider
   swpEntryField.fl = SWP_SIZE;
   swpEntryField.cx = swpFrame.cx - swpEntryField.x - swpOkButton.x;
   swpEntryField.cy -= 7;
   WinSetMultWindowPos( hwndEntryField, &swpEntryField, 1);

   swpDriveBox.fl = SWP_SIZE;
   swpDriveBox.cx = swpFrame.cx - swpDriveBox.x - swpOkButton.x;
   WinSetMultWindowPos( hwndDriveBox, &swpDriveBox, 1);

   swpDirBox.fl = SWP_SIZE;
   swpDirBox.cx = swpFrame.cx - swpDirBox.x - swpOkButton.x;
   WinSetMultWindowPos( hwndDirBox, &swpDirBox, 1);

   // --------------------------------

   // drag top contols down to fill the gap
   ulGap = swpEntryField.y - swpDriveText.y - (1.5 * swpDriveText.cy);

   swpEntryText.fl  = SWP_MOVE;
   swpEntryText.y  -= ulGap;
   WinSetMultWindowPos( hwndEntryText, &swpEntryText, 1);

   swpEntryField.fl  = SWP_MOVE;
   swpEntryField.y  -= ulGap;
   WinSetMultWindowPos( hwndEntryField, &swpEntryField, 1);

   swpFrame.fl  = SWP_SIZE;
   swpFrame.cy -= ulGap;
   WinSetMultWindowPos( hwnd, &swpFrame, 1);

   // --------------------------------

   // write text of directory listbox to the static text for the entryfield
   WinQueryWindowText( hwndDirText, sizeof( szText), szText);
   WinSetWindowText( hwndEntryText, szText);
   if ((swpDirBox.cy + swpDirText.cy) < swpDirText.y)
      {
      swpDirBox.fl  = SWP_SIZE;
      swpDirBox.cy += swpDirText.cy;
      WinSetMultWindowPos( hwndDirBox, &swpDirBox, 1);
      }

   // --------------------------------

   // remove unnecessary controls
   WinDestroyWindow( WinWindowFromID( hwnd, DID_FILES_TXT));
   WinDestroyWindow( WinWindowFromID( hwnd, DID_FILES_LB));
   WinDestroyWindow( WinWindowFromID( hwnd, DID_FILTER_TXT));
   WinDestroyWindow( WinWindowFromID( hwnd, DID_FILTER_CB));
   WinDestroyWindow( WinWindowFromID( hwnd, DID_DIRECTORY_TXT));
   WinDestroyWindow( WinWindowFromID( hwnd, DID_APPLY_PB));

   // --------------------------------

   // remove help button if not requested
   if (!(pdd->fl & WTK_DDS_HELPBUTTON))
      WinDestroyWindow( WinWindowFromID( hwnd, DID_HELP_PB));

   } while (FALSE);

return;
}

// =============================================================================
/*
@@WtkDefDirDlgProc@SYNTAX
This function is the default dialog procedure for the directory dialog.

@@WtkDefDirDlgProc@PARM@hwnd@in
Dialog-window handle.

@@WtkDefDirDlgProc@PARM@msg@in
Message identity.

@@WtkDefDirDlgProc@PARM@mp1@in
Parameter 1.

@@WtkDefDirDlgProc@PARM@mp2@in
Parameter 2.

@@WtkDefDirDlgProc@RETURN
Message-return data.

@@WtkDefDirDlgProc@REMARKS
All unprocessed messages in a custom dialog procedure for the WtkDirDlg
function should be passed to this default file dialog procedure so that
the dialog can implement its default behavior.

@@
*/

MRESULT EXPENTRY WtkDefDirDlgProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
         MRESULT        mresult;
         PDIRDLG        pdd = WinQueryWindowPtr( hwnd, QWL_USER);

static   CHAR           szDefaultTitle[ _MAX_PATH];

switch (msg)
   {
   case WM_INITDLG:
      {
      // save ptr to data
      pdd = (PDIRDLG)mp2;
      if (!pdd)
         break;
      WinSetWindowULong( hwnd, QWL_USER, (ULONG)pdd);

      // patch the window ID in order to support specific help panel
      if (pdd->fl & WTK_DDS_FORCEDIALOGID)
         WinSetWindowUShort( hwnd, QWS_ID, pdd->usDlgId);

      // take care for controls of default dialog
      if(!(pdd->fl & WTK_DDS_CUSTOM))
         _initializeControls( hwnd);

      // disallow entering wildcard characters in entry field
      WtkFilterEntryField( WinWindowFromID( hwnd, DID_FILENAME_ED), NULL, "?*");

      // set ownerdraw style for listbox
      WinSetWindowBits( WinWindowFromID( hwnd, DID_DIRECTORY_LB),
                        QWL_STYLE, LS_OWNERDRAW, LS_OWNERDRAW);

      // set maximum text length of enty field for path
      WinSendDlgItemMsg( hwnd, DID_FILENAME_ED, EM_SETTEXTLIMIT,
                         MPFROMLONG( sizeof( pdd->szFullDir) - 1),
                         0);

      // either set requested title or get default title for message boxes !)
      if (pdd->pszTitle)
         WinSetDlgItemText( hwnd, FID_TITLEBAR, pdd->pszTitle);
      else
         {
         WinQueryDlgItemText( hwnd, FID_TITLEBAR, sizeof( szDefaultTitle), szDefaultTitle);
         pdd->pszTitle = szDefaultTitle;
         }

      // set ok button
      if (pdd->pszOKButton)
         WinSetDlgItemText( hwnd, FID_TITLEBAR, pdd->pszOKButton);

      // center in parent if requested
      if (pdd->fl & WTK_DDS_CENTER)
         WtkCenterWindow( hwnd, WinQueryWindow( hwnd, QW_PARENT), WTK_CENTERWINDOW_BOTH);
      else
         WinSetWindowPos( hwnd, NULLHANDLE, pdd->x, pdd->y, 0, 0, SWP_MOVE);

      // update controls with provided path
      _updateDriveCombobox( hwnd);
      _updateDirListbox( hwnd);

      // call subclass procedure
      if (pdd->pfnDlgProc)
         {
         // call window procedure for WM_INITDLG first
         // -> oops, disabled, because otherwise we are called recursively !!!!
         // (pdd->pfnDlgProc)( hwnd, msg, mp1, mp2);

         // now subclass window
         WinSubclassWindow( hwnd, pdd->pfnDlgProc);
         }

      // set focus to directory entry field
      WinSetFocus( HWND_DESKTOP, WinWindowFromID( hwnd, DID_DIRECTORY_LB));

      return (MRESULT) TRUE;
      }
      break; // case WM_INITDLG:

   // ------------------------------------------------------------------------

   case WM_HELP:
      // forward help messages to client of owner
      WinSendMsg( WinWindowFromID( WinQueryWindow( hwnd, QW_OWNER), FID_CLIENT), msg, mp1, mp2);
      return (MRESULT) FALSE;
      break;

   // ------------------------------------------------------------------------

   case WM_COMMAND:
      switch (SHORT1FROMMP( mp1))
         {
         case DID_OK:
            {
                     APIRET         rc;
                     PSZ            pszLastChar;
                     CHAR           szDrive[ _MAX_PATH];

            // if anything is selected in the listbox, take default action
            if (WinQueryLboxSelectedItem( WinWindowFromID( hwnd, DID_DIRECTORY_LB)) != LIT_NONE)
               break;

            // retrieve currently entered path
            if (!WinQueryDlgItemText( hwnd, DID_FILENAME_ED, sizeof( pdd->szFullDir), pdd->szFullDir))
               break;

            // check if drive is valid
            rc = _checkDrive( hwnd, pdd->szFullDir, TRUE);
            if (rc != NO_ERROR)
               {
               // set focus back to entry field
               WinSetFocus( HWND_DESKTOP, WinWindowFromID( hwnd, DID_FILENAME_ED));

               return (MRESULT) TRUE;
               }

            // if last char is a backslash, switch to
            // directory instead of leaving the dialog
            pszLastChar = &pdd->szFullDir[ strlen( pdd->szFullDir) - 1];
            if (*pszLastChar == '\\')
               {
               // delete backslash for non root directories
               if (*(pszLastChar - 1) != ':')
                  *pszLastChar = 0;

               // make sure to uppercase drive letters
               if (pdd->szFullDir[ 1] == ':')
                  pdd->szFullDir[ 0] = toupper( pdd->szFullDir[ 0]);

               // display new status
               _updateDriveCombobox( hwnd);
               _updateDirListbox( hwnd);

               return (MRESULT) TRUE;
               }
            else
               {
               // check for directory
               if (!WtkDirExists( pdd->szFullDir))
                  {
                  pdd->rc = ERROR_PATH_NOT_FOUND;
                  WinDismissDlg( hwnd, DID_ERROR);
                  return (MRESULT) TRUE;
                  }
               }

            }
            break; // case DID_OK:

         } // switch (SHORT1FROMMP( mp1))

      break; // case WM_COMMAND:

   // ------------------------------------------------------------------------

   case WM_CONTROL:

      switch (SHORT1FROMMP( mp1))
         {
         case DID_DIRECTORY_LB:
            if (SHORT2FROMMP( mp1) == LN_ENTER)
               {
               _determineSelectedPath( hwnd);
               _updateDriveCombobox( hwnd);
               _updateDirListbox( hwnd);
               }

         // --------------------

         case DID_DRIVE_CB:
            if (SHORT2FROMMP( mp1) == CBN_ENTER)
               {
               _determinePathOfSelectedDrive( hwnd);
               _updateDriveCombobox( hwnd);
               _updateDirListbox( hwnd);
               }

            break; // case DID_DIRECTORY_LB:

         // --------------------

         case DID_FILENAME_ED:
            if (SHORT2FROMMP( mp1) == EN_CHANGE)
               {
                        CHAR           szCurrentPath[ _MAX_PATH];

               // deselect all item in listbox if entryfield got changed
               WinQueryDlgItemText( hwnd, DID_FILENAME_ED, sizeof( szCurrentPath), szCurrentPath);
               if (stricmp(pdd->szFullDir, szCurrentPath))
                  WinSendDlgItemMsg( hwnd, DID_DIRECTORY_LB, LM_SELECTITEM,
                                     MPFROMLONG( LIT_NONE), MPFROMLONG( FALSE));
               }
            break; // case DID_FILENAME_ED:

         } // switch (SHORT1FROMMP( mp1))

      break; // case WM_CONTROL:

   // ------------------------------------------------------------------------

   case WM_DRAWITEM:
      // act on listbox only
      if (LONGFROMMP( mp1) != DID_DIRECTORY_LB)
         break;

      // draw item
      _drawListboxItem( hwnd, PVOIDFROMMP( mp2));

      // we did it
      return (MRESULT) TRUE;
      break;

   // ------------------------------------------------------------------------

   case WM_MEASUREITEM:
      {
               ULONG          ulListBoxItem = LONGFROMMP( mp1);
               ULONG          ulItemIndex =  LONGFROMMP( mp2);
               HPS            hps = WinGetPS( hwnd);
               FONTMETRICS    fm;
               CHAR           szItemText[ _MAX_PATH];

               ULONG          ulItemHeight;
               ULONG          ulItemWidth;

      // act on listbox only
      if (ulListBoxItem != DID_DIRECTORY_LB)
         break;

      // get height according to font
      GpiQueryFontMetrics( hps, sizeof( fm), &fm);
      ulItemHeight = fm.lMaxAscender + fm.lMaxDescender;

      // get length of string
      WinQueryLboxItemText( WinWindowFromID( hwnd, ulListBoxItem), ulItemIndex,
                            szItemText, sizeof( szItemText));
      ulItemWidth = WtkGetStringDrawLen( hps, szItemText);

      WinReleasePS( hps);

      return (MRESULT)MPFROM2SHORT( ulItemHeight, ulItemWidth);
      }
      break;

   } // switch (msg)

return WinDefDlgProc( hwnd, msg, mp1, mp2);
}

// -----------------------------------------------------------------------------

/*
@@WtkDirDlg@SYNTAX
This function creates and displays the file dialog for
selection of a directory and returns the user's selection, similar
to the WinFileDlg function.

@@WtkDirDlg@PARM@hwndParent@in
Parent-window handle of the created dialog window.
:parml compact.
:pt.HWND_DESKTOP
:pd.The desktop window.
:pt.Other
:pd.Specified window.
:eparml.

@@WtkDirDlg@PARM@hwndOwner@in
Requested owner-window handle of the created dialog window.
:p.
The actual owner window is calculated using the algorithm specified in the
description of the WinLoadDlg function.

@@WtkDirDlg@PARM@pdd@in
Pointer to a DIRDLG structure.

@@WtkDirDlg@RETURN
Success indicator.
:parml compact.
:pt.TRUE
:pd.The directory dialog could be processed.
:pt.FALSE
:pd.An error occurred.
:eparml.

@@WtkDirDlg@REMARKS
The following flags can be set within DIRDLG.fl&colon
:parml compact.
:pt.WTK_DDS_CENTER
:pd.The dialog is positioned in the center of its parent window,
overriding any specified x, y position.
:pt.WTK_DDS_CUSTOM
:pd.A custom dialog template is used to create the dialog.
The hMod and usDlgID fields must be initialized. The following controls
must exist in that dialog:
.* -----
:parml compact tsize=30 break=none.
:pt.Resource identifier
:pd.PM class
:pt.DID_FILENAME_ED
:pd.WC_ENTRYFIELD
:pt.DID_DRIVE_CB
:pd.WC_COMBOBOX
:pt.DID_DIRECTORY_LB
:pd.WC_LISTBOX
:pt.DID_OK
:pd.WC_BUTTON
:pt.DID_CANCEL
:pd.WC_BUTTON
:eparml.
.* -----
:pt.WTK_DDS_HELPBUTTON
:pd.A Help push button of style (BS_HELP|BS_NOPOINTERFOCUS) with an
identifier of DID_HELP_PB is added to the dialog. When this push button is
pressed, a WM_HELP message is sent to the window procedure of the owner.
.br
As an alternative, a subclass procedure may be specified in DIRDLG.pfnDlgProc,
where that procedure can catch the WM_HELP message and return without
calling the default directory dialog procedure.
:pt.WTK_DDS_PRELOAD_VOLINFO
:pd.If this flag is set, the dialog will preload the volume information for
the drives and will preset the current default directory for each drive.
The default behavior is for the volume label to be blank.
:pt.WTK_DDS_FORCEDIALOGID
:pd.If this flag is set, also the resource identifier of the default dialog
is overridden by the identifier specified in the DIRDLG structure. Normally,
this identifier is always DID_FILE_DIALOG.
:p.
Overriding this default identifier allows to associate different help panels
to default directory dialogs being launched for different purposes.
:eparml.
:p.
On return, the FILEDLG structure is updated with any user alterations, and the
field is set to the value returned by the file dialog's WinDismissDlg function. By
default, this is the ID of the push button pressed to dismiss the dialog, DID_OK
or DID_CANCEL, unless the application supplied additional push buttons in its
template.
:p.
For convenience, the pointer to the DIRDLG structure is placed in the
QWL_USER field of the dialog's frame window. If in a custom file
dialog procedure the pointer to the DIRDLG structure is desired,
it should be queried from the frame window with the WinQueryWindowULong
function.
:p.
To subclass the default file dialog with a new template, the application must give
the module and ID of the new file dialog template and the address of a dialog
procedure for message handling. Window IDs in the range 0x0000 through
0x0FFF are reserved for the standard file dialog controls. IDs from outside this
range must be chosen for any controls or windows added to a custom file dialog.
:p.
:note.
The subclass window procedure must call WtkDefDirDlgProc as the default window
procedure.
:p.
Unlike WinFileDlg, WtkDirDlg cannot create a modeless dialog.
:p.
This function supports boot drive recognition.
Specify :hp2.?&colon.:ehp2. for the drive in order to
preset a directory from the boot drive.
@@
*/

BOOL APIENTRY WtkDirDlg( HWND hwndParent, HWND hwndOwner, PDIRDLG pdd)
{
         BOOL           fResult = FALSE;
         APIRET         rc = NO_ERROR;

         ULONG          ulDialogId = IDSYSDLG_FILEDIALOG;
         CHAR           szError[ 20];
         BOOL           fModuleLoaded = FALSE;
         HMODULE        hmodResource;

do
   {
   // check parms
   if (!pdd)
      break;

   // check structure and flags
   if ((pdd->cbSize != sizeof( DIRDLG)) ||
       (pdd->fl & ~WTK_DDS_VALID_FLAGS))
      {
      pdd->lReturn = DID_ERROR;
      pdd->rc      = ERROR_INVALID_PARAMETER;
      break;
      }

   // if a start directory is given, which
   // does not exist, return
   if ((pdd->szFullDir[ 0]) &&
       (!WtkDirExists( pdd->szFullDir)))
      {
      pdd->lReturn = DID_ERROR;
      pdd->rc      = ERROR_PATH_NOT_FOUND;
      break;
      }

   if (pdd->fl & WTK_DDS_CUSTOM)
      {
      // use custom dialog
      hmodResource = pdd->hMod;
      ulDialogId = pdd->usDlgId;
      }
   else
      {
      // get module handle from system dll
      rc = DosQueryModuleHandle( pszModuleName, &hmodResource);
      if (rc != NO_ERROR)
         {
         rc = DosLoadModule( szError, sizeof( szError), pszModuleName, &hmodResource);
         if (rc != NO_ERROR)
            break;
         fModuleLoaded = TRUE;
         }

      ulDialogId = IDSYSDLG_FILEDIALOG;
      }

   // support boot drive recognition
   __PatchBootDrive( pdd->szFullDir);

   // execute dialog
   pdd->lReturn  = WinDlgBox( hwndParent, hwndOwner, WtkDefDirDlgProc, hmodResource, ulDialogId, pdd);

   // done
   fResult = TRUE;

   } while (FALSE);

// cleanup
if (fModuleLoaded)
   DosFreeModule( hmodResource);
return fResult;
}


