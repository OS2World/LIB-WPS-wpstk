/****************************** Module Header ******************************\
*
* Module Name: wtktmf.c
*
* Source for text message file functions
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2002
*
* $Id: wtktmf.c,v 1.10 2009-11-17 22:00:01 cla Exp $
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

#define INCL_DOS
#define INCL_ERRORS
#include <os2.h>

#include "wtkeas.h"
#include "wtkufil.h"
#include "wtktmf.h"
#include "wpstk.ih"

#define DPRINTF(p)

#define MAX(a,b)        (a > b ? a : b)
#define MIN(a,b)        (a < b ? a : b)

#define EA_TIMESTAMP "TMF.FILEINFO"
#define EA_MSGTABLE  "TMF.MSGTABLE"

#define MSG_NAME_START   "\r\n<--"
#define MSG_NAME_END     "-->:"

#define MSG_COMMENT_LINE "\r\n;"

static   PSZ            pszNameStart = "\r\n<--";
static   PSZ            pszNameEnd   = "-->:";

// ###########################################################################


static APIRET __getTimeStamp( PFILESTATUS3 pfs3, PSZ pszBuffer, ULONG ulBufferlen)
{
         APIRET         rc = NO_ERROR;
         CHAR           szTimeStamp[ 15];
static   PSZ            pszFormatTimestamp = "%4u%02u%02u%02u%02u%02u%";

do
   {
   // check parms
   if ((!pfs3)||
       (!pszBuffer))
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   // create stamp
   sprintf( szTimeStamp,
            pszFormatTimestamp,
            pfs3->fdateLastWrite.year + 1980,
            pfs3->fdateLastWrite.month,
            pfs3->fdateLastWrite.day,
            pfs3->ftimeLastWrite.hours,
            pfs3->ftimeLastWrite.minutes,
            pfs3->ftimeLastWrite.twosecs * 2);

   // check bufferlen
   if (strlen( szTimeStamp) + 1 > ulBufferlen)
      {
      rc = ERROR_BUFFER_OVERFLOW;
      break;
      }

   // hand over result
   strcpy( pszBuffer, szTimeStamp);

   } while (FALSE);

return rc;
}

// ==============================================================================

static APIRET __compileMsgTable( PSZ pszMessageFile, PBYTE *ppbTableData)
{
         APIRET         rc = NO_ERROR;
         CHAR           szMessageFile[ _MAX_PATH];

         FILESTATUS3    fs3;
         ULONG          ulStampLength;
         PBYTE          pbFileData = NULL;
         ULONG          ulFileDataLength;

         CHAR           szFileStampOld[ 18];     // yyyymmddhhmmssms.
         CHAR           szFileStampCurrent[ 18];

         PBYTE          pbTableData = NULL;
         ULONG          ulTableDataLength;
         ULONG          ulTableDataContentsLength = 0;
         CHAR           szEntry[ _MAX_PATH];

         HFILE          hfileMessageFile = -1;
         ULONG          ulBytesRead;

         COUNTRYCODE    cc = {0,0};

         PSZ            pszCommentLine;
         PSZ            pszCurrentNameStart;
         PSZ            pszCurrentNameEnd;
         PSZ            pszCurrentMessageStart;
         PSZ            pszCurrentMessageEnd;
         ULONG          ulCurrentMessagePos;
         ULONG          ulCurrentMessageLen;
         PSZ            pszNextNameStart;
         PSZ            pszEntry;

do
   {
   // check parms
   if ((!pszMessageFile)  ||
       (!ppbTableData))
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   // get length and timestamp of file
   rc = DosQueryPathInfo( pszMessageFile,
                          FIL_STANDARD,
                          &fs3,
                          sizeof( fs3));
   if (rc != NO_ERROR)
      break;
   ulFileDataLength = fs3.cbFile;

   // determine current timestamp
   __getTimeStamp( &fs3, szFileStampCurrent, sizeof( szFileStampCurrent));

   // determine saved timestamp
   ulStampLength = sizeof( szFileStampOld);
   rc = WtkReadStringEa( pszMessageFile, EA_TIMESTAMP, szFileStampOld, &ulStampLength);

   // compare timestamps
   if ((rc == NO_ERROR)                                     &&
       (ulStampLength == (strlen( szFileStampCurrent) + 1)) &&
       (!strcmp( szFileStampCurrent, szFileStampOld)))
      {

      // read table out of EAs
      do
         {
         // get ea length of table
         ulTableDataLength = 0;
         rc = WtkQueryEaSize( pszMessageFile, EA_MSGTABLE, &ulTableDataLength);
         if (rc != ERROR_BUFFER_OVERFLOW)
            break;

         // get memory
         if ((pbTableData = malloc( ulTableDataLength)) == NULL)
            {
            rc = ERROR_NOT_ENOUGH_MEMORY;
            break;
            }

         // read table
         rc = WtkReadStringEa( pszMessageFile, EA_MSGTABLE, pbTableData, &ulTableDataLength);

         } while (FALSE);

      // if no error occurred, we are finished
      if (rc == NO_ERROR)
         {
         DPRINTF(( "TMF: using precompiled table of %s\n", pszMessageFile));
         break;
         }
      }

   DPRINTF(( "TMF: (re)compile table for %s\n", pszMessageFile));

   // recompilation needed
   // get memory for table data
   ulTableDataLength = ulFileDataLength / 2;
   if ((pbTableData = malloc( ulTableDataLength)) == NULL)
      {
      rc = ERROR_NOT_ENOUGH_MEMORY;
      break;
      }
   memset( pbTableData, 0, ulTableDataLength);

   // read file conmtents
   rc = WtkReadFile( pszMessageFile, &pbFileData, &ulBytesRead);
   if (rc != NO_ERROR)
      break;

   // ------------------------------------------------------------------

   // search first message name
   pszCurrentNameStart = pbFileData;
   if (!strncmp( pszCurrentNameStart, pszNameStart + 2, strlen( pszNameStart) - 2))
      {
      // no CRLF before the very first line !
      pszCurrentNameStart += strlen( pszNameStart) - 2;
      }
   else
      {
      // search first name
      pszCurrentNameStart = strstr( pszCurrentNameStart, pszNameStart);

      if (pszCurrentNameStart)
         pszCurrentNameStart += strlen( pszNameStart);
      else
         {
         rc = ERROR_INVALID_DATA;
         break;
         }
      }

   // is first name complete ?
   pszCurrentNameEnd = strstr( pszCurrentNameStart, pszNameEnd);
   if (!pszCurrentNameEnd)
      {
      rc = ERROR_INVALID_DATA;
      break;
      }

   // scan through all names
   while ((pszCurrentNameStart) && (*pszCurrentNameStart))
      {
      // search end of name, if not exist, skip end of file
      pszCurrentNameEnd = strstr( pszCurrentNameStart, pszNameEnd);
      if (!pszCurrentNameEnd)
         break;

      // search next name, if none, use end of string
      pszCurrentMessageEnd = strstr( pszCurrentNameEnd, pszNameStart);
      if (!pszCurrentMessageEnd)
         {
         pszCurrentMessageEnd = pszCurrentNameStart + strlen( pszCurrentNameStart);
         pszNextNameStart = NULL;

         // cut off last CRLF
         if (*(PUSHORT) (pszCurrentMessageEnd - 2) == 0x0a0d)
            pszCurrentMessageEnd -=2;
         }
      else
         pszNextNameStart = pszCurrentMessageEnd + strlen( pszNameStart);

      // calculate table entry data
      *pszCurrentNameEnd  = 0;
      ulCurrentMessagePos = pszCurrentNameEnd + strlen( pszNameEnd) - pbFileData;
      ulCurrentMessageLen = pszCurrentMessageEnd - pbFileData - ulCurrentMessagePos;

      // determine entry
      sprintf( szEntry, "%s %u %u\n", pszCurrentNameStart, ulCurrentMessagePos, ulCurrentMessageLen);

      // need more space ?
      if ((ulTableDataContentsLength + strlen( szEntry) + 1) > ulTableDataLength)
         {
                   PBYTE          pbTmp;

         ulTableDataLength += ulFileDataLength / 2;
         pbTmp = realloc( pbTableData, ulTableDataLength);
         if (!pbTmp)
            {
            rc = ERROR_NOT_ENOUGH_MEMORY;
            break;
            }
         else
            pbTableData = pbTmp;
         }

      // add entry
      strcat( pbTableData, szEntry);
      ulTableDataContentsLength += strlen( szEntry);

      // adress next entry
      pszCurrentNameStart = pszNextNameStart;

      } // while (pszCurrentNameStart)

   // close file, so that we can use DosSetPathInfo to write Eas -
   // this avoids reset of lastwritestamp when using DosSetFileInfo instead
   DosClose( hfileMessageFile);
   hfileMessageFile = NULLHANDLE;

   // write EAs
   // ### handle 64 kb limit here !!!
   rc = WtkWriteStringEa( pszMessageFile, EA_TIMESTAMP, szFileStampCurrent);
   if (rc != NO_ERROR)
      break;
   rc = WtkWriteStringEa( pszMessageFile, EA_MSGTABLE,  pbTableData);
   if (rc != NO_ERROR)
      break;


   // ------------------------------------------------------------------

   } while (FALSE);


if (rc == NO_ERROR)
   {
   // hand over result
   *ppbTableData = pbTableData;

   // make text uppercase
   rc = DosMapCase( ulTableDataLength, &cc, pbTableData);
   }

// cleanup
if (pbFileData)       free( pbFileData);
DosClose( hfileMessageFile);

if (rc != NO_ERROR)
   if (pbTableData)      free( pbTableData);

return rc;
}

// -----------------------------------------------------------------------------

static PSZ __expandParms( PSZ pszStr, PSZ *apszParms, ULONG ulParmCount)
{
         PSZ      pszResult = NULL;

         PSZ      pszNewValue;

         PSZ      pszStartPos;
         PSZ      pszVarNum;

         ULONG    ulNameLen = 1;
         PSZ      pszVarValue;
         CHAR     szVarName[] = "?";
         ULONG    ulParmIndex;

         PSZ      pszNewResult;
         ULONG    ulNewResultLen;

static   CHAR     chDelimiter = '%';

         ULONG    ulSkipValue = 0;

do
   {
   // check parms
   if (!pszStr)
      break;

   // create a copy
   pszResult = strdup( pszStr);
   if (!pszResult)
      break;

   // if no parms to replace, don't expand
   if (!ulParmCount)
      break;

   // maintain the copy
   pszStartPos = strchr( pszResult + ulSkipValue, chDelimiter);
   while (pszStartPos)
      {
      // find index
      pszVarNum = pszStartPos + 1;

      // check which parm is meant
      szVarName[ 0] = *pszVarNum;
      ulParmIndex = atol( szVarName);
      if ((ulParmIndex) && (ulParmIndex <= ulParmCount))
         {

         // first of all, elimintate the variable
         strcpy( pszStartPos, pszVarNum + 1);

         // get value
         pszVarValue = apszParms[ ulParmIndex - 1];
         if (pszVarValue)
            {
            // embedd new value
            pszNewResult = malloc( strlen( pszResult) + 1 + strlen( pszVarValue));
            if (pszNewResult)
               {
               strcpy( pszNewResult, pszResult);
               strcpy( pszNewResult + (pszStartPos - pszResult), pszVarValue);
               strcat( pszNewResult, pszStartPos);
               free( pszResult);
               pszResult = pszNewResult;
               }
            else
               {
               // kick any result, as we are out of memory
               free( pszResult);
               pszResult = NULL;
               break;
               }
            }
         }
      else
         // skip this percent sign, as it is not replaced
         ulSkipValue = pszStartPos - pszResult + 1;

      // next var please
      pszStartPos = strchr( pszResult + ulSkipValue, chDelimiter);
      }


   } while (FALSE);


// no cleanup - caller must free memory !
return pszResult;
}

// ###########################################################################

/*
@@WtkGetTextMessage@SYNTAX
This function retrieves a message fro a text message file.
It is quite similar to the :hp2.DosGetMessage:ehp2. control program function,
but with :link reftype=hd viewport refid=RM_WtkGetTextMessage.some important differences:elink.

@@WtkGetTextMessage@PARM@pTable@in
Pointer table.
:p.
Each doubleword pointer points to an ASCIIZ string or a double-byte
character-set (DBCS) string ending in nulls. A maximum of nine strings can be
present.

@@WtkGetTextMessage@PARM@cTable@in
Number of variable insertion text strings.
:p.
Possible values range from 0 to 9. If cTable is 0, pTable is ignored

@@WtkGetTextMessage@PARM@pbBuffer@out
The address of the caller's buffer area where the system returns the
requested message.
:p.
If the message is too long to fit in the caller's buffer, then as much of the
message text as possible is returned, with the appropriate error return code.

@@WtkGetTextMessage@PARM@cbBuffer@in
The length, in bytes, of the caller's buffer area.

@@WtkGetTextMessage@PARM@pszMessageName@in
Address of the ASCIIZ name of the message

@@WtkGetTextMessage@PARM@pszFile@in
Address of the ASCIIZ name of the text message file.
:p.
The path name may contain :hp2.?&colon.:ehp2. for the drive in order to
open the text message file on the boot drive.
:p.
The name may not include wildcards.
:p.
It is recommended, but not enforced to use filenames with the extension
:hp2.tmf:ehp2. to indicate that this is a text message file.

@@WtkGetTextMessage@PARM@pcbMsg@out
The address of a variable containing the length, in bytes, of the
returned message.


@@WtkGetTextMessage@RETURN
Return Code.
:p.
WtkGetTextMessage returns one of the following return codes&colon.

:parml compact break=none.
:pt.0
:pd.NO_ERROR
:pt.5
:pd.ERROR_ACCESS_DENIED
:pt.87
:pd.ERROR_INVALID_PARAMETER
:eparml.
:p.
or return codes of the following functions
:ul compact.
:li.DosSearchPath
:li.DosOpen
:li.DosQueryPathInfo
:eul.

@@WtkGetTextMessage@REMARKS
The WtkGetTextMessage is different to :hp2.DosGetMessage:ehp2. in the following
concerns&colon.
:ul.
:li.the message is not taken from a compiled OS/2 message file, but from an
:link reftype=hd viewport refid=G_WTKTMF.editable text file:elink..

:li.a zero byte is appended to the message returned, so that it can be used
right away with standard C/C++ APIs

:li.placeholders from :hp2.%1:ehp2. to :hp2.%9:ehp2. in the message text are replaced if
values for those are handed over to WtkGetTextMessage in the pointer table referred to
by the :hp2.pTable:ehp2. parameter.
.br
In opposite to the :hp2.DosGetMessage:ehp2. call text replacement is performed even
if not enough parameters are supplied - where values are missing, the placeholder
remains unmodified within the message text.

:eul.

@@
*/

