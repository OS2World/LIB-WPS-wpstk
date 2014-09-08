/****************************** Module Header ******************************\
*
* Module Name: wtkfslvl.c
*
* Source for access functions for syslevel files
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2000
*
* $Id: wtkfslvl.c,v 1.10 2008-09-10 00:12:12 cla Exp $
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
* ===========================================================================
*
\***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define INCL_DOS
#define INCL_ERRORS
#include <os2.h>

#include "wtkfslvl.h"
#include "wpstk.ih"

#define WTK_SYSLEVEL_SIG_NUMERIC   0xFFFF
#define WTK_SYSLEVEL_SIG_LITERAL   "SYSLEVEL"
#define WTK_SYSLEVEL_VERSION       1
#define WTK_SYSLEVEL_VERSION_DATE  0x1234

#define WTK_SYSLEVEL_MAXLEN_CSD  7

#define COPYFIELD( t,s,f) memcpy( t->f, s->f, sizeof( f))


// ---------------------------------------------------------------------------

#pragma pack(1)

// structures used in this module

typedef struct _SYSLEVELHEADER
   {
         USHORT         usSignature;
         CHAR           achSignature[ 8];
         CHAR           achJulian[ 5];
         USHORT         usSlfVersion;
         USHORT         ausReserved[8];
         ULONG          ulTableOffset;
   } SYSLEVELHEADER, *PSYSLEVELHEADER;

typedef struct _SYSLEVELTABLE
   {
         USHORT         usSysId;
         BYTE           bSysEdition;
         BYTE           bSysVersion;
         BYTE           bSysModify;
         USHORT         usSysDate;
         CHAR           achCsdLevel[ WTK_SYSLEVEL_MAXLEN_CSD + 1];
         CHAR           achCsdPrev[ WTK_SYSLEVEL_MAXLEN_CSD + 1];
         CHAR           achSysName[ WTK_SYSLEVEL_MAXLEN_NAME + 1];
         CHAR           achCompId[ WTK_SYSLEVEL_MAXLEN_COMPID];
         BYTE           bRefreshLevel;
         BYTE           bReserved;
         CHAR           achType[ WTK_SYSLEVEL_MAXLEN_APPTYPE + 1];
         BYTE           usReserved[ 5];
   } SYSLEVELTABLE, *PSYSLEVELTABLE;

typedef struct _SYSLEVELDATA
   {
         HFILE          hfile;
         BOOL           fNewFile;
         BOOL           fDataRead;
         BOOL           fDataModified;
         SYSLEVELHEADER slh;
         SYSLEVELTABLE  slt;
   } SYSLEVELDATA, *PSYSLEVELDATA;

#pragma pack()

// ---------------------------------------------------------------------------

static APIRET _openSyslevel( PSZ pszName, PHSYSLEVEL phsl, BOOL fCreateNew)
{
         APIRET         rc = NO_ERROR;
         CHAR           szName[ _MAX_PATH];
         ULONG          ulAction;
         PSYSLEVELDATA  psld;

do
   {
   // check parameters
   if ((pszName  == NULL) ||
       (*pszName == 0))
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   // get memory for data struct
   psld = malloc( sizeof( SYSLEVELDATA));
   if (!psld)
      {
      rc = ERROR_NOT_ENOUGH_MEMORY;
      break;
      }
   memset( psld, 0, sizeof( SYSLEVELDATA));

   // copy name and replace ?: with bootdrive
   strcpy( szName, pszName);
   __PatchBootDrive( szName);

   // search entry
   rc = DosOpen( szName,
                 &psld->hfile,
                 &ulAction,
                 0, 0,
                 OPEN_ACTION_FAIL_IF_NEW | OPEN_ACTION_OPEN_IF_EXISTS,
                 OPEN_FLAGS_FAIL_ON_ERROR | OPEN_SHARE_DENYWRITE | OPEN_ACCESS_READWRITE,
                 NULL);

   if (rc != NO_ERROR)
      break;

   // report pointer as handle
   psld->fNewFile = fCreateNew;
   *phsl = (HSYSLEVEL) psld;

   } while (FALSE);

return rc;

}

// ---------------------------------------------------------------------------

/*
@@WtkOpenSyslevel@SYNTAX
This function opens an existing SYSLEVEL file.

@@WtkOpenSyslevel@PARM@pszName@in
Address of the ASCIIZ path name of the SYSLEVEL file.
:p.
The path name may contain :hp2.?&colon.:ehp2. for the drive in order to
search the file on the boot drive.
:p.
The name may not include wildcards.

@@WtkOpenSyslevel@PARM@phsl@out
The address of a buffer in into which the handle to the
requested SYSLEVEL file is to be returned.

@@WtkOpenSyslevel@RETURN
Return Code.
:p.
WtkOpenSyslevel returns one of the following return codes&colon.

:parml compact break=none.
:pt.0
:pd.NO_ERROR
:pt.87
:pd.ERROR_INVALID_PARAMETER
:eparml.
:p.
or return codes of the following functions
:ul compact.
:li.DosOpen
:eul.

@@WtkOpenSyslevel@REMARKS
This function supports boot drive recognition.
Specify :hp2.?&colon.:ehp2. for the drive in order to
search the file on the boot drive.

The SYSLEVEL file is opened in exclusive mode and therefore may not be
in access for writing, when :hp2.WtkOpenSyslevel:ehp2. is called.

@@
*/

