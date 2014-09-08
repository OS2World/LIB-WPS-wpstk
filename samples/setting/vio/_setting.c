/****************************** Module Header ******************************\
*
* Module Name: settings.c
*
* Source settings and details manager VIO sample
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2000
*
* $Id: _setting.c,v 1.5 2005-10-23 18:28:35 cla Exp $
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
#include <conio.h>
#include <ctype.h>

#define INCL_ERRORS
#include <os2.h>

#define INCL_WTK
#include <wtk.h>

#define PRINTSEPARATOR printf("\n------------------------------------------\n")

#define NEXTSTRING(s)  (s+strlen(s)+1)

#define SYSPTR_COUNT 9
#define DUMMYPTR (PVOID) -1

#ifdef getch
#define PAUSE if (fInteractive) getch()
#else
#define PAUSE if (fInteractive) _getch() /* use the non-standard c-alias for IBM compilers */
#endif

// prototype of callback function for parameter value handling
BOOL ValueCallbackProc( ULONG ulAction, PVOID pvData, PVOID somSelf, PVOID somThis);


// structure for holding multiple values for the setting POINTER
typedef struct _POINTERLIST
   {
         CHAR           szAnimationFile[ _MAX_PATH];
         BOOL           fAnimated;
   } POINTERLIST, *PPOINTERLIST;

// global object instance variables for also being used by the settings manager
         BOOL           _fAnimationActive;
         CHAR           _szPointerIndex[ 10];
         CHAR           _szAnimationFile[ _MAX_PATH];
         BOOL           _fEnablePointer;
         ULONG          _ulAnimationInitDelay;
         BOOL           _fHidePointer;
         CHAR           _szTestfile[ _MAX_PATH];
         CHAR           _szIpAddress[ 16];
         POINTERLIST    _appl[ SYSPTR_COUNT];

// global variable to control interactivity
         BOOL           fInteractive = FALSE;

// helper routines -------------------------------

BOOL __FileExist( PSZ pszFilename)
{
         BOOL           fResult = FALSE;
         APIRET         rc;
         FILESTATUS3    fs3;

do
   {
   if (!pszFilename)
      break;

   // search entry
   rc = DosQueryPathInfo( pszFilename,
                          FIL_STANDARD,
                          &fs3,
                          sizeof( fs3));
   if (rc != NO_ERROR)
      break;

   // no directory please
   if (fs3.attrFile & FILE_DIRECTORY)
      break;

   fResult = TRUE;

   } while (FALSE);

return fResult;
}

// -----------------------------------------------------------------------------

