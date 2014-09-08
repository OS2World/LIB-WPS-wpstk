/****************************** Module Header ******************************\
*
* Module Name: wtkupm.h
*
* include file for PM helper functions
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2000
*
* $Id: wtkupm.h,v 1.23 2006-08-13 13:57:00 cla Exp $
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

#ifndef WTKUTLPM_INCLUDED
#define WTKUTLPM_INCLUDED PM helper functions

#ifdef __cplusplus
      extern "C" {
#endif

// make sure to include pmhelp.h only if
// os2.h of emx has not been included
#ifndef _OS2EMX_H
#include <pmhelp.h>
#endif

#pragma pack(1)

#define WTK_CENTERWINDOW_BOTH          0x0000
#define WTK_CENTERWINDOW_HORIZONTAL    0x0001
#define WTK_CENTERWINDOW_VERTICAL      0x0002
BOOL APIENTRY WtkCenterWindow( HWND hwnd, HWND hwndCenter, ULONG ulFlags);

BOOL APIENTRY WtkIsFontAvailable( PSZ pszFontname);

BOOL APIENTRY WtkSetWindowFont( HWND hwnd, PSZ pszFontname, BOOL fIncludeChildren);

#define WTK_INSERTMENU_TOPSEPARATOR    0x0001
#define WTK_INSERTMENU_BOTTOMSEPARATOR 0x0002
BOOL APIENTRY WtkInsertMenu( HWND hwnd, HWND hwndMenu, ULONG ulInsertAfter,
                             HMODULE hmodResource, ULONG ulMenuId, ULONG ulOptions);

ULONG APIENTRY WtkGetStringDrawLen( HPS hps, PSZ pszString);

BOOL APIENTRY WtkCreateHelpInstance( HWND hwnd, PSZ pszWindowTitle, PSZ pszHelpLibrary);
BOOL APIENTRY WtkCreateHelpInstanceWithTable( HWND hwnd, PSZ pszWindowTitle, PSZ pszHelpLibrary,
                                              HMODULE hmodResource, PHELPTABLE pht);

BOOL APIENTRY WtkDisplayHelpPanel( HWND hwnd, ULONG ulHelpPanel);

BOOL APIENTRY WtkDestroyHelpInstance( HWND hwnd);


/* structures and prototypes for WtkNotebookDlg callback functions */
typedef VOID (FNWINCMD)( HWND hwnd, PVOID pvData);
typedef FNWINCMD *PFNWINCMD;
typedef VOID (FNWINCNT)( HWND hwnd, PVOID pvData, MPARAM mp1, MPARAM  mp2);
typedef FNWINCNT *PFNWINCNT;

typedef struct _NOTEBOOKPAGEINFO {
  ULONG          cbFix;             /* length of structure                                     */
  ULONG          ulDlgResId;        /* resource identifier of notebook page dialog             */
  USHORT         usPageStyle;       /* page style like BKA_MAJOR | BKA_AUTOPAGESIZE            */
  USHORT         usPageOrder;       /* page order like BKA_MAJOR or BKA_MINOR                  */
  PSZ            pszTabText;        /* text for major tab or NULL for using the window title   */
  PSZ            pszMinorTabText;   /* text for minor tab or NULL                              */
  PSZ            pszStatusText;     /* text of static status text or NULL                      */
  ULONG          ulHelpPanel;       /* help panel id for the notebook page                     */
  ULONG          ulFocusId;         /* resource identifier of the control to get focus or zero */
  ULONG          ulUndoButtonId;    /* resource identifier of the undo button                  */
  ULONG          ulDefaultButtonId; /* resource identifier of the default button               */
  ULONG          ulOkButtonId;      /* resource identifier of the Ok button                    */
  ULONG          ulCancelButtonId;  /* resource identifier of the Cancel button                */
  ULONG          ulHelpButtonId;    /* resource identifier of the help button                  */
} NOTEBOOKPAGEINFO, *PNOTEBOOKPAGEINFO;