APIRET APIENTRY WtkOpenSyslevel( PSZ pszName, PHSYSLEVEL phsl)
{
return _openSyslevel( pszName, phsl, FALSE);
}

// ---------------------------------------------------------------------------

/*
@@WtkCloseSyslevel@SYNTAX
This function closes a SYSLEVEL file previously opened with :hp2.WtkCloseSyslevel:ehp2..

@@WtkCloseSyslevel@PARM@hsl@in
Handle to the SYSLEVEL file.

@@WtkCloseSyslevel@PARM@ulUpdateMode@in
The update operation to be performed.

:parml.
:pt.WTK_SYSLEVEL_CLOSE_DISCARD
:pd.discard any changes by previously set by :hp2.WtkSetSyslevelInfo:ehp2. and close the SYSLEVEL file without changes.
:pt.WTK_SYSLEVEL_CLOSE_UPDATE
:pd.write any changes previously set by :hp2.WtkSetSyslevelInfo:ehp2. to disk and then close the SYSLEVEL file
:eparml.

@@WtkCloseSyslevel@RETURN
Return Code.
:p.
WtkCloseSyslevel returns one of the following return codes&colon.

:parml compact break=none.
:pt.0
:pd.NO_ERROR
:pt.87
:pd.ERROR_INVALID_PARAMETER
:eparml.
:p.
or return codes of the following functions
:ul compact.
:li.DosSetFilePtr
:li.DosWrite
:eul.

@@WtkCloseSyslevel@REMARKS
When closing a SYSLEVEL file, where nothing has been modified by a call to
:hp2.WtkSetSyslevelInfo:ehp2., no write operation will take place even when
the update mode WTK_SYSLEVEL_CLOSE_UPDATE is specified.

@@
*/

APIRET APIENTRY WtkCloseSyslevel( HSYSLEVEL hsl, ULONG ulUpdateMode)
{
         APIRET         rc = NO_ERROR;
         PSYSLEVELDATA  psld = (PSYSLEVELDATA) hsl;

         ULONG          ulFilePtr;
         ULONG          ulBytesWritten;

do
   {
   // check parameters
   if (!hsl)
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   // has data been retrieved ?
   // Has data been modified ?
   // If not, we can quit quickly
   if ((!psld->fDataRead) || (!psld->fDataModified))
      break;

   // if data has been modified, is update requested ?
   // if not, close silently
   if (ulUpdateMode == WTK_SYSLEVEL_CLOSE_DISCARD)
      break;

   // update SYSLEVELTABLE data
   rc = DosSetFilePtr( psld->hfile, psld->slh.ulTableOffset, FILE_BEGIN, &ulFilePtr);
   if (rc != NO_ERROR)
      break;
   rc = DosWrite( psld->hfile, &psld->slt, sizeof( SYSLEVELTABLE), &ulBytesWritten);
   if (rc != NO_ERROR)
      break;

   } while (FALSE);

// close file here
if (rc == NO_ERROR)
   {
   DosClose( psld->hfile);
   memset( psld, 0, sizeof( SYSLEVELDATA));
   free( psld);
   }

return rc;

}

