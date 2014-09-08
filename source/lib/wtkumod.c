/****************************** Module Header ******************************\
*
* Module Name: wtkumod.c
*
* Source for module utility functions
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2000
*
* $Id: wtkumod.c,v 1.17 2008-08-21 20:00:06 cla Exp $
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
#define INCL_ERRORS
#include <os2.h>

#include "wtkumod.h"
#include "wtkufil.h"
#include "wpstk.ih"

#ifndef DosQueryModFromEIP 
APIRET APIENTRY  DosQueryModFromEIP(HMODULE *phMod,
                                     ULONG *pObjNum,
                                     ULONG BuffLen,
                                     PCHAR pBuff,
                                     ULONG *pOffset,
                                     ULONG Address);
#endif

// ---------------------------------------------------------------------------

/*
@@WtkGetModuleHandle@SYNTAX
This function returns the handle of the module containing a given function.

@@WtkGetModuleHandle@PARM@pfn@in
Address of a function from within the module.

@@WtkGetModuleHandle@RETURN
Handle of the module containing the function or NULLHANDLE,
if the handle could not be obtained.

@@WtkGetModuleHandle@REMARKS
This function calls
:link reftype=hd refid=WtkGetModuleInfo.WtkGetModuleInfo:elink.
in order to obtain the module handle.

@@
*/

HMODULE APIENTRY WtkGetModuleHandle( PFN pfn)
{
         HMODULE        hmodResult = NULLHANDLE;

WtkGetModuleInfo( (PFN) pfn, &hmodResult, NULL, 0);
return hmodResult;
}

// ---------------------------------------------------------------------------

/*
@@WtkGetModuleInfo@SYNTAX
This function returns information about the module containing a given function.

@@WtkGetModuleInfo@PARM@pfn@in
Address of a function from within the module.

@@WtkGetModuleInfo@PARM@phmod@out
Address of the module handle, or NULL, if this infortation is not required.

@@WtkGetModuleInfo@PARM@pszBuffer@out
Address of a buffer, where the full qualified name of the module
is being returned.

@@WtkGetModuleInfo@PARM@ulBuflen@in
The length, in bytes, of the buffer described by :hp1.pszBuffer:ehp1..

@@WtkGetModuleInfo@RETURN
Return Code.
:p.
WtkGetModuleInfo returns one of the following return codes&colon.

:parml compact break=none.
:pt.0
:pd.NO_ERROR
:pt.87
:pd.ERROR_INVALID_PARAMETER
:pt.111
:pd.ERROR_BUFFER_OVERFLOW
:eparml.
:p.
or return codes of the following functions
:ul compact.
:li.DosQueryModuleName
:eul.

@@WtkGetModuleInfo@REMARKS
- none - 
@@
*/

APIRET APIENTRY WtkGetModuleInfo( PFN pfn, PHMODULE phmod, PSZ pszBuffer, ULONG ulBuflen)
{
         APIRET         rc = NO_ERROR;
         HMODULE        hmod = NULLHANDLE;
         ULONG          ulObjectNumber  = 0;
         ULONG          ulOffset        = 0;
         CHAR           szDummy[ _MAX_PATH];

do
   {
   // check parms
   if (pfn == NULL)
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   // use own buffer, if none provided
   if (pszBuffer == NULL)
      {
      pszBuffer = szDummy;
      ulBuflen = sizeof( szDummy);
      }

   // get module handle from function adress
   *pszBuffer = 0;
   rc = DosQueryModFromEIP( &hmod, &ulObjectNumber,
                            ulBuflen, pszBuffer,
                            &ulOffset, (ULONG) pfn);

   if (rc != NO_ERROR)
      break;

   // path specified ?
   if (strstr( pszBuffer, ":\\") == NULL)
       rc = DosQueryModuleName( hmod,
                                ulBuflen, pszBuffer);


   } while (FALSE);


// hand over result anyway
if (phmod) *phmod = hmod;
return rc;

}


// ---------------------------------------------------------------------------

