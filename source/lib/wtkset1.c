/****************************** Module Header ******************************\
*
* Module Name: wtkset1.c
*
* Source for the settings string functions -
* Part of settings and details manager for WPS classes.
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2000
*
* $Id: wtkset1.c,v 1.1 2005-08-17 21:25:59 cla Exp $
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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


#define INCL_DOS
#define INCL_WIN
#define INCL_WINWORKPLACE
#define INCL_ERRORS
#include <os2.h>

#include "wtkset.h"

#define CHAR_DELIMITER ';'
#define CHAR_ESCAPE    '^'
#define CHAR_EQUAL     '='

// static global vars
static   PSZ            pszEmptyString = "";

// -----------------------------------------------------------------------------

static PSZ _splitSetupString( PSZ pszSetup, PSZ *ppszName, PSZ *ppszValue)
{
         PSZ            pszNextSetting = NULL;
         BOOL           fBreak = FALSE;

         PSZ            p;

do
   {
   if ((!pszSetup)      ||
       (!*pszSetup)     ||
       (!ppszValue))
      break;

   // init target vars
   *ppszName  = pszSetup;
   *ppszValue = NULL; 
   
   p = pszSetup;
   while ((*p) && (!fBreak))
      {
      switch (*p)
         {
         case CHAR_ESCAPE:
            if (*ppszValue)
               // remove escape characters (in values only)
               strcpy( p , p + 1);
            break;
   
         case CHAR_DELIMITER:
            // overwrite delimiter
            *p = 0;
   
            // is there another setting ?
            if (*(p + 1))
               pszNextSetting = p + 1;
   
            // end here
            fBreak = TRUE;
            break;
   
         case CHAR_EQUAL:
            // eliminate equal sign
            *ppszValue = p + 1;
            *p = 0;

            // uppercase settings name
            strupr( *ppszName);
            break;
         }

      // examine next char
      p++;

      } // while ((*p) && (!fBreak))

   // if no value found, return empty string as value
   if (!*ppszValue)
      *ppszValue = pszEmptyString;

   } while (FALSE);

// return ptr to next setting or NULL
return pszNextSetting;
}

// -----------------------------------------------------------------------------

static APIRET _maintainSetupString( PSZ pszSetup, PSZ pszName, PSZ pszBuffer, ULONG ulBuflen, BOOL fDelete)
{

         APIRET         rc  = NO_ERROR;
         BOOL           fSettingFound = FALSE;

         PSZ            pszSetupCopy = NULL;

         PSZ            pszSettingsName;
         PSZ            pszSettingsValue;
         PSZ            pszNextSetting;

do
   {
   // check parms
   if ((!pszSetup)   ||
       (!*pszSetup)  ||
       (!pszName)    ||
       (!*pszName)   ||
       (!pszBuffer)  ||
       (!ulBuflen))
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   // make temp copy
   pszSetupCopy = strdup( pszSetup);
   if (!pszSetupCopy)
      {
      rc = ERROR_NOT_ENOUGH_MEMORY;
      break;
      }

   // loop through all values
   pszNextSetting = pszSetupCopy;
   while ((pszNextSetting) && (!fSettingFound))
      {
      // scan off next setting from setup
      pszNextSetting = _splitSetupString( pszNextSetting, &pszSettingsName, &pszSettingsValue);

      // check result
      if ((!pszSettingsName) || (!pszSettingsValue))
         continue;

      // check attribute name
      fSettingFound = !stricmp( pszSettingsName, pszName);

      // delete setting in original string
      if ((fSettingFound) && (fDelete))
         {
         strcpy( pszSetup + (pszSettingsName - pszSetupCopy),
                 pszSetup + (pszNextSetting - pszSetupCopy));
         }
      }

   // setting found ?
   if (fSettingFound)
      {
      // check buffer len
      if (strlen( pszSettingsValue) + 1 > ulBuflen)
         {
         rc = ERROR_BUFFER_OVERFLOW;
         break;
         }

      // copy value
      strcpy( pszBuffer, pszSettingsValue);

      }
   else
      rc = ERROR_FILE_NOT_FOUND;

   } while (FALSE);

// cleanup
if (pszSetupCopy) free( pszSetupCopy);
return rc;
}

// ---------------------------------------------------------------------------

/*
@@WtkScanSetupString@SYNTAX
This function scans a given setup string for a setting and returns its value.
Unlike with :link reftype=hd refid=WtkExtractSetupString.WtkExtractSetupString:elink.,
the setup string is not modified.

@@WtkScanSetupString@PARM@pszSetup@in
Pointer to a variable holding the setup string.

@@WtkScanSetupString@PARM@pszName@in
Pointer to a variable holding the name of the setting to be searched.
:p.
The name is searched case insensitive.

@@WtkScanSetupString@PARM@pszBuffer@in
The address of a variable, in which the value of the specified setting is returned.

@@WtkScanSetupString@PARM@ulBuflen@in
The length, in bytes, of the buffer described by :hp1.pszBuffer:ehp1..

@@WtkScanSetupString@RETURN
WtkScanSetupString returns one of the following return codes&colon.
:parml compact break=none.
:pt.0
:pd.NO_ERROR
:pt.2
:pd.ERROR_FILE_NOT_FOUND (the setting could not be found)
:pt.8
:pd.ERROR_NOT_ENOUGH_MEMORY
:pt.87
:pd.ERROR_INVALID_PARAMETER
:pt.111
:pd.ERROR_BUFFER_OVERFLOW
:eparml.

@@WtkScanSetupString@REMARKS
:hp2.WtkScanSetupString:ehp2. does not modify the setup string.
:p.
If a found setting should be removed from a given setup string,
:link reftype=hd refid=WtkExtractSetupString.WtkExtractSetupString:elink.
can be used instead.

@@
*/

