/****************************** Module Header ******************************\
*
* Module Name: _file.c
*
* file and directory functions sample
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2000
*
* $Id: _file.c,v 1.8 2005-02-19 15:45:30 cla Exp $
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
#define INCL_WTKUTLMODULE
#include <wtk.h>

// interlan prototypes
static APIRET __CleanupPath( PSZ pszTemporaryPath);
static APIRET __TestFilespec( PSZ pszFile);
static APIRET __TestEmpty( PSZ pszType, PSZ pszContents, BOOL fExpected);

// -----------------------------------------------------------------------------

#define MAX_TEMP_FILES 5
#define PRINTSEPARATOR printf("\n------------------------------------------\n")

int main ( int argc, char *argv[])
{

         APIRET         rc = NO_ERROR;
         BOOL           fResult;
         ULONG          i;
         PSZ            p;

         CHAR           szDir[ _MAX_PATH];
         ULONG          ulPathLen = sizeof( szDir);
         HDIR           hdir;
         USHORT         usSignature;

         PSZ            pszTemporaryPath = NULL;

static   PSZ            pszSubPath = "test\\subdir\\sample";
static   PSZ            pszBootDrivePath = "\?:\\_wpstk\\sample\\subdir";

static   PSZ            pszBootDriveTestFile = "\?:\\os2\\ini.rc";
static   PSZ            pszBootDriveTestDir  = "\?:\\os2\\install";
static   PSZ            pszOs2Dir = "\?:\\os2";

static   PSZ            pszTmpFile = "file.???";

static   PSZ            pszSourceFile;
         PSZ            pszContents = NULL;
         ULONG          ulDataLen;
         ULONG          ulFileSize;

         FILESTATUS3    fs3;
         CHAR           szFile[ _MAX_PATH];
         CHAR           szFileMask[ _MAX_PATH];

static   PSZ            pszSomeData = "This is a testfile\n";
static   PSZ            pszFileInLocalDir = "file.txt";

do
   {

   PRINTSEPARATOR;

   // directory specified ?
   if (argc > 1)
      {
      printf( "\nusing commandline parameter %s as test directory.\n", argv[ 1]);
      rc = WtkQueryFullname( argv[ 1], szDir, sizeof( szDir));
      if (rc != NO_ERROR)
         {
         printf( "\nerror: WtkQueryFullname: cannot query fullname of parameter. rc=%u\n", rc);
         break;
         }
      else
         printf( "\nWtkQueryFullname: query fullname of %s\n  %s\n", argv[ 1], szDir);

      if (!WtkIsDirectory( szDir))
         {
         if (WtkIsFile( szDir))
            printf( "\nerror: %s is not a directory.\n");
         else
            printf( "\nerror: directory does not exist.\n");
         rc = ERROR_INVALID_PARAMETER;
         break;
         }
      }
   else
      {
      // get current dir
      rc = WtkQueryCurrentDir( 0, szDir, sizeof( szDir));
      if (rc != NO_ERROR)
         {
         printf( "\n\nerror: WtkQueryCurrentDir: cannot query the current directory. rc=%u\n", rc);
         break;
         }
      else
         printf( "\nWtkQueryCurrentDir: query current dir\n  %s\n", szDir);
      }


   // create subpath
   strcat( szDir, "\\");
   strcat( szDir, pszSubPath);
   rc = WtkCreatePath( szDir);
   if (rc == ERROR_ACCESS_DENIED)
      printf( "\nerror: WtkCreatePath: path already exists %s\n", szDir);
   else if (rc != NO_ERROR)
      {
      printf( "\nerror: WtkCreatePath: cannot create path rc=%u\n  %s\n", rc, szDir);
      break;
      }
   else
      {
      printf( "\nWtkCreatePath: created path\n  %s\n", szDir);
      pszTemporaryPath = strdup( szDir);
      }

   // create tmp file in temp directory
   rc = WtkCreateTmpFile( pszTmpFile, szFile, sizeof( szFile));
   if (rc != NO_ERROR)
      {
      printf( "\nerror: WtkCreateTmpFile: cannot create temporary file in TMP dir. rc=%u\n  %s\n", rc, pszTmpFile);
      break;
      }
   else
      printf( "\nWtkCreateTmpFile: created temporary file\n  %s\n", szFile);


   // delete file
   rc = WtkDeleteFile( szFile);
   if (rc != NO_ERROR)
      {
      printf( "\nerror: WtkDeleteFile: Cannot delete temporary file. rc=%u\n  %s\n", rc, szFile);
      break;
      }
   else
      printf( "\nWtkDeleteFile: deleted temporary file\n  %s\n", szFile);

   // create tmp file in our temporary directory
   sprintf( szFile, "%s\\%s", szDir, pszTmpFile);
   rc = WtkCreateTmpFile( szFile, szFile, sizeof( szFile));
   if (rc != NO_ERROR)
      {
      printf( "\nerror: WtkCreateTmpFile: cannot create temporary file. rc=%u\n  %s\n", rc, szFile);
      break;
      }
   else
      printf( "\nWtkCreateTmpFile: created temporary file\n  %s\n", szFile);

   // write some data to it
   rc = WtkWriteFile( szFile, pszSomeData, strlen( pszSomeData), FALSE);
   if (rc == NO_ERROR)
      printf( "- WtkWriteFile: write file %s, len %u bytes.\n", szFile, ulDataLen);
   else
      {
      printf( "- error: WtkWriteFile: file %s could NOT be written rc=%u\n", szFile, rc);
      break;
      }

   // check functions for file and dir existance
   printf( "\nWtkFileExists: can%s locate temporary file \n  %s\n", (WtkFileExists( szFile)) ? "" : "not", szFile);
   printf( "\nWtkDirExists: can%s locate temporary directory\n  %s\n", (WtkDirExists( szDir)) ? "" : "not", szDir);

   // make sure that file in local dir does not exit already
   WtkDeleteFile( pszFileInLocalDir);

   // rename and move file between partitions
   rc = WtkMoveFile( szFile, pszFileInLocalDir);
   if (rc != NO_ERROR)
      {
      printf( "\nerror: WtkMoveFile: Cannot move file %s to %s. rc=%u\n",
              szFile, pszFileInLocalDir, rc);
      break;
      }
   else
      printf( "\nWtkMoveFile: moved file %s to %s.\n", szFile, pszFileInLocalDir);

   // delete file
   rc = WtkDeleteFile( pszFileInLocalDir);
   if (rc != NO_ERROR)
      {
      printf( "\nerror: WtkDeleteFile: cannot delete temporary file. rc=%u\n  %s\n", rc, pszFileInLocalDir);
      break;
      }
   else
      printf( "\nWtkDeleteFile: deleted temporary file\n  %s\n", pszFileInLocalDir);

   // now create some more files in our temp directory
   printf( "\nWtkCreateTmpFile: creating %u files in temporary directory ", MAX_TEMP_FILES);
   for (i = 0; i < MAX_TEMP_FILES; i++)
      {
      printf( ".");
      sprintf( szFile, "%s\\%s", szDir, pszTmpFile);
      rc = WtkCreateTmpFile( szFile, szFile, sizeof( szFile));
      if (rc != NO_ERROR)
         break;
      }
   if (rc == NO_ERROR)
      printf( "Ok.\n");
   else
      {
      printf( "ERROR (rc=%u) after creating %u file(s)\n", rc, i);
      break;
      }

   // now get all files and delete them
   printf( "\nWtkGetNextFile/WtkDeleteFile: removing all files in temporary directory\n");
   sprintf( szFile, "%s\\*", szDir);
   hdir = NULLHANDLE; // let WtkGetNextFile start a new search

   do
      {
      // get first/next file
      rc = WtkGetNextFile( szFile,  &hdir, szFile, sizeof( szFile));

      // break, if no more files
      if (rc == ERROR_NO_MORE_FILES)
         {
         // everything ok
         rc = NO_ERROR;
         break;
         }

      // break also on real error
      if (rc != NO_ERROR)
         break;

      // now delete the file
      printf( "  %s ", szFile);
      rc = WtkDeleteFile( szFile);
      printf( "%s\n", (rc == NO_ERROR) ? " - deleted" : ": error : NOT DELETED !!!");
      if (rc != NO_ERROR)
         break;

      } while (TRUE);
   if (rc != NO_ERROR)
      break;

   // cleanup path
   rc = __CleanupPath( pszTemporaryPath);
   free( pszTemporaryPath);
   pszTemporaryPath = NULL;
   if (rc  != NO_ERROR)
      break;

   // ======================================================================

   PRINTSEPARATOR;

   printf( "\nTesting read text file:\n");
   // read config.sys
   pszSourceFile = "?:\\CONFIG.SYS";
   rc = WtkReadFile( "?:\\CONFIG.SYS", &pszContents, &ulDataLen);
   if (rc == NO_ERROR)
      printf( "- WtkReadFile: read file %s, len %u bytes.\n", pszSourceFile, ulDataLen);
   else
      {
      printf( "- error: WtkReadFile: file %s could NOT be read. rc=%u\n", pszSourceFile, rc);
      break;
      }

   // copy to a temporary file
   rc = WtkCreateTmpFile( pszTmpFile, szFile, sizeof( szFile));
   if (rc != NO_ERROR)
      {
      printf( "- error: WtkCreateTmpFile: cannot create temporary file in TMP dir. rc=%u\n  %s\n", rc, pszTmpFile);
      break;
      }
   else
      printf( "- WtkCreateTmpFile: created temporary file\n  %s\n", szFile);


   // write complete file in one call and check modification
   rc = DosQueryPathInfo( szFile, FIL_STANDARD, &fs3, sizeof( fs3));
   rc = WtkWriteFile( szFile, pszContents, ulDataLen, TRUE);
   if (rc == NO_ERROR)
      printf( "- WtkWriteFile: write file %s, len %u bytes.\n", szFile, ulDataLen);
   else
      {
      printf( "- error: WtkWriteFile: file %s could NOT be written rc=%u\n", szFile, rc);
      break;
      }
  printf( "- file has %sbeen modified\n",
          WtkFileModified( szFile, &fs3) ? "" : "not ");

   // check file size
   ulFileSize = WtkQueryFileSize( szFile);
   if (ulFileSize == ulDataLen)
      printf( "- WtkQueryFileSize: filesize matches datalen\n");
   else
      {
      printf( "- error: WtkQueryFileSize: filesize does not match datalen: %u != %u\n", ulFileSize, ulDataLen);
      break;
      }

   // cleanup
   WtkDeleteFile( szFile);

   // ------------------

   // determine path of this executable
   // and read its signature
   WtkGetModuleInfo( (PFN) main, NULL, szFile, sizeof( szFile));
   printf( "\nTesting read part of executable file %s:\n", szFile);

   rc = WtkReadFilePart( szFile, 0, &usSignature, sizeof( usSignature));
   if (rc == NO_ERROR)
      printf( "- WtkReadFilePart: read signature: 0x%04x.\n", usSignature);
   else
      {
      printf( "- error: WtkReadFilePart: file %s could NOT be read. rc=%u\n", szFile, rc);
      break;
      }

   // ======================================================================

   PRINTSEPARATOR;

   printf( "\nTesting bootdrive replacement:\n");
   fResult = WtkFileExists( pszBootDriveTestFile);
   printf( "- WtkFileExists: file %s could %sbe found.\n", pszBootDriveTestFile, (fResult) ? "" : "NOT ");
   if (!fResult)
      break;
   rc = WtkQueryFullname( pszBootDriveTestFile, szFile, sizeof( szFile));
   if (rc == NO_ERROR)
      printf( "- WtkQueryFullname: reports name: %s\n", szFile);
   else
      {
      printf( "- error: WtkQueryFullname: could not determine fullname of %s\n", pszBootDriveTestFile);
      break;
      }

   fResult = WtkDirExists( pszBootDriveTestDir);
   printf( "- WtkDirExists: directory %s could %sbe found.\n", pszBootDriveTestDir, (fResult) ? "" : "not ");
   if (!fResult)
      break;
   rc = WtkQueryFullname( pszBootDriveTestDir, szDir, sizeof( szDir));
   if (rc == NO_ERROR)
      printf( "- WtkQueryFullname: reports name: %s\n", szDir);
   else
      {
      printf( "- error: WtkQueryFullname: could not determine fullname of %s\n", pszBootDriveTestDir);
      break;
      }

   // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

   rc = WtkCreatePath( pszBootDrivePath);
   if (rc == NO_ERROR)
      printf( "- WtkCreatePath: directory %s could be created.\n", pszBootDrivePath);
   else
      {
      printf( "- error: WtkCreatePath: directory %s could NOT be created. rc=%u\n", pszBootDrivePath, rc);
      break;
      }
   rc = WtkQueryFullname( pszBootDrivePath, szDir, sizeof( szDir));
   if (rc == NO_ERROR)
      printf( "- WtkQueryFullname: reports name: %s\n", szDir);
   else
      {
      printf( "- error: WtkQueryFullname: could not determine fullname of %s\n", pszBootDrivePath);
      break;
      }

   sprintf( szFileMask, "%s\\%s", pszBootDrivePath, pszTmpFile);
   rc = WtkCreateTmpFile( szFileMask, szFile, sizeof( szFile));
   if (rc == NO_ERROR)
      printf( "- WtkCreateTmpFile: temporary file %s could be created.\n  -> %s\n",
               szFileMask, szFile);
   else
      {
      printf( "- error: WtkCreateTmpFile: temporary file %s could NOT be created. rc=%u\n", rc);
      break;
      }

   // loop until error so that DosFindClose is executed impilcitely !!!
   // we also could use WtkFileMaskExists here, which reports only one
   // file and calls DosFindClose immediately.
   fResult = FALSE;
   hdir = HDIR_CREATE;
   while (rc == NO_ERROR)
      {
      rc = WtkGetNextFile( szFileMask, &hdir, szFile, sizeof( szFile));
      if (rc == NO_ERROR)
         {
         fResult = TRUE;
         WtkDeleteFile( szFile);
         printf( "- WtkGetNextFile: temporary file %s deleted.\n -> %s\n",
                 szFileMask, szFile);
         }
      }
   if (!fResult)
      printf( "- error: WtkGetNextFile: could NOT determine temporary file %s. rc=%u\n", szFileMask, rc);
      // break later, clenaup path first

   // ======================================================================

   PRINTSEPARATOR;

   {
            CHAR           szCurrentPath[ _MAX_PATH];
            CHAR           szNewPath[ _MAX_PATH];

   printf( "\nTesting directory change:\n");
   // save current dir
   rc = WtkQueryCurrentDir( 0, szCurrentPath, sizeof( szCurrentPath));
   if (rc != NO_ERROR)
      {
      printf( "- WtkQueryCurrentDir: cannot query the current directory. rc=%u\n", rc);
      break;
      }
   else
      printf( "- WtkQueryCurrentDir: saved current dir %s. rc=%u\n", szCurrentPath, rc);

   // change to different dir
   rc = WtkSetCurrentDir( pszOs2Dir);
   if (rc != NO_ERROR)
      {
      printf( "- error: WtkSetCurrentDir: cannot change to directory %s. rc=%u\n", pszOs2Dir, rc);
      break;
      }
   else
      printf( "- WtkSetCurrentDir: changed to directory %s. rc=%u\n", pszOs2Dir, rc);

   // compare dir and reset to old
   WtkQueryCurrentDir( 0, szNewPath, sizeof( szNewPath));
   printf( "- directory compare does %s match\n",
           (strcmp( &szNewPath[ 1], &szCurrentPath[ 1])) ? "not " : "");
   WtkSetCurrentDir( szCurrentPath);

   }



   // ======================================================================


   PRINTSEPARATOR;

   // delete our tmp path
   rc = WtkDeletePath( pszBootDrivePath);
   if (rc == NO_ERROR)
      printf( "\n- WtkDeletePath: directory %s could be deleted.\n", pszBootDrivePath);
   else
      {
      printf( "\n- error: WtkDeletePath: directory %s could NOT be deleted. rc=%u\n", pszBootDrivePath, rc);
      break;
      }

   if (!fResult)
      break;

   // ======================================================================


   PRINTSEPARATOR;

   printf( "\nTesting filespec determination\n");

   // fully qualified name
   WtkQueryFullname( pszBootDriveTestFile, szFile, sizeof( szFile)); // has worked before ...
   rc = __TestFilespec( szFile);
   if (rc != NO_ERROR)
      break;

   // without path, but with drive
   strcpy( szFile, "c:file.txt");
   __TestFilespec( szFile);
   printf( "(ignore error here !)\n");

   // without drive and path
   strcpy( szFile, "file.txt");
   __TestFilespec( szFile);
   printf( "(ignore error here !)\n");

   // without name -> error !
   strcpy( szFile, "d:\\subdir\\test\\");
   __TestFilespec( szFile);
   printf( "(ignore error here !)\n");


   printf("\n");

   // ======================================================================

   PRINTSEPARATOR;

   // show all directories below ?:\OS2
   p = "?:\\OS2\\ARCHIVES";
   printf( "\nWtkGetNextDirectory: show all directories below %s\n", p);
   sprintf( szDir, "%s\\*", p);
   hdir = NULLHANDLE; // let WtkGetNextFile start a new search

   do
      {
      // get first/next file
      rc = WtkGetNextDirectory( szDir,  &hdir, szDir, sizeof( szDir));

      // break, if no more files
      if (rc == ERROR_NO_MORE_FILES)
         {
         // everything ok
         rc = NO_ERROR;
         break;
         }

      // break also on real error
      if (rc != NO_ERROR)
         break;

      // now delete the file
      printf( "  - %s\n", szDir);
      } while (TRUE);

   if (rc != NO_ERROR)
      break;


   // ======================================================================

   PRINTSEPARATOR;

   // check for empty text file
   rc = __TestEmpty( "empty", "\r\n\r\n\r\n\r\n\r\n\r\n", TRUE);
   if (rc != NO_ERROR)
      break;
   rc = __TestEmpty( "non-empty", "\r\n\r\n\r\nThis is one line\r\n\r\n\r\n", FALSE);
   if (rc != NO_ERROR)
      break;

   // ======================================================================

   PRINTSEPARATOR;


   } while (FALSE);

// cleanup
if (pszTemporaryPath)
   {
   __CleanupPath( pszTemporaryPath);
   free( pszTemporaryPath);
   pszTemporaryPath = NULL;
   }
if (pszContents) free( pszContents);

return rc;
}

// -----------------------------------------------------------------------------

static APIRET __CleanupPath( PSZ pszTemporaryPath)
{
         APIRET         rc = NO_ERROR;

do
   {
   // check parm
   if (!pszTemporaryPath)
      break;

   // delete path
   rc = WtkDeletePath( pszTemporaryPath);
   if (rc == NO_ERROR)
      printf( "\nWtkDeletePath: deleted temporary path\n  %s\n", pszTemporaryPath);
   else
      printf( "\nWtkDeletePath: cannot delete temporary path. rc=%u\n  %s\n", rc, pszTemporaryPath);
   } while (FALSE);

return rc;

}

// -----------------------------------------------------------------------------

static APIRET __TestFilespec( PSZ pszFile)
{
         APIRET         rc = NO_ERROR;
         PSZ            pszFileSpec;

do
   {
   printf( "\nWtkFilespec examining \"%s\":\n", pszFile);
   pszFileSpec = WtkFilespec( pszFile, WTK_FILESPEC_PATHNAME);
   if (pszFileSpec)
      printf( " - path starts at: %s\n", pszFileSpec);
   else
      {
      printf( " - path could not be determined\n");
      rc= ERROR_INVALID_FUNCTION;
      }

   pszFileSpec = WtkFilespec( pszFile, WTK_FILESPEC_NAME);
   if (pszFileSpec)
      printf( " - name starts at: %s\n", pszFileSpec);
   else
      {
      printf( " - WtkFilespec: name could not be determined\n");
      rc= ERROR_INVALID_FUNCTION;
      }
   pszFileSpec = WtkFilespec( pszFile, WTK_FILESPEC_EXTENSION);
   if (pszFileSpec)
      printf( " - extension starts at: %s\n", pszFileSpec);
   else
      {
      printf( " - WtkFilespec: extension could not be determined\n");
      rc= ERROR_INVALID_FUNCTION;
      }

   } while (FALSE);

return rc;

}

// -----------------------------------------------------------------------------

static APIRET __TestEmpty( PSZ pszType, PSZ pszContents, BOOL fExpected)
{
         APIRET         rc = NO_ERROR;
         CHAR           szFile[ _MAX_PATH];
         BOOL           fCleanup = FALSE;

do
   {

   // create file with some spaces and newlines
   rc = WtkCreateTmpFile( "test.???", szFile, sizeof( szFile));
   if (rc != NO_ERROR)
      {
      printf( "- WtkCreateTmpFile: error creating temporary file rc=%u\n", rc);
      break;
      }
   fCleanup = TRUE;

   printf( "\nTesting for %s test file:\n", pszType);
   rc = WtkWriteFile( szFile, pszContents, strlen( pszContents), FALSE);
   if (rc != NO_ERROR)
      {
      printf( "- WtkWriteFile: error writing temporary file rc=%u\n", rc);
      break;
      }

   if (WtkFileIsEmpty( szFile) == fExpected)
      printf( "- WtkFileIsEmpty: file %s has been checked %s successfully\n",  szFile, pszType);
   else
      {
      printf( "- WtkFileIsEmpty:: error: file %s has not been %s, but should be\n", szFile, pszType);
      break;
      }

   } while (FALSE);

// cleanup
if (fCleanup) WtkDeleteFile( szFile);
return rc;
}