/*
@@WtkGetModulePath@SYNTAX
This function returns the directory, where the specified module resides in that
contains a given function.

@@WtkGetModulePath@PARM@pfn@in
Address of a function from within the module.

@@WtkGetModulePath@PARM@pszBuffer@out
Address of a buffer, where the full qualified directory name
is being returned.

@@WtkGetModulePath@PARM@ulBuflen@in
The length, in bytes, of the buffer described by :hp1.pszBuffer:ehp1..

@@WtkGetModulePath@RETURN
Return Code.
:p.
WtkGetModulePath returns one of the following return codes&colon.

:parml compact break=none.
:pt.0
:pd.NO_ERROR
:pt.87
:pd.ERROR_INVALID_PARAMETER
:pt.111
:pd.ERROR_BUFFER_OVERFLOW
:eparml.
:p.
or return codes of the following functions
:ul compact.
:li. DosQueryModuleName
:eul.

@@WtkGetModulePath@REMARKS
This function calls :link reftype=hd refid=WtkGetModuleInfo.WtkGetModuleInfo:elink..

@@
*/

APIRET APIENTRY WtkGetModulePath( PFN pfn, PSZ pszBuffer, ULONG ulBuflen)
{
         APIRET         rc = NO_ERROR;
         PSZ            p;
         CHAR           szDirectory[ _MAX_PATH];
         PSZ            pszPart;

do
   {
   // check parms
   if ((pfn == NULL) || (pszBuffer == NULL))
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   // get filename of module
   rc = WtkGetModuleInfo( pfn, NULL, szDirectory, sizeof( szDirectory));
   if (rc != NO_ERROR)
      break;

   // cut off path
   strcpy( strrchr( szDirectory, '\\'), "");

   // check buffer len
   if (strlen( szDirectory) + 1 > ulBuflen)
      {
      rc = ERROR_BUFFER_OVERFLOW;
      break;
      }

   // copy result
   strcpy( pszBuffer, szDirectory);

   } while (FALSE);

return rc;

}

// -----------------------------------------------------------------------------

/*
@@WtkGetPackageFilename@SYNTAX
This function determines a fully qualified pathname for a file
stored in a path relative to the pathname of a calling DLL.

@@WtkGetPackageFilename@PARM@pfnMod@in
Address of a function residing in the calling DLL code or NULL.
:p.
This parameter determines if the path of the calling DLL or the executable
is used as a base path. If this parameter is NULL, the path of the
executable is used as a base for the resulting filename.
:p.
For easy code maintenance, specifying :hp2.(PFN)__FUNCTION__:ehp2.
is always valid.

@@WtkGetPackageFilename@PARM@pszFileMaskPath@in
Address of the ASCIIZ name of a path with file entries.
:p.
Each entry of the path consists of a path relatively to the
path of the executing module, and a filenanme.

@@WtkGetPackageFilename@PARM@pszBuffer@out
Address of a buffer, where the full qualified filename
is being returned.

@@WtkGetPackageFilename@PARM@ulBuflen@in
The length, in bytes, of the buffer described by :hp1.pszBuffer:ehp1..

@@WtkGetPackageFilename@RETURN
Return Code.
:p.
WtkGetPackageFilename returns one of the following return codes&colon.

:parml compact break=none.
:pt.0
:pd.NO_ERROR
:pt.2
:pd.ERROR_FILE_NOT_FOUND
:pt.87
:pd.ERROR_INVALID_PARAMETER
:pt.111
:pd.ERROR_BUFFER_OVERFLOW
:eparml.
:p.
or return codes of the following functions
:ul compact.
:li. WtkGetModuleInfo
:li. DosQueryPathInfo
:eul.

@@WtkGetPackageFilename@REMARKS
When calling this function, all entries within the file path specified with
parameter :hp1.pszFileMaskPath:ehp1. are
:ol compact.
:li.appended to the path of the executing module
:eol.
:p.
In order to retrieve the name of a file being stored
relative to the pathname of a calling exetubable, call
:link reftype=hd refid=WtkGetPackageFilename.WtkGetPackageFilename:elink.
instead.
@@
*/
APIRET APIENTRY WtkGetPackageFilename( PFN pfnMod, PSZ pszFileMask, PSZ pszBuffer, ULONG ulBuflen)
{
         APIRET         rc = NO_ERROR;
         PSZ            pszLanguage = NULL;

         PPIB           ppib;
         PTIB           ptib;

         CHAR           szFile[ _MAX_PATH];

         CHAR           szMaskEntry[ _MAX_PATH];
         PSZ            pszMaskEntry;
         PSZ            p;

do
   {
   // check parms
   if ((!pszFileMask)  ||
       (!*pszFileMask) ||
       (!pszBuffer))
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   // get module handle from executable or DLL
   if (!pfnMod)
      {
      DosGetInfoBlocks( &ptib, &ppib);
      rc = DosQueryModuleName( ppib->pib_hmte, sizeof( szFile), szFile);
      }
   else
      {
               HMODULE        hmod;
               ULONG          ulObjNum;
               ULONG          ulOffs;

      rc = DosQueryModFromEIP( &hmod, &ulObjNum, sizeof( szFile), szFile, &ulOffs, (ULONG)pfnMod);
      rc = DosQueryModuleName( hmod, sizeof( szFile), szFile);
      }
   if (rc != NO_ERROR)
      break;

   strcpy( strrchr( szFile, '\\') + 1, "");
   strcat( szFile, pszFileMask);


   // if no wildcard given, check existance of file
   if ((!strchr( szFile, '?')) && (!strchr( szFile, '*')))
      if (!WtkFileExists( szFile))
         {
         rc = ERROR_FILE_NOT_FOUND;
         break;
         }

   // hand over result, query full pathname
   rc = DosQueryPathInfo( szFile, FIL_QUERYFULLNAME, pszBuffer, ulBuflen);

   } while (FALSE);

return rc;
}