APIRET APIENTRY WtkScanSetupString( PSZ pszSetup, PSZ pszName, PSZ pszBuffer, ULONG ulBuflen)
{
return _maintainSetupString( pszSetup, pszName, pszBuffer, ulBuflen, FALSE);
}

// -----------------------------------------------------------------------------

/*
@@WtkExtractSetupString@SYNTAX
This function scans a given setup string for a setting and returns its value,
and removes the complete setting from the setup string.
Therefore, unlike with 
:link reftype=hd refid=WtkScanSetupString.WtkScanSetupString:elink.,
the setup string is modified.

@@WtkExtractSetupString@PARM@pszSetup@inout
Pointer to a variable holding the setup string.
:p.
On output, a found setting is removed from the setup string.

@@WtkExtractSetupString@PARM@pszName@in
Pointer to a variable holding the name of the setting to be searched.
:p.
The name is searched case insensitive.

@@WtkExtractSetupString@PARM@pszBuffer@in
The address of a variable, in which the value of the specified setting is returned.

@@WtkExtractSetupString@PARM@ulBuflen@in
The length, in bytes, of the buffer described by :hp1.pszBuffer:ehp1..

@@WtkExtractSetupString@RETURN
WtkExtractSetupString returns one of the following return codes&colon.
:parml compact break=none.
:pt.0
:pd.NO_ERROR
:pt.2
:pd.ERROR_FILE_NOT_FOUND (the setting could not be found)
:pt.8
:pd.ERROR_NOT_ENOUGH_MEMORY
:pt.87
:pd.ERROR_INVALID_PARAMETER
:pt.111
:pd.ERROR_BUFFER_OVERFLOW
:eparml.

@@WtkExtractSetupString@REMARKS
:hp2.WtkExtractSetupString:ehp2. modifies the setup string.
:p.
If a found setting should not be removed from a given setup string,
:link reftype=hd refid=WtkScanSetupString.WtkScanSetupString:elink.
can be used instead.

@@
*/

