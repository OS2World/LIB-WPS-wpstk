/****************************** Module Header ******************************\
*
* Module Name: wtkset.h
*
* Include file for settings and details manager functions.
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2000
*
* $Id: wtkset.h,v 1.14 2005-08-17 21:25:55 cla Exp $
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

#ifndef WTKSETTINGS_INCLUDED
#define WTKSETTINGS_INCLUDED Settings and details manager functions

#ifdef __cplusplus
      extern "C" {
#endif

#pragma pack(1)

#define _STM_MAX_SETTINGSVALUE  64    /* only be used for string translation for indexed */
                                      /* values by callback STM_CALLBACK_QUERYSTRING     */

/*** handle types used by Setting manager **********************************/
typedef LHANDLE HSETTINGTABLE;
typedef HSETTINGTABLE *PHSETTINGTABLE;

typedef LHANDLE HVALUETABLE;
typedef HVALUETABLE *PHVALUETABLE;

/*** callback procedure prototype ******************************************/
typedef BOOL (FNCB)(ULONG ulAction, PVOID pvData, PVOID pvObjectInstance, PVOID pvObjectData);
typedef FNCB *PFNCB;

/*** callback actions ******************************************************/
#define STM_CALLBACK_REPORTINIT        0x0000  /* reports that class is being initialized         */
#define STM_CALLBACK_QUERYVALUEINFO    0x0001  /* queries infos about (sub)value                  */
#define STM_CALLBACK_QUERYTARGETBUF    0x0002  /* queries target buffer when creating value table */
#define STM_CALLBACK_VALIDATE          0x0003  /* ask WPS class to validate a (sub)value          */
#define STM_CALLBACK_REPORTCHANGED     0x0004  /* reports the change of a setting                 */
#define STM_CALLBACK_QUERYVALUE        0x0005  /* asks for update of target buffers               */
#define STM_CALLBACK_QUERYINDEX        0x0006  /* translates settings strings to array indicees   */
#define STM_CALLBACK_QUERYSTRING       0x0007  /* translates array indicees to settings strings   */
#define STM_CALLBACK_INITCONTROL       0x0008  /* asks for initialization of GUI controls,        */
                                               /* useful for listboxes comboboxes etc.            */
#define STM_CALLBACK_REPORTERROR       0x0009  /* reports errors from WtkValidateObjectValueTable */
#define STM_CALLBACK_REPORTSAVED       0x000A  /* reports that settings have been saved           */
#define STM_CALLBACK_REPORTDESTROYED   0x000B  /* reports that class settings table is destroyed  */
#define STM_CALLBACK_QUERYDETAILINFO   0x000C  /* query info about index strings (maxlen)         */
#define STM_CALLBACK_QUERYDETAILSTRING 0x000D  /* translates array indicees to detail strings     */

/*** data structure for callback STM_CALLBACK_REPORTINIT *************/
/*** Note: on this callback, pvObjectInstance and pvObjectData are NULL ****/
typedef struct _CBREPORTINIT {
  BOOL           fInitialized;              /*  no data really required here yet       */
} CBREPORTINIT, *PCBREPORTINIT;

/*** data structures for callback STM_CALLBACK_QUERYVALUEINFO ********/
/*** Note: on this callback, pvObjectInstance and pvObjectData are NULL ****/
typedef struct _CBQUERYVALUEINFO {
  PSZ            pszName;                   /* out - name of setting                   */
  ULONG          ulSettingId;               /* out - id of setting                     */
  ULONG          ulValueIndex;              /* out - index of subvalue                 */
  ULONG          ulValueType;               /* #in - type of value - see VALUETYPE_*   */
  USHORT         usDialogid;                /*  in - id of dialog containing control   */
  USHORT         usControlid;               /*  in - id of control                     */
  PFNWP          pfnwpSubclass;             /*  in - subclass window proc for control  */
  PSZ            pszDetailsTitle;           /*  in - title of folderdetails view       */
} CBQUERYVALUEINFO, *PCBQUERYVALUEINFO;


/*** data structures for callback CALLBACK_QUERYTARGETBUF ************/
/* values marked with # MUST be specified, all others are optional */
typedef struct _CBQUERYTARGETBUF {
  PVOID          pvObjectInstance;          /* out - somSelf of object instance        */
  PSZ            pszName;                   /* out - name of setting                   */
  ULONG          ulSettingId;               /* out - id of setting                     */
  ULONG          ulValueIndex;              /* out - index of subvalue                 */
                                            /*    (only > 0 for multivalue settings)   */
  ULONG          ulBufMax;                  /* #in - len of target buffer              */
  PVOID          pvTarget;                  /* #in - target buffer                     */
} CBQUERYTARGETBUF, *PCBQUERYTARGETBUF;     /*  return TRUE to use the returned data   */

