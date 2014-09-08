/****************************** Module Header ******************************\
*
* Module Name: wtkuctl.c
*
* Source for PM control utility functions.
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2000
*
* $Id: wtkuctl.c,v 1.9 2005-02-15 19:50:09 cla Exp $
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

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INCL_DOS
#define INCL_WIN
#define INCL_WINSEI
#define INCL_ERRORS
#include <os2.h>

#include "wtkuctl.h"
#include "wtkuerr.h"
#include "wpstk.ih"

// some equates
#define MAXBUFFER_ULONG 11

// ---------------------------------------------------------------------------

/*
@@WtkQueryClassIndex@SYNTAX
This function determines the class index (like WC_*)
for public PM classes.

@@WtkQueryClassIndex@PARM@hwnd@in
Handle of the window to be examined.

@@WtkQueryClassIndex@RETURN
Class index (like WC_*).
:parml.
:pt.NULLHANDLE
:pd.the class index could not be obtained or the class is not a public PM class.
:pt.other
:pd.class index (like WC_*)
:eparml.
:p.
Extended error information can be obtained using :hp2.WinGetLastError:ehp2./:hp2.WinGetErrorInfo:ehp2..

@@WtkQueryClassIndex@REMARKS
The class index can only be queried for public PM classes,
that is, classes, that return "#n" as the PM class name, where
n is a number.
.br
Your application may register private PM classes compliant
to that name scheme with an unused number, so that this function can
be used anyway.
:p.
In order to use the resulting PSZ value in a switch statement,
precede both the switch variable and the case values with
:hp1.(const):ehp1..

@@
*/



PSZ APIENTRY WtkQueryClassIndex( HWND hwnd)
{
         APIRET         rc = NO_ERROR;

         PSZ            pszClassIndex = NULL;
         CHAR           szClassname[ 10];

do
   {
   // check parms
   if (!hwnd)
      {
      rc = PMERR_INVALID_HWND;
      break;
      }

   // get class name
   if (!WinQueryClassName( hwnd, sizeof( szClassname), szClassname))
      {
      rc = PMHERR_USE_EXISTING_ERRORINFO;
      break;
      }

   if (szClassname[ 0] != '#')
      {
      rc = PMERR_INVALID_PARAMETER_TYPE;
      break;
      }

   // calculate index value
   pszClassIndex = (PSZ) (atol( &szClassname[ 1]) | 0xffff0000);

   } while (FALSE);

WtkSetErrorInfo( rc);
return pszClassIndex;
}

// ---------------------------------------------------------------------------