APIRET APIENTRY WtkExtractSetupString( PSZ pszSetup, PSZ pszName, PSZ pszBuffer, ULONG ulBuflen)
{
return _maintainSetupString( pszSetup, pszName, pszBuffer, ulBuflen, TRUE);
}

// ---------------------------------------------------------------------------

/*
@@WtkSplitSetupString@SYNTAX
This function separates the next setting from a given settings string.

@@WtkSplitSetupString@PARM@ppszSetup@inout
Pointer to a variable holding a pointer to the setup string.
:p.
On input, the variable holds the pointer to the buffer holding the setup string.
:p.
On Output, the variable holds the pointer to the next setting within the same buffer.

@@WtkSplitSetupString@PARM@pszNameBuffer@in
The address of a variable, in which the pointer to the settings name is returned.

@@WtkSplitSetupString@PARM@ulNameBufLen@in
The length, in bytes, of the buffer described by :hp1.pszNameBuffer:ehp1..

@@WtkSplitSetupString@PARM@pszValueBuffer@in
The address of a variable, in which the pointer to the settings value is returned.

@@WtkSplitSetupString@PARM@ulValueBufLen@in
The length, in bytes, of the buffer described by :hp1.pszValueBuffer:ehp1..

@@WtkSplitSetupString@RETURN
WtkSplitSetupString returns one of the following return codes&colon.
:parml compact break=none.
:pt.0
:pd.NO_ERROR
:pt.13
:pd.ERROR_INVALID_DATA
:pt.87
:pd.ERROR_INVALID_PARAMETER
:pt.111
:pd.ERROR_BUFFER_OVERFLOW
:eparml.

@@WtkSplitSetupString@REMARKS
:hp2.WtkSplitSetupString:ehp2. is intended for processing all settings within
a setup string.
:p.
If it is required to process only a specific setting, as an alternative
to scanning all settings with :hp2.WtkSplitSetupString:ehp2.,
the functions
:link reftype=hd refid=WtkScanSetupString.WtkScanSetupString:elink. or
:link reftype=hd refid=WtkExtractSetupString.WtkExtractSetupString:elink..

@@
*/

APIRET APIENTRY WtkSplitSetupString( PSZ *ppszSetup, PSZ pszNameBuffer, ULONG ulNameBufLen,
                                     PSZ pszValueBuffer, ULONG ulValueBufLen)
{

         APIRET         rc  = NO_ERROR;

         PSZ            pszSetupCopy = NULL;

         PSZ            pszSettingsName;
         PSZ            pszSettingsValue;
         PSZ            pszNextSetting = NULL;

do
   {
   // check parms
   if ((!ppszSetup)      ||
       (!*ppszSetup)     ||
       (!pszNameBuffer)  ||
       (!ulNameBufLen)   ||
       (!pszValueBuffer) ||
       (!ulValueBufLen))
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   // make temp copy
   pszSetupCopy = strdup( *ppszSetup);
   if (!pszSetupCopy)
      {
      rc = ERROR_NOT_ENOUGH_MEMORY;
      break;
      }

   // scan off next setting from setup
   pszNextSetting = _splitSetupString( pszSetupCopy, &pszSettingsName, &pszSettingsValue);

   // check result
   if ((!pszSettingsName) || (!pszSettingsValue))
      {
      rc = ERROR_INVALID_DATA;
      break;
      }

   // check buffer length
   if ((strlen( pszSettingsName) + 1 > ulNameBufLen) ||
       (strlen( pszSettingsValue) + 1 > ulValueBufLen))
      {
      rc = ERROR_BUFFER_OVERFLOW;
      break;
      }

   // hand over results
   strcpy( pszNameBuffer, pszSettingsName);
   strcpy( pszValueBuffer, pszSettingsValue);

   // adjust string pointer
   *ppszSetup = (pszNextSetting) ?
                   pszNextSetting - pszSetupCopy + *ppszSetup :
                   NULL;

   } while (FALSE);

// cleanup
if (pszSetupCopy) free( pszSetupCopy);
return rc;
}

