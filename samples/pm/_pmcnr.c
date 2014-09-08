/****************************** Module Header ******************************\
*
* Module Name: _pmcnr.c
*
* PM helper functions sample - container control related code
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2004
*
* $Id: _pmcnr.c,v 1.4 2008-12-25 21:12:09 cla Exp $
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

#include "_pmcnr.h"
#include "_pm.rch"

#pragma pack( 1)

// --- data for container items

typedef struct _DATAENTRY
{
  PSZ            pszName;
  PSZ            pszDescription;
} DATAENTRY, *PDATAENTRY;

static   DATAENTRY      ade[] = { {"Item 1", "This is data entry 1"},
                                  {"Item 2", "This is data entry 2"},
                                  {"Item 3", "This is data entry 3"},
                                  {"Item 4", "This is data entry 4"},
                                  {"Item 5", "This is data entry 5"},
                                  {"Item 6", "This is data entry 6"} };

#define DATACOUNT (sizeof( ade) / sizeof( DATAENTRY))

static   PSZ            apszDetailsTitle [] = { "Item", "Desription"};

// --- definitions for page handling

typedef struct _DLGDATA
{
  HMODULE        hmodResource;
} DLGDATA, *PDLGDATA;

// --- definitions for container handling

typedef struct _SAMPLERECORD {
  PSZ            pszName;
  PSZ            pszDescription;
} SAMPLERECORD , *PSAMPLERECORD;

#define CNR_COLUMNS_COUNT 2

#pragma pack()

// --------------------------------------------------------------------------

static BOOL _usesMiniRecordcore(  HWND hwnd, ULONG ulCnrId)
{
         BOOL           fResult = FALSE;
         HWND           hwndCnr = WinWindowFromID( hwnd, ulCnrId);
         ULONG          ulContainerStyle;

if (hwndCnr)
   {
   ulContainerStyle = WinQueryWindowULong( hwndCnr, QWL_STYLE);
   fResult = ((ulContainerStyle  & CCS_MINIRECORDCORE) != 0);
   }

return fResult;
}

// --------------------------------------------------------------------------

static ULONG _getRecordDataOfs(  HWND hwnd, ULONG ulCnrId)
{
return _usesMiniRecordcore( hwnd, ulCnrId) ?
                            sizeof( MINIRECORDCORE) :
                            sizeof( RECORDCORE);
}

// --------------------------------------------------------------------------

static PVOID _getRecordDataPtr( HWND hwnd, ULONG ulCnrId, PRECORDCORE prec)
{
return (PVOID) ((PBYTE)prec + _getRecordDataOfs( hwnd, ulCnrId));
}

// --------------------------------------------------------------------------

static BOOL _initializeContainer( HWND hwnd, ULONG ulCnrId)
{
         BOOL           fResult = FALSE;
         PFIELDINFO     pfi, pfiFirst, pfiLast;
         FIELDINFOINSERT fii;
         CNRINFO        cnri;
         ULONG          ulDataOfs = _getRecordDataOfs( hwnd, ulCnrId);

do
   {
   // get memory for details info
   pfi = WinSendDlgItemMsg( hwnd, ulCnrId,
                            CM_ALLOCDETAILFIELDINFO,
                            MPFROMLONG( CNR_COLUMNS_COUNT), 0);
   if (!pfi)
      break;

   pfiFirst = pfi;  // keep pointer to first structure
   pfiLast  = pfi;  // keep ptr to last column left to splitbar
                    // here both are the same...

   // column for item name
   pfi->flData     = CFA_STRING | CFA_LEFT | CFA_HORZSEPARATOR;
   pfi->flTitle    = CFA_LEFT | CFA_FITITLEREADONLY;
   pfi->pTitleData = apszDetailsTitle[ 0];
   pfi->offStruct  = ulDataOfs + FIELDOFFSET( SAMPLERECORD, pszName);
   pfi->cxWidth    = 0;

   // column for item description
   pfi             = pfi->pNextFieldInfo;
   pfi->flData     = CFA_STRING  | CFA_LEFT | CFA_HORZSEPARATOR;
   pfi->flTitle    = CFA_LEFT | CFA_FITITLEREADONLY;
   pfi->pTitleData = apszDetailsTitle[ 1];
   pfi->offStruct  = ulDataOfs + FIELDOFFSET( SAMPLERECORD, pszDescription);
   pfi->cxWidth    = 0;

   // send details info to container
   memset( &fii, 0, sizeof( FIELDINFOINSERT));
   fii.cb                   = sizeof( FIELDINFOINSERT);
   fii.pFieldInfoOrder      = (PFIELDINFO) CMA_END;
   fii.cFieldInfoInsert     = (SHORT) CNR_COLUMNS_COUNT;
   fii.fInvalidateFieldInfo = TRUE;

   if (WinSendDlgItemMsg( hwnd, ulCnrId,
                          CM_INSERTDETAILFIELDINFO,
                          MPFROMP (pfiFirst),
                          MPFROMP (&fii)) != (MRESULT) CNR_COLUMNS_COUNT)
      break;

   // -----------------------------------------------------

   // setup container attributes
   memset( &cnri, 0, sizeof( CNRINFO));
   cnri.pFieldInfoLast    = pfiLast;
   cnri.flWindowAttr      = CA_DETAILSVIEWTITLES | CV_DETAIL;
   cnri.xVertSplitbar     = 55;
   WinSendDlgItemMsg( hwnd, ulCnrId,
                            CM_SETCNRINFO,
                            MPFROMP (&cnri),
                            MPFROMLONG( CMA_PFIELDINFOLAST |
                                        CMA_FLWINDOWATTR   |
                                        CMA_XVERTSPLITBAR));

   // done
   fResult = TRUE;

   } while (FALSE);

return fResult;
}

// --------------------------------------------------------------------------

static PRECORDCORE _insertItem( HWND hwnd, ULONG ulCnrId,
                                PVOID pvPrivateData, ULONG ulExtraMem,
                                PSZ pszName, HPOINTER hptrIcon)

{
         PRECORDCORE    prec = NULL;
         PVOID          pvRecordData;
         RECORDINSERT   recinsert;

do
   {
   // get memory for container records
   prec = (PRECORDCORE) WinSendDlgItemMsg( hwnd, ulCnrId,
                                           CM_ALLOCRECORD,
                                           MPFROMLONG( ulExtraMem),
                                           MPFROMLONG( 1));
   if (!prec)
      break;

   // setup standard values
   prec->flRecordAttr = CRA_RECORDREADONLY;
   prec->pszIcon      = pszName;
   prec->hptrIcon     = hptrIcon;

   // copy over private data
   pvRecordData = _getRecordDataPtr( hwnd, ulCnrId, prec);
   memcpy( pvRecordData, pvPrivateData, ulExtraMem);

   // pass record to container
   memset( &recinsert, 0, sizeof( RECORDINSERT));
   recinsert.cb                 = sizeof( RECORDINSERT);
   recinsert.pRecordOrder       = (PVOID) CMA_END;
   recinsert.pRecordParent      = NULL;
   recinsert.fInvalidateRecord  = TRUE;
   recinsert.zOrder             = CMA_TOP;
   recinsert.cRecordsInsert     = 1;
   if (!WinSendDlgItemMsg( hwnd, ulCnrId,
                           CM_INSERTRECORD,
                           MPFROMP( prec),
                           MPFROMP( &recinsert)))
      break;

   } while (FALSE);

return prec;
}

// --------------------------------------------------------------------------

VOID _unloadContainer( HWND hwnd, ULONG ulCnrId)
{
WinSendDlgItemMsg( hwnd, ulCnrId, CM_REMOVERECORD, 0, MPFROM2SHORT( 0, CMA_FREE));
return;
}

// --------------------------------------------------------------------------

static BOOL _loadContainer( HWND hwnd, ULONG ulCnrId)
{
         BOOL           fResult = FALSE;
         ULONG          i;

         PRECORDCORE    prec;
         SAMPLERECORD   sr;
         PDATAENTRY     pde;
         HPOINTER       hptrStandard = WinQuerySysPointer( HWND_DESKTOP, SPTR_FILE, FALSE);
do
   {
   // avoid flickering
   WinEnableWindowUpdate( WinWindowFromID( hwnd, ulCnrId), FALSE);

   // remove all items
   _unloadContainer( hwnd, ulCnrId);

   // insert items
   for (i = 0, pde = ade;
        i < DATACOUNT;
        i++, pde++)
      {
      // setup data for record and insert
      memset( &sr, 0, sizeof( sr));
      sr.pszName        = pde->pszName;
      sr.pszDescription = pde->pszDescription;
      prec = _insertItem( hwnd, ulCnrId,
                          &sr, sizeof( sr),
                          pde->pszName, hptrStandard);
      }

   // arrange items
   WinSendDlgItemMsg( hwnd, ulCnrId, CM_ARRANGE, CMA_ARRANGESTANDARD, 0);

   // done
   fResult = TRUE;

   } while (FALSE);

// reenable window update
WinEnableWindowUpdate( WinWindowFromID( hwnd, ulCnrId), TRUE);

// set focus on container
WinSetFocus( HWND_DESKTOP, WinWindowFromID( hwnd, ulCnrId));

return fResult;
}

// --------------------------------------------------------------------------

static VOID _removeSelectedContainerItems( HWND hwnd, ULONG ulCnrId)
{
         PRECORDCORE    prec;

// get 1st selected record
prec = (PRECORDCORE) WinSendDlgItemMsg( hwnd, ulCnrId,
                                        CM_QUERYRECORDEMPHASIS,
                                        MPFROMLONG( CMA_FIRST),
                                        MPFROMLONG( CRA_SELECTED));

while ((prec) && (prec != (PRECORDCORE) -1))
   {
   // remove this record
   WinSendDlgItemMsg( hwnd, ulCnrId,
                      CM_REMOVERECORD,
                      MPFROMP( &prec),
                      MPFROM2SHORT( 1, CMA_FREE | CMA_INVALIDATE));

   // get next selected record
   prec = (PRECORDCORE) WinSendDlgItemMsg( hwnd, ulCnrId,
                                                   CM_QUERYRECORDEMPHASIS,
                                                   MPFROMLONG( CMA_FIRST),
                                                   MPFROMLONG( CRA_SELECTED));
   } // while (prec)

return;
}

// --------------------------------------------------------------------------

// window procedure for the notebook page with the container

MRESULT EXPENTRY _cnrDlgProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{

         PDLGDATA       pdd  = (PVOID) WinQueryWindowPtr( hwnd, QWL_USER);

switch (msg)
   {

   case WM_INITDLG:
      {
      // store ptr to window data
      pdd = (PDLGDATA)mp2;
      if (!pdd)
         break;

      // store page data
      WinSetWindowPtr( hwnd, QWL_USER, pdd);

      // initialize container
      _initializeContainer( hwnd, IDCNR_CNRSAMPLE);

      // perform undo to load values for the first time
      WinSendMsg( hwnd, WM_COMMAND, MPFROMLONG( IDPBS_UNDO), 0);

      return (MRESULT) FALSE;
      }
      break; // case WM_INITDLG:


   // ---------------------

   case WM_COMMAND:
      {
      switch (LONGFROMMP( mp1))
         {
         case IDPBS_ADD:
            break;

         case IDPBS_REMOVE:
            _removeSelectedContainerItems( hwnd, IDCNR_CNRSAMPLE);
            break; // case IDPBS_REMOVE:

         case IDPBS_UNDO:
            // (re)load container
            _loadContainer( hwnd, IDCNR_CNRSAMPLE);
            break; // case IDPBS_UNDO:

         case IDPBS_HELP:

            break; // case IDPBS_HELP:

         }  // switch (LONGFROMMP( mp1))

      return (MRESULT) TRUE;
      }
      break; // case WM_COMMAND:

   // ---------------------

   case WM_CONTROL:
      {
      switch (SHORT1FROMMP( mp1))
         {
         case IDCNR_CNRSAMPLE:

            switch (SHORT2FROMMP( mp1))
               {

               // act on modification of item selection
               // en/disable remove button
               case CN_EMPHASIS:
                  {
                           PNOTIFYRECORDEMPHASIS pnre = PVOIDFROMMP( mp2);
                           BOOL            fItemSelected;

                  if ((!pnre) ||
                      (!(pnre->fEmphasisMask & CRA_SELECTED)))
                     break;

//                  _handlePushButtons( hwnd);
                  }
                  break; // case :


               }  // switch (SHORT2FROMMP( mp1))

            break;

         }  // switch (SHORT1FROMMP( mp1))

      }
      break; // case WM_CONTROL:

   // ---------------------

   case WM_DESTROY:
      // cleanup container
      _unloadContainer( hwnd, IDCNR_CNRSAMPLE);
      break; // case WM_DESTROY:

   } //switch (msg)

return WinDefDlgProc( hwnd, msg, mp1, mp2);
}


// -----------------------------------------------------------------------------

ULONG LaunchContainerDialog( HWND hwnd, HMODULE hmodResource)
{
         ULONG          ulResult;
         DLGDATA        dd;


// init page data
memset( &dd, 0, sizeof( dd));
dd.hmodResource = hmodResource;

ulResult = WinDlgBox( HWND_DESKTOP, HWND_DESKTOP,
                      _cnrDlgProc, hmodResource,
                      IDDLG_CNRSAMPLE, &dd);

return ulResult;
}