APIRET APIENTRY WtkGetTextMessage( PCHAR* pTable, ULONG cTable,
                                   PBYTE pbBuffer, ULONG cbBuffer,
                                   PSZ pszMessageName, PSZ pszFile,
                                   PULONG pcbMsg)
{
         APIRET         rc = NO_ERROR;
         CHAR           szMessageFile[ _MAX_PATH];
         PBYTE          pbTableData = NULL;
         BOOL           fFound = FALSE;
         PSZ            pszEntry;
         PSZ            pszEndOfEntry;
         ULONG          ulMessagePos;
         ULONG          ulMessageLen;

         ULONG          ulBytesToRead;
         ULONG          ulBytesRead;

         PSZ            pszExpanded = NULL;
         PSZ            pszCommentPos;

do
   {

   // check parms
   if ((!pbBuffer)        ||
       (!cbBuffer)        ||
       (!pszMessageName)  ||
       (!*pszMessageName) ||
       (!*pszFile)        ||
       (!pcbMsg))
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   if (cbBuffer < 2)
      {
      rc = ERROR_BUFFER_OVERFLOW;
      break;
      }

   // reset target vars
   *pcbMsg = 0;

   // search file
   if ((strchr( pszFile, ':'))  ||
       (strchr( pszFile, '\\')) ||
       (strchr( pszFile, '/')))
      // drive and/or path given: no search in path
      strcpy( szMessageFile, pszFile);
   else
      {
      // onlfy filename, search in current dir and DPATH
      rc = DosSearchPath( SEARCH_IGNORENETERRS |
                          SEARCH_ENVIRONMENT   |
                          SEARCH_CUR_DIRECTORY,
                          "DPATH",
                          pszFile,
                          szMessageFile,
                          sizeof( szMessageFile));
      if (rc != NO_ERROR)
         break;
      }

   // compile table if neccessary
   rc = __compileMsgTable( szMessageFile, &pbTableData);
   if (rc != NO_ERROR)
      break;

   // search the name
   pszEntry = pbTableData;
   while (!fFound)
      {
      // search string
      pszEntry = strstr( pszEntry, pszMessageName);
      if (!pszEntry)
         {
         rc = ERROR_MR_MID_NOT_FOUND;
         break;
         }

      // check that it really is the name
      if (((pszEntry == pbTableData) ||
           (*(pszEntry - 1) == '\n'))   &&
           (*(pszEntry + strlen( pszMessageName)) == ' '))
         fFound = TRUE;

      // proceed to the entry data
      pszEntry += strlen( pszMessageName) + 1;
      }

   if (rc != NO_ERROR)
      break;

   // isolate entry
   pszEndOfEntry = strchr( pszEntry, '\n');
   if (pszEndOfEntry)
      *pszEndOfEntry = 0;

   // get numbers
   ulMessagePos = atol( pszEntry);
   if (ulMessagePos == 0)
   if (!pszEntry)
      {
      rc = ERROR_MR_INV_MSGF_FORMAT;
      break;
      }

   pszEntry = strchr( pszEntry, ' ');
   if (!pszEntry)
      {
      rc = ERROR_MR_INV_MSGF_FORMAT;
      break;
      }
   ulMessageLen = atol( pszEntry);


   // report "buffer too small" here, if not at least a zero byte can be appended
   if (ulMessageLen >= cbBuffer)
      {
      rc = ERROR_BUFFER_OVERFLOW;
      break;
      }

   // determine maximum read len
   ulBytesToRead = MIN( ulMessageLen, cbBuffer);

   // read part of the file
   rc = WtkReadFilePart( szMessageFile, ulMessagePos, pbBuffer, ulBytesToRead);
   if (rc != NO_ERROR)
      break;

   // make message an ASCCIIZ string and report len without zerobyte
   *pcbMsg = ulBytesToRead;
   *(pbBuffer + ulBytesToRead) = 0;

   // expand parms
   pszExpanded = __expandParms( pbBuffer, pTable, cTable);
   if (pszExpanded)
      {
      if (strlen( pszExpanded) + 1 > cbBuffer)
         {
         rc = ERROR_BUFFER_OVERFLOW;
         break;
         }
      else
         strcpy( pbBuffer, pszExpanded);
      }

   // check if any comment is included
   // if yes, the message ends there
   pszCommentPos = strstr( pbBuffer, MSG_COMMENT_LINE);
   if (pszCommentPos)
      *pszCommentPos = 0;


   } while (FALSE);

// cleanup
if (pszExpanded) free( pszExpanded);
if (pbTableData) free( pbTableData);
return rc;

}

