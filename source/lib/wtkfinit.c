/****************************** Module Header ******************************\
*
* Module Name: wtkfinit.c
*
* Source for access functions for text initialization files
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2000
*
* $Id: wtkfinit.c,v 1.4 2003-04-24 15:41:42 cla Exp $
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

#define INCL_DOS
#define INCL_ERRORS
#include <os2.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wtkfinit.h"
#include "wpstk.ih"

// ---------------------------------------------------------------------------

// define max line size (+ LF + ZEROBYTE)
#define WTK_INIT_MAX_LINESIZE         (512 + 2)
#define WTK_INIT_CHARS_COMMENT        ";"
#define WTK_INIT_CHARS_DELIMITER      "="

#define WTK_INIT_STR_COMMENT          "//"
#define WTK_INIT_CHAR_SECTION_START   '['
#define WTK_INIT_CHAR_SECTION_END     ']'

// ---------------------------------------------------------------------------

#pragma pack(1)

// ---- init file data entities
//      all PSZs point to malloced memory (strdup)
//      same applies to "first" and "next" fields

typedef struct _KEY
   {
         PSZ            pszKeyName;
         PSZ            pszKeyValue;
         PSZ            pszComment;
         PSZ            pszValueComment;
         PSZ            pszTailComment;
         CHAR           chDelimiter;
         ULONG          ulKeyIndent;
         ULONG          ulKeyNameLen;
         ULONG          ulValueIndent;
         ULONG          ulValueCommentIndent;
  struct _KEY          *pkeyNext;
   } KEY, *PKEY;

typedef struct _SECTION
   {
         PSZ            pszSectionName;
         PSZ            pszComment;
         PSZ            pszTailComment;
         PKEY           pkeyFirst;
  struct _SECTION      *psectionNext;
   } SECTION, *PSECTION;

// ---- global init file description structure
//      pointer mapped to HINIT

typedef struct _INIT
   {
         FILE           *pfile;
         CHAR           szFilename[ _MAX_PATH];
         ULONG          ulOpenMode;
         ULONG          ulUpdateMode;
         BOOL           fModified;
         CHAR           chDelimiter;
         CHAR           chComment;
         PSECTION       psectionFirst;
         KEY            keyLast;
   } INIT, *PINIT;

#pragma pack()

// ---------------------------------------------------------------------------

static VOID _freeKEY( PKEY pkey)
{
if (pkey)
   {
   if (pkey->pszKeyName)      free( pkey->pszKeyName);
   if (pkey->pszKeyValue)     free( pkey->pszKeyValue);
   if (pkey->pszComment)      free( pkey->pszComment);
   if (pkey->pszValueComment) free( pkey->pszValueComment);
   if (pkey->pszTailComment)  free( pkey->pszTailComment);

   _freeKEY( pkey->pkeyNext);
   memset( pkey, 0, sizeof( KEY));
   free( pkey);
   }
}

static VOID _freeSECTION( PSECTION psec)
{
if (psec)
   {
   if (psec->pszSectionName)  free( psec->pszSectionName);
   if (psec->pszComment)      free( psec->pszComment);
   if (psec->pszTailComment)  free( psec->pszTailComment);

   _freeKEY( psec->pkeyFirst);
   _freeSECTION( psec->psectionNext);
   memset( psec, 0, sizeof( SECTION));
   free( psec);
   }
}

static VOID _freeINIT( PINIT pinit)
{
if (pinit)
   {
   if (pinit->pfile)          fclose( pinit->pfile);

   _freeSECTION( pinit->psectionFirst);
   memset( pinit, 0, sizeof( INIT));
   free( pinit);
   }
}

// ----------------------------------------------------------------------

static APIRET _writeKEY( FILE *pfile, PKEY pkey, ULONG ulUpdateMode)
{
         APIRET         rc = NO_ERROR;
         ULONG          i;

if (pkey)
   {
   if (!(ulUpdateMode & WTK_INIT_UPDATE_DISCARDCOMMENTS))
      fprintf( pfile, "%s", pkey->pszComment);

   for (i = 0; i < pkey->ulKeyIndent; i++)
      {
      fprintf( pfile, " ");
      }

   fprintf( pfile, "%-*s%c",
            pkey->ulKeyNameLen,
            pkey->pszKeyName,
            pkey->chDelimiter);

   for (i = 0; i < pkey->ulValueIndent; i++)
      {
      fprintf( pfile, " ");
      }
   fprintf( pfile, "%s", pkey->pszKeyValue);

   if (pkey->pszValueComment)
      {
      for (i = 0; i < pkey->ulValueCommentIndent; i++)
         {
         fprintf( pfile, " ");
         }
      fprintf( pfile, "%s", pkey->pszValueComment);
      }

   fprintf( pfile, "\n");

   if (pkey->pszTailComment)
      fprintf( pfile, "%s", pkey->pszTailComment);

   _writeKEY( pfile, pkey->pkeyNext, ulUpdateMode);
   }

return rc;
}


static APIRET _writeSECTION( FILE *pfile, PSECTION psec, ULONG ulUpdateMode)
{
         APIRET         rc = NO_ERROR;

if (psec)
   {
   if (!(ulUpdateMode & WTK_INIT_UPDATE_DISCARDCOMMENTS))
      fprintf( pfile, "%s", psec->pszComment);
   fprintf( pfile, "[%s]\n", psec->pszSectionName);

   if (psec->pszTailComment)
      fprintf( pfile, "%s", psec->pszTailComment);

   _writeKEY( pfile,      psec->pkeyFirst,    ulUpdateMode);
   _writeSECTION( pfile,  psec->psectionNext, ulUpdateMode);
   }

return rc;
}

// ----------------------------------------------------------------------

static PKEY _findKEY( PSECTION psec, PSZ pszKeyName)
{
         PKEY           pkey = NULL;

if ((psec) && (pszKeyName))
   {
   pkey = psec->pkeyFirst;
   while (pkey)
      {
      if (!stricmp( pkey->pszKeyName, pszKeyName))
         break;
      pkey = pkey->pkeyNext;
      }
   }
return pkey;
}

static PSECTION _findSECTION( PINIT pinit, PSZ pszSectionName)
{
         PSECTION       psec = NULL;

if ((pinit) && (pszSectionName))
   {
   psec = pinit->psectionFirst;
   while (psec)
      {
      if (!stricmp( psec->pszSectionName, pszSectionName))
         break;
      psec = psec->psectionNext;
      }
   }
return psec;
}

// ----------------------------------------------------------------------

static APIRET _collectKEY( PSECTION psec, PSZ pszBuffer, ULONG ulBuflen, PULONG pulProfileSize)
{
         APIRET         rc = NO_ERROR;
         PKEY           pkey;

         PSZ            pszThisValue     = pszBuffer;
         ULONG          ulValueLen;
         ULONG          ulRemainingSpace = ulBuflen;

if (psec)
   {
   pkey = psec->pkeyFirst;
   *pulProfileSize = 0;
   while (pkey)
      {
      ulValueLen = strlen( pkey->pszKeyName) + 1;
      *pulProfileSize += ulValueLen;

      if (pszBuffer)
         {
         if (ulRemainingSpace < ulValueLen + 1)
            {
            rc = ERROR_BUFFER_OVERFLOW;
            break;
            }

         // store value
         strcpy( pszThisValue, pkey->pszKeyName);
         ulRemainingSpace -= ulValueLen;
         pszThisValue += ulValueLen;
         }

      // next section
      pkey = pkey->pkeyNext;

      }

   // do NOT count double zero byte - like PrfQueryProfileString
   // (*pulProfileSize)++;

   }

return rc;
}

static APIRET _collectSECTION( PINIT pinit, PSZ pszBuffer, ULONG ulBuflen, PULONG pulProfileSize)
{
         APIRET         rc = NO_ERROR;
         PSECTION       psec;

         PSZ            pszThisValue     = pszBuffer;
         ULONG          ulValueLen;
         ULONG          ulRemainingSpace = ulBuflen;

if (pinit)
   {
   psec = pinit->psectionFirst;
   *pulProfileSize = 0;
   while (psec)
      {
      ulValueLen = strlen( psec->pszSectionName) + 1;
      *pulProfileSize += ulValueLen;

      if (pszBuffer)
         {
         if (ulRemainingSpace < ulValueLen + 1)
            {
            rc = ERROR_BUFFER_OVERFLOW;
            break;
            }

         // store value
         strcpy( pszThisValue, psec->pszSectionName);
         ulRemainingSpace -= ulValueLen;
         pszThisValue += ulValueLen;
         }


      // next section
      psec = psec->psectionNext;
      }

   // do NOT count double zero byte - like PrfQueryProfileString
   // (*pulProfileSize)++;

   }

return rc;
}

// ----------------------------------------------------------------------

static PKEY _createKEY( PINIT pinit, PSECTION psec, PSZ pszKeyName, PSZ pszNewValue)
{
         PKEY           pkey = NULL;
         PKEY           pkeyLast;
         PKEY          *pkeyParent;

if (psec)
   {
   pkeyLast   = psec->pkeyFirst;
   pkeyParent = &psec->pkeyFirst;
   while (*pkeyParent)
      {
      if (pkeyLast->pkeyNext)
         pkeyLast   = pkeyLast->pkeyNext;
      pkeyParent = &((*pkeyParent)->pkeyNext);
      }

   // create new key
   pkey = malloc( sizeof( KEY));
   if (pkey)
      {
      *pkeyParent = pkey;
      memset( pkey, 0, sizeof( KEY));
      pkey->chDelimiter   = pinit->chDelimiter;
      pkey->pszComment    = strdup( "");
      pkey->pszKeyName    = strdup( pszKeyName);
      pkey->pszKeyValue = strdup( pszNewValue);
      if ((!pkey->pszKeyName) || (!pkey->pszKeyValue))
         {
         free( pkey);
         *pkeyParent = NULL;
         }

      // use ident vars either from last key of
      // this section or from last saved key of this file
      if (!pkeyLast)
         pkeyLast = &pinit->keyLast;
      pkey->ulKeyIndent   = pkeyLast->ulKeyIndent;
      pkey->ulKeyNameLen  = pkeyLast->ulKeyNameLen;
      pkey->ulValueIndent = pkeyLast->ulValueIndent;
      }
   }

return pkey;
}

// ----------------------------------------------------------------------

static PSECTION _createSECTION( PINIT pinit, PSZ pszSectionName)
{
         PSECTION       psec = NULL;
         PSECTION      *psecParent;

if (pinit)
   {
   psecParent = &pinit->psectionFirst;
   while (*psecParent)
      {
      psecParent = &((*psecParent)->psectionNext);
      }

   // create new section
   psec = malloc( sizeof( SECTION));
   if (psec)
      {
      *psecParent = psec;
      memset( psec, 0, sizeof( SECTION));
      psec->pszComment     = strdup( "\n");
      psec->pszSectionName = strdup( pszSectionName);
      if (!psec->pszSectionName)
         {
         free( psec);
         *psecParent = NULL;
         }
      }

   }
return psec;
}

// ----------------------------------------------------------------------

static BOOL _removeKEY( PINIT pinit, PSECTION psec, PSZ pszKeyName)
{
         BOOL           fRemoved = FALSE;
         PKEY           pkey;
         PKEY          *pkeyParent;
         PSZ           *ppszLastTailComment;

         ULONG          i;
         ULONG          ulDeleteCommentLen = 0;
         ULONG          ulCommentCharsLen;
         PSZ            pszNewTailComment;

if (psec)
   {
   pkeyParent          = &psec->pkeyFirst;
   ppszLastTailComment = &psec->pszTailComment;
   pkey                = psec->pkeyFirst;
   while (pkey)
      {
      if (!stricmp( pkey->pszKeyName, pszKeyName))
         break;
      pkeyParent          = &pkey->pkeyNext;
      ppszLastTailComment = &pkey->pszTailComment;
      pkey                = pkey->pkeyNext;
      }

   if (pkey)
      {
      if (pinit->ulUpdateMode & WTK_INIT_UPDATE_SOFTDELETEKEYS)
         {
         // softdelete: add line to tail comment of key before
         // - determine len and get memory for new tail comment
         if (pkey->pszComment)
            ulDeleteCommentLen += strlen( pkey->pszComment)    + 1;

         ulDeleteCommentLen += pkey->ulKeyIndent               +
                               pkey->ulKeyNameLen              +
                               strlen(  pkey->pszKeyName)      + 1 +
                                                                 1 +  // delimter char
                               pkey->ulValueIndent             +
                               strlen(  pkey->pszKeyValue)     + 1;

         if (pkey->pszValueComment)
            ulDeleteCommentLen += strlen( pkey->pszValueComment) + 1;
         if (*ppszLastTailComment)
            ulDeleteCommentLen += strlen( *ppszLastTailComment) + 1;

         pszNewTailComment = malloc( ulDeleteCommentLen);
         if (!pszNewTailComment)
            return FALSE;

         // assemble new comment
         if (*ppszLastTailComment)
            strcpy( pszNewTailComment, *ppszLastTailComment);
         else
            *pszNewTailComment = 0;

         if (pkey->pszComment)
            strcat( pszNewTailComment, pkey->pszComment);

         if (pinit->chComment == '/')
            {
            strcat( pszNewTailComment, "//");
            ulCommentCharsLen = 2;
            }
         else
            {
            sprintf( pszNewTailComment + strlen( pszNewTailComment), "%c", pinit->chComment);
            ulCommentCharsLen = 1;
            }

         if (pkey->ulKeyIndent > ulCommentCharsLen)
            {
            for (i = 0; i < pkey->ulKeyIndent - ulCommentCharsLen; i++)
               {
               strcat( pszNewTailComment, " ");
               }
            }

         sprintf( pszNewTailComment + strlen( pszNewTailComment),
                  "%-*s%c",
                  pkey->ulKeyNameLen,
                  pkey->pszKeyName,
                  pkey->chDelimiter);

         for (i = 0; i < pkey->ulValueIndent; i++)
            {
            strcat( pszNewTailComment, " ");
            }
         sprintf( pszNewTailComment + strlen( pszNewTailComment), "%s\n", pkey->pszKeyValue);

         // replace/add new tail comment
         if (*ppszLastTailComment)
            free( *ppszLastTailComment);
         *ppszLastTailComment = pszNewTailComment;

         }

      // now delete key entry
      *pkeyParent = pkey->pkeyNext;
      memset( pkey, 0, sizeof( KEY));
      free( pkey);
      fRemoved = TRUE;

      } // if (pkey)
   }

return fRemoved;
}

// ----------------------------------------------------------------------

static BOOL _removeSECTION( PINIT pinit, PSZ pszSectionName)
{
         BOOL           fRemoved = FALSE;
         PSECTION       psec;
         PSECTION      *psecParent;

if (pinit)
   {
   psecParent = &pinit->psectionFirst;
   psec       = pinit->psectionFirst;
   while (psec)
      {
      if (!stricmp( psec->pszSectionName, pszSectionName))
         break;
      psecParent = &psec->psectionNext;
      psec       = psec->psectionNext;
      }

   if (psec)
      {
      *psecParent = psec->psectionNext;
      memset( psec, 0, sizeof( SECTION));
      free( psec);
      fRemoved = TRUE;
      }
   }

return fRemoved;
}

// ======================================================================

/*
@@WtkOpenInitProfile@SYNTAX
This function opens an existing text initialization file or a pseudo in-memory file.

@@WtkOpenInitProfile@PARM@pszName@in
Address of the ASCIIZ path name of the text initialization file.
:p.
The path name may contain :hp2.?&colon.:ehp2. for the drive in order to
search the file on the boot drive.
:p.
The name may not include wildcards.

@@WtkOpenInitProfile@PARM@phinit@out
The address of a buffer in into which the handle to the
requested text initialization file is to be returned.

@@WtkOpenInitProfile@PARM@ulOpenMode@in
A variable specifying the mode for opening the text initialization file.
:p.
Specify one of the following flags for opening text initialization files:
:parml.
:pt.WTK_INIT_OPEN_READONLY
:pd.open the text initialization file in readonly mode. Any changes by :hp2.WtkWriteInitProfileString:ehp2.
will be ignored.
:pt.WTK_INIT_OPEN_READWRITE
:pd.open the text initialization file exclusively in readwrite mode. This means that the file may
not be in read or write access, otherwise opening the file will fail.
:pt.WTK_INIT_OPEN_INMEMORY
:pd.open the text initialization file in memory only. No updates can be saved, the flag
fUpdate is ignored on :hp2.WtkCloseInitProfile:ehp2..
:eparml.
:p.
Specify additional flags like:
:parml.
:pt.WTK_INIT_OPEN_ALLOWERRORS
:pd.allows parse errors, especially for comment layouts. When saving files that have been read 
in ignoring parse errors, the contents of the new file may be invalid.
:eparml.

@@WtkOpenInitProfile@PARM@ulUpdateMode@in
A variable specifying the update mode for a text initialization file.
:p.
Specify one of the following flags:
:parml.
:pt.WTK_INIT_UPDATE_DISCARDCOMMENTS
:pd.discards any comments when a file is saved
:pt.WTK_INIT_UPDATE_SOFTDELETEKEYS
:pd.delete comments of deleted keys as well
:eparml.

@@WtkOpenInitProfile@PARM@pip@in
A structure defining optional parameters for parsing a text initialization file.
:p.
By defining this parameter structure, the parser of :hp2.WtkOpenInitProfile:ehp2. can
be forced to use custom comment and delimiter characters as well as default indent values.
:p.
However, in order to parse standard Win-16 text initialization files, it is
recommended to use the standard parameters for parsing. For that, specify
NULL for this parameter.

@@WtkOpenInitProfile@RETURN
Return Code.
:p.
WtkOpenInitProfile returns one of the following return codes&colon.

:parml compact break=none.
:pt.0
:pd.NO_ERROR
:pt.8
:pd.ERROR_NOT_ENOUGH_MEMORY
:pt.13
:pd.ERROR_INVALID_DATA
:pt.87
:pd.ERROR_INVALID_PARAMETER
:pt.110
:pd.ERROR_OPEN_FAILED
:eparml.
:p.
or return codes of the following functions
:ul compact.
:li.DosOpen
:eul.

@@WtkOpenInitProfile@REMARKS
This function supports boot drive recognition.
Specify :hp2.?&colon.:ehp2. for the drive in order to
search the file on the boot drive.

@@
*/