// ---------------------------------------------------------------------------

/*
@@WtkQuerySyslevelInfo@SYNTAX
This function queries a SYSLEVEL information

@@WtkQuerySyslevelInfo@PARM@hsl@in
Handle to the SYSLEVEL file.

@@WtkQuerySyslevelInfo@PARM@psli@in
An address to the buffer where the SYSLEVEL information is to be returned.

This pointer can be NULL in order to only validate the data within the file.

@@WtkQuerySyslevelInfo@RETURN
Return Code.
:p.
WtkQuerySyslevelInfo returns one of the following return codes&colon.

:parml compact break=none.
:pt.0
:pd.NO_ERROR
:pt.13
:pd.ERROR_INVALID_DATA
:pt.87
:pd.ERROR_INVALID_PARAMETER
:eparml.
:p.
or return codes of the following functions
:ul compact.
:li.DosSetFilePtr
:li.DosRead
:eul.

@@WtkQuerySyslevelInfo@REMARKS
If the SYSLEVEL information has been modified by previous calls to :hp2.WtkSetSyslevelInfo:ehp2.,
the modified data is returned.

@@
*/

APIRET APIENTRY WtkQuerySyslevelInfo( HSYSLEVEL hsl, PSYSLEVELINFO psli)
{
         APIRET         rc = NO_ERROR;
         PSYSLEVELDATA  psld = (PSYSLEVELDATA) hsl;

static   PSZ            pszFileSig = WTK_SYSLEVEL_SIG_LITERAL;

         ULONG          ulFilePtr;
         ULONG          ulBytesRead;

         PSYSLEVELHEADER pslh;
         PSYSLEVELTABLE  pslt;

do
   {
   // check parameters
   if (!hsl)
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   // if data not yet read, fetch from disk
   if (!psld->fDataRead)
      {
      // read header
      rc = DosSetFilePtr( psld->hfile, 0, FILE_BEGIN, &ulFilePtr);
      if (rc != NO_ERROR)
         break;
      rc = DosRead( psld->hfile, &psld->slh, sizeof( psld->slh), &ulBytesRead);
      if (rc != NO_ERROR)
         break;

      // check signature
      pslh = &psld->slh;
      if ((pslh->usSignature != WTK_SYSLEVEL_SIG_NUMERIC) ||
          (memcmp( pslh->achSignature, pszFileSig, strlen( pszFileSig))))
         {
         rc = ERROR_INVALID_DATA;
         break;
         }

      // read SYSLEVELTABLE
      rc = DosSetFilePtr( psld->hfile, pslh->ulTableOffset, FILE_BEGIN, &ulFilePtr);
      if (rc != NO_ERROR)
         break;
      rc = DosRead( psld->hfile, &psld->slt, sizeof( psld->slt), &ulBytesRead);
      if (rc != NO_ERROR)
         break;

      // mark data as being read
      psld->fDataRead = TRUE;
      }

   // if no data queried, only data is checked
   if (!psli)
      break;


   // access and transfer data to public structure
   pslt = &psld->slt;
   memset( psli, 0, sizeof( SYSLEVELINFO));

   psli->usSysId = pslt->usSysId;
   memcpy( psli->szComponentId, pslt->achCompId,   WTK_SYSLEVEL_MAXLEN_COMPID);
   memcpy( psli->szName,        pslt->achSysName,  WTK_SYSLEVEL_MAXLEN_NAME);

   memcpy( psli->szCsdPrefix,   pslt->achCsdLevel, WTK_SYSLEVEL_MAXLEN_PREFIX);
   memcpy( psli->szCurrentCsd,  &pslt->achCsdLevel[ WTK_SYSLEVEL_MAXLEN_PREFIX + 1], WTK_SYSLEVEL_MAXLEN_CSDNUM);
   memcpy( psli->szPreviousCsd, &pslt->achCsdPrev[ WTK_SYSLEVEL_MAXLEN_PREFIX + 1],  WTK_SYSLEVEL_MAXLEN_CSDNUM);

   memcpy( psli->szAppType,     pslt->achType, WTK_SYSLEVEL_MAXLEN_APPTYPE);


   psli->chCsdLanguage    = pslt->achCsdLevel[ WTK_SYSLEVEL_MAXLEN_PREFIX];
   psli->usDate           = pslt->usSysDate;

   // doc from syslevel.txt
   // version is:                V x.xy R z
   // pslt->bSysVersion    xx =    x.x
   // pslt->bSysModify      y =       y
   // pslt->bRefreshLevel   z =           z

   psli->bVersionMajor    = (pslt->bSysVersion & 0xF0) >> 4;
   psli->bVersionMinor    = ((pslt->bSysVersion & 0x0F) << 4);
   psli->bVersionMinor   += pslt->bSysModify;
   psli->bVersionRefresh  = pslt->bRefreshLevel;

   // cut of trailing underscore from CSD number
   __strip( psli->szCurrentCsd,  '_');
   __strip( psli->szPreviousCsd, '_');

   } while (FALSE);

return rc;
}

