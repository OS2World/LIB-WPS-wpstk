/****************************** Module Header ******************************\
*
* Module Name: wtkfcfgs.c
*
* Source for access functions for CONFIG.SYS
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2000
*
* $Id: wtkfcfgs.c,v 1.16 2006-08-24 12:48:24 cla Exp $
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

#include "wtkfcfgs.h"
#include "wtkufil.h"
#include "wpstk.ih"

typedef struct _CFGSYSCHANGE
   {
         PSZ            pszCommand;
         PSZ            pszValue;
         PSZ            pszParameter;
         BOOL           fSetCommand;
         BOOL           fValueList;
         ULONG          ulUpdateLine;
   } CFGSYSCHANGE, *PCFGSYSCHANGE;

static   PSZ            pszCrLf = "\r\n";

#define VAR_DELIMITER '%'
#define LIST_DELIMITER ';'

// ---------------------------------------------------------------------

static char * _nextchar( PSZ pszStr, CHAR chChar1, CHAR chChar2)
{

         PSZ            pszResult = pszStr;
         PSZ            pszChar1;
         PSZ            pszChar2;

do
   {
   pszChar1 = strchr( pszStr, chChar1);
   pszChar2 = strchr( pszStr, chChar2);
   if ((pszChar1) && (pszChar2))
      pszResult = MIN( pszChar1, pszChar2);
   else if (pszChar1)
      pszResult = pszChar1;
   else if (pszChar2)
      pszResult = pszChar2;
   else
      pszResult = ENDSTR( pszStr);

   } while (FALSE);

return pszResult;
}

// ---------------------------------------------------------------------

static void _removeEmptyEntries( PSZ pszList)
{
         PSZ            p;

p = pszList;
// remove all empty entries but the first
while (*p)
   {
   if ((*p == LIST_DELIMITER) && (*(p+1) == LIST_DELIMITER))
      {
      strcpy( p, p + 1);
      continue;
      }
   p++;
   }

// remove first empty entry
p = pszList;
if (*p == LIST_DELIMITER)
   strcpy( p, p + 1);
}

// ---------------------------------------------------------------------

static APIRET _newListValue( PSZ *ppszResult, PSZ pszVarName, PSZ pszCommand, PSZ pszOldValue, PSZ pszNewValue, ULONG ulUpdateMode)
{
         APIRET         rc = NO_ERROR;

         PSZ            pszCheckValue = NULL;
         PSZ            pszOldValueCopy = NULL;
         PSZ            pszNewValueCopy = NULL;

         PSZ            p;
         ULONG          ulListValueLen;

         PSZ            pszVarStart = NULL;
         PSZ            pszVarEnd = NULL;
         CHAR           szVarName[ 64];

         PSZ            pszEoe;
         BOOL           fLastEntry = FALSE;

         PSZ            pszEntry;
         PSZ            pszEntryPos;
         PSZ            pszOldEntryPos;
         CHAR           szEntry[ _MAX_PATH];

         CHAR           chNext;

do
   {
   // check parms
   if ((!ppszResult)  ||
       (!pszCommand)  ||
       (!pszOldValue) ||
       (!pszNewValue))
      break;

   // remove update items of old list
   // this will also check, if addition is required anyway
   // pszCheckValue will be a upcase copy of old value for quick strstr sreach
   // pszOldValueCopy will be used to remove entries from the old value
   // pszNewValueCopy will be used to tokenize the items of the new value
   pszCheckValue   = strdup( pszOldValue);
   pszOldValueCopy = strdup( pszOldValue);
   pszNewValueCopy = strdup( pszNewValue);
   if ((!pszCheckValue) || (!pszOldValueCopy))
      {
      rc = ERROR_NOT_ENOUGH_MEMORY;
      break;
      }
   strupr( pszCheckValue);
   strupr( pszNewValueCopy);

   // remove var from copy of new list
   pszVarStart = strchr( pszNewValueCopy, VAR_DELIMITER);
   if (pszVarStart)
      pszVarEnd = strchr( pszVarStart + 1, VAR_DELIMITER);
   if ((pszVarStart) && (pszVarEnd))
      strcpy( pszVarStart, pszVarEnd + 1);

   // now check all new entries
   pszEntry = pszNewValueCopy;
   fLastEntry = FALSE;
   while (*pszEntry)
      {
      // determine end of line
      pszEoe = strchr( pszEntry, LIST_DELIMITER);
      if (pszEoe)
         *pszEoe = 0;
      else
         fLastEntry = TRUE;

      do
         {
         // ignore empty entries
         if (!*pszEntry)
            break;

         // search entry in upcase copy of old value
         pszEntryPos = strstr( pszCheckValue, pszEntry);
         if (!pszEntryPos)
            break;

         // make sure that it is not a part of a similar entry
         chNext = *(pszEntryPos + strlen( pszEntry));
         if ((chNext) && (chNext != LIST_DELIMITER))
            break;

         // remove entry from copy of old value
         // by replacing it with all semicolons (get parsed out below)
         pszOldEntryPos = pszOldValueCopy + (pszEntryPos - pszCheckValue);
         memset( pszOldEntryPos, LIST_DELIMITER, strlen( pszEntry));

         } while (FALSE);

      // proceed with next entry
      if (fLastEntry)
         break;
      pszEntry = NEXTSTR( pszEntry);
      }

   // remove duplicate semicolon
   _removeEmptyEntries( pszOldValueCopy);

   // -----------------------------------------------------------------------

   if (ulUpdateMode == WTK_CFGSYS_UPDATE_DELETE)
      {
      ulListValueLen = strlen( pszCommand) + strlen( pszOldValueCopy) + 1;
      ulListValueLen *= 2;
      *ppszResult = malloc( ulListValueLen);
      if (!*ppszResult)
         {
         rc = ERROR_NOT_ENOUGH_MEMORY;
         break;
         }
      // assemble line
      strcpy( *ppszResult, pszCommand);
      strcat( *ppszResult, pszOldValueCopy);
      }

   // determine new value
   else if (ulUpdateMode == WTK_CFGSYS_UPDATE_ADD)
      {
      ulListValueLen = strlen( pszCommand) + strlen( pszOldValueCopy) + strlen( pszNewValue);
      ulListValueLen *= 2;
      *ppszResult = malloc( ulListValueLen);
      if (!*ppszResult)
         {
         rc = ERROR_NOT_ENOUGH_MEMORY;
         break;
         }
      memset( *ppszResult, 0, ulListValueLen);
      if (*ppszResult)
         {
         // start with command
         strcpy( *ppszResult, pszCommand);

         // check if var is given
         pszVarStart = strchr( pszNewValue, VAR_DELIMITER);
         if (pszVarStart)
            pszVarEnd = strchr( pszVarStart + 1, VAR_DELIMITER);

         if ((pszVarStart) && (pszVarEnd))
            {
            // copy part before var
            strncpy( EOS( *ppszResult), pszNewValue, pszVarStart - pszNewValue);

            // check varname
            memset( szVarName, 0, sizeof( szVarName));
            strncpy( szVarName, pszVarStart + 1, pszVarEnd - pszVarStart - 1);
            if (!stricmp( szVarName, pszVarName))
               {
               // insert old value
               strcat( *ppszResult, pszOldValueCopy);
               }

            // copy part after var
            strcat( *ppszResult, pszVarEnd + 1);

            }
         else
            strcat( *ppszResult, pszNewValue);

         // remove duplicate semicolon
         _removeEmptyEntries( *ppszResult);

         // append semikolon at end, if not there
         p = EOS( *ppszResult);
         if (*(p - 1) != LIST_DELIMITER)
            *p = LIST_DELIMITER;

         }

      } // if (ulUpdateMode == WTK_CFGSYS_UPDATE_ADD)

   // no result ?
   if (!*ppszResult)
      break;

   // is new value identical with old ?
   // don't report a change
   if (!stricmp( *ppszResult, pszOldValue))
      {
      free( *ppszResult);
      *ppszResult = NULL;
      }

   } while (FALSE);

// cleanup
if (pszCheckValue) free( pszCheckValue);
if (pszOldValueCopy) free( pszOldValueCopy);
if (pszNewValueCopy) free( pszNewValueCopy);
return rc;
}

// -----------------------------------------------------------------------------

static APIRET _writeFile( HFILE hfile, PSZ pszString)
{
         APIRET         rc = NO_ERROR;
         ULONG          ulBytesToWrite;
         ULONG          ulBytesWritten;

do
   {
   // check parms
   if (!pszString)
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   if (*pszString)
      {
      // write the string
      ulBytesToWrite = strlen( pszString);
      rc = DosWrite( hfile, pszString, ulBytesToWrite, &ulBytesWritten);
      if ((rc != NO_ERROR) || (ulBytesToWrite != ulBytesWritten))
         break;
      }

   } while (FALSE);

return rc;
}

// -----------------------------------------------------------------------------

static APIRET _writelineFile( HFILE hfile, PSZ pszString)
{
         APIRET         rc = NO_ERROR;
         ULONG          ulBytesToWrite;
         ULONG          ulBytesWritten;

do
   {
   // check parms
   if (!pszString)
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   if (*pszString)
      {
      rc = _writeFile( hfile, pszString);
      if (rc != NO_ERROR)
         break;
      }

   // add crlf
   ulBytesToWrite = strlen( pszCrLf);
   rc = DosWrite( hfile, pszCrLf, ulBytesToWrite, &ulBytesWritten);
   if ((rc != NO_ERROR) || (ulBytesToWrite != ulBytesWritten))
      break;

   } while (FALSE);

return rc;
}

// -----------------------------------------------------------------------------

static BOOL _scanStatement( PSZ pszLine, PCFGSYSCHANGE pcsc)
{
         BOOL           fResult = FALSE;
static   PSZ            pszCmdSet = "SET ";
static   PSZ            pszCmdLibpath = "LIBPATH";

do
   {
   // check parms
   if ((!pszLine)  ||
       (!*pszLine) ||
       (!pcsc))
      break;

   memset( pcsc, 0, sizeof( CFGSYSCHANGE));
   pcsc->pszCommand   = "";
   pcsc->pszValue     = "";
   pcsc->pszParameter = "";

   pcsc->pszCommand = pszLine;
   if (!strnicmp( pszCmdSet, pcsc->pszCommand, strlen( pszCmdSet)))
      {
      pcsc->fSetCommand = TRUE;
      // process SET command
      pcsc->pszValue = _nextchar( pszLine, 32, 9);
      if (!*pcsc->pszValue)
         break;
      // from here success !
      fResult = TRUE;
      *(pcsc->pszValue) = 0;
      pcsc->pszValue++;
      pcsc->pszValue = __skipblanks( pcsc->pszValue);
      pcsc->pszParameter = strchr( pcsc->pszValue, '=');
      if (!pcsc->pszParameter)
         break;
      *(pcsc->pszParameter) = 0;
      pcsc->pszParameter++;
      }
   else
      {
      pcsc->pszValue = strchr( pcsc->pszCommand, '=');
      if (!pcsc->pszValue)
         break;
      // from here success !
      fResult = TRUE;
      *(pcsc->pszValue) = 0;
      pcsc->pszValue++;
      pcsc->pszValue = __skipblanks( pcsc->pszValue);
      pcsc->pszParameter =  _nextchar( pcsc->pszValue, 32, 9);
      if (!*pcsc->pszParameter)
         break;
      *(pcsc->pszParameter) = 0;
      pcsc->pszParameter++;
      pcsc->pszParameter = __skipblanks( pcsc->pszParameter);
      }



   } while (FALSE);

if ((pcsc) && (pcsc->pszCommand) && (pcsc->pszParameter))
   pcsc->fValueList = (((pcsc->fSetCommand) && (strchr( pcsc->pszParameter, ';'))) ||
                       (!strcmp( pcsc->pszCommand, pszCmdLibpath)));

return fResult;
}

// ---------------------------------------------------------------------------

static VOID _patchBootDrive( PSZ pszString)
{
         PSZ            p;
         ULONG          ulBootDrive;
         CHAR           chBootDrive;

static   PSZ            pszPlaceHolder = "?:";

do
   {
   // check parm
   if ((!pszString) ||
      (!*pszString))
      break;

   // is placeholder included ?
   p = strstr( pszString, pszPlaceHolder);
   if (!p)
      break;

   // determine boot drive
   DosQuerySysInfo( QSV_BOOT_DRIVE, QSV_BOOT_DRIVE, &ulBootDrive, sizeof(ULONG));
   chBootDrive = (CHAR) ulBootDrive + 'A' - 1;

   // copy name and replace ?: with bootdrive
   while (p)
      {
      // patch in drive
      *p = chBootDrive;

      // search next
      p = strstr( pszString, pszPlaceHolder);
      }

   } while (FALSE);

return;
}

// -----------------------------------------------------------------------------

/*
@@WtkUpdateConfigsys@SYNTAX
This function updates the CONFIG.SYS file of the currently booted operating system.
When an update is not required, because all update data is already included,
ERROR_ALREADY_ASSIGNED is returned.

@@WtkUpdateConfigsys@PARM@pszUpdate@in
Address of the update data.
:p.
The update data contains one or multiple CONFIG.SYS statements, each in a line,
where several lines are delimited by CRLF. The CONFIG.SYS gets updated according to
:link reftype=hd refid=RM_WtkUpdateConfigsys.certain update rules:elink..
:p.
The update data may contain :hp2.?&colon.:ehp2. for any drive specification
in order to refer to the boot drive.

@@WtkUpdateConfigsys@PARM@ulUpdateMode@in
The type of operation requested.

:parml.
:pt.WTK_CFGSYS_UPDATE_ADD
:pd.adds and/or replaces the update information to CONFIG.SYS
:pt.WTK_CFGSYS_UPDATE_DELETE
:pd.removes the update information from CONFIG.SYS
:eparml.

@@WtkUpdateConfigsys@PARM@pszBackupExtension@in
Address of the filename extension to be used for backup, excluding the dot.
:p.
It is recommended to use a backup extension with not more than three letters
in order to stay FAT filesystem compliant. An existing CONFIG.xxx file with
the specified backup extension will be overwritten, despite of any file attribute
set. If the file cannot be overwritten due to a sharing violation, WtkUpdateConfigsys
will fail.
:p.
This parameter can be NULL, in which case the backup extension is determined
automatically by counting up a three digit number, starting with 000. This
avoids overwriting old backups.

@@WtkUpdateConfigsys@RETURN
Return Code.
:p.
WtkUpdateConfigsys returns one of the following return codes,
where ERROR_ALREADY_ASSIGNED does NOT indicate an error, but rather 
that no update was necessary&colon.
:parml compact break=none.
:pt.0
:pd.NO_ERROR
:pt.8
:pd.ERROR_NOT_ENOUGH_MEMORY
:pt.13
:pd.ERROR_INVALID_DATA
:pt.85
:pd.ERROR_ALREADY_ASSIGNED
:pt.87
:pd.ERROR_INVALID_PARAMETER
:eparml.
:p.
or return codes of the following functions
:ul compact.
:li.DosOpen
:li.WtkReadFile
:li.WtkQueryFullname
:li.WtkCreateTmpFile
:li.WtkDeleteFile
:li.WtkMoveFile
:eul.

@@WtkUpdateConfigsys@REMARKS
The update data has to be provided as CONFIG.SYS statements,
where lines are delimited by CRLF. Here is an example:
:xmp.
LIBPATH=?&colon.\MYAPP;%LIBPATH%
SET PATH=%PATH%;?&colon.\MYAPP;
SET MOZILLA_HOME=O&colon.\TEST\HOME
SET BOOKSHELF=?&colon.\MYAPP;%BOOKSHELF%
SET HOSTNAME=WPSTK
DEVICE=?&colon.\MYAPP\MYDRIVER.SYS
:exmp.
:p.
This function supports boot drive recognition.
Specify :hp2.?&colon.:ehp2. for any drive specification within the update
data to refer to the boot drive.
:p.
The following rules apply to the update operation (WTK_CFGSYS_UPDATE_ADD):
:parml.
:pt.list value SET commands (like PATH, HELP etc) and LIBPATH statement
:pd.the statement gets modified in place, adding the update
elements to the list.
:pt.single value SET commands and all other statements
:pd.the old statement gets removed from within the CONFIG.SYS data
and then appended with the new value at the end of the file.
:eparml.
:p.
The following rules apply to the remove operation (WTK_CFGSYS_UPDATE_DELETE):
:parml.
:pt.list value SET commands (like PATH, HELP etc) and LIBPATH statement
:pd.the statement gets modified in place, removing the update
elements from the list.
:pt.single value SET commands
:pd.SET statements are not removed from CONFIF.SYS for security reasons
(they may be important for keeping the system running)
:pt.all other statements
:pd.the statement gets removed from CONFIG.SYS
:eparml.

@@
*/

