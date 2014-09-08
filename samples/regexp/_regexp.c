/****************************** Module Header ******************************\
*
* Module Name: _regexp.c
*
* Regular expression functions sample
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2000
*
* $Id: _regexp.c,v 1.8 2005-11-01 13:55:04 cla Exp $
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

#define INCL_WTKUTLREGEXP
#define INCL_WTKUTLFILE
#include <wtk.h>

// -----------------------------------------------------------------------------

#define MAX_TEMP_FILES 5
#define PRINTSEPARATOR  printf("\n------------------------------------------\n")
#define PRINTSEPARATOR2 printf("----------------------------\n")


// -----------------------------------------------------------------------------

int main ( int argc, char *argv[])
{

         APIRET         rc = NO_ERROR;
         PSZ            pszTestdataFile = argv[ 1];
         PSZ            pszSearchdataFile = argv[ 2];

         PSZ            pszTestdata = NULL;
         PSZ            pszSearchData = NULL;
         ULONG          ulDataLen;

         PSZ            pszMatchData = NULL;
         ULONG          ulMatchDataLen;

         PSZ            pszLine;
         ULONG          ulResult;
         PSZ            pszExpression;
         BOOL           fResult;

static   PSZ            pszDelimiter = "\r\n";
         PSZ            pszReplacePattern;
do
   {

   // check parameter
   if (argc < 3)
      {
      printf( "error: no files with test data and search data specified\n");
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   // ======================================================================

   PRINTSEPARATOR;

   printf( "- reading file: %s\n", pszTestdataFile);
   rc = WtkReadFile( pszTestdataFile, &pszTestdata, &ulDataLen);
   if (rc != NO_ERROR)
      {
      printf( "- error: WtkReadFile: update data could not be read.\n");
      break;
      }
   ulMatchDataLen = ulDataLen * 3;
   pszMatchData = malloc( ulMatchDataLen);
   if (!pszMatchData)
      {
      printf( "- error: not enough memory.\n");
      rc = ERROR_NOT_ENOUGH_MEMORY;
      break;
      }


   printf( "- reading file: %s\n", pszSearchdataFile);
   rc = WtkReadFile( pszSearchdataFile, &pszSearchData, &ulDataLen);
   if (rc != NO_ERROR)
      {
      printf( "- error: WtkReadFile: update data could not be read.\n");
      break;
      }


   PRINTSEPARATOR2;
   printf( "%s\n", pszTestdata);
   PRINTSEPARATOR2;

   pszLine = strtok(  pszSearchData, pszDelimiter);
   while (pszLine)
      {
      ulResult = strtol( pszLine, &pszExpression, 10);
      if ((pszExpression) && (*pszExpression))
         {
         // skip blanks
         while (*pszExpression <= 32)
            {
            pszExpression++;
            }

         // search expression
         printf( "- check for expression \"%s\"\n", pszExpression);
         if (!WtkIsRegularExpressionValid( pszExpression))
            printf( "error: expression is not valid\n");
         else
            {
            rc = WtkMatchRegularExpression( pszExpression, pszTestdata, pszMatchData, ulMatchDataLen);
            fResult = (rc == NO_ERROR);

            if (fResult)
               printf( "  could be found: %s\n", pszMatchData);
            else
               printf( "  could not be found\n");

            if (ulResult != fResult)
               {
               printf( "\n\nerror: result does not match expected result in search data !\n");
               rc = ERROR_INVALID_DATA;
               break;
               }
            }

         }

      pszLine = strtok(  NULL, pszDelimiter);
      }

   // one special replace case
   pszExpression = "This file is part of the (.*) package and is free software";
   printf( "\n- explicit replace of match of expression: %s\n", pszExpression);
   pszReplacePattern = "The string found is: \"&\"\nthe package name is \"\\1\"";
   rc = WtkSubstRegularExpressionMatch( pszExpression, pszTestdata, pszReplacePattern, pszMatchData, ulMatchDataLen);
   if (rc == NO_ERROR)
      {
      printf( "  pattern is:\n%s\n", pszReplacePattern);
      printf( "  result is:\n%s\n", pszMatchData);
      }
   printf( "\n");

   } while (FALSE);

// cleanup
if (pszTestdata) free( pszTestdata);
if (pszSearchData) free( pszSearchData);
return rc;
}