/*
@@WtkIsOfPublicPmClass@SYNTAX
This function determines wether a given window is of a
specific PM class and has the given primary style, thus
is a specific kind of this class.

@@WtkIsOfPublicPmClass@PARM@hwnd@in
Handle of the window to be examined.

@@WtkIsOfPublicPmClass@PARM@pszClassIndex@in
Class index (like WC_*)
:p.
This value specifies, of what public PM class the window should be.
Some possible values are&colon.
:ul compact.
:li.WC_FRAME
:li.WC_COMBOBOX
:li.WC_BUTTON
:li.WC_MENU
:li.WC_STATIC
:li.WC_ENTRYFIELD
:li.WC_LISTBOX
:li.WC_SCROLLBAR
:li.WC_TITLEBAR
:eul.
:p.
These classes return "#n" as the PM class name, where n is a number.
This is a requirement for WtkIsOfPublicPmClass to identify
a PM Class as public.
.br
Your application may register PM classes compliant
to that name scheme with a high number, so that this
function can be used anyway.

@@WtkIsOfPublicPmClass@PARM@ulPrimaryWindowStyle@in
Primary window style.
:p.
This style is specified to further specify, what characteristic
the PM class of the examined window should have.
If that further specification is not required, leave this parameter zero.
:p.
Before examining the window style of the given window,
it is first masked with BS_PRIMARYSTYLES (0x000fL).
:p.
Here are some examples of window styles, that make sense in conjunction
with their respective public PM class&colon.

:parml tsize=20 break=none.
:pt.:hp2.public PM class:ehp2.
:pd.:hp2.primary window styles:ehp2.
:pt.WC_BUTTON
:pd.BS_PUSHBUTTON, BS_CHECKBOX, BS_AUTOCHECKBOX,
BS_RADIOBUTTON, BS_AUTORADIOBUTTON,
BS_3STATE, BS_3STATE
:pt.WC_STATIC
:pd.SS_TEXT, SS_GROUPBOX, SS_ICON, SS_BITMAP, SS_HALFTONERECT, SS_BKGNDRECT,
SS_FGNDFRAME, SS_BKGNDFRAME, SS_SYSICON
:eparml.


@@WtkIsOfPublicPmClass@RETURN
Class member indicator.
:parml.
:pt.TRUE
:pd.the class is of the given class and has the given primary window style.
:pt.FALSE
:pd.the class is not of the given class or does not have the given primary window style.
:eparml.
:p.
Extended error information can be obtained using :hp2.WinGetLastError:ehp2./:hp2.WinGetErrorInfo:ehp2..

@@WtkIsOfPublicPmClass@REMARKS
Ths function can only handle windows of classes, that are
public PM classes, that is, classes, that return "#n" as the
PM class, where n is a number.
:p.
This function can also be used with privately registered PM Classes, if you use
a user-defined WC_* constant like ((PSZ)0xffffxxxxL), where xxxx is the hexadecimal value
of the class index. In order to not interfere with public system classes, it is recommended
to use numbers larger than 0x1000. This user-defined WC_ constant must be used when registering
the private class with WinRegisterClass.
:p.
The following example illustrates using WtkIsOfPublicPmClass with a self-defined PM class&colon.
:xmp.

 #define WC_USER_MYCLASS          ((PSZ) 0xffff1000L)
 #define CS_USER_MYCLASS_SPECIAL  0x1000

 // register the class and create a window
 fResult  = WinRegisterClass( hab, WC_USER_MYCLASS, MyClassWindowProc, 0, 0);
 hwndCtrl = WinCreateWindow( HWND_DESKTOP, WC_USER_MYCLASS, NULL, 0, 0, 0, 0,
                             CS_USER_MYCLASS_SPECIAL, hwndFrame,
                             HWND_TOP, 0, NULL, NULL);

 // now check if this control is of our private class and our special style is set
 fIsOurClass = WtkIsOfPublicPmClass( hwndCtrl, WC_USER_MYCLASS, CS_USER_MYCLASS_SPECIAL);

:exmp.
@@
*/

BOOL APIENTRY WtkIsOfPublicPmClass( HWND hwnd, PSZ pszClassIndex, ULONG ulPrimaryWindowStyle)
{
         BOOL           fResult = FALSE;
         APIRET         rc = NO_ERROR;

         ULONG          ulWindowStyle = 0;

do
   {
   // check parms
   if ((!hwnd) || (HIUSHORT( pszClassIndex) != (USHORT) -1))
      {
      rc = PMERR_INVALID_HWND;
      break;
      }

   // isolate main class styles
   if (ulPrimaryWindowStyle)
      ulWindowStyle = WinQueryWindowULong( hwnd, QWL_STYLE) & BS_PRIMARYSTYLES;

   // check parms
   if ((!pszClassIndex)                                     ||
       (WtkQueryClassIndex( hwnd) != pszClassIndex)         ||
       (ulWindowStyle             != ulPrimaryWindowStyle))
      {
      rc = PMERR_INVALID_PARAMETERS;
      break;
      }

   } while (FALSE);

return WtkSetErrorInfo( rc);
}

// ---------------------------------------------------------------------------

/*
@@WtkGetDefaultWindowProcPtr@SYNTAX
This function determines the address of the default window
procedure of the PM class of the specified window.

@@WtkGetDefaultWindowProcPtr@PARM@hwnd@in
Handle of the window to be examined.

@@WtkGetDefaultWindowProcPtr@RETURN
Success indicator.
:parml compact.
:pt.NULL
:pd.error occurred
:pt.any other value
:pd.pointer to the default window procedure
:eparml.
:p.
Extended error information can be obtained using :hp2.WinGetLastError:ehp2./:hp2.WinGetErrorInfo:ehp2..

@@WtkGetDefaultWindowProcPtr@REMARKS
None.

:exmp.
@@
*/

PFNWP APIENTRY WtkGetDefaultWindowProcPtr( HWND hwnd)
{
         CHAR           szClassName[ 20];
         CLASSINFO      ci;

WinQueryClassName( hwnd, sizeof( szClassName), szClassName);
WinQueryClassInfo( WinQueryAnchorBlock( hwnd), szClassName, &ci);
return ci.pfnWindowProc;
}