/* supported GUI controls:                                                             */
/* - WC_ENTRYFIELD WC_MLE WC_BUTTON (checkbox and radio button)                        */
/* - WC_LISTBOX WC_COMBOBOX WC_SPINBUTTON                                              */
/* - WC_SLIDER WC_CIRCULARSLIDER                                                       */

/* types of possible values                                                            */
/*                                                                                     */
/* type          validation target buffer possible GUI update                          */
/* ------------- ---------- ------------- -------------------------------              */
/* STRING        NO         CHAR[]        <all supported> except WC_*SLIDER            */
/*                                        - WC_COMBOBOX, WC_SPINBUTTON as listbox      */
/* INDEX         YES        LONG          <all supported>                              */
/* ITEM          YES        LONG          selects item with WC_LISTBOX, WC_COMBOBOX,   */
/*                                        behaves like LONG otherwise                  */
/* LONG          NO         LONG          <all supported>                              */
/* TRUEFALSE     YES        BOOL          <all supported> except WC_*SLIDER            */
/* YESNO         YES        BOOL          <all supported> except WC_*SLIDER            */
/* ONOFF         YES        BOOL          <all supported> except WC_*SLIDER            */


/*** value types for value target buffers **********************************/
#define STM_VALUETYPE_STRING    0
#define STM_VALUETYPE_INDEX     1
#define STM_VALUETYPE_LONG      2
#define STM_VALUETYPE_TRUEFALSE 3
#define STM_VALUETYPE_YESNO     4
#define STM_VALUETYPE_ONOFF     5
#define STM_VALUETYPE_CDATE     6    /* for details only ! */
#define STM_VALUETYPE_CTIME     7    /* for details only ! */
#define STM_VALUETYPE_INDEXITEM 8

/*** data structures for callback CALLBACK_VALIDATION ****************/
typedef struct _CBVALIDATE {
  PVOID          pvObjectInstance;          /* out - somSelf of object instance        */
  ULONG          ulSettingId;               /* out - id of setting                     */
  PSZ            pszName;                   /* out - name of setting                   */
  PSZ            pszValue;                  /* out - name of value to be validated     */
  ULONG          ulValueIndex;              /* out - index of subvalue                 */
  BOOL           fResult;                   /*  in - result of validation              */
} CBVALIDATE, *PCBVALIDATE;                 /* return TRUE if your callback validates, */
                                            /* FALSE for using standard validation     */

/*** data structures for callback CALLBACK_REPORTCHANGED ****************/
typedef struct _CBREPORTCHANGED {
  PVOID          pvObjectInstance;          /* out - somSelf of object instance        */
  ULONG          ulSettingId;               /* out - id of setting                     */
  PSZ            pszName;                   /* out - name of setting                   */
} CBREPORTCHANGED, *PCBREPORTCHANGED;

/*** data structures for callback CALLBACK_QUERYVALUE ****************/
typedef struct _CBQUERYVALUE {
  PVOID          pvObjectInstance;          /* out - somSelf of object instance        */
  ULONG          ulSettingId;               /* out - id of setting                     */
  PSZ            pszName;                   /* out - name of setting                   */
  ULONG          ulQueryIndex;              /* out - index of query                    */
} CBQUERYVALUE, *PCBQUERYVALUE;

/*** data structures for callback CALLBACK_QUERYINDEX ****************/
typedef struct _CBQUERYINDEX {
  PVOID          pvObjectInstance;          /* out - somSelf of object instance        */
  ULONG          ulSettingId;               /* out - id of setting                     */
  PSZ            pszName;                   /* out - name of setting                   */
  PSZ            pszValue;                  /* out - string value to be translated     */
  ULONG          ulValueIndex;              /* out - index of subvalue                 */
  ULONG          ulStringIndex;             /*  in - index to be used                  */
} CBQUERYINDEX, *PCBQUERYINDEX;             /* return TRUE if processed, else FALSE    */