APIRET APIENTRY WtkOpenInitProfile( PSZ pszName, PHINIT phinit,
                                    ULONG ulOpenMode, ULONG ulUpdateMode, PINITPARMS pip)
{
         APIRET         rc = NO_ERROR;
         PINIT          pinit = NULL;
         INITPARMS      ip;
         PSZ            p;

         CHAR           szName[ _MAX_PATH];

         PSZ            pszOpenMode;
         BOOL           fInMemory = FALSE;
         BOOL           fAllowErrors = FALSE;

static   PSZ            pszLineComment = WTK_INIT_STR_COMMENT;
static   PSZ            pszNewline     = "\n";

         PSECTION      *ppsecParentPtr = NULL;
         PSECTION       psec           = NULL;

         PKEY          *ppkeyParentPtr = NULL;
         PKEY           pkey           = NULL;

         PSZ            pszLine    = NULL;
         PSZ            pszComment = NULL;
         PSZ            pszValue;

         PSZ            pszThisDelimiter;
         PSZ            pszCheckLine;
         PSZ            pszValueComment;

do
   {
   // check parms
   if (!phinit)
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   // check special bits of open mode
   if (ulOpenMode != WTK_INIT_OPEN_INMEMORY)
      {
      if (ulOpenMode & WTK_INIT_OPEN_ALLOWERRORS)
         {
         fAllowErrors = TRUE;
         ulOpenMode &= ~WTK_INIT_OPEN_ALLOWERRORS;
         }
      }

   // check open modes
   switch (ulOpenMode)
      {
      case WTK_INIT_OPEN_READONLY:  pszOpenMode = "r";            break;
      case WTK_INIT_OPEN_READWRITE: pszOpenMode = "r+";           break;
      case WTK_INIT_OPEN_INMEMORY:  fInMemory = TRUE;             break;
      default:                  rc = ERROR_INVALID_PARAMETER; break;
      }
   if (rc != NO_ERROR)
      break;

   // filename required, if it is not an in-memory init only
   // for in-memory operatin, filename must be NULL
   if (((!fInMemory) && (!pszName)) ||
       ((fInMemory) && (pszName)))
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   // copy name and replace ?: with bootdrive
   if (pszName)
      {
      strcpy( szName, pszName);
      __PatchBootDrive( szName);
      }


   // use defaults
   if (!pip)
      {
      memset( &ip, 0, sizeof( ip));
      pip = &ip;
      }

   if (!pip->pszCommentChars)
      pip->pszCommentChars = WTK_INIT_CHARS_COMMENT;
   if (!pip->pszDelimiterChars)
      pip->pszDelimiterChars = WTK_INIT_CHARS_DELIMITER;

   // check memory for temporary fields
   pszLine    = malloc( WTK_INIT_MAX_LINESIZE);
   pszComment = malloc( 2 * WTK_INIT_MAX_LINESIZE);
   if ((!pszLine) || (!pszComment))
      {
      rc = ERROR_NOT_ENOUGH_MEMORY;
      break;
      }
   *pszLine = 0;
   *pszComment = 0;

   // get memory for data struct
   pinit = malloc( sizeof( INIT));
   if (!pinit)
      {
      rc = ERROR_NOT_ENOUGH_MEMORY;
      break;
      }
   memset( pinit, 0, sizeof( INIT));
   if (!fInMemory)
      strcpy( pinit->szFilename, szName);
   pinit->ulOpenMode            = ulOpenMode;
   pinit->ulUpdateMode          = ulUpdateMode;
   pinit->chComment             = *(pip->pszCommentChars);
   pinit->chDelimiter           = *(pip->pszDelimiterChars);
   pinit->keyLast.ulKeyIndent   = pip->ulKeyIndent;
   pinit->keyLast.ulKeyNameLen  = pip->ulKeyNameLen;
   pinit->keyLast.ulValueIndent = pip->ulValueIndent;


   // --------------------------------------------------------

   // do not read a file, if it is an in-memory init
   if (fInMemory)
      {
      // report pointer as handle
      *phinit = (HINIT) pinit;
      break;
      }

   // --------------------------------------------------------

   // store address for ptr to first section
   ppsecParentPtr = &pinit->psectionFirst;

   // open the file
   pinit->pfile = fopen( szName, pszOpenMode);

   // second try in write mode
   if ((!pinit->pfile) && (ulOpenMode == WTK_INIT_OPEN_READWRITE))
      pinit->pfile = fopen( szName, "w+");

   if (!pinit->pfile)
      {
      rc = ERROR_OPEN_FAILED;
      break;
      }

   while (!feof( pinit->pfile))
      {
      // read line
      if (!fgets( pszLine, WTK_INIT_MAX_LINESIZE, pinit->pfile))
         break;

      // - - - - - - - - - - - - - - - - - - - -

      // handle comments and empty lines
      pszCheckLine = __skipblanks( pszLine);

      if (strchr( pip->pszCommentChars, * pszCheckLine))
         {

         // extra check for C++ comments
         if ((*pszCheckLine != '/') || (*(pszCheckLine + 1) == '/'))
            {
            strcat( pszComment, pszLine);
            continue;
            }
         }
      if (!strncmp( pszCheckLine, pszNewline, strlen( pszNewline)))
         {
         strcat( pszComment, pszLine);
         continue;
         }

      // cut off NEWLINE
      *(pszLine + strlen( pszLine) - 1) = 0;

      // handle new section
      if (*pszLine == WTK_INIT_CHAR_SECTION_START)
         {
         strcpy( pszLine, pszLine + 1);
         p = strchr( pszLine, WTK_INIT_CHAR_SECTION_END);
         if (!p)
            {
            if (fAllowErrors)
               continue;
            else
               {
               rc = ERROR_INVALID_DATA;
               break;
               }
            }
         *p = 0;

         // open a new section
         psec = malloc( sizeof( SECTION));
         if (!psec)
            {
            rc = ERROR_NOT_ENOUGH_MEMORY;
            break;
            }
         memset( psec, 0, sizeof( SECTION));
         *ppsecParentPtr = psec;
         ppsecParentPtr = &psec->psectionNext;

         psec->pszSectionName = strdup( pszLine);
         psec->pszComment     = strdup( pszComment);
         if ((!psec->pszSectionName) ||
             (!psec->pszComment))
            {
            rc = ERROR_NOT_ENOUGH_MEMORY;
            break;
            }
         *pszComment = 0;

         // store address for ptr to first key
         ppkeyParentPtr = &psec->pkeyFirst;

         // we are done so far
         continue;

         }

      // - - - - - - - - - - - - - - - - - - - -

      // handle new key
      if (!ppkeyParentPtr)
         {
         rc = ERROR_INVALID_DATA;
         break;
         }

      // open a new key
      pkey = malloc( sizeof( KEY));
      if (!pkey)
         {
         rc = ERROR_NOT_ENOUGH_MEMORY;
         break;
         }
      memset( pkey, 0, sizeof( KEY));
      *ppkeyParentPtr = pkey;
      ppkeyParentPtr = &pkey->pkeyNext;

      // handle all specified delimter characters like ':' and '='
      pszThisDelimiter = pip->pszDelimiterChars;
      pszValue         = NULL;
      while ((*pszThisDelimiter) && (!pszValue))
         {
         pszValue = strchr( pszLine, *pszThisDelimiter);
         pkey->chDelimiter = *pszThisDelimiter;
         pszThisDelimiter++;
         }
      if (!pszValue)
         {
         if (fAllowErrors)
            continue;
         else
            {
            rc = ERROR_INVALID_DATA;
            break;
            }
         }

      // store key data
      pkey->pszComment    = strdup( pszComment);

      pkey->ulKeyIndent   = __skipblanks( pszLine) - pszLine;

      pkey->pszKeyValue   = strdup(  pszValue + 1);
      pkey->ulValueIndent = __skipblanks( pkey->pszKeyValue)- pkey->pszKeyValue;
      strcpy( pkey->pszKeyValue, pkey->pszKeyValue + pkey->ulValueIndent);

      *pszValue = 0;
      pkey->ulKeyNameLen  = strlen( pszLine) -  pkey->ulKeyIndent;
      pkey->pszKeyName    = strdup( __stripblanks( pszLine));

      if ((!pkey->pszKeyName)  ||
          (!pkey->pszKeyValue) ||
          (!pkey->pszComment))
         {
         rc = ERROR_NOT_ENOUGH_MEMORY;
         break;
         }
      *pszComment = 0;

      // take care for c++ comments at the end of a key value
      // create a copy and detach it from the key value
      if (strchr( pip->pszCommentChars, '/'))
         {
         pszValueComment = strstr( pkey->pszKeyValue, "//");
         if (pszValueComment)
            {
               pkey->pszValueComment = strdup( pszValueComment);
            p = pszValueComment - 1;
            while (*p <= 32)
               {
               p--;
               }
            pkey->ulValueCommentIndent = pszValueComment - p - 1;
            *(p + 1) = 0;
            }
         }

      // take care for multiline values, being enclosed in double quotes
      if (*pkey->pszKeyValue == '"') 
         while (*(pkey->pszKeyValue + strlen( pkey->pszKeyValue) - 1) != '"')
            {
            // read next line
            if (!fgets( pszLine, WTK_INIT_MAX_LINESIZE, pinit->pfile))
               break;
            __stripblanks( pszLine);
   
            pkey->pszKeyValue = realloc( pkey->pszKeyValue, strlen( pkey->pszKeyValue) + strlen( pszLine) + 1);
            strcat( pkey->pszKeyValue, pszLine);
            }

      // save key data for appending new sections and keys
      memcpy( &pinit->keyLast, pkey, sizeof( KEY));

      } // while (!feof( pinit->pfile))

   // take care for errors
   if (rc != NO_ERROR)
      break;

   // --------------------------------------------------------

   // report pointer as handle
   *phinit = (HINIT) pinit;

   } while (FALSE);

// cleanup
if (rc != NO_ERROR)
   _freeINIT( pinit);
if (pszLine)    free( pszLine);
if (pszComment) free( pszComment);
return rc;
}