// ---------------------------------------------------------------------------

/*
@@WtkInitializeNumSpinbuttonArray@SYNTAX
This function initializes a spinbutton, which is to hold a range
of numeric values with steps in between.

@@WtkInitializeNumSpinbuttonArray@PARM@hwndSpinbutton@in
Handle of the spinbutton window.

@@WtkInitializeNumSpinbuttonArray@PARM@ulMinValue@in
The minimum value of the spinbutton.

@@WtkInitializeNumSpinbuttonArray@PARM@ulMaxValue@in
The maximum value of the spinbutton.

@@WtkInitializeNumSpinbuttonArray@PARM@ulStep@in
The step between the values in the range.

@@WtkInitializeNumSpinbuttonArray@RETURN
Success indicator.
:parml compact.
:pt.TRUE
:pd.Successful.
:pt.FALSE
:pd.Not successful.
:eparml.
:p.
Extended error information can be obtained using :hp2.WinGetLastError:ehp2./:hp2.WinGetErrorInfo:ehp2..

@@WtkInitializeNumSpinbuttonArray@REMARKS
Unfortunately the OS/2 Presentation Manager provides numeric spinbuttons
does not provide a parameter for steps in a numeric range, when setting up
a spinbutton with numeric values. This forces you to either
:ul compact.
:li. to have a spinbutton, which gives access to all the values between the minimum
and maximum value, resulting in a very ineffective control when using large ranges
:li. to setup a temporary array of strings for initializing the spinbutton, resulting
in a numeric spinbutton having only as much as elements between the minimum and maximum
value, as useful.
:eul.
:p.
WtkInitializeNumSpinbuttonArray aids you in the latter case. Use also
:link reftype=hd refid=WtkQueryNumSpinbuttonIndex.WtkQueryNumSpinbuttonIndex:elink.
for an easy conversion from a numeric value of a such initialized numeric spinbutton
to an array index. You will need this one to set the spinbutton to a desired value.

@@
*/

BOOL APIENTRY WtkInitializeNumSpinbuttonArray( HWND hwndSpinbutton, ULONG ulMinValue, ULONG ulMaxValue, ULONG ulStep)
{
         BOOL           fResult = FALSE;
         APIRET         rc = NO_ERROR;

         ULONG          ulValueCount;
         PSZ            apszValues;
         PSZ            pszThisValue;
         PSZ           *ppszValueEntry;
         ULONG          i;

do
   {
   // check parms
   if ((ulMinValue >= ulMaxValue) || (ulMinValue + ulStep >= ulMaxValue))
      {
      rc = PMERR_INVALID_PARAMETERS;
      break;
      }

   if (!WinIsWindow( WinQueryAnchorBlock( HWND_DESKTOP), hwndSpinbutton))
      {
      rc = PMERR_INVALID_HWND;
      break;
      }

   if (!WtkIsOfPublicPmClass( hwndSpinbutton, WC_SPINBUTTON, 0))
      {
      rc = PMERR_INVALID_PARAMETER_TYPE;
      break;
      }

   // get memory
   ulValueCount = ((ulMaxValue - ulMinValue) / ulStep) + 1;
   apszValues = malloc( ulValueCount * (sizeof( PSZ) + MAXBUFFER_ULONG));
   if (!apszValues)
      {
      rc = PMERR_INSUFFICIENT_MEMORY;
      break;
      }

   // create value array in memory
   for (i = 0,
        ppszValueEntry = (PSZ*) apszValues,
        pszThisValue = apszValues + (sizeof( PSZ) * ulValueCount);
           i < ulValueCount;
              i++,
              ppszValueEntry++,
              pszThisValue += MAXBUFFER_ULONG)
      {
      _ltoa( (i * ulStep) + ulMinValue, pszThisValue, 10);
      *ppszValueEntry = pszThisValue;
      }

   // activate array
   fResult = (BOOL) WinSendMsg( hwndSpinbutton, SPBM_SETARRAY, (MPARAM) apszValues, MPFROMLONG( ulValueCount));
   if (!fResult)
      {
      rc = PMHERR_USE_EXISTING_ERRORINFO;
      break;
      }

   } while (FALSE);

// cleanup
if (apszValues) free ( apszValues);
return WtkSetErrorInfo( rc);
}

