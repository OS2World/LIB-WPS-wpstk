/****************************** Module Header ******************************\
*
* Module Name: _crc32.c
*
* crc32 related functions sample
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2000
*
* $Id: _crc32.c,v 1.2 2004-04-15 21:27:49 cla Exp $
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

#define INCL_WTKUTLCRC
#define INCL_WTKUTLFILE
#include <wtk.h>

// -----------------------------------------------------------------------------

#define PRINTSEPARATOR printf("\n------------------------------------------\n")


int main ( int argc, char *argv[])
{

         APIRET         rc = NO_ERROR;

static   PSZ            pszSourceFile = "\?:\\CONFIG.SYS";

         PSZ            pszContents = NULL;
         ULONG          ulDataLen;

         ULONG          ulCrcFile;
         ULONG          ulCrcMem;

do
   {

   // ----------------------------------------------------------------------

   PRINTSEPARATOR;

   printf( "Testing calculation of CRC32:\n");
   // calc checksum of file config.sys
   ulCrcFile = 0;
   rc = WtkCalcFileCRC32( pszSourceFile, &ulCrcFile);
   if (rc != NO_ERROR)
      {
      printf( "- error: WtkCalcFileCRC32: cannot calculate CRC32 of file %s. rc=%u\n", pszSourceFile, rc);
      break;
      }
   else
      printf( "- WtkCalcFileCRC32: CRC32 of file %s is %0x08x\n", pszSourceFile, ulCrcFile);

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
   ulCrcMem  = -1;
   rc = WtkCalcMemCRC32( pszContents, ulDataLen, &ulCrcMem);
   if (rc != NO_ERROR)
      {
      printf( "- error: WtkCalcMemCRC32: cannot calculate CRC32 of memory contents. rc=%u\n", rc);
      break;
      }
   else
      printf( "- WtkCalcMemCRC32: CRC32 of memory contents is %0x08x\n", ulCrcMem);

   // check if both sum are equal
   if (ulCrcFile == ulCrcMem)
      printf( "-> CRC32 sums match !\n");
   else
      {
      printf( "-> error: CRC32 sums do not match !\n");
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