// ---------------------------------------------------------------------------

/*
@@WtkAssemblePackageFilename@SYNTAX
This function assembles a fully qualified pathname for a given
filename or file extension corresponding to the name of the module /
application executable containing a given function. In contrast to
the function WtkGetPackageFilename the file may not exist.

@@WtkAssemblePackageFilename@PARM@pfn@in
Address of a function from within the module.

@@WtkAssemblePackageFilename@PARM@pszSubdir@in
Address of the ASCIIZ subdirectory name.
:p.
If the required filename should point to a subdirectory,
of where the executable resides, specify it here.
Otherwise, leave this parameter NULL.

@@WtkAssemblePackageFilename@PARM@pszFilename@in
Address of the ASCIIZ file name.
:p.
If the filename should have a different name than the
executable, specify it here.
Otherwise, leave this parameter NULL.
:p.
You can specify an extension either here or in :hp1.pszFileExt:ehp1..

@@WtkAssemblePackageFilename@PARM@pszFileext@in
Address of the ASCIIZ file extension without leading dot.
:p.
If the filename should have a different extension than the
executable, specify it here.
Otherwise, leave this parameter NULL.

@@WtkAssemblePackageFilename@PARM@pszBuffer@out
Address of a buffer, where the full qualified filename
is being returned.

@@WtkAssemblePackageFilename@PARM@ulBuflen@in
The length, in bytes, of the buffer described by :hp1.pszBuffer:ehp1..

@@WtkAssemblePackageFilename@RETURN
Return Code.
:p.
WtkGetPackageFilename returns one of the following return codes&colon.

:parml compact break=none.
:pt.0
:pd.NO_ERROR
:pt.87
:pd.ERROR_INVALID_PARAMETER
:pt.111
:pd.ERROR_BUFFER_OVERFLOW
:eparml.
:p.
or return codes of the following functions
:ul compact.
:li. DosQueryModuleName
:eul.

@@WtkAssemblePackageFilename@REMARKS
This function is mostly used to determine names of INI files
etc. residing e.g. in the same directory as the executable
or in a subdirectory, of where the executable resides.
:p.
In order to determine a file specific to a language (resource modules
or inf files) related to the current executing module,
use one of the following files instead:
:ul compact.
:li.:link reftype=hd refid=WtkGetPackageFilename.WtkGetPackageFilename:elink.
:li.:link reftype=hd refid=WtkGetNlsPackageFilename.WtkGetNlsPackageFilename:elink.
:li.:link reftype=hd refid=WtkLoadNlsResourceModule.WtkLoadNlsResourceModule:elink.
:li.:link reftype=hd refid=WtkLoadNlsInfFile.WtkLoadNlsInfFile:elink.
:eul.
@@
*/