// ---------------------------------------------------------------------------

/*
@@WtkSetSyslevelInfo@SYNTAX
This function modifies a SYSLEVEL information.

@@WtkSetSyslevelInfo@PARM@hsl@in
Handle to the SYSLEVEL file.

@@WtkSetSyslevelInfo@PARM@ulUpdateFlags@in
A variable specifying which fields have to be updated.

Sepcify one of the following flags to udate the corresponding field
passed within the SYSLEVELINFO buffer or specify WTK_SYSLEVEL_UPDATE_ALL
to update all fields:
:ul compact.
:li.WTK_SYSLEVEL_UPDATE_ALL
:li.WTK_SYSLEVEL_UPDATE_SYSID
:li.WTK_SYSLEVEL_UPDATE_COMPONENTID
:li.WTK_SYSLEVEL_UPDATE_NAME
:li.WTK_SYSLEVEL_UPDATE_VERSION
:li.WTK_SYSLEVEL_UPDATE_CSDPREFIX
:li.WTK_SYSLEVEL_UPDATE_CSDLANGUAGE
:li.WTK_SYSLEVEL_UPDATE_CURRENTCSD
:li.WTK_SYSLEVEL_UPDATE_PREVIOUSCSD
:li.WTK_SYSLEVEL_UPDATE_DATE
:li.WTK_SYSLEVEL_UPDATE_APPTYPE
:eul.

@@WtkSetSyslevelInfo@PARM@psli@in
An address to the buffer where the updated SYSLEVEL information is provided.

@@WtkSetSyslevelInfo@RETURN
Return Code.
:p.
WtkSetSyslevelInfo returns one of the following return codes&colon.

:parml compact break=none.
:pt.0
:pd.NO_ERROR
:pt.13
:pd.ERROR_INVALID_DATA
:pt.87
:pd.ERROR_INVALID_PARAMETER
:eparml.

@@WtkSetSyslevelInfo@REMARKS
The changes specified by this call are not directly written to disk. Instead,
:hp2.WtkCloseSyslevel:ehp2. must be called with WTK_SYSLEVEL_CLOSE_UPDATE set as update mode in
order to make the changes permanent.

@@
*/