/*** data structures for callback CALLBACK_QUERYSTRING ***************/
typedef struct _CBQUERYSTRING {
  PVOID          pvObjectInstance;          /* out - somSelf of object instance        */
  ULONG          ulSettingId;               /* out - id of setting                     */
  PSZ            pszName;                   /* out - name of setting                   */
  ULONG          ulStringIndex;             /* out - index to be translated            */
  ULONG          ulValueIndex;              /* out - index of subvalue                 */
  CHAR           szValue[ _STM_MAX_SETTINGSVALUE];/* in - string value to be used      */
} CBQUERYSTRING, *PCBQUERYSTRING;           /* return TRUE if processed, else FALSE    */

/*** data structures for callback CALLBACK_INITCONTROL ***************/
typedef struct _CBINITCONTROL {
  USHORT         usDialogid;                /* out - id of dialog                      */
  USHORT         usControlid;               /* out - id of control                     */
  HWND           hwndDialog;                /* out - handle of dialog                  */
  HWND           hwndControl;               /* out - handle of control                 */
} CBINITCONTROL, *PCBINITCONTROL;

/*** data structures for callback STM_CALLBACK_REPORTERROR ***********/
typedef struct _CBREPORTERROR {
  PVOID          pvObjectInstance;          /* out - somSelf of object instance        */
  ULONG          ulSettingId;               /* out - id of setting                     */
  PSZ            pszName;                   /* out - name of setting                   */
  USHORT         usDialogid;                /* out - id of dialog                      */
  USHORT         usControlid;               /* out - id of control                     */
  HWND           hwndDialog;                /* out - handle of dialog                  */
  HWND           hwndControl;               /* out - handle of control                 */
} CBREPORTERROR, *PCBREPORTERROR;           /* return TRUE to ignore error             */

/*** data structures for callback STM_CALLBACK_REPORTSAVED ***********/
typedef struct _CBREPORTSAVED {
  BOOL           fSaved;                    /*  no data really required here yet       */
} CBREPORTSAVED, *PCBREPORTSAVED;

/*** data structures for callback STM_CALLBACK_DESTROYED *************/
/*** Note: on this callback, pvObjectInstance and pvObjectData are NULL ***/
typedef struct _CBREPORTDESTROYED {
  BOOL           fDestroyed;                /*  no data really required here yet       */
} CBREPORTDESTROYED, *PCBREPORTDESTROYED;

/*** data structures for callback STM_CALLBACK_QUERYDETAILINFO *******/
/*** Note: on this callback, pvObjectInstance and pvObjectData are NULL ****/
typedef struct _CBQUERYDETAILINFO {
  PSZ            pszName;                   /* out - name of setting                   */
  ULONG          ulSettingId;               /* out - id of setting                     */
  ULONG          ulValueIndex;              /* out - index of subvalue                 */
  ULONG          ulDispStringMaxlen;        /*  in - maxlen of string inc. zero byte   */
} CBQUERYDETAILINFO, *PCBQUERYDETAILINFO;

/*** data structures for callback CALLBACK_QUERYDETAILSSTRING ********/
typedef struct _CBQUERYDETAILSTRING {
  PVOID          pvObjectInstance;          /* out - somSelf of object instance        */
  ULONG          ulSettingId;               /* out - id of setting                     */
  PSZ            pszName;                   /* out - name of setting                   */
  ULONG          ulStringIndex;             /* out - index to be translated            */
  ULONG          ulValueIndex;              /* out - index of subvalue                 */
  PSZ            pszValue;                  /* in - display string value to be used    */
} CBQUERYDETAILSTRING, *PCBQUERYDETAILSTRING;   /* return TRUE if processed, else FALSE    */


// -----------------------------------------
// prototypes - Settings and Details Manager
// -----------------------------------------

/*** prototypes for (de)initializing the settings table for the metaclass **/
HSETTINGTABLE APIENTRY WtkCreateClassSettingsTable( PVOID pvObjectClass, PFNCB pfnCallbackValue);
BOOL APIENTRY WtkDestroyClassSettingsTable( HSETTINGTABLE hst);
BOOL APIENTRY WtkAddClassSetting( HSETTINGTABLE hst, ULONG ulSettingId, PSZ pszSetting, ULONG ulQueryCount);
BOOL APIENTRY WtkAddClassDetail( HSETTINGTABLE hst, ULONG ulSettingId);
BOOL APIENTRY WtkCloseClassSettingsTable( HSETTINGTABLE hst);
BOOL APIENTRY WtkDumpClassSettingsTable( HSETTINGTABLE hst);  // for testing purposes only, dumps to console
BOOL APIENTRY WtkQueryObjectClass( HSETTINGTABLE hst, PVOID *ppvObjectClass);