APIRET APIENTRY WtkAssemblePackageFilename( PFN pfn, PSZ pszSubdir, PSZ pszFilename, PSZ pszFileext, PSZ pszBuffer, ULONG ulBuflen)
{
         APIRET         rc = NO_ERROR;
         CHAR           szTargetFile[ 2 * _MAX_PATH];
         PSZ            pszPart;

do
   {
   // check parms
   if ((pfn == NULL) || (pszBuffer == NULL))
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }


   // get filename of module
   rc = WtkGetModuleInfo( pfn, NULL, szTargetFile, sizeof( szTargetFile));
   if (rc != NO_ERROR)
      break;

   // subdir specified ?
   if (pszSubdir)
      {
      pszPart = strrchr( szTargetFile, '\\') + 1;
      strcpy( pszPart, pszSubdir);
      if (pszFilename) strcat( pszPart, pszFilename);
      if (pszFileext)  strcat( pszPart, pszFileext);
      }
   else if (pszFilename)
      {
      pszPart = strrchr( szTargetFile, '\\') + 1;
      strcpy( pszPart, pszFilename);
      if (pszFileext)  strcat( pszPart, pszFileext);
      }
   else if (pszFileext)
      {
      pszPart = strrchr( szTargetFile, '.') + 1;
      strcpy( pszPart, pszFileext);
      }

   // check buffer len
   if (strlen( szTargetFile) + 1 > ulBuflen)
      {
      rc = ERROR_BUFFER_OVERFLOW;
      break;
      }

   // copy result
   strcpy( pszBuffer, szTargetFile);

   } while (FALSE);

return rc;
}

// -----------------------------------------------------------------------------

/*
@@WtkLoadModules@SYNTAX
This function loads modules stored in a path relative to the pathname of 
a calling DLL.
The same module is loaded several times, if it
is available more than once within the specified path.

@@WtkLoadModules@PARM@pfnMod@in
Address of a function residing in the calling DLL code or NULL.
:p.
This parameter determines if the path of the calling DLL or the executable
is used as a base path. If this parameter is NULL, the path of the
executable is used as a base for the resulting filename.
:p.
For easy code maintenance, specifying :hp2.(PFN)__FUNCTION__:ehp2.
is always valid.

@@WtkLoadModules@PARM@pahmod@out
Address of a buffer receiving the module handles of the loaded
modules.

@@WtkLoadModules@PARM@pulCount@inout
Address of a buffer specifiying the maximum number of loadable modules
and receiving the number of modules loaded.

@@WtkLoadModules@PARM@pszFileMaskPath@in
Address of the ASCIIZ name of a path with file entries.
:p.
Each entry of the path consists of a path relatively to the
path of the executing module, and a filenanme including a %s
as a placeholder for a language identifier,
like e.g. "myapp%s.dll;dll\myapp%s.dll".

@@WtkLoadModules@RETURN
Return Code.
:p.
WtkLoadModules returns one of the following return codes&colon.

:parml compact break=none.
:pt.0
:pd.NO_ERROR
:pt.2
:pd.ERROR_FILE_NOT_FOUND
:pt.87
:pd.ERROR_INVALID_PARAMETER
:pt.111
:pd.ERROR_BUFFER_OVERFLOW
:eparml.
:p.
or return codes of the following functions
:ul compact.
:li. WtkGetNlsPackageFilename
:li. DosLoadModule
:eul.

@@WtkLoadModules@REMARKS
When calling this function, all entries within the file path specified with
parameter :hp1.pszFileMaskPath:ehp1. are
:ol compact.
:li.appended to the path of the executing module
:eol.
:p.
In order to load a resource file being stored
relative to the pathname of a calling executable, call
:link reftype=hd refid=WtkLoadModules.WtkLoadModules:elink.
instead.
:p.
The same module is loaded several times, if it
is available more than once within the specified path.
In order to prevent the caller from using the same module 
twice, it is recommended to implement an API within the
module returning a unique module identifier, allowing
to unload duplicates after the call to WtkModLoadModules.
@@
*/