APIRET APIENTRY WtkSetSyslevelInfo( HSYSLEVEL hsl, ULONG ulUpdateFlags, PSYSLEVELINFO psli)
{
         APIRET         rc = NO_ERROR;
         PSYSLEVELDATA  psld = (PSYSLEVELDATA) hsl;

         PSYSLEVELTABLE  pslt;

do
   {
   // check parameters
   if ((!hsl)           ||
       (!ulUpdateFlags) ||
       (!psli))
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   // validation checks
   if (ulUpdateFlags & WTK_SYSLEVEL_UPDATE_VERSION)
      {
      if (psli->bVersionMajor > 0x0F)
         rc = ERROR_INVALID_DATA;

      if (rc != NO_ERROR)
         break;
      }


   // if data of an existing file is not yet read, go for it
   if ((!psld->fDataRead) && (!psld->fNewFile))
      {
      rc = WtkQuerySyslevelInfo( hsl, psli);
      if (rc != NO_ERROR)
         break;
      }

   pslt = &psld->slt;

   // update fields
   if (ulUpdateFlags & WTK_SYSLEVEL_UPDATE_SYSID)
      {
      pslt->usSysId = psli->usSysId ;
      psld->fDataModified = TRUE;
      }

   if (ulUpdateFlags & WTK_SYSLEVEL_UPDATE_COMPONENTID)
      {
      memset( pslt->achCompId, 0, sizeof( pslt->achCompId));
      memcpy( pslt->achCompId, psli->szComponentId, WTK_SYSLEVEL_MAXLEN_COMPID);
      psld->fDataModified = TRUE;
      }

   if (ulUpdateFlags & WTK_SYSLEVEL_UPDATE_NAME)
      {
      memset( pslt->achSysName, 0, sizeof( pslt->achSysName));
      memcpy( pslt->achSysName, psli->szName, WTK_SYSLEVEL_MAXLEN_NAME);
      psld->fDataModified = TRUE;
      }

   if (ulUpdateFlags & WTK_SYSLEVEL_UPDATE_VERSION)
      {
      pslt->bSysVersion   = psli->bVersionMajor << 4;
      pslt->bSysVersion  += (psli->bVersionMinor & 0xF0) >> 4;
      pslt->bSysModify    = psli->bVersionMinor & 0x0F;
      pslt->bRefreshLevel = psli->bVersionRefresh;
      psld->fDataModified = TRUE;
      }

   if (ulUpdateFlags & WTK_SYSLEVEL_UPDATE_CSDPREFIX)
      {
      memcpy( pslt->achCsdLevel, psli->szCsdPrefix, WTK_SYSLEVEL_MAXLEN_PREFIX);
      psld->fDataModified = TRUE;
      }

   if (ulUpdateFlags & WTK_SYSLEVEL_UPDATE_CSDLANGUAGE)
      {
      pslt->achCsdLevel[ WTK_SYSLEVEL_MAXLEN_PREFIX] = psli->chCsdLanguage;
      pslt->achCsdPrev[ WTK_SYSLEVEL_MAXLEN_PREFIX]  = psli->chCsdLanguage;
      psld->fDataModified = TRUE;
      }

   if (ulUpdateFlags & WTK_SYSLEVEL_UPDATE_CURRENTCSD)
      {
      memcpy( &pslt->achCsdLevel[ WTK_SYSLEVEL_MAXLEN_PREFIX + 1], psli->szCurrentCsd, WTK_SYSLEVEL_MAXLEN_CSDNUM);
      pslt->achCsdLevel[ WTK_SYSLEVEL_MAXLEN_PREFIX + WTK_SYSLEVEL_MAXLEN_CSDNUM + 1] = '_';
      psld->fDataModified = TRUE;
      }

   if (ulUpdateFlags & WTK_SYSLEVEL_UPDATE_PREVIOUSCSD)
      {
      memcpy( &pslt->achCsdPrev[ WTK_SYSLEVEL_MAXLEN_PREFIX + 1], psli->szPreviousCsd, WTK_SYSLEVEL_MAXLEN_CSDNUM);
      pslt->achCsdPrev[ WTK_SYSLEVEL_MAXLEN_PREFIX + WTK_SYSLEVEL_MAXLEN_CSDNUM + 1] = '_';
      psld->fDataModified = TRUE;
      }

   if (ulUpdateFlags & WTK_SYSLEVEL_UPDATE_DATE)
      {
      pslt->usSysDate = psli->usDate;
      psld->fDataModified = TRUE;
      }

   if (ulUpdateFlags & WTK_SYSLEVEL_UPDATE_APPTYPE)
      {
      memset( pslt->achType, 0, sizeof( pslt->achType));
      memcpy( pslt->achType, psli->szAppType, WTK_SYSLEVEL_MAXLEN_APPTYPE);
      psld->fDataModified = TRUE;
      }

   } while (FALSE);

return rc;
}