// ---------------------------------------------------------------------------

/*
@@WtkQueryNumSpinbuttonIndex@SYNTAX
This function calculates the array index for a given value
for a numeric spinbutton  being initialized with
WtkInitializeNumSpinbuttonArray.

@@WtkQueryNumSpinbuttonIndex@PARM@hwndSpinbutton@in
Handle of the spinbutton window.

@@WtkQueryNumSpinbuttonIndex@PARM@ulMinValue@in
The minimum value of the spinbutton.
:p.
It is required, that the same minimum value is specified
here, as it was when initializing the numeric spinbutton with
:link reftype=hd refid=WtkInitializeNumSpinbuttonArray.WtkInitializeNumSpinbuttonArray:elink.,
otherwise the array value will be incorrect.

@@WtkQueryNumSpinbuttonIndex@PARM@ulMaxValue@in
The maximum value of the spinbutton.
:p.
It is required, that the same maximum value is specified
here, as it was when initializing the numeric spinbutton with
:link reftype=hd refid=WtkInitializeNumSpinbuttonArray.WtkInitializeNumSpinbuttonArray:elink.,
otherwise the array value will be incorrect.

@@WtkQueryNumSpinbuttonIndex@PARM@ulStep@in
The step between the values in the range.
:p.
It is required, that the same step value is specified
here, as it was when initializing the numeric spinbutton with
:link reftype=hd refid=WtkInitializeNumSpinbuttonArray.WtkInitializeNumSpinbuttonArray:elink.,
otherwise the array value will be incorrect.

@@WtkQueryNumSpinbuttonIndex@PARM@ulValue@in
The value, for which the index shall be calculated.
:p.
It is required, that the same maximum value is specified
here, as it was when initializing the numeric spinbutton with
:link reftype=hd refid=WtkInitializeNumSpinbuttonArray.WtkInitializeNumSpinbuttonArray:elink.,
otherwise the array value will be incorrect.


@@WtkQueryNumSpinbuttonIndex@RETURN
Array Index.
:parml compact.
:pt.-1
:pd.Array Index could not be calculated or the window is not an array spinbutton control.
:pt.Other
:pd.Array index corresponding to the given value.
:eparml.
:p.
Extended error information can be obtained using :hp2.WinGetLastError:ehp2./:hp2.WinGetErrorInfo:ehp2..

@@WtkQueryNumSpinbuttonIndex@REMARKS
This function checks the given window and calculates the index with the
following formula&colon.
:xmp.
lResult = (ulValue  - ulMinValue ) / ulStep;
:exmp.
:p.
If calculation of the index is performed frequently (whic is mostly not the case),
you might want not to use WtkQueryNumSpinbuttonIndex because of performance reasons,
but skip the validation and calculate the index yourself.

@@
*/

LONG APIENTRY WtkQueryNumSpinbuttonIndex( HWND hwndSpinbutton, ULONG ulMinValue, ULONG ulMaxValue, ULONG ulStep, ULONG ulValue)

{
         LONG           lResult = -1;
         APIRET         rc = NO_ERROR;

         ULONG          ulDummy;

do
   {
   // check parms

   if ((ulMinValue >= ulMaxValue) || (ulMinValue + ulStep >= ulMaxValue))
      {
      rc = PMERR_INVALID_PARAMETERS;
      break;
      }

   if (!WinIsWindow( WinQueryAnchorBlock( HWND_DESKTOP), hwndSpinbutton))
      {
      rc = PMERR_INVALID_HWND;
      break;
      }

   if (!WtkIsOfPublicPmClass( hwndSpinbutton, WC_SPINBUTTON, 0))
      {
      rc = PMERR_INVALID_PARAMETER_TYPE;
      break;
      }

   // is it a numeric spinbutton ?
   if (WinSendMsg( hwndSpinbutton, SPBM_QUERYLIMITS, MPFROMP( &ulDummy), MPFROMP( &ulDummy)))
      {
      rc = PMERR_INVALID_PARAMETER_TYPE;
      break;
      }

   // determine the index for the value
   lResult = (ulValue  - ulMinValue ) / ulStep;

   } while (FALSE);

WtkSetErrorInfo( rc);
return lResult;
}

// ---------------------------------------------------------------------------