// ----------------------------------------------------------------------

/*
@@WtkCloseInitProfile@SYNTAX
This function closes a text initialization file previously opened with :hp2.WtkOpenInitProfile:ehp2..

@@WtkCloseInitProfile@PARM@hinit@in
Handle to the text initialization file.

@@WtkCloseInitProfile@PARM@fUpdate@in
Flag specifiying wether the file should be updated.
:p.
Specify one of the following flags:
:parml.
:pt.WTK_INIT_CLOSE_DISCARD
:pd.any changes are discarded
:pt.WTK_INIT_CLOSE_UPDATE
:pd.changes previously applied by :hp2.WtkWriteInitProfileString:ehp2. are written to disk.
This requires that the initialization file
:ul compact.
:li.is not a pseudo in-memory file
:li.has been opened in readwrite mode
:eul.
:eparml.

@@WtkCloseInitProfile@RETURN
Return Code.
:p.
WtkCloseInitProfile returns one of the following return codes&colon.

:parml compact break=none.
:pt.0
:pd.NO_ERROR
:pt.87
:pd.ERROR_INVALID_PARAMETER
:eparml.

@@WtkCloseInitProfile@REMARKS
Regardless to the fUpdate parameter, WtkCloseInitProfile does not perform any update operations if
:ul compact.
:li.the initialization file referred to by the specified file handle is a pseudo in-memory file
:li.the initialization file was opened in WTK_INIT_OPEN_READONLY mode.
:li.the initialization file has not been modified in memory yet
:eul.

@@
*/