APIRET APIENTRY WtkUpdateConfigsys( PSZ pszUpdate, ULONG ulUpdateMode, PSZ pszBackupExtension)
{
         APIRET         rc = NO_ERROR;
         ULONG          i;

         BOOL           fModified = FALSE;
static   PSZ            pszConfigMask = "?:\\CONFIG.%s";
static   PSZ            pszCmdRem = "REM";
static   PSZ            pszCmdSet = "SET ";
         CHAR           szBackupExtension[ 10];

         BOOL           fLastLine = FALSE;
         PSZ            pszUpdatePatched = NULL;
         PSZ            pszUpdateCopy = NULL;
         ULONG          ulUpdateCount = 0;
         ULONG          ulUpdateMax = 0;

         PSZ           *apszUpdateFinal = NULL;
         PSZ           *ppszUpdateLine;
         ULONG          ulUpdateLine = 0;
         ULONG          ulUpdateLineCount = 0;

         PCFGSYSCHANGE  pcscList = NULL;
         PCFGSYSCHANGE  pcsc;
         ULONG          ulListSize;

         CHAR           szConfigSys[ _MAX_PATH];
         CHAR           szConfigTmp[ _MAX_PATH];
         CHAR           szConfigBak[ _MAX_PATH];

         ULONG          ulConfigSysSize;
         PSZ            pszConfigSysData = NULL;
         PSZ            pszLine;
         PSZ            pszLineCopy;
         PSZ            pszNextLine;
         BOOL           fSkipLine;
         BOOL           fReplaceLine;

         PSZ            pszVarName;
         PSZ            pszOldListValue;
         PSZ            pszNewListValue;
         CHAR           szCommand[ 64];
         PSZ            pszReplaceLine;

         PSZ            pszCheckLine;
         PSZ            pszCheckLineCopy;
         PSZ            pszEol;

         CFGSYSCHANGE   csc;
         HFILE          hfileTarget = -1;
         ULONG          ulAction;
do
   {
   if ((!pszUpdate)  ||
       (!*pszUpdate))
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   // ----------------------------------------------------------------

   // create a copy of the update data, so that we can modify it
   pszUpdatePatched = strdup( pszUpdate);
   pszUpdateCopy    = strdup( pszUpdate);
   if ((!pszUpdatePatched) || (!pszUpdateCopy))
      {
      rc = ERROR_NOT_ENOUGH_MEMORY;
      break;
      }
   _patchBootDrive( pszUpdatePatched);
   strcpy( pszUpdateCopy, pszUpdatePatched);

   // determine line count
   pszLine = pszUpdateCopy;
   fLastLine = FALSE;
   while ((pszLine) && (*pszLine) && (!fLastLine))
      {
      // cut off current line
      pszEol =  _nextchar( pszLine, 13, 10);
      if (pszEol)
         *pszEol = 0;
      else
         fLastLine = TRUE;
      ulUpdateLineCount++;

      // skip whitespace when checking line
      pszCheckLine = pszLine;
      while ((*pszCheckLine) && (*pszCheckLine <= 32))
         {
         pszCheckLine++;
         }

      // skip empty lines
      if (!*pszCheckLine)
         {
         strcpy( pszLine, pszCheckLine + 1);
         while ((*pszLine == '\r') ||
                (*pszLine == '\n'))
            {
            strcpy( pszLine, pszLine + 1);
            }
         continue;
         }

      // count this line
      ulUpdateCount++;

      // proceed with next line,skip LF
      if (fLastLine)
         break;
      pszLine = NEXTSTR( pszLine);
      if (*pszLine == 10)
         pszLine++;
      }
   strcpy( pszUpdateCopy, pszUpdatePatched);

   // no changes ? then quit
   if (!ulUpdateCount)
      {
      rc = ERROR_INVALID_DATA;
      break;
      }

   // get memory for changes list
   ulListSize = ulUpdateCount * sizeof( CFGSYSCHANGE);
   pcscList = malloc( ulListSize);
   if (!pcscList)
      {
      rc = ERROR_NOT_ENOUGH_MEMORY;
      break;
      }
   memset( pcscList, 0, ulListSize);

   // get memory for update list
   ulListSize = ulUpdateLineCount * sizeof( PSZ);
   apszUpdateFinal = malloc( ulListSize);
   if (!apszUpdateFinal)
      {
      rc = ERROR_NOT_ENOUGH_MEMORY;
      break;
      }
   memset( apszUpdateFinal, 0, ulListSize);

   // fill structures
   strcpy( pszUpdateCopy, pszUpdatePatched);
   pszLine = pszUpdateCopy;
   fLastLine = FALSE;
   pcsc = pcscList;
   ulUpdateMax = ulUpdateCount;
   ulUpdateCount = 0;
   ulUpdateLine = 0;
   while ((pszLine) && (*pszLine) && (!fLastLine))
      {
      // cut off current line
      pszEol =  _nextchar( pszLine, 13, 10);
      if (pszEol)
         {
         *pszEol = 0;
         pszNextLine =  NEXTSTR( pszLine);
         }
      else
         fLastLine = TRUE;

      pszLineCopy = strdup( pszLine);

      // skip whitespace when checking line
      pszCheckLine = pszLine;
      while ((*pszCheckLine) && (*pszCheckLine <= 32))
         {
         pszCheckLine++;
         }

      // skip empty lines
      if (!*pszCheckLine)
         {
         strcpy( pszLine, pszCheckLine + 1);
         while ((*pszLine == '\r') ||
                (*pszLine == '\n'))
            {
            strcpy( pszLine, pszLine + 1);
            }
         continue;
         }

      // store values of this line
      if (_scanStatement( pszCheckLine, pcsc))
         {
         if (ulUpdateCount == ulUpdateMax)
            {
            rc = 99;
            break;
            }
         if (!pcsc->fValueList)
            {
            // save line for later addition
            *(apszUpdateFinal + ulUpdateLine) = strdup( pszLineCopy);
            pcsc->ulUpdateLine = ulUpdateLine;
            ulUpdateLine++;
            }
         ulUpdateCount++;
         pcsc++;
         }
      if (pszLineCopy) free( pszLineCopy);

      // proceed with next line,skip LF
      if (fLastLine)
         break;
      pszLine = pszNextLine;
      if ((*pszLine == '\r') ||
          (*pszLine == '\n'))
         pszLine++;
      }

   // ----------------------------------------------------------------

   // determine CONFIG.SYS and backup filename
   sprintf( szConfigSys, pszConfigMask, "SYS");

   if (!pszBackupExtension)
   for (i = 0; i < 999; i++)
      {
      sprintf( szBackupExtension, "%03u", i);
      sprintf( szConfigBak, pszConfigMask, szBackupExtension);
      if (!WtkFileExists(  szConfigBak))
         break;
      }
   else
      {
      sprintf( szConfigBak, pszConfigMask, pszBackupExtension);

      // if file exists, deletete it
      rc = WtkDeleteFile( szConfigBak);
      if (rc != NO_ERROR)
         break;
      }

   // create temporary file
   rc = WtkCreateTmpFile( "????????", szConfigTmp, sizeof( szConfigTmp));
   if (rc != NO_ERROR)
      break;

   // read CONFIG.SYS
   rc = WtkReadFile( szConfigSys, &pszConfigSysData, &ulConfigSysSize);
   if (rc != NO_ERROR)
      break;

   // open the target file
   rc = DosOpen( szConfigTmp,
                 &hfileTarget,
                 &ulAction,
                 0,
                 FILE_NORMAL,
                 OPEN_ACTION_CREATE_IF_NEW | OPEN_ACTION_REPLACE_IF_EXISTS,
                 OPEN_ACCESS_WRITEONLY | OPEN_SHARE_DENYWRITE,
                 (PEAOP2)NULL);
   if (rc != NO_ERROR)
      break;

   // go through the file
   pszLine = pszConfigSysData;
   fLastLine = FALSE;
   pszCheckLineCopy = NULL;
   while ((pszLine) && (*pszLine) && (!fLastLine))
      {
      // cut off current line
      pszEol =  _nextchar( pszLine, 13, 10);
      if (pszEol)
         *pszEol = 0;

      fSkipLine = FALSE;
      fReplaceLine = FALSE;
      do
         {
         // skip whitespace when checking line
         pszCheckLine = pszLine ;
         while ((*pszCheckLine) && (*pszCheckLine <= 32))
            {
            pszCheckLine++;
            }

         // don't touch empty lines
         if (!*pszCheckLine)
            break;

         // don't touch REM lines
         if (!strnicmp( pszCmdRem, pszCheckLine, strlen( pszCmdRem)))
            break;

         // parse line for statement value and parameters
         if (pszCheckLineCopy)
            free( pszCheckLineCopy);
         pszCheckLineCopy = strdup( pszCheckLine);

         if (_scanStatement( pszCheckLineCopy, &csc))
            {
            // decide here to skip the line, effectively deleting it by that
            for (i = 0, pcsc = pcscList; i < ulUpdateCount; i++, pcsc++)
               {
               if (csc.fSetCommand)
                  {
                  // process SET command
                  fSkipLine = (!stricmp( csc.pszValue, pcsc->pszValue));

                  // don't touch line if value is equal
                  if (fSkipLine)
                     {
                     if (!stricmp( csc.pszParameter, pcsc->pszParameter))
                        {
                        // remove line from update data
                        ppszUpdateLine = apszUpdateFinal + pcsc->ulUpdateLine;
                        if (*ppszUpdateLine)
                           {
                           free( *ppszUpdateLine);
                           *ppszUpdateLine = NULL;
                           }

                        fSkipLine = FALSE;
                        }
                     }
                  }
               else
                  {
                  // process all other commands
                  if (pcsc->fValueList)
                     fSkipLine = (!stricmp( csc.pszCommand, pcsc->pszCommand));
                  else
                     fSkipLine = ((!stricmp( csc.pszCommand, pcsc->pszCommand)) &&
                                  (!stricmp( csc.pszValue, pcsc->pszValue)));
                  }

               if (fSkipLine)
                  {
                  // don't replace for value lists
                  if (pcsc->fValueList)
                     {
                     // determine values accoding to the type of command
                     if (csc.fSetCommand)
                        {
                        sprintf( szCommand, "%s %s=", csc.pszCommand, csc.pszValue);
                        pszVarName = csc.pszValue;
                        pszNewListValue = pcsc->pszParameter;
                        pszOldListValue = csc.pszParameter;
                        }
                     else
                        {
                        sprintf( szCommand, "%s=", csc.pszCommand);
                        pszVarName = csc.pszCommand;
                        pszNewListValue = pcsc->pszValue;
                        pszOldListValue = csc.pszValue;
                        }

                     // get memory for new value
                     rc = _newListValue( &pszReplaceLine, pszVarName, szCommand, pszOldListValue, pszNewListValue, ulUpdateMode);
                     if (rc != NO_ERROR)
                        break;
                     if ((pszReplaceLine) && (*pszReplaceLine))
                        {
                        fReplaceLine = TRUE;
                        fSkipLine = FALSE;
                        break;
                        }
                     }
                   else
                      {
                      // don't skip set commands when deleting information
                      if ((ulUpdateMode == WTK_CFGSYS_UPDATE_DELETE) && (csc.fSetCommand))
                          fSkipLine = FALSE;
                      }

                   break;
                   }

               } // for (i = 0, pcsc = pcscList; i < ulUpdateCount; i++, pcsc++)

            } // if (_scanStatement( pszCheckLineCopy, &csc))

         if (rc != NO_ERROR)
            break;

         } while (FALSE);

      if (rc != NO_ERROR)
         break;

      // write line to target file
      if (!fSkipLine)
         rc = _writelineFile( hfileTarget, (fReplaceLine) ? pszReplaceLine : pszLine);
      else
         fModified = TRUE;

      if ((fReplaceLine) && (pszReplaceLine))
         free( pszReplaceLine);

      // proceed with next line,skip LF
      if (fLastLine)
         break;
      pszLine = NEXTSTR( pszLine);
      if (*pszLine == 10)
         pszLine++;

      } // while ((pszLine) && (*pszLine))

   if (rc != NO_ERROR)
      break;

   // now add the contents
   if (ulUpdateMode == WTK_CFGSYS_UPDATE_ADD)
      {
      for (i = 0, ppszUpdateLine = apszUpdateFinal; i < ulUpdateLine; i++, ppszUpdateLine++)
         {
         if (*ppszUpdateLine)
            {
            rc = _writelineFile( hfileTarget, *ppszUpdateLine);
            fModified = TRUE;
            }
         }
      }

   // finally swap files, if modified
   DosClose( hfileTarget);  hfileTarget = -1;

   if (fModified)
      {
      rc = WtkMoveFile( szConfigSys, szConfigBak);
      if (rc != NO_ERROR)
         break;
      rc = WtkMoveFile( szConfigTmp, szConfigSys);
      if (rc != NO_ERROR)
         {
         // swap back old CONFIG.SYS, if copy fails
         WtkMoveFile( szConfigBak, szConfigSys);
         break;
         }
      }
   else
      {
      DosDelete( szConfigTmp);
      rc = ERROR_ALREADY_ASSIGNED;
      }

   } while (FALSE);

// ceanup
if (pszUpdatePatched) free( pszUpdatePatched);
if (pszUpdateCopy) free( pszUpdateCopy);
if (apszUpdateFinal) free( apszUpdateFinal);
if (pcscList) free( pcscList);
if (pszConfigSysData) free( pszConfigSysData);
if (hfileTarget != -1)
   {
   DosClose( hfileTarget);
   DosDelete( szConfigTmp);
   }
return rc;
}

