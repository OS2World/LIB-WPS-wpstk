/****************************** Module Header ******************************\
*
* Module Name: _syslvl.c
*
* buidlevel information functions sample
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2005
*
* $Id: _bldl.c,v 1.2 2006-12-02 16:01:20 cla Exp $
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

#define INCL_ERRORS
#define INCL_DOSFILEMGR
#include <os2.h>

#define INCL_WTKUTLFILE
#define INCL_WTKFILEBLDL
#include <wtk.h>

// -----------------------------------------------------------------------------

#define PRINTSEPARATOR printf("\n------------------------------------------\n")

// -----------------------------------------------------------------------------

static VOID _printInfo( PSZ pszTitle, PBLDLEVEL pbl)
{
if (pszTitle && pbl)
   {
   printf( "\n- %s:\n"
           "vendor identifier: %s\n"
           "revision number: %s\n"
           "description: %s\n",
           pszTitle,
           pbl->pszVendor,
           pbl->pszRevision,
           pbl->pszDescription);
   }
return;
}

// -----------------------------------------------------------------------------

int main ( int argc, char *argv[])
{
         APIRET         rc = NO_ERROR;
         BOOL           fResult;
         PSZ            pszExecutable     = NULL;
         PSZ            pszSearchdataFile = NULL;

         ULONG          ulBuflen;
         PBLDLEVEL      pbl = NULL;

         PSZ            pszSearchData = NULL;
         ULONG          ulDataLen;

         PSZ            pszLine;
         PSZ            pszParms;
static   PSZ            pszDelimiter = "\r\n";
         ULONG          ulResult;

         CHAR           szVendor[ 32];
         CHAR           szRevision[ 16];
         CHAR           szCheckType[ 32];
         CHAR           szDescription[ 128];
         ULONG          ulCheckType;

         PSZ            pszVendor;
         PSZ            pszRevision;
         PSZ            pszCheckType;
         PSZ            pszDescription;

do
   {
   // get parms
   pszExecutable     = (argc > 1) ?  argv[ 1] : NULL;
   pszSearchdataFile = (argc > 2) ?  argv[ 2] : NULL;

   // check parameter
   if (!pszSearchdataFile)
      {
      printf( "error: not enough parameters, exec file and test data file required\n");
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   // get memory
   ulBuflen = 1024;
   pbl = malloc( ulBuflen);
   if (!pbl)
      {
      printf( "- error: cannot obtain memory, rc=%u\n", rc);
      rc = ERROR_NOT_ENOUGH_MEMORY;
      break;
      }
   memset( pbl, 0, ulBuflen);

   // ======================================================================

   PRINTSEPARATOR;

   printf( "\nquerying buildlevel information of: %s\n", pszExecutable);
   rc = WtkGetBlSignature( pszExecutable, pbl, ulBuflen);
   if (rc == NO_ERROR)
      _printInfo( "WtkGetBlSignature", pbl);
   else
      {
      printf( "- error: WtkGetBlSignature: buildlevel infomation could NOT be read. rc=%u\n", rc);
      break;
      }

   // ======================================================================

   PRINTSEPARATOR;

   printf( "- reading file: %s\n", pszSearchdataFile);
   rc = WtkReadFile( pszSearchdataFile, &pszSearchData, &ulDataLen);
   if (rc != NO_ERROR)
      {
      printf( "- error: WtkReadFile: update data could not be read.\n");
      break;
      }

   pszLine = strtok(  pszSearchData, pszDelimiter);
   while (pszLine)
      {
      // determine expected result
      ulResult = strtol( pszLine, &pszParms, 10);
      if ((pszParms) && (*pszParms))
         {
         // skip blanks
         while (*pszParms <= 32)
            {
            pszParms++;
            }

         // tokenize parameters
         memset( szVendor,      0,  sizeof( szVendor));
         memset( szRevision,    0,  sizeof( szRevision));
         memset( szCheckType,   0,  sizeof( szCheckType));
         memset( szDescription, 0,  sizeof( szDescription));
         if (!sscanf( pszParms, "%s %s %s %s", szVendor, szRevision, szCheckType, szDescription))
            break;

         pszVendor       = szVendor[ 0]      ? szVendor      : NULL;
         pszRevision     = szRevision[ 0]    ? szRevision    : NULL;
         pszCheckType    = szCheckType[ 0]   ? szCheckType   : NULL;
         pszDescription  = szDescription[ 0] ? szDescription : NULL;

         // copy complete rest of string to last field
         if (pszDescription)
            strcpy( szDescription, strstr( pszLine, szDescription));

         // determine check type
         ulCheckType = 0;
         if (pszCheckType)
            {
            if (!strcmp( "EQUAL",            pszCheckType)) ulCheckType = WTK_BLREVCHECK_EQUAL;
            if (!strcmp( "GREATER",          pszCheckType)) ulCheckType = WTK_BLREVCHECK_GREATER;
            if (!strcmp( "LESSER",           pszCheckType)) ulCheckType = WTK_BLREVCHECK_LESSER;
            if (!strcmp( "GREATER_OR_EQUAL", pszCheckType)) ulCheckType = WTK_BLREVCHECK_GREATER_OR_EQUAL;
            if (!strcmp( "LESSER_OR_EQUAL",  pszCheckType)) ulCheckType = WTK_BLREVCHECK_LESSER_OR_EQUAL;
            if (!strcmp( "NOT_EQUAL",        pszCheckType)) ulCheckType = WTK_BLREVCHECK_NOT_EQUAL;
            if (!ulCheckType)
               {
               printf( "error in check file: invalid checktype %s\n", pszCheckType);
               continue;
               }
            }

         printf( " - checking %s",  pszLine);

         // perform check
         rc = WtkCheckBlSignature( pszExecutable, pszVendor, pszRevision, ulCheckType, pszDescription);
         fResult = (rc == NO_ERROR);

         if (fResult)
            printf( " - check is valid");
         else
            printf( " - check is not valid");

         if (ulResult != fResult)
            {
            printf( "- *** no match ***\n\nerror: result does not match expected result in check data !\n");
            rc = ERROR_INVALID_DATA;
            break;
            }
          else
             printf( " - match\n");

         // reset error
         rc = NO_ERROR;
         }

      pszLine = strtok(  NULL, pszDelimiter);
      }


   // ======================================================================

   PRINTSEPARATOR;

   printf( "\nsetting buildlevel information\n");

   pbl->pszVendor = "DUMMY";
   rc = WtkSetBlSignature( pszExecutable, pbl, WTK_BLSET_VENDOR);
   if (rc == NO_ERROR)
      _printInfo( "WtkSetBlSignature updating vendor", pbl);
   else
      {
      printf( "- error: WtkSetBlSignature: buildlevel infomation could NOT be set. rc=%u\n", rc);
      break;
      }

   pbl->pszRevision = "5.6.7";
   rc = WtkSetBlSignature( pszExecutable, pbl, WTK_BLSET_REVISION);
   if (rc == NO_ERROR)
      _printInfo( "WtkSetBlSignature updating revision", pbl);
   else
      {
      printf( "- error: WtkSetBlSignature: buildlevel infomation could NOT be set. rc=%u\n", rc);
      break;
      }

   pbl->pszDescription = "Modified Test sample";
   rc = WtkSetBlSignature( pszExecutable, pbl, WTK_BLSET_DESCRIPTION);
   if (rc == NO_ERROR)
      _printInfo( "WtkSetBlSignature updating description", pbl);
   else
      {
      printf( "- error: WtkSetBlSignature: buildlevel infomation could NOT be set. rc=%u\n", rc);
      break;
      }

   pbl->pszVendor = "COMPLETE";
   pbl->pszRevision = "0.99.98";
   pbl->pszDescription = "Tests are complete now";
   rc = WtkSetBlSignature( pszExecutable, pbl, WTK_BLSET_ALL);
   if (rc == NO_ERROR)
      _printInfo( "WtkSetBlSignature updating all values", pbl);
   else
      {
      printf( "- error: WtkSetBlSignature: buildlevel infomation could NOT be set. rc=%u\n", rc);
      break;
      }

   } while (FALSE);

// cleanup
if (pbl) free( pbl);
if (pszSearchData) free( pszSearchData);
return rc;
}