APIRET APIENTRY WtkCloseInitProfile( HINIT hinit, BOOL fUpdate)
{
         APIRET         rc = NO_ERROR;
         PINIT          pinit = (PINIT) hinit;

do
   {
   // check parms
   if (!hinit)
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   // update not required ?
   if ((!pinit->fModified) ||
       (!fUpdate)          ||
       (pinit->ulOpenMode != WTK_INIT_OPEN_READWRITE) ||
       (!pinit->pfile))
      break;

   // goto start of file
   rewind( pinit->pfile);
   DosSetFileSize( fileno( pinit->pfile), 0);
   _writeSECTION( pinit->pfile, pinit->psectionFirst, pinit->ulUpdateMode);
   fprintf( pinit->pfile, "\n");

   } while (FALSE);

// cleanup
if (rc == NO_ERROR)
   _freeINIT( pinit);
return rc;
}

// ----------------------------------------------------------------------

/*
@@WtkCloseInitProfileBackup@SYNTAX
This function closes a text initialization file previously opened with :hp2.WtkOpenInitProfile:ehp2. and
creates a backup file.

@@WtkCloseInitProfileBackup@PARM@hinit@in
Handle to the text initialization file.

@@WtkCloseInitProfileBackup@PARM@fUpdateOriginal@in
Flag specifiying which file should hold the updates.
:p.
Specify either
:parml.
:pt.TRUE
:pd.to update the original file and keep the old version in the backup file
:pt.FALSE
:pd.to update the backup file and leave the original file unchanged
:eparml.

@@WtkCloseInitProfileBackup@PARM@pszBackupFile@in
Address of the ASCIIZ path name of the backup file.

@@WtkCloseInitProfileBackup@RETURN
Return Code.
:p.
WtkCloseInitProfile returns one of the following return codes&colon.

:parml compact break=none.
:pt.0
:pd.NO_ERROR
:pt.1
:pd.ERROR_INVALID_FUNCTION
:pt.87
:pd.ERROR_INVALID_PARAMETER
:eparml.

@@WtkCloseInitProfileBackup@REMARKS
WtkCloseInitProfileBackup returns ERROR_INVALID_FUNCTION, if called to close a pseudo in-memory file.
:p.
WtkCloseInitProfileBackup does not perform any update operations if
:ul compact.
:li.the initialization file was opened in WTK_INIT_OPEN_READONLY mode.
:li.the initialization file has not been modified in memory yet
:eul.

@@
*/