int main ( int argc, char *argv[])
{

         APIRET         rc      = NO_ERROR;

         BOOL           fAdded  = FALSE;
         BOOL           fClosed = FALSE;

         HMODULE        hmodule;
         CHAR           szIniFile[ _MAX_PATH];
         PSZ            pszIniName;

         PVOID          somSelf = DUMMYPTR; // SOM not really been used here ;-)
         PVOID          somThis = DUMMYPTR; //

         CHAR           szSettings[ 1024];
         ULONG          ulBuflen;

         CHAR           szName[ 32];
         CHAR           szValue[ 128];
         PSZ            pszSetup;
         PSZ            pszSetting;

         // =============  meta class data starts ==============
         HSETTINGTABLE  hst    = NULLHANDLE;
         // =============  meta class data ends ================

         // -------------  object data starts ------------------
         HVALUETABLE    hvt    = NULLHANDLE;
         // -------------  object data ends --------------------

do
   {
   // get parms
   if (argv[ 1])
      {
      fInteractive = (stricmp( "/i", argv[1]) == 0);
      }

   // -------------  settings string tests start -------------------

   strcpy( szSettings, "NAME=My Object;DESCRIPTION=This is my Object;CREATED=12.01.2005;"
                       "COLOR=green;SHAPE=CIRCLE;SIZE=5;");

   pszSetting = "CREATED";
   printf( "\nWtkScanSetupString: Scanning for setting %s in setup string: %s\n", pszSetting, szSettings);
   rc = WtkScanSetupString( szSettings, pszSetting, szValue, sizeof( szValue));
   if (rc == NO_ERROR)
      printf( "- WtkScanSetupString: value of \"%s\" is: %s\n", pszSetting, szValue);
   else
      {
      printf( "\nerror: WtkScanSetupString: cannot scan setting, rc=%u\n", rc);
      break;
      }

   pszSetting = "DESCRIPTION";
   printf( "\nWtkExtractSetupString: Extracting setting %s in setup string: %s\n", pszSetting, szSettings);
   rc = WtkExtractSetupString( szSettings, pszSetting, szValue, sizeof( szValue));
   if (rc == NO_ERROR)
      printf( "- WtkExtractSetupString: value of \"%s\" is: %s\n"
              "     remaining setup string is: %s\n", pszSetting, szValue, szSettings);
   else
      {
      printf( "\nerror: WtkExtractSetupString: cannot extract setting, rc=%u\n", rc);
      break;
      }

   printf( "\nWtkSplitSetupString: scanning all following settings\n");
   pszSetup = szSettings;
   while (pszSetup)
      {
      rc = WtkSplitSetupString( &pszSetup, szName, sizeof( szName), szValue, sizeof( szValue));
      if (rc == NO_ERROR)
         printf( "- %s=%s\n", szName, szValue);
      else
         {
         printf( "\nerror: WtkSplitSetupString: cannot determine next setting, rc=%u\n", rc);
         break;
         }
      }

   printf( "\n remaing settings setup string was: %s\n", szSettings);

   // -------------  settings string tests end ---------------------


   PRINTSEPARATOR;

   // =============  meta class initialization starts ==============

   // initialize own variables, normally done within wpInitData
   _fAnimationActive     = FALSE;
   _szPointerIndex[ 0]   = 0;
   _szAnimationFile[ 0]  = 0;
   _fEnablePointer       = FALSE;
   _ulAnimationInitDelay =
   _fHidePointer         = FALSE;
   _szTestfile[ 0]       = 0;
   memset( _appl, 0, sizeof( _appl));

   // WtkCreateClassSettingsTable - initialize settingsmanager for a meta class
   // - you can specify multiple tables for a meta class, but take care for
   //   setting duplicates then, as they can only be detected within one table !
   // - query an error reason within the WPS environment with
   //      rc = _wpQueryError( somSelf);
   printf( "\n"
           "meta class level: Initialize settings data table:\n"
           "=================================================\n");
   hst = WtkCreateClassSettingsTable( somSelf, ValueCallbackProc);
                                      // somSelf of metaclass to be used here !
   if (!hst)
      {
      printf( "\n\n"
              "*** FATAL ERROR ***\n\n"
              "The class settings table could not be created.\n"
              "Possibly the library is not being compiled with\n"
              "DEBUG activated, which is required for this sample.\n\n");
      rc = ERROR_INVALID_FUNCTION;
      break;
      }
   printf( "\n");
   fflush( NULL);
   PAUSE;

   // WtkAddClassSetting - add settings
   // - specify default values (empty string for no value), subvalues separated
   //   by comma.
   //   Examples:
   //     TEST=;            - specifies one empty value with no default
   //     TEST=ON,,OFF;     - specifies three subvalues with two of them having a default
   ///    TEST=,,,;         - specifies four subvalues with no defaul
   // - the last parameter of this API determines, how often a setting should be
   //   inserted, when current settings are queried via WtkQueryObjectSettings.
   //   Most of all, this value will be 1, in this example the setting POINTER
   //   can be specified for all nine mouse pointers separately, so we need
   //   it to be queried nine times - see callback routine for this example !
   // - adding a setting twice to a table will result in
   //   rc=5 (ERROR_ACCESS_DENIED) - see second call trying to add setting
   //   ANIMATION.
   // - the class table has to be closed with a last call to make sure
   //   that no settings can be added later.
   //     Note:
   //     You cannot use this class table within object instances (e.g. with
   //     WtkCreateObjectValueTable) unless you did not close it !
   // - query an error reason within the WPS environment with
   //      rc = _wpQueryError( somSelf);

   printf( "\n"
           "meta class level: Add settings data:\n"
           "====================================\n");
   fAdded = WtkAddClassSetting( hst, 0, "ANIMATION=ON;", 1);
   fAdded = WtkAddClassSetting( hst, 0, "POINTER=ALL,,ON;", 9);
   fAdded = WtkAddClassSetting( hst, 0, "ANIMINITDELAY=0", 1);
   fAdded = WtkAddClassSetting( hst, 0, "HIDEPOINTER=OFF", 1);
   fAdded = WtkAddClassSetting( hst, 0, "ANIMATION=123;", 1);
   fAdded = WtkAddClassSetting( hst, 0, "TESTFILE=;", 1);
   fClosed = WtkCloseClassSettingsTable( hst);
   printf( "\n");
   fflush( NULL);
   PAUSE;

   // WtkDumpClassSettingsTable - show what we have got until now
   // - this call is for testing purposes in this testcase only, it will dump
   //   all settings to the console only
   printf( "\n"
           "meta class level: dump settings data (test function):\n"
           "=====================================================\n");
   WtkDumpClassSettingsTable( hst);
   printf( "\n");
   fflush( NULL);
   PAUSE;


   // =============  meta class initialization ends ================


   // -------------  object initalisation starts -------------------

   // WtkCreateObjectValueTable - create a table vor settings values
   // - this call creates a value table for the given object instance
   // - WtkCreate* will call your callback routine to
   //   get the types of and the pointer to value buffers for these variables
   //   see settings.h for
   //     action STM_CALLBACK_QUERYVALUEINFO and CBVALUE_ACTION_QUERYTARGETBUF
   //   and
   //     data structure CBVALUEQUERYVALUEINFO and CBVALUEQUERYTARGETBUF
   //   and
   //     all valid datatypes VALUETYPE_*
   //
   //     Note:
   //     - If you do not provide a type and a target buffer for a (sub)value
   //       or an invalid datatype, this (sub)value is being ignored
   //       completely.
   // - query an error reason within the WPS environment with
   //      rc = _wpQueryError( somSelf);
   printf( "\n"
           "object instance level: creating value table:\n"
           "--------------------------------------------\n");
   hvt =  WtkCreateObjectValueTable( hst, somSelf, somThis);
                                  // somSelf of object instance to be used here
   printf( "\n");
   fflush( NULL);
   PAUSE;


   // -------------  object initalisation ends ---------------------

   // -------------  object operations start --------------------

   // WtkEvaluateObjectSettings - send some settings string to the settings API
   // - WtkEvaluateObjectSettings will call your callback routine for each
   //   (sub)value in order to let you validate the provided value.
   //   If you don't process the call, the standard validation for the given
   //   value type will be used.
   //   see settings.h for
   //     action CBVALUE_ACTION_VALIDATE
   //   and
   //     data structure CBVALUEVALIDATE
   //     Note:
   //     - there is no standard validation of types STRING and LONG, so you
   //       have to check the range of (sub)values in the callback routine
   //       yourself !
   // - if one subvalue for a setting is invalid, all values of a statement
   //   for the given setting will be ignored.
   // - WtkEvaluateObjectSettings will call your callback routine for each
   //   setting in order to notify you of the fact, that the target variables
   //   for a setting have changed.
   //   see settings.h for
   //     action CBVALUE_ACTION_REPORTCHANGED
   //   and
   //     data structure CBVALUEREPORTCHANGED
   // - query an error reason within the WPS environment with
   //      rc = _wpQueryError( somSelf);

   printf( "\n"
           "object instance level: scan setup string:\n"
           "-----------------------------------------\n");
   WtkEvaluateObjectSettings( hvt, "POINTER=,STARTREK;");
   printf( "\n");
   fflush( NULL);

   printf( "\n"
           "object instance level: scan setup string:\n"
           "-----------------------------------------\n");
   WtkEvaluateObjectSettings( hvt, "ANIMATION=ON;"
                                   "HIDEPOINTER=ON;"
                                   "POINTER=ALL,BIGARROW,OFF;"
                                   "POINTER=2,MAGGIE,ON;");
   printf( "\n");
   fflush( NULL);

   printf( "\n"
           "object instance level: scan setup string:\n"
           "-----------------------------------------\n");
   WtkEvaluateObjectSettings( hvt, "POINTER=2,MAGGIE,ON;"
                                   "ANIMINITDELAY=50;"
                                   "TESTFILE=C:\\CONFIG.SYS;");
   printf( "\n");
   fflush( NULL);

   printf( "\n"
           "object instance level: scan setup string:\n"
           "-----------------------------------------\n");
   WtkEvaluateObjectSettings( hvt, "TESTFILE=c:\\bla.txt;");
   printf( "\n");                // this should fail, see validation callback !
   fflush( NULL);

   // WtkQueryObjectSettings - query current settings
   // - This call assembles a settings string of all existing settings
   //   and the current values
   // - WtkQueryObjectSettings will call your callback routine for each
   //   (sub)value in order to let you (optionally) update the values for
   //   a setting before being inserted into the resulting string
   //   If you have specified, that for a setting the value should be
   //   specified several times (see WtkAddClassSetting),
   //   WtkQueryObjectSettings will call your callback routine several times,
   //   enabling you to provide several values for a multivalue setting,
   //   like for POINTER in this example.
   //   see settings.h for
   //     action CBVALUE_ACTION_QUERYVALUE
   //   and
   //     data structure PCBQUERYVALUE
   // - query an error reason within the WPS environment with
   //      rc = _wpQueryError( somSelf);

   printf( "\n"
           "object instance level: querying current settings:\n"
           "-------------------------------------------------\n");
   ulBuflen = sizeof( szSettings);
   if (WtkQueryObjectSettings( hvt, szSettings, &ulBuflen))
      printf( "current settings:\n%s\n", szSettings);
   else
      printf( "error querying settings !!!\n");
   printf( "\n");
   fflush( NULL);
   PAUSE;


   printf( "\n"
           "saving setttings:\n"
           "-----------------\n");
   rc = WtkAssemblePackageFilename( (PFN) &main, NULL, NULL, "ini", szIniFile, sizeof( szIniFile));
   if (rc != NO_ERROR)
      {
      printf( "error: cannot determine ini filename\n");
      break;
      }
   if (WtkSaveObjectSettings( hvt, szIniFile, "VIO testcase", "TestObject"))
      printf( "settings saved.");
   else
      printf( "error saving settings !!!\n");
   printf( "\n");
   fflush( NULL);
   PAUSE;

   printf( "\n"
           "restoring settings:\n"
           "-------------------\n");
   if (WtkRestoreObjectSettings( hvt, szIniFile, "VIO testcase", "TestObject"))
      printf( "settings restored.");
   else
      printf( "error restoring settings !!!\n");
   printf( "\n");
   fflush( NULL);
   PAUSE;

   // -------------  object operations ends --------------------

   } while (FALSE);


// ----- cleanup of object starts ------------
if (hvt)
   {
   printf( "\n"
           "object instance level: cleanup of value data:\n"
           "---------------------------------------------\n");
   fflush( NULL);
   WtkDestroyObjectValueTable( hvt);
   PAUSE;
   }

// ----- cleanup of object ends --------------

// ===== cleanup of meta class starts ============

if (hst)
   {
   printf( "\n"
           "meta class level: cleanup of settings data:\n"
           "===========================================\n");
   fflush( NULL);
   WtkDestroyClassSettingsTable( hst);
   PAUSE;
   }
// ===== cleanup of meta class ends ==============

return rc;
}


