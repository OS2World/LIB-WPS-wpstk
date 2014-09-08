/****************************** Module Header ******************************\
*
* Module Name: wtkfinit.h
*
* include file for access functions for text initialization files
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2000
*
* $Id: wtkfinit.h,v 1.2 2003-04-24 15:41:41 cla Exp $
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

#ifndef WTKFILEINIT_INCLUDED
#define WTKFILEINIT_INCLUDED File access functions for text initialization files

#ifdef __cplusplus
      extern "C" {
#endif

#pragma pack(1)


/* file handle type and structures */
typedef LHANDLE HINIT;
typedef HINIT  *PHINIT;

// structure to override default behaviour of parser
typedef struct _INITPARMS
   {
         // specify all valid comment characters
         // - specify NULL to use ';' as default and only comment character
         // - specify '/' to use "//" (c++ style comments)

         PSZ            pszCommentChars;

         // specify all valid delimiter characters
         // - specify NULL to use '=' as default and only delimiter character
         // - when multiple characters are specified, the first will be used
         //   for new entries

         PSZ            pszDelimiterChars;

         // define layout for keys in new files
         // (if keys already exist, the layout of
         //  the last read key is used)
         //
         // [SECTION]
         //
         //      newkeyname1   =   keyvalue1
         // |   |  -> ulKeyIndent
         //
         //      newkeyname2   =   keyvalue2
         //      |            | ->  ulKeyNameLen
         //
         //      newkeyname3   =   keyvalue3
         //                     | | -> ulValueIndent

         ULONG          ulKeyIndent;
         ULONG          ulKeyNameLen;
         ULONG          ulValueIndent;
   } INITPARMS, *PINITPARMS;


/* define open modes (WtkOpenInitProfile - ulOpenMode) */
#define WTK_INIT_OPEN_READONLY           0x0000
#define WTK_INIT_OPEN_READWRITE          0x0001

#define WTK_INIT_OPEN_ALLOWERRORS        0x8000
#define WTK_INIT_OPEN_INMEMORY           0xFFFF

/* define update modes (WtkOpenInitProfile - ulUpdateMode) */
#define WTK_INIT_UPDATE_DISCARDCOMMENTS  0x0001
#define WTK_INIT_UPDATE_SOFTDELETEKEYS   0x0002

/* --- prototypes --- */

/* open and close file */
APIRET APIENTRY WtkOpenInitProfile( PSZ pszName, PHINIT phinit, ULONG ulOpenMode,
                          ULONG ulUpdateMode, PINITPARMS pip);
APIRET APIENTRY WtkCloseInitProfile( HINIT hinit, BOOL fUpdate);
#define WTK_INIT_CLOSE_DISCARD    0
#define WTK_INIT_CLOSE_UPDATE     1

APIRET APIENTRY WtkCloseInitProfileBackup( HINIT hinit, BOOL fUpdateOriginal, PSZ pszBackupFile);
BOOL APIENTRY WtkInitProfileModified( HINIT hinit);

/* query values */
ULONG APIENTRY WtkQueryInitProfileString( HINIT hinit, PSZ pszSectionName, PSZ pszKeyName, PSZ pszDefault, PSZ pszBuffer, ULONG ulBuflen);
BOOL APIENTRY WtkQueryInitProfileSize( HINIT hinit, PSZ pszSectionName, PSZ pszKeyName, PULONG pulDataLen);

/* update or delete keys and/or sections */
APIRET APIENTRY WtkWriteInitProfileString( HINIT hinit, PSZ pszSectionName, PSZ pszKeyName, PSZ pszNewValue);


#pragma pack()

#ifdef __cplusplus
        }
#endif

#endif /* WTKFILEINIT_INCLUDED */