APIRET APIENTRY WtkCloseInitProfileBackup( HINIT hinit, BOOL fUpdateOriginal, PSZ pszBackupFile)
{
         APIRET         rc = NO_ERROR;
         PINIT          pinit = (PINIT) hinit;
         FILE          *pfile = NULL;

do
   {
   // check parms
   if (!hinit)
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   // in-memory init cannot be written to file
   if (!pinit->pfile)
      {
      rc = ERROR_INVALID_FUNCTION;
      break;
      }

   // backup wanted ? So we need a backup filename
   if ((!fUpdateOriginal) && (!pszBackupFile))
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }


   // update not required ?
   if ((!pinit->fModified) ||
       (pinit->ulOpenMode != WTK_INIT_OPEN_READWRITE))
      break;

   // close original file anyway
   fclose( pinit->pfile);
   pinit->pfile  = NULL;

   // create backup, if original shall be rewritten
   if (fUpdateOriginal)
      {
      rc = DosCopy( pinit->szFilename, pszBackupFile, DCPY_EXISTING);
      if (rc != NO_ERROR)
         break;
      }

   // (re)open original/backup file for write
   pfile = fopen( (fUpdateOriginal) ? pinit->szFilename : pszBackupFile, "w");
   if (!pfile)
      {
      rc = ERROR_OPEN_FAILED;
      break;
      }

   // write file
   _writeSECTION( pfile, pinit->psectionFirst, pinit->ulUpdateMode);
   fprintf( pfile, "\n");

   } while (FALSE);