/*
@@WtkAddTextResourceToMLE@SYNTAX
This function appends the contents of a text resource to a multiline entryfield (MLE).

@@WtkAddTextResourceToMLE@PARM@hwnd@in
Handle of the dialog window containing the multiline entryfield (MLE).

@@WtkAddTextResourceToMLE@PARM@ulControlId@in
The id of the multiline entryfield control.

@@WtkAddTextResourceToMLE@PARM@hmod@in
The handle to the resource DLL holding the text resource to be added.

@@WtkAddTextResourceToMLE@PARM@ulResId@in
The id of the text resource to be added.

@@WtkAddTextResourceToMLE@PARM@ulResourceType@in
The type of the text resource to be added.
:p.
Several
:link reftype=hd refid=RM_WtkAddTextResourceToMLE.resource type identifiers:elink.
can be used here.

@@WtkAddTextResourceToMLE@RETURN
Success indicator.
:parml compact.
:pt.TRUE
:pd.Text could be added
:pt.FALSE
:pd.An error occurred.
:eparml.

@@WtkAddTextResourceToMLE@REMARKS
In general, all resource type identifiers can be used that
would identify resources holding text. such as RT_STRING and
RT_MESSAGE. If larger text is to be used within resources,
it is recommended to include files with a user defined
resource type identifier. While strings and messages are
defined with STRINGTABLE and MESSAGETABLE definitions,
the data for user defined resource types are included in
a resource file as in the following example:
:xmp.

// define own  resource type
#define RT_USER_DATAFILE  1234

// define resource ID
#define IDRES_TEXTFILE  1000

// include a file
RESOURCE RT_USER_DATAFILE IDRES_TEXTFILE d&colon.\myproject\sample.txt
:exmp.
:p.
Using this resource definition, the text of this file would be added to an MLE
with the following call:
:xmp.
WtkAddTextResourceToMLE( hwnd, IDRES_MLE, hmod, RT_USER_DATAFILE, IDRES_TEXTFILE);
:exmp.

@@
*/

BOOL WtkAddTextResourceToMLE( HWND hwnd, ULONG ulControlId, HMODULE hmod, ULONG ulResourceType, ULONG ulResId)
{

         BOOL           fResult = FALSE;
         APIRET         rc = NO_ERROR;

         PVOID          pvFileContents  = NULL;
         PSZ            pszFileContents = NULL;
         ULONG          ulResourceSize;
         PSZ            pszEof;

do
   {
   // copy read-only resource to own memory to
   // - append zero byte
   // - discard EOF character
   rc = DosGetResource( hmod, ulResourceType, ulResId, &pvFileContents);
   if (rc != NO_ERROR)
      break;
   DosQueryResourceSize( hmod, ulResourceType, ulResId, &ulResourceSize);
   pszFileContents = malloc( ulResourceSize + 1);
   if (!pszFileContents)
      break;
   memcpy( pszFileContents, pvFileContents, ulResourceSize);
   pszEof = pszFileContents + ulResourceSize;
   *pszEof = 0;
   pszEof--;
   if (*pszEof == 0x1a)
      *pszEof = 0;

   // add contents to MLE
   WinSendDlgItemMsg( hwnd, ulControlId, MLM_INSERT, MPFROMP( pszFileContents), 0);

   // go to top of MLE
   WinSendDlgItemMsg( hwnd, ulControlId, MLM_SETSEL, 0, 0);

   fResult = TRUE;

   } while (FALSE);

if (pvFileContents) DosFreeResource( pvFileContents);
if (pszFileContents) free( pszFileContents);
return fResult;
}

// ---------------------------------------------------------------------------

/*
@@WtkFilterEntryField@SYNTAX
This function subclasses an entry field to filter out
unwanted characters and/or restrict it to wanted charaters only.

@@WtkFilterEntryField@PARM@hwndEntryField@in
Handle of the entryfield window to be subclassed and filtered.

@@WtkFilterEntryField@PARM@pszValidChars@in
Address of the ASCIIZ string specifying valid characters or NULL.
:p.
Either or both pszValidChars or/and pszInvalidChars must be specified, 
otherwise he function returns with an error.

@@WtkFilterEntryField@PARM@pszInvalidChars@in
Address of the ASCIIZ string specifying invalid characters or NULL.
:p.
Either or both pszInvalidChars or/and pszValidChars must be specified, 
otherwise the function returns with an error.

@@WtkFilterEntryField@RETURN
Success indicator.
:parml compact.
:pt.TRUE
:pd.Entryfield window could be subclassed
:pt.FALSE
:pd.
:eparml.

@@WtkFilterEntryField@REMARKS
QWL_USER of the subclassed entryfield window is being used for 
access to internal data and may not be modified!

@@
*/

