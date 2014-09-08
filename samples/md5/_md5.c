/****************************** Module Header ******************************\
*
* Module Name: _md5.c
*
* md5 related functions sample
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2000
*
* $Id: _md5.c,v 1.2 2008-02-03 22:45:03 cla Exp $
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
#include <string.h>
#include <time.h>

#define INCL_ERRORS
#include <os2.h>

#define INCL_WTKUTLMD5
#define INCL_WTKUTLFILE
#include <wtk.h>

// -----------------------------------------------------------------------------

static PSZ _MD5Digest2Str( PMD5DIGEST pmd5d)
{
         ULONG          i;
         PBYTE          p;
static   CHAR           szDigest[ 33];

memset( szDigest, 0, sizeof( szDigest));

if (pmd5d)
   {
   for (i = 0, p = (PBYTE)pmd5d; i < sizeof( MD5DIGEST); i++, p++)
      {
      sprintf( &szDigest[ strlen( szDigest)], "%x", *p);
      }
   }

return szDigest;
}

// -----------------------------------------------------------------------------
#define PRINTSEPARATOR printf("\n------------------------------------------\n")


int main ( int argc, char *argv[])
{

         APIRET         rc = NO_ERROR;

static   PSZ            pszSourceFile = "\?:\\CONFIG.SYS";

         PSZ            pszContents = NULL;
         ULONG          ulDataLen;

         MD5DIGEST      md5dFile;
         MD5DIGEST      md5dMem;
         CHAR           szDigest[ 33];
do
   {

   // ----------------------------------------------------------------------

   PRINTSEPARATOR;

   printf( "Testing calculation of MD5:\n");
   // calc checksum of file config.sys
   rc = WtkCalcFileMD5( pszSourceFile, &md5dFile);
   if (rc != NO_ERROR)
      {
      printf( "- error: WtkCalcFileMD5: cannot calculate MD5 of file %s. rc=%u\n", pszSourceFile, rc);
      break;
      }
   else
      {
      WtkMD5DigestToStr( &md5dFile, szDigest, sizeof( szDigest));
      printf( "- WtkCalcFileMD5: MD5 of file %s is:\n    %s \n",
              pszSourceFile, szDigest);
      }

   // read file into memory
   rc = WtkReadFile( pszSourceFile, &pszContents, &ulDataLen);
   if (rc != NO_ERROR)
      {
      printf( "- error: WtkReadFile: cannot read file %s. rc=%u\n", pszSourceFile, rc);
      break;
      }
   else
      printf( "- WtkReadFile: read %u bytes of file %s into memory\n", ulDataLen, pszSourceFile);

   // calc checksum of file contents in memory
   rc = WtkCalcMemMD5( pszContents, ulDataLen, &md5dMem);
   if (rc != NO_ERROR)
      {
      printf( "- error: WtkCalcMemMD5: cannot calculate MD5 of memory contents. rc=%u\n", rc);
      break;
      }
   else
      {
      WtkMD5DigestToStr( &md5dMem, szDigest, sizeof( szDigest));
      printf( "- WtkCalcMemMD5: MD5 of memory contents is:\n    %s\n", szDigest);
      }

   // check if both sum are equal
   if (!memcmp( &md5dFile, &md5dMem, sizeof( MD5DIGEST)))
      printf( "-> MD5 sums match !\n");
   else
      {
      printf( "-> error: MD5 sums do not match !\n");
      rc = ERROR_INVALID_DATA;
      break;
      }

   // ----------------------------------------------------------------------

   PRINTSEPARATOR;

   } while (FALSE);

// cleanup
if (pszContents) free( pszContents);

return rc;
}