// cleanup
if ((!fUpdateOriginal) && (pfile))
   fclose( pfile);
if (rc == NO_ERROR)
   _freeINIT( pinit);
return rc;
}

// ----------------------------------------------------------------------

/*
@@WtkInitProfileModified@SYNTAX
This function queries wether a text initialization file has previously been modified in memory.

@@WtkInitProfileModified@PARM@hinit@in
Handle to the text initialization file.

@@WtkInitProfileModified@RETURN
Change status indicator
:p.
:parml.
:pt.TRUE
:pd.the file has previously been modified in memory and differs from file contents read from harddisk etc.
:pt.FALSE
:pd.the file has not been modified in memory
:eparml.

@@WtkInitProfileModified@REMARKS
none

@@
*/

BOOL APIENTRY WtkInitProfileModified( HINIT hinit)
{
         BOOL           fResult = FALSE;
         PINIT          pinit = (PINIT) hinit;

do
   {
   // check parms
   if (!hinit)
      break;

   fResult = pinit->fModified;
   } while (FALSE);

return fResult;
}

// ----------------------------------------------------------------------

/*
@@WtkQueryInitProfileString@SYNTAX
This function retrieves a string from the specified text initialization file.

@@WtkQueryInitProfileString@PARM@hinit@in
Handle to the text initialization file.

@@WtkQueryInitProfileString@PARM@pszSectionName@in
Section name.
:p.
The name of the section for which the text initialization file data is required. 
:p.
The search performed on the application name is always case-dependent. 
:p.
If this parameter is NULL, this function enumerates all the application names 
present in the initialization profile and returns the names as a list in the pszBuffer 
parameter. Each application name is terminated with a NULL character and the 
last name is terminated with two successive NULL characters. In this instance, 
the ulResult parameter contains the total length of the list excluding the 
final NULL character. 

@@WtkQueryInitProfileString@PARM@pszKeyName@in
Key name. 
:p.
The name of the key for which the text initialization file data is returned. 
:p.
The search on key name is always case-dependent. 
:p.
If this parameter equals NULL, and if the pszSectionName parameter is not equal to 
NULL, this function enumerates all key names associated with the named 
application and returns the key names (not their values) as a list in the 
pBuffer parameter. Each key name is terminated with a NULL character and 
the last name is terminated with two successive NULL characters. In this 
instance, the ulResult parameter contains the total length of the list 
excluding the final NULL character. 
  
@@WtkQueryInitProfileString@PARM@pszDefault@in
Default string. 
:p.
The string that is returned in the pszBuffer parameter, if the key defined by the 
pszKey parameter cannot be found in the text initialization file.
:p.
If the pointer to this parameter is passed as NULL, then nothing is copied into 
the pszKeyName parameter if the key cannot be found. ulResult is returned as 0 in 
this case. 

@@WtkQueryInitProfileString@PARM@pszBuffer@in
text initialization file string. 
:p.
The text string obtained from the text initialization file for the key defined by
the pszKeyName parameter. 

@@WtkQueryInitProfileString@PARM@ulBuflen@in
Maximum string length. 
:p.
The maximum number of characters that can be put into the pszBuffer 
parameter, in bytes. If the data from the profile is longer than
this, it is truncated. 

@@WtkQueryInitProfileString@RETURN
String length returned. 
:p.
The actual number of characters (including the null
termination character) returned in the pszBuffer parameter, in bytes. 

@@WtkQueryInitProfileString@REMARKS
The call searches the text initialization file for a key matching the name
specified by the pszKeyName parameter under the section specified by the pszSectionName 
parameter. If the key is found, the corresponding string is copied. If the key does 
not exist, the default character string, specified by the pszDefault parameter, is 
copied. 
:p.
This function is case-dependent; thus the strings in the pszSectionName
parameter and the pszKeyName parameter must match exactly. This avoids
any code-page dependency. The application storing the data must do any
case-independent matching. 

@@
*/