// -----------------------------------------------------------------------------

BOOL ValueCallbackProc( ULONG ulAction, PVOID pvData, PVOID pvObjectInstance, PVOID pvObjectData)
{

         BOOL           fResult = FALSE;


         // required in WPS code: get the pointer to object data
      /* xxx           *somSelf = (xxx *) pvObjectInstance; */
      /* xxxData       *somThis = (xxxData *) pvObjectData; */

switch (ulAction)
   {

   // =================================

   case STM_CALLBACK_QUERYVALUEINFO:

      // called by WtkCreateObjectValueTable for each subvalue
      // On this callback report target adress for settings value:
      // point to object instance vars for each subvalue and return TRUE;
      //   Note:
      //   - for all (sub)values, target information must be provided,
      //     otherwise a (sub)value is completely ignored
      {
                PCBQUERYVALUEINFO pqvi = pvData;

      printf( "- CALLBACK: reporting value info for setting %s, index %u\n", pqvi->pszName, pqvi->ulValueIndex);
      // --------------------

      if (!strcmp( pqvi->pszName, "ANIMATION"))
         {
         pqvi->ulValueType = STM_VALUETYPE_ONOFF;         // convert ON|OFF to BOOL
         fResult = TRUE;
         }

      // --------------------

      else if (!strcmp( pqvi->pszName, "POINTER"))
         {
         // handle multivalue/multitype setting here
         switch (pqvi->ulValueIndex)
            {
            case 0:
               pqvi->ulValueType = STM_VALUETYPE_STRING;  // no conversion
               fResult = TRUE;
            break;

            case 1:
               pqvi->ulValueType = STM_VALUETYPE_STRING;  // no conversion
               fResult = TRUE;
            break;

            case 2:
               pqvi->ulValueType = STM_VALUETYPE_ONOFF;   // convert ON|OFF to BOOL
               fResult = TRUE;
            break;
            }
         }

      // --------------------

      else if (!strcmp( pqvi->pszName, "ANIMINITDELAY"))
         {
         pqvi->ulValueType = STM_VALUETYPE_LONG;          // convert to LONG
         fResult = TRUE;
         }

      // --------------------

      else if (!strcmp( pqvi->pszName, "HIDEPOINTER"))
         {
         pqvi->ulValueType = STM_VALUETYPE_ONOFF;         // convert ON|OFF to BOOL
         fResult = TRUE;
         }

      // --------------------

      else if (!strcmp( pqvi->pszName, "TESTFILE"))
         {
         pqvi->ulValueType = STM_VALUETYPE_STRING;        // no conversion
         fResult = TRUE;
         }
      }
      break; // case STM_CALLBACK_QUERYVALUEINFO:

   // =================================

   case STM_CALLBACK_QUERYTARGETBUF:

      // called by WtkCreateObjectValueTable for each subvalue
      // On this callback report target adress for settings value:
      // point to object instance vars for each subvalue and return TRUE;
      //   Note:
      //   - for all (sub)values, target information must be provided,
      //     otherwise a (sub)value is completely ignored
      {
                PCBQUERYTARGETBUF pqtb = pvData;

      printf( "- CALLBACK: reporting target buffer for setting %s, index %u\n", pqtb->pszName, pqtb->ulValueIndex);
      // --------------------

      if (!strcmp( pqtb->pszName, "ANIMATION"))
         {
         pqtb->ulBufMax = sizeof( _fAnimationActive);
         pqtb->pvTarget = &_fAnimationActive;
         fResult = TRUE;
         }

      // --------------------

      else if (!strcmp( pqtb->pszName, "POINTER"))
         {
         // handle multivalue/multitype setting here
         switch (pqtb->ulValueIndex)
            {
            case 0:
               pqtb->ulBufMax    = sizeof( _szPointerIndex);
               pqtb->pvTarget    = _szPointerIndex;
               fResult = TRUE;
            break;

            case 1:
               pqtb->ulBufMax    = sizeof( _szAnimationFile);
               pqtb->pvTarget    = _szAnimationFile;
               fResult = TRUE;
            break;

            case 2:
               pqtb->ulBufMax    = sizeof( _fEnablePointer);
               pqtb->pvTarget    = &_fEnablePointer;
               fResult = TRUE;
            break;
            }
         }

      // --------------------

      else if (!strcmp( pqtb->pszName, "ANIMINITDELAY"))
         {
         pqtb->ulBufMax = sizeof( _ulAnimationInitDelay);
         pqtb->pvTarget = &_ulAnimationInitDelay;
         fResult = TRUE;
         }

      // --------------------

      else if (!strcmp( pqtb->pszName, "HIDEPOINTER"))
         {
         pqtb->ulBufMax = sizeof( _fHidePointer);
         pqtb->pvTarget = &_fHidePointer;
         fResult = TRUE;
         }

      // --------------------

      else if (!strcmp( pqtb->pszName, "TESTFILE"))
         {
         pqtb->ulBufMax = sizeof( _szTestfile);
         pqtb->pvTarget = _szTestfile;
         fResult = TRUE;
         }
      }
      break; // case ST_CBVALUE_ACTION_QUERYTARGETBUF:

   // =================================

   case STM_CALLBACK_VALIDATE:

      // called by WtkCreateObjectValueTable for each subvalue
      // On this callback validate settings where standard validation
      // does not fit. Return TRUE, if you have validated, FALSE, if
      // WtkCreateObjectValueTable shall perform standard validation.
      //   Note:
      //   - Note, that for VALUETYPE_STRING and VALUETYPE_LONG no
      //     standard validation is performed

      {
                PCBVALIDATE pv = pvData;

      // --------------------

      if (!strcmp( pv->pszName, "POINTER"))
         {
                ULONG          ulValue = atol( pv->pszName);

         // for first value of this setting
         // allow only "ALL" and "0" to "8"
         switch (pv->ulValueIndex)
            {
            case 0:
               // tell that we do the validation
               fResult = TRUE;

               // for first value allow only "ALL" and "0" to "8"
               if (!stricmp( pv->pszValue, "ALL"))
                  pv->fResult = TRUE;

               else if ((strlen( pv->pszValue) == 1) &&
                        (isdigit( * pv->pszValue))   &&
                        (ulValue < 9))
                  pv->fResult = TRUE;
               break;

            case 1:
               // tell that we do the validation
               fResult = TRUE;

               // for first value allow only existing file
               // pv->fResult = __FileExist( pv->pszValue);

               // let all be valid for testing
               pv->fResult = TRUE;

               break;
            }
         }


      else if (!strcmp( pv->pszName, "TESTFILE"))
         {
         // tell that we do the validation
         fResult = TRUE;

         // allow only existing file
         pv->fResult = __FileExist( pv->pszValue);
         }
      }

      break; // case STM_CALLBACK_VALIDATE:

   // =================================

   case STM_CALLBACK_REPORTCHANGED:

      // called by WtkCreateObjectValueTable for each setting
      // On this callback react on changes for settings, if required.
      //
      // This is mostly the case, when a setting is used to maintain
      // maintain multiple value sets. In our example we store the new
      // value of the setting POINTER to the appropriate list variable.
      //   Note:
      //   - Note, that for VALUETYPE_STRING and VALUETYPE_LONG no
      //     standard validation is performed
      {
                PCBREPORTCHANGED prch = pvData;

      printf( "- CALLBACK: change notification for setting %s\n  -->  ", prch->pszName);

      if (!prch->pszName)
         printf( "all changes complete\n");

      // --------------------

      else if (!strcmp( prch->pszName, "ANIMATION"))
         printf( "ANIMATION changed to %s\n", (_fAnimationActive) ? "ON" : "OFF");

      // --------------------

      else if (!strcmp( prch->pszName, "POINTER"))
         {
                   ULONG          i;

         printf( "POINTER loaded for ptr %s of file %s, %sactivated\n",
                 _szPointerIndex, _szAnimationFile, (_fEnablePointer) ? "" : "de");

         // store the value to the pointerlist, as this setting
         // can be specified and later will queried for all nine
         // mouse pointers separately
         if (!stricmp( "ALL", _szPointerIndex))
            {
            for (i = 0; i < SYSPTR_COUNT; i++)
               {
               strcpy( _appl[ i].szAnimationFile, _szAnimationFile);
               _appl[ i].fAnimated = _fEnablePointer;
               }
            }
         else
            {
            i = atol( _szPointerIndex);
            strcpy( _appl[ i].szAnimationFile, _szAnimationFile);
            _appl[ i].fAnimated = _fEnablePointer;
            }

         }

      // --------------------

      else if (!strcmp( prch->pszName, "ANIMINITDELAY"))
         printf( "ANIMINITDELAY set to %u\n", _ulAnimationInitDelay);

      // --------------------

      else if (!strcmp( prch->pszName, "HIDEPOINTER"))
         printf( "HIDEPOINTER %sactivated\n", (_fHidePointer) ? "" : "de");

      // --------------------

      else if (!strcmp( prch->pszName, "TESTFILE"))
         printf( "TESTFILE set to %s\n", _szTestfile);

      // pause in interactive mode here
      PAUSE;

      }

      break; // case STM_CALLBACK_REPORTCHANGED:

   // =================================

   case STM_CALLBACK_QUERYVALUE:
      // called by WtkQueryObjectSettings for each setting
      // On this callback report update the target buffers for
      // the subvalues, if neccessary.
      //   Note:
      //   - For settings, that need to be queried only once, this call
      //     has not neccessarily to be processed, as the target buffers
      //     for the (sub)values always hold the current values
      //   - Only when a setting can be specified to maintain multiple
      //     sets of values, you will specify a value of > 1 for
      //     ulQueryCount an registration of a setting with WtkAddClassSetting.
      //     In our example it is the setting POINTER, which will be queried
      //     nine times in order to return the animation settings for all nine
      //     mouse pointers.

      {
                PCBQUERYVALUE pqv = pvData;
      // --------------------

      if (!strcmp( pqv->pszName, "POINTER"))
         {
         // put back the appropriate values from the
         // pointerlist to the variables being used
         // by the settings manager

         _ltoa( pqv->ulQueryIndex, _szPointerIndex, 10); // both values equal in this example
         strcpy( _szAnimationFile, _appl[ pqv->ulQueryIndex].szAnimationFile);
         _fEnablePointer = _appl[ pqv->ulQueryIndex].fAnimated;
         }
      }
      break; // case STM_CALLBACK_QUERYVALUE:


   }

return fResult;
}