typedef struct _NOTEBOOKINFO {
  ULONG          cbFix;             /* length of structure                                      */
  HWND           hwndParent;        /* handle of the parent window                              */
  HWND           hwndOwner;         /* handle of the owner window                               */
  ULONG          ulDlgResId;        /* resource identifier of notebook frame dialog             */
  ULONG          ulNotebookResId;   /* resource identifier of the notebook control              */
  HMODULE        hmodResource;      /* resource module handle or NULL                           */
  ULONG          ulSysIconId;       /* resource identifier of the system icon                   */
  PSZ            pszHelpTitle;      /* title of the help window                                 */
  PVOID          pvData;            /* address of user data to be passed to callbacks           */
  PSZ            pszHelpLibrary;    /* address of pathname of the help library                  */
  PSZ            pszStatusMask;     /* NLS sprintf string for automatic page numbering or NULL */
  BOOL           fUndoAllPages;     /* flag to undo all pages when undo button is pressed       */
  BOOL           fDefaultAllPages;  /* flag to default all pages when default button is pressed */
  ULONG          ulNbPageCount;     /* number of pages                                          */
  PNOTEBOOKPAGEINFO panpi;          /* address to list of notebook page infos                   */
  PFNWINCMD      pfnwcmdInitialize; /* function called on WM_INITDLG or NULL                    */
  PFNWINCNT      pfnwcntControl;    /* function called on WM_CONTROL/WM_COMMAND or NULL         */
  PFNWINCMD      pfnwcmdSave;       /* function called on WM_QUIT or NULL                       */
  PFNWINCMD      pfnwcmdRestore;    /* function called on WM_COMMAND for undo button or NULL    */
  PFNWINCMD      pfnwcmdDefault;    /* function called on WM_COMMAND for default button or NULL */
} NOTEBOOKINFO, *PNOTEBOOKINFO;

ULONG APIENTRY WtkNotebookDlg( PNOTEBOOKINFO pni);

HWND APIENTRY WtkQueryNotebookButton( HWND hwndPage, ULONG ulButtonId, HWND hwndBefore);

BOOL APIENTRY WtkEnableNotebookButton( HWND hwndPage, ULONG ulButtonId, BOOL fEnable);


/* data structure for WtkDirDlg */
typedef struct _DIRDLG {
  ULONG          cbSize;                  /*  Structure size. */
  ULONG          fl;                      /*  WTK_DDS_* flags. */
  ULONG          ulUser;                  /*  Used by the application. */
  LONG           lReturn;                 /*  Result code. */
  APIRET         rc;                      /*  OS/2 ERROR_* return code. */
  PSZ            pszTitle;                /*  Dialog title string. */
  PSZ            pszOKButton;             /*  OK push button text. */
  PFNWP          pfnDlgProc;              /*  Custom dialog procedure. */
  HMODULE        hMod;                    /*  Module for custom dialog resources. */
  CHAR           szFullDir[CCHMAXPATH];   /*  Character array. */
  USHORT         usDlgId;                 /*  Custom dialog ID. */
  SHORT          x;                       /*  X-axis dialog position. */
  SHORT          y;                       /*  Y-axis dialog position. */
} DIRDLG, *PDIRDLG;

#define WTK_DDS_CENTER            0x0001
#define WTK_DDS_CUSTOM            0x0002
#define WTK_DDS_HELPBUTTON        0x0004
#define WTK_DDS_PRELOAD_VOLINFO   0x0008
#define WTK_DDS_FORCEDIALOGID     0x1000
BOOL APIENTRY WtkDirDlg( HWND hwndParent, HWND hwndOwner, PDIRDLG pdd);
MRESULT EXPENTRY WtkDefDirDlgProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

#pragma pack()

#ifdef __cplusplus
        }
#endif

#endif /* WTKUTLPM_INCLUDED */