ULONG APIENTRY WtkQueryInitProfileString( HINIT hinit, PSZ pszSectionName, PSZ pszKeyName,
                                          PSZ pszDefault, PSZ pszBuffer, ULONG ulBuflen)
{
         APIRET         rc = NO_ERROR;
         ULONG          ulValueLen = 0;
         PINIT          pinit = (PINIT) hinit;

         PSECTION       psec;
         PKEY           pkey;
         PSZ            pszResult;

do
   {
   // check parms
   if ((!hinit)       ||
       (!pszBuffer))
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }
   memset( pszBuffer, 0, ulBuflen);

   // find section and/or key
   if (pszSectionName)
      psec = _findSECTION( pinit, pszSectionName);
   else
      {
      rc = _collectSECTION( pinit, pszBuffer, ulBuflen, &ulValueLen);
      break;
      }
   if (!psec)
      break;

   if (pszKeyName)
      pkey = _findKEY( psec, pszKeyName);
   else
      {
      rc = _collectKEY( psec, pszBuffer, ulBuflen, &ulValueLen);
      break;
      }
   if (!pkey)
      break;

   // key not found ?
   if (pkey)
      pszResult = pkey->pszKeyValue;
   else
      pszResult = pszDefault;

   // report result
   if (pszResult)
      {
      memcpy( pszBuffer, pkey->pszKeyValue, ulBuflen);
      ulValueLen = strlen( pszBuffer) + 1;
      }

   } while (FALSE);

// cleanup
return ulValueLen;

}

// ----------------------------------------------------------------------

/*
@@WtkQueryInitProfileSize@SYNTAX
This function obtains the size in bytes of the value of a specified key for a 
specified application in the text initialization file.

@@WtkQueryInitProfileSize@PARM@hinit@in
Handle to the text initialization file.

@@WtkQueryInitProfileSize@PARM@pszSectionName@in
Section name.
:p.
The name of the section for which the text initialization file data is required. 
:p.   
If the pszSectionName parameter is NULL, then the pulDataLen parameter returns the 
length of the buffer required to hold the enumerated list of application names, 
as returned by the WtkQueryInitProfileString function when its pszSectionName parameter is 
NULL. In this case, the pszKeyName parameter is ignored. 

@@WtkQueryInitProfileSize@PARM@pszKeyName@in
Key name. 
:p.
The name of the key for which the text initialization file data is returned. 
:p.
If the pszKey parameter is NULL, and if the pszSectionName parameter is not NULL, the 
pulDataLen returns the length of the buffer required to hold the enumerated 
list of key names for that application name, as returned by the 
PrfQueryProfileString function when its pszKeyName parameter is NULL, and its 
pszSectionName parameter is not NULL. 
  
@@WtkQueryInitProfileSize@PARM@pulDataLen@out
Data length. 
:p.
This parameter is the length of the value data related to the pszKey 
parameter. If an error occurs, this parameter is undefined.

@@WtkQueryInitProfileSize@RETURN
Success indicator. 
:p.
:parml.
:pt.TRUE 
:pd.Successful completion 
:pt.FALSE 
:pd.Error occurred. 
:eparml.

@@WtkQueryInitProfileSize@REMARKS
The pszSectionName parameter and pszKeyName parameter are case sensitive and must match 
the names stored in the file exactly. There is no case-independent searching. 
:p.
This function can be used before using the WtkQueryInitProfileString call, to allocate
space for the returned data. 

@@
*/