/*** prototypes for maintaining setting values for object instances ********/
HVALUETABLE APIENTRY WtkCreateObjectValueTable( HSETTINGTABLE hst, PVOID pvObjectInstance, PVOID pvObjectData);
BOOL APIENTRY WtkDestroyObjectValueTable( HVALUETABLE hvt);
BOOL APIENTRY WtkEvaluateObjectSettings( HVALUETABLE hvt, PSZ pszSetup);
BOOL APIENTRY WtkQueryObjectSettings( HVALUETABLE hvt, PSZ pszBuffer, PULONG pulMaxlen);
BOOL APIENTRY WtkQueryObjectInstance( HVALUETABLE hvt, PVOID *ppvObjectInstance, PVOID *ppvObjectData);
BOOL APIENTRY WtkQuerySettingstable( HVALUETABLE hvt, PHSETTINGTABLE phst);

/*** prototypes for providing automatic updates to GUI controls ************/
BOOL APIENTRY WtkRegisterSettingsDialog( HVALUETABLE hvt, HWND hwndDialog);
BOOL APIENTRY WtkDeregisterSettingsDialog( HVALUETABLE hvt, HWND hwndDialog);
BOOL APIENTRY WtkReadObjectValueTable( HVALUETABLE hvt, HWND hwndDialog);
BOOL APIENTRY WtkWriteObjectValueTable( HVALUETABLE hvt, HWND hwndDialog);
BOOL APIENTRY WtkQueryGUIControlsChanged( HVALUETABLE hvt, HWND hwnd, MPARAM mp1, MPARAM mp2, PBOOL pfOrgValue);

/*** prototype for using same dialog templates under WARP 3 and WARP 4 *****/
BOOL APIENTRY WtkRelocateNotebookpageControls( HWND hwndDialog);

/*** prototypes for providing automatic validation of controls   *************/
/*** on close of settings notebook.                              *************/
/*** NOTE: WtkValidateObjectValueTable needs not to be           *************/
/***       explicitely called for this, but is avaliable anyway. *************/
BOOL APIENTRY WtkRegisterSettingsNotebook( HVALUETABLE hvt, HWND hwndNotebook);
BOOL APIENTRY WtkValidateObjectValueTable( HVALUETABLE hvt, HWND hwndNotebook);

/*** prototypes for providing details data *********************************/
ULONG APIENTRY WtkQueryClassDetailsInfo( HSETTINGTABLE hst, PVOID* ppClassFieldInfo,
                                         PULONG pSize, ULONG ulParentColumns);
ULONG APIENTRY WtkQueryObjectDetailsData( HVALUETABLE hvt, PVOID* ppDetailsData, PULONG pcp);

/*** prototypes for saving/restoring data in WPS repository ****************/
BOOL APIENTRY WtkSaveObjectState( HVALUETABLE hvt, PSZ pszClass);
BOOL APIENTRY WtkRestoreObjectState( HVALUETABLE hvt, PSZ pszClass);

/*** prototypes for saving/restoring data in extern ini file ***************/
/*** NOTE: you have to make sure yourself that you save to   ***************/
/***       a unique place per object instance !!!            ***************/
/*** NOTE: specify USER or SYSTEM as filename to write to    ***************/
/***       HINI_USERPROFILE or HINI_SYSTEMPROFILE            ***************/
BOOL APIENTRY WtkSaveObjectSettings( HVALUETABLE hvt, PSZ pszFilename, PSZ pszApp, PSZ pszKey);
BOOL APIENTRY WtkRestoreObjectSettings( HVALUETABLE hvt, PSZ pszFilename, PSZ pszApp, PSZ pszKey);

/*** prototypes for functions handling other methods ***********************/
ULONG APIENTRY WtkQueryIconData( PICONINFO pIconInfo, HMODULE hmodResource, ULONG ulResId);


// --------------------------------------
// prototypes - Settings string functions
// --------------------------------------

APIRET APIENTRY WtkScanSetupString( PSZ pszSetup, PSZ pszName, PSZ pszBuffer, ULONG ulBuflen);
APIRET APIENTRY WtkExtractSetupString( PSZ pszSetup, PSZ pszName, PSZ pszBuffer, ULONG ulBuflen);
APIRET APIENTRY WtkSplitSetupString( PSZ* ppszSetup, PSZ pszNameBuffer, ULONG ulNameBufLen,
                                     PSZ pszValueBuffer, ULONG ulValueBufLen);

#pragma pack()

#ifdef __cplusplus
        }
#endif

#endif /* WTKSETTINGS_INCLUDED */