APIRET APIENTRY WtkLoadModules( PFN pfnMod, PHMODULE pahmod, PULONG pulCount, PSZ pszFileMaskPath)
{
         APIRET         rc = NO_ERROR;
         ULONG          ulMaxCount;
         HDIR           hdir = HDIR_CREATE;

         CHAR           szDllFile[ _MAX_PATH];
         CHAR           szDllMask[ _MAX_PATH];
         CHAR           szError[ 20];

         PSZ            pszFileMaskPathCopy = NULL;
         PSZ            pszEntry;
         PSZ            pszNextEntry;

do
   {
   // check parms
   if ((!pulCount)        ||
       (!pszFileMaskPath) ||
       (!*pszFileMaskPath))
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   // reset count vars
   ulMaxCount = *pulCount;
   *pulCount = 0;

   // reset table
   memset( pahmod, 0, (ulMaxCount * sizeof( HMODULE)));

   // create copy of path
   pszFileMaskPathCopy = strdup( pszFileMaskPath);
   if (!pszFileMaskPathCopy)
      {
      rc = ERROR_NOT_ENOUGH_MEMORY;
      break;
      }

   // outer loop: scan for path entries
   pszEntry = pszFileMaskPathCopy;
   do
      {
      pszNextEntry = strchr( pszEntry, ';');
      if (pszNextEntry)
         *pszNextEntry++ = 0;
      else
         pszNextEntry = pszEntry + strlen( pszEntry);

      do
         {
         // determine filename
         rc = WtkGetPackageFilename( pfnMod, pszEntry, szDllMask, sizeof( szDllMask));
         if (rc != NO_ERROR)
            break;
   
         // inner loop: find all files matching the mask
         hdir = HDIR_CREATE;
         do
            {
            // first/next one
            rc = WtkGetNextFile( szDllMask, &hdir, szDllFile, sizeof( szDllFile));
            if (rc != NO_ERROR)
               break;
      
            // load library
            rc = DosLoadModule( szError, sizeof( szError), szDllFile, pahmod);
            if (rc != NO_ERROR)
               continue;
      
            // increase counter and table ptr
            (*pulCount)++;
            pahmod++;
      
            } while (*pulCount != ulMaxCount);
      
         // ignore error for end of search
         if ((rc == ERROR_NO_MORE_FILES) || (rc == ERROR_PATH_NOT_FOUND))
            rc = NO_ERROR;

         } while (FALSE);

      // next entry
      if (*pszNextEntry)
         pszEntry = pszNextEntry;
      else
         break;

      } while ((*pulCount != ulMaxCount) && (pszEntry) && (*pszEntry));

   // ignore error for end of search of last try
   if ((rc == ERROR_NO_MORE_FILES) || (rc == ERROR_PATH_NOT_FOUND))
      rc = NO_ERROR;

   // if nothing found overall, return error
   if (!*pulCount)
      rc = ERROR_NO_MORE_FILES;

   } while (FALSE);

// cleanup
if (hdir != HDIR_CREATE) DosFindClose( hdir);
if (pszFileMaskPathCopy) free( pszFileMaskPathCopy);
return rc;
}

// -----------------------------------------------------------------------------

/*
@@WtkFreeModules@SYNTAX
This function frees modules stored in a path relative to the pathname of 
a calling executable.

@@WtkFreeModules@PARM@pahmod@in
Address of a buffer holding the module handles of the modules to be freed.

@@WtkFreeModules@PARM@ulCount@in
The number of module handles to free.

@@WtkFreeModules@RETURN
Return Code.
:p.
WtkFreeModules returns one of the following return codes&colon.

:parml compact break=none.
:pt.0
:pd.NO_ERROR
:pt.87
:pd.ERROR_INVALID_PARAMETER
:eparml.

@@WtkFreeModules@REMARKS
-none -
@@
*/

APIRET APIENTRY WtkFreeModules( PHMODULE pahmod, ULONG ulCount)
{

         APIRET         rc = NO_ERROR;
         ULONG          i;

do
   {
   // check parms
   if ((!pahmod) ||
       (!ulCount))
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   // free modules
   for (i = 0; i < ulCount; i++, pahmod++)
      {
      DosFreeModule( *pahmod);
      }
   ulCount = 0;

   } while (FALSE);

return rc;
}