BOOL APIENTRY WtkQueryInitProfileSize( HINIT hinit, PSZ pszSectionName, PSZ pszKeyName, PULONG pulDataLen)
{
         APIRET         rc = NO_ERROR;
         PINIT          pinit = (PINIT) hinit;

         PSECTION       psec;
         PKEY           pkey;

do
   {
   // check parms
   if ((!hinit)        ||
       (!pulDataLen))
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   // find section and/or key
   if (pszSectionName)
      psec = _findSECTION( pinit, pszSectionName);
   else
      {
      rc = _collectSECTION( pinit, NULL, 0, pulDataLen);
      break;
      }
   if (!psec)
      {
      rc = ERROR_INVALID_DATA;
      break;
      }

   if (pszKeyName)
      pkey = _findKEY( psec, pszKeyName);
   else
      {
      rc = _collectKEY( psec, NULL, 0, pulDataLen);
      break;
      }
   if (!pkey)
      {
      rc = ERROR_INVALID_DATA;
      break;
      }

   if (pkey)
      *pulDataLen = strlen( pkey->pszKeyValue) + 1;
   else
      rc = ERROR_FILE_NOT_FOUND;


   } while (FALSE);

// cleanup
return (rc == NO_ERROR);

}

// ----------------------------------------------------------------------

/*
@@WtkWriteInitProfileString@SYNTAX
This function writes a string of character data into the specified text initialization file.

@@WtkWriteInitProfileString@PARM@hinit@in
Handle to the text initialization file.

@@WtkWriteInitProfileString@PARM@pszSectionName@in
Section name. 
:p.
The case-dependent name of the section for which profile data is to be 
written.

@@WtkWriteInitProfileString@PARM@pszKeyName@in
Key name. 
:p.
The case-dependent name of the key for which profile data is to be written. 
:p.
This parameter can be NULL, in which case all the pszKeyName pairs 
associated with the pszSectionName parameter are deleted. 

@@WtkWriteInitProfileString@PARM@pszNewValue@in
Text string. 
:p.
This is the value of the pszKeyName pair that is written to the text initialization file.
:p.
If this parameter is NULL, the string associated with the pszKeyName is deleted 
(that is, the entry is deleted). 
:p.
If this parameter is not NULL, the string is used as the value of the pszKeyName,
even if the string has zero length. 

@@WtkWriteInitProfileString@RETURN
Success indicator. 
:p.
:parml.
:pt.TRUE 
:pd.Successful completion 
:pt.FALSE 
:pd.Error occurred. 
:eparml.

@@WtkWriteInitProfileString@REMARKS
If there is no section in the file that matches the pszSectionName, a new 
sectionis created when the pszKeyName entry is made. 
:p.
If the key name does not exist for the section, a new pszKeyName
entry is created for that section. If the pszKeyName already exists
in the file, the existing value is overwritten. 

@@
*/

APIRET APIENTRY WtkWriteInitProfileString( HINIT hinit, PSZ pszSectionName, PSZ pszKeyName, PSZ pszNewValue)
{
         APIRET         rc = NO_ERROR;
         PINIT          pinit = (PINIT) hinit;
         PSECTION       psec;
         PKEY           pkey;

         BOOL           fNotChanged = FALSE;
         PSZ            pszOldValue;
         BOOL           fRemoved = FALSE;

do
   {
   // check parms
   if ((!hinit)           ||
       (!pszSectionName))
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   // find section and key
   psec = _findSECTION( pinit, pszSectionName);
   pkey = _findKEY( psec, pszKeyName);

   // key was found (thus section was found)
   // handle following situations here
   // section   keyname  keyvalue    action
   //  given     given    given      update key
   //  given     given    NULL       delete key

   // handle keys first
   if (pkey)
      {
      // create/update key
      if (pszNewValue)
         {
         if (!strcmp( pkey->pszKeyValue, pszNewValue))
            fNotChanged = TRUE;
         else
            {
            // just replace value
            pszOldValue = pkey->pszKeyValue;
            pkey->pszKeyValue = strdup( pszNewValue);
            if (!pkey->pszKeyValue)
               {
               pkey->pszKeyValue = pszOldValue;
               rc = ERROR_NOT_ENOUGH_MEMORY;
               break;
               }
            else
               free( pszOldValue);
            }
         }
      else
         {
         // remove key
         fRemoved = _removeKEY( pinit, psec, pszKeyName);
         rc = (fRemoved) ? NO_ERROR : ERROR_FILE_NOT_FOUND;
         }

      } // if (pkey)

   // key was not found
   // handle following situations here
   // section   keyname  keyvalue    action
   //  given     given    given      create section and/or key

   // shall a new section and/or key be created ?

   else if (pszNewValue)
      {
      if (!psec)
         {
         // add new section if required
         psec = _createSECTION( pinit, pszSectionName);
         if (!psec)
            {
            rc = ERROR_NOT_ENOUGH_MEMORY;
            break;
            }
         }

      // add new key
      pkey = _createKEY( pinit, psec, pszKeyName, pszNewValue);
      if (!pkey)
         {
         rc = ERROR_NOT_ENOUGH_MEMORY;
         break;
         }
      }

   // handle following situations here
   // section   keyname  keyvalue    action
   //  given     NULL     NULL       delete section

   else if (!pszKeyName)
      {
      // keyName not given, delete section
      fRemoved = _removeSECTION( pinit, pszSectionName);
      rc = (fRemoved) ? NO_ERROR : ERROR_FILE_NOT_FOUND;
      }

   else
      // this occurs, when a key should be deleted,
      // that does not exist
      rc = ERROR_FILE_NOT_FOUND;

   } while (FALSE);

// check if something has been modified
if ((pinit) && (rc == NO_ERROR) && (!fNotChanged))
   pinit->fModified = TRUE;

return rc;

}