typedef struct _ENTRYSCDATA {
  PSZ            pszValidChars;
  PSZ            pszInvalidChars;
  PFNWP          pfnwpOrg;
} ENTRYSCDATA, *PENTRYSCDATA;


static MRESULT EXPENTRY _entryFieldSubclassProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{

static   PFNWP          pfnwpOrgWindowProc = NULL;
         PENTRYSCDATA   pescd = (PENTRYSCDATA)WinQueryWindowULong( hwnd, QWL_USER);


if (!pfnwpOrgWindowProc)
   pfnwpOrgWindowProc = (pescd) ? pescd->pfnwpOrg : WtkGetDefaultWindowProcPtr( hwnd);

if (pescd)
   {
   switch (msg)
      {

      case WM_CHAR:
         {
                  BOOL           fSkipKey = FALSE;
                  USHORT         fs = CHARMSG(&msg)->fs;
                  CHAR           ch = CHARMSG(&msg)->chr;
                  USHORT         vbits = fs & (KC_ALT | KC_CTRL | KC_VIRTUALKEY);
                  BOOL           fKeyIsUp = (fs & KC_KEYUP > 0);

         if (((ch == 32) && (vbits == KC_VIRTUALKEY)) ||  // check all blanks and
             ((ch >  32) && (vbits == 0)))                // all non control chars
            {
            // compare case insensitive
            ch = toupper( ch);

            // check for valid chars
            if (pescd->pszValidChars)
               fSkipKey = (strchr( pescd->pszValidChars, ch) == NULL);

            // check for invalid chars
            if ((!fSkipKey) && (pescd->pszInvalidChars))
               fSkipKey = (strchr( pescd->pszInvalidChars, ch) != NULL);

            if (fSkipKey)
               {
               // if not, make one sound ...
               if(!fKeyIsUp)
                  WinAlarm( HWND_DESKTOP, WA_ERROR);
               // ... and skip key
               return (MRESULT) FALSE;
               }

            } // if (((ch == 32) && (vbits == KC_VIRTUALKEY)) ...
         }
         break; // case WM_CHAR:

      // --------------------------------

      case WM_DESTROY:
         if (pescd)
            {
            WinSetWindowULong( hwnd, QWL_USER, 0);
            if (pescd->pszValidChars)   free( pescd->pszValidChars);
            if (pescd->pszInvalidChars) free( pescd->pszInvalidChars);
            free( pescd);
            }
         break;

      } // switch (msg)

   } // if (pescd)

return pfnwpOrgWindowProc( hwnd, msg, mp1, mp2);
}

// ---------------------

BOOL APIENTRY WtkFilterEntryField( HWND hwndEntryField, PSZ pszValidChars, PSZ pszInvalidChars)
{
         BOOL           fResult = FALSE;
         APIRET         rc = NO_ERROR;
         PENTRYSCDATA   pescd;

do
   {
   // check parms
   if ((!pszValidChars) &&
       (!pszInvalidChars))
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   // window must be an entry field
   if (!WtkIsOfPublicPmClass( hwndEntryField, WC_ENTRYFIELD, 0))
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   // prepare subclass data
   pescd = malloc( sizeof( ENTRYSCDATA));
   if (!pescd)
      {
      rc = ERROR_NOT_ENOUGH_MEMORY;
      break;
      }
   memset( pescd, 0, sizeof( ENTRYSCDATA));
   pescd->pfnwpOrg = WinSubclassWindow( hwndEntryField, _entryFieldSubclassProc);

   if (pszValidChars)
      {
      pescd->pszValidChars = strdup( pszValidChars);
      strupr( pescd->pszValidChars);
      }
   if (pszInvalidChars)
      {
      pescd->pszInvalidChars = strdup( pszInvalidChars);
      strupr( pescd->pszInvalidChars);
      }

   // make data known to entry field
   WinSetWindowULong( hwndEntryField, QWL_USER, (ULONG)pescd);

   } while (FALSE);

// set info and exit
WtkSetErrorInfo( rc);
return (rc == NO_ERROR);
}


