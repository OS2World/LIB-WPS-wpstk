/****************************** Module Header ******************************\
*
* Module Name: _eas.c
*
* extended attributes manager sample
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2000
*
* $Id: _eas.c,v 1.5 2006-08-17 23:15:26 cla Exp $
*
* ===========================================================================*
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

#define INCL_WTKEAS
#define INCL_WTKUTLFILE
#include <wtk.h>

#define namecat(s,n) \
{                    \
strcat( s, n);       \
strcat( s, " ");     \
}                    \


static   PSZ            pszFormatTypeLen = " > type %-15s  length %5u\n";

// -----------------------------------------------------------------------------

static PSZ __getEaType( ULONG ulEaType)
{
         PSZ  pszEaType;
switch (ulEaType)
   {
   case EAT_EA:        pszEaType = "EAT_EA";       break;
   case EAT_BINARY:    pszEaType = "EAT_BINARY";   break;
   case EAT_ASCII:     pszEaType = "EAT_ASCII";    break;
   case EAT_BITMAP:    pszEaType = "EAT_BITMAP";   break;
   case EAT_METAFILE:  pszEaType = "EAT_METAFILE"; break;
   case EAT_ICON:      pszEaType = "EAT_ICON";     break;
   default:            pszEaType = "?????";        break;
   }

return pszEaType;
}

// -----------------------------------------------------------------------------

static APIRET _appendEaToList( PSZ pszType, HEA hea, ULONG ulMultiType, ULONG ulEaType, PSZ pszValue, ULONG ulEaValueLen)
{
         APIRET         rc = NO_ERROR;

do
   {
   printf( pszFormatTypeLen, __getEaType( ulEaType), ulEaValueLen);
   rc = WtkAppendEaValue( hea, EAT_MVMT, ulEaType, pszValue, ulEaValueLen);
   if (rc != NO_ERROR)
      {
      printf( "\nWtkAppendEaValue: cannot append %s EA to ea list, rc=%u\n", pszType, rc);
      break;
      }

   fflush( stdout);

   } while (FALSE);

return rc;

}

// -----------------------------------------------------------------------------

#define PRINTSEPARATOR printf("\n------------------------------------------\n")

int main ( int argc, char *argv[])
{

         APIRET         rc = NO_ERROR;
         BOOL           fResult;
         ULONG          i;
         CHAR           szCommand[ 128];

static   PSZ            pszFile = "eatest";
         CHAR           szFile [ _MAX_PATH];

         PSZ            pszEaName;

         HEA            hea = NULLHANDLE;
         ULONG          ulEaType;
         ULONG          ulEaValueLen;
         CHAR           szEaValue[ 200];
static   PSZ            pszStartEnd = "--------------------";

         PSZ            pszValue;
static   PSZ            apszTestComments[] = {"this is line 1", "this is line 2", "this is line 3", ""};
static   PSZ            apszComments[]     = {"Comment 1", "Comment 2", "Comment 3", ""};
static   PSZ            apszTest[]         = {"Test 1", "Test 2", "Test 3", ""};


         CHAR           szFaultyFiles[ 3 * _MAX_PATH];
static   PSZ            pszIconFile      = "test.ico";
static   PSZ            pszBitmapFile    = "test.bmp";
static   PSZ            pszMetafileFile  = "test.met";

         PSZ            pszIconData   = NULL;
         ULONG          ulIconDatalen = 0;
         PSZ            pszBitmapData   = NULL;
         ULONG          ulBitmapDatalen = 0;

         PSZ            pszMetafileData   = NULL;
         ULONG          ulMetafileDatalen = 0;

         PBYTE          pbData = NULL;
         ULONG          ulDatalen;

do
   {

   // --------------------------------------------------------------------------------
   // directory specified ?
   if (argc < 2)
      {
      printf("\nerror: no target directory specified for output file.\n");
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   if (!WtkIsDirectory( argv[1]))
      {
      if (WtkIsFile( argv[1]))
         printf( "\nerror: %s is not a directory.\n");
      else
         printf( "\nerror: directory does not exist.\n");
      rc = ERROR_INVALID_PARAMETER;
      break;
      }
   sprintf( szFile, "%s\\%s", argv[1], pszFile);

   // --------------------------------------------------------------------------------

   // read files with test data

   // read icon file into memory
   szFaultyFiles[ 0] = 0;
   if (!WtkFileExists( pszIconFile))
      namecat( szFaultyFiles, pszIconFile);
   if (!WtkFileExists( pszBitmapFile))
      namecat( szFaultyFiles, pszBitmapFile);
   if (!WtkFileExists( pszMetafileFile))
      namecat( szFaultyFiles, pszMetafileFile);

   if (szFaultyFiles[ 0])
      {
      printf( "\nWtkFileExists: cannot find the file(s) %s\n", szFaultyFiles);
      rc = ERROR_FILE_NOT_FOUND;
      break;
      }

   // read data
   rc = WtkReadFile( pszIconFile, &pszIconData, &ulIconDatalen);
   if (rc != NO_ERROR)
      {
      printf( "\nWtkReadFile: cannot read the icon file %s, rc=%u\n", pszIconFile,rc);
      break;
      }
   rc = WtkReadFile( pszBitmapFile, &pszBitmapData, &ulBitmapDatalen);
   if (rc != NO_ERROR)
      {
      printf( "\nWtkReadFile: cannot read the bitmap file %s, rc=%u\n", pszBitmapFile,rc);
      break;
      }
   rc = WtkReadFile( pszMetafileFile, &pszMetafileData, &ulMetafileDatalen);
   if (rc != NO_ERROR)
      {
      printf( "\nWtkReadFile: cannot read the meta file %s, rc=%u\n", pszMetafileFile,rc);
      break;
      }

   // --------------------------------------------------------------------------------

   // create a completely new file
   sprintf( szCommand, "type nul > %s", szFile);
   system( szCommand);

   // --------------------------------------------------------------------------------

   // write single value single type EA of this file
   pszEaName = ".SUBJECT";
   pszValue = "This is a file for testing EAs";
   printf( "\nWtkPutEaValue:\n Write single-value, single-type extended attribute %s to file %s\n", pszEaName, szFile);
   rc = WtkPutEaValue( szFile, pszEaName, EAT_ASCII, EAT_ASCII, pszValue, strlen( pszValue), FALSE, 0);
   if (rc != NO_ERROR)
      {
      printf( "\nWtkPutEaValue: cannot write single-value, single-type extended attribute, rc=%u\n", rc);
      break;
      }

   // read single value single type EA of this file
   hea      = HEA_CREATE;
   ulEaType = EAT_ASCII;
   printf( "\nWtkGetNextEaValue:\n dump single-value, single-type extended attribute %s of file %s:\n", pszEaName, szFile);
   printf( "%s\n", pszStartEnd);
   while (rc == NO_ERROR)
      {
      ulEaValueLen = sizeof( szEaValue);
      rc = WtkGetNextEaValue( szFile , pszEaName, &ulEaType, &hea, szEaValue, &ulEaValueLen);
      if (rc == NO_ERROR)
         printf( "> %s\n", szEaValue);
      }

   if (rc == ERROR_NO_MORE_FILES)
      {
      printf( "%s\n", pszStartEnd);
      rc = NO_ERROR;
      }
   else
      {
      printf( "\nWtkGetNextEaValue: cannot get the extended attribute, rc=%u\n", rc);
      break;
      }

   // --------------------------------------------------------------------------------

   // write multi value single type EA of this file
   i = 0;
   pszEaName = ".TEST_MVST";
   pszValue = apszTest[ i];
   printf( "\nWtkPutEaValue:\n Write multi-value, single-type extended attribute %s to file %s\n", pszEaName, szFile);
   while ( *pszValue)
      {
      rc = WtkPutEaValue( szFile, pszEaName, EAT_MVST, EAT_ASCII, pszValue, strlen( pszValue), FALSE, 0);
      if (rc != NO_ERROR)
         {
         printf( "\nWtkPutEaValue: cannot write mulit-value, single-type extended attribute, rc=%u\n", rc);
         break;
         }

      // next please
      i++;
      pszValue = apszTest[ i];
      }

   // read multi value single type EA of this file
   hea      = HEA_CREATE;
   ulEaType = EAT_ASCII;
   printf( "\nWtkGetNextEaValue:\n dump multi-value, single-type extended attribute %s of file %s:\n", pszEaName, szFile);
   printf( "%s\n", pszStartEnd);
   while (rc == NO_ERROR)
      {
      ulEaValueLen = sizeof( szEaValue);
      rc = WtkGetNextEaValue( szFile , pszEaName, &ulEaType, &hea, szEaValue, &ulEaValueLen);
      if (rc == NO_ERROR)
         printf( "> %s\n", szEaValue);
      }

   if (rc == ERROR_NO_MORE_FILES)
      {
      printf( "%s\n", pszStartEnd);
      rc = NO_ERROR;
      }
   else
      {
      printf( "\nWtkGetNextEaValue: cannot get the extended attribute, rc=%u\n", rc);
      break;
      }

   // --------------------------------------------------------------------------------

   // write multi value multi type of this file
   i = 0;
   pszEaName = ".COMMENTS";
   pszValue = apszComments[ i];
   printf( "\nWtkPutEaValue:\n Write multi-value, multi-type extended attribute %s to file %s\n", pszEaName, szFile);
   while ( *pszValue)
      {
      rc = WtkPutEaValue( szFile, pszEaName, EAT_MVMT, EAT_ASCII, pszValue, strlen( pszValue), FALSE, 0);
      if (rc != NO_ERROR)
         {
         printf( "\nWtkPutEaValue: cannot write multi-value, multi-type extended attribute, rc=%u\n", rc);
         break;
         }

      // next please
      i++;
      pszValue = apszComments[ i];
      }

   // read multi value multi type history EAs of this file
   hea      = HEA_CREATE;
   ulEaType = EAT_ASCII;
   printf( "\nWtkGetNextEaValue:\n dump multi-value, multi-type extended attribute %s of file %s:\n", pszEaName, szFile);
   printf( "%s\n", pszStartEnd);
   while (rc == NO_ERROR)
      {
      ulEaValueLen = sizeof( szEaValue);
      rc = WtkGetNextEaValue( szFile , pszEaName, &ulEaType, &hea, szEaValue, &ulEaValueLen);
      if (rc == NO_ERROR)
         printf( "> %s\n", szEaValue);
      }

   if (rc == ERROR_NO_MORE_FILES)
      {
      printf( "%s\n", pszStartEnd);
      rc = NO_ERROR;
      }
   else
      {
      printf( "\nWtkGetNextEaValue: cannot get the extended attribute, rc=%u\n", rc);
      break;
      }

   // --------------------------------------------------------------------------------

   PRINTSEPARATOR;

   // write icon data (being a SVST EA)
   pszEaName = ".ICON";
   printf( "\nWtkPutEaValue:\n Write icon data attribute %s to file %s\n", pszEaName, szFile);
   rc = WtkPutEaValue( szFile, pszEaName, EAT_ICON, EAT_ICON, pszIconData, ulIconDatalen, FALSE, 0);
   if (rc != NO_ERROR)
      {
      printf( "\nWtkPutEaValue: cannot write icon data attribute %s to file, rc=%u\n", pszEaName, rc);
      break;
      }

   // --------------------------------------------------------------------------------

   PRINTSEPARATOR;
  
   // write some different types into one multi value multi type attribute
   // this time using the create/save APIs
   printf( "\nWrite different types to multi-value, multi-type\n attribute %s to file %s\n", pszEaName, szFile);

   do
      {
      pszEaName = ".TEST_MVMT";
      rc = WtkCreateEa( szFile, pszEaName, &hea);
      if (rc != NO_ERROR)
         {
         printf( "\nWtkCreateEa: cannot create new EA list %s for file %s in memory , rc=%u\n",
                 pszEaName, szFile, rc);
         break;
         }
      else
         printf( "- created list for EA in memory\n");

      printf( "- appending values:\n");


      pszValue = "This is a test string.";
      rc = _appendEaToList( "ascii", hea, EAT_MVMT, EAT_ASCII, pszValue, strlen( pszValue));
      if (rc != NO_ERROR)
         break;

      rc = _appendEaToList( "binary", hea, EAT_MVMT, EAT_BINARY, pszValue, strlen( pszValue));
      if (rc != NO_ERROR)
         break;

      rc = _appendEaToList( "icon", hea, EAT_MVMT, EAT_ICON, pszIconData, ulIconDatalen);
      if (rc != NO_ERROR)
         break;

      rc = _appendEaToList( "bitmap", hea, EAT_MVMT, EAT_BITMAP, pszBitmapData, ulBitmapDatalen);
      if (rc != NO_ERROR)
         break;

      rc = _appendEaToList( "metafile", hea, EAT_MVMT, EAT_METAFILE, pszMetafileData, ulMetafileDatalen);
      if (rc != NO_ERROR)
         break;

      pszValue = ".COMMENTS";
      rc = _appendEaToList( "referer", hea, EAT_MVMT, EAT_ASCII, pszValue, strlen( pszValue));
      if (rc != NO_ERROR)
         break;

      } while (FALSE);

   rc = WtkSaveEa( hea, FALSE, 0);
   if (rc != NO_ERROR)
      {
      printf( "\nWtkSaveEa:\n Cannot write multi-value, multi-type\n attribute %s to file %s\n", pszEaName, szFile);
      break;
      }
   else
      printf( "- saved EA list\n");
  
   // --------------------------------------------------------------------------------

   PRINTSEPARATOR;

   // use the FindFirst/next/CloseEa API to read comments
   // istead of the WtkGetNextEaValue
   pszEaName = ".TEST_MVMT";
   printf( "\nWtkQueryEaSize:\n query the size of attribute %s of file %s\n", pszEaName, szFile);
   rc = WtkQueryEaSize( szFile, pszEaName, &ulDatalen);
   if (rc != NO_ERROR)
      {
      printf( "\nWtkQueryEaSize: cannot query the size of attribute, rc=%u\n", rc);
      break;
      }
   printf( "EA size is %u bytes\n", ulDatalen);

   if (ulDatalen)
      {
      pbData = malloc( ulDatalen);
      if (!pbData)
         {
         rc = ERROR_NOT_ENOUGH_MEMORY;
         printf( "\nerror: not enough memory.\n");
         break;
         }
   
      // search first attribute
      hea       = HEA_CREATE;
      ulEaType  = WTK_EAT_ANY;
      printf( "\nWtkReadEa:\n read extended attribute %s of file %s\n", pszEaName, szFile);
      ulEaValueLen = ulDatalen;
      rc = WtkReadEa( szFile , pszEaName, &hea);
      if (rc != NO_ERROR)
         {
         printf( "\nWtkFindFirstEa: cannot read first EA %s, rc=%u\n", pszEaName, rc);
         break;
         }
   
      // find first value
      ulEaValueLen = ulDatalen;
      rc = WtkFindFirstEaValue( hea, &ulEaType, pbData, &ulEaValueLen);
      if (rc != NO_ERROR)
         {
         // ignore error and break out
         rc = NO_ERROR;
         break;
         }
   
      printf( "%s\n", pszStartEnd);
      printf( pszFormatTypeLen, __getEaType( ulEaType), ulEaValueLen);
   
      // loop for more values
      while (TRUE)
         {
         ulEaValueLen = ulDatalen;
         rc = WtkFindNextEaValue( hea, &ulEaType, pbData, &ulEaValueLen);
         if (rc != NO_ERROR)
            {
            // ignore error and break out
            printf( "%s\n", pszStartEnd);
            rc = NO_ERROR;
            break;
            }
   
         printf( pszFormatTypeLen, __getEaType( ulEaType), ulEaValueLen);
         }
   
      // close the handle
      WtkCloseEa( hea);
      hea = NULLHANDLE;

      // --------------------------------------------------------------------------------
   
      PRINTSEPARATOR;
   
      // now do the same thing with WtkGetNextEaValue
      printf( "\nWtkGetNextEaValue:\n query all values of extended attribute %s of file %s:\n", pszEaName, szFile);
      hea       = HEA_CREATE;
      ulEaType  = WTK_EAT_ANY;
      printf( "%s\n", pszStartEnd);
      while (rc == NO_ERROR)
         {
         ulEaValueLen = ulDatalen;
         rc = WtkGetNextEaValue( szFile , pszEaName, &ulEaType, &hea, pbData, &ulEaValueLen);
         if (rc == NO_ERROR)
            printf( pszFormatTypeLen, __getEaType( ulEaType), ulEaValueLen);
         }
   
      if (rc == ERROR_NO_MORE_FILES)
         {
         printf( "%s\n", pszStartEnd);
         rc = NO_ERROR;
         }

      }

      // --------------------------------------------------------------------------------
   
      PRINTSEPARATOR;
   
      // test simple string APIs
      pszEaName = ".TEST_EAT";
      printf( "\nWtkWriteStringEa:\n write simple string EA to file %s\n", pszEaName, szFile);
      rc = WtkWriteStringEa( szFile, pszEaName, "This is the easiest EA API call !");
      if (rc != NO_ERROR)
         {
         printf( "\nWtkWriteStringEa: cannot write ascii attribute %s to file, rc=%u\n", pszEaName, rc);
         break;
         }

      printf( "\nWtkReadStringEa:\n read simple string EA from file %s:\n", pszEaName, szFile);
      ulEaValueLen = sizeof( szEaValue);
      rc = WtkReadStringEa( szFile, pszEaName, szEaValue, &ulEaValueLen);
      if (rc != NO_ERROR)
         {
         printf( "\nWtkReadStringEa: cannot read ascii attribute %s from file, rc=%u\n", pszEaName, rc);
         break;
         }
      printf("- EA value is: %s\n", szEaValue);

      // --------------------------------------------------------------------------------
   
      PRINTSEPARATOR;

   } while (FALSE);

// cleanup
if (pszIconData)     free( pszIconData);
if (pszBitmapData)   free( pszBitmapData);
if (pszMetafileData) free( pszMetafileData);
if (pbData) free( pbData);
if (hea) WtkCloseEa( hea);
return rc;
}
  
