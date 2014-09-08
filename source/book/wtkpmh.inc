..******************************* Module Header *******************************
..*
..* Module Name: wtkpmh.im
..*
..* Hypertext/2 include file for programmers guide section on
..* PM helper function related functions
..*
..* Copyright (c) WPS Toolkit Project - Christian Langanke 2000
..*
..* $Id: wtkpmh.inc,v 1.2 2008-08-19 18:24:01 cla Exp $
..*
..* ===========================================================================
..*
..* This file is part of the WPS Toolkit package and is free software.  You can
..* redistribute it and/or modify it under the terms of the GNU Library General
..* Public License as published by the Free Software Foundation, in version 2
..* as it comes in the "COPYING.LIB" file of the WPS Toolkit main distribution.
..* This library is distributed in the hope that it will be useful, but WITHOUT
..* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
..* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
..* License for more details.
..*
..*****************************************************************************

.. ----------------------------------------
.1 PM helper functions
.. ----------------------------------------
.an IDPNL_PMH
.
[=TOPICS]
.su V30

.. ----------------------------------------
.2 Notebook Dialog related functions
.. ----------------------------------------
.an IDPNL_PMH_NBKDLG
.
The API [.WtkNotebookDlg WtkNotebookDlg] is intended for to create 
a notebook dialog with its pages from dialog resource templates.

_*Callback handling of notebook pages*_
.
By setting the appropriate fields of the structure [.NOTEBOOKINFO NOTEBOOKINFO]
for the notbook conrol and the structure [.NOTEBOOKPAGEINFO NOTEBOOKPAGEINFO]
for each notebook page you can let it automatically:
.ul compact
- define callback functions for the following events
  - initialization of the GUI controls (WM__INITDLG)
  - control and certain command events (WM__CONTROL/WM__COMMAND)
  - save status of controls (WM__QUIT)
  - restore status of controls (WM__COMMAND for undo button)
  - set controls to default (WM__COMMAND for default button)
- create and destroy a help instance for the notebook control 
  and assign help resource identifiers to the notebook pages
- undo all pages when the undo button is pressed on one page
- set all pages to default values when the default button is pressed on one page
- set the focus to a given control per notebook page

_*Enumerating Notebook Buttons*_
.
The APIs [.WtkEnableNotebookButton WtkEnableNotebookButton] and
[.WtkQueryNotebookButton WtkQueryNotebookButton] allow to easily
query and enable notebook buttons. This is particularly interesting
because after WM__INITDLG buttons with the BS__NOTEBOOKBUTTON button
style set are not longer a child of the window page, but of the 
notebook child window instead. Moreover, if several pages create a
pushbutton control with the same resource identifier, they cannot 
be anylonger be accessed directly by its resource identifier
(e.g. with WinWindowFromID). Instead, the notebook buttons must
be enumerated, which is done by *WtkQueryNotebookButton*.

_*Extensions to the graphical user interface*_
.
The notebook control created by [.WtkNotebookDlg WtkNotebookDlg]
works differently compared to a standard notebook control in the
following concerns:
.ul
- when clicking repeatedly on a major tab, the notebook cycles through
  all pages of that major tab, including the major and all minor tab pages.
  Normally, clicks on the major tab only bring up the major tab page

.. ----------------------------------------
.2 Help Manager related functions
.. ----------------------------------------
.an IDPNL_PMH_HLPMAN
.
The following APIs help you with easy creation of a help instance and
bringing up help panels during a WM__HELP message without setting up help tables:
.pl tsize=1
- [.WtkCreateHelpInstance WtkCreateHelpInstance]
= creates a help instance and associates it to the given window. At least one window
  in the window chain must be a frame window for this, and no help instance may already
  be associated to that frame window.
..
- [.WtkDestroyHelpInstance WtkDestroyHelpInstance]
= destroys the help instance associated with the specified window.
..
- [.WtkDisplayHelpPanel WtkDisplayHelpPanel]
= displays a help panel with the specified resource identifier from the
  help instance associated with the specified window

.. ----------------------------------------
.2 other PM helper functions
.. ----------------------------------------
.an IDPNL_PMH_DIVERSE
.
The following APIs help you with certain tasks in PM programming:
.pl tsize=1
- [.WtkCenterWindow WtkCenterWindow]
= centers a window within the owner window, either horizontally or vertically or both
- [.WtkInsertMenu WtkInsertMenu]
=  inserts a new menu, loaded from a PM resource, into an existing menu.   
- [.WtkIsFontAvailable WtkIsFontAvailable]
= checks, if a given font is available.
- [.WtkSetWindowFont WtkSetWindowFont]
= explicitely sets the specified font for a given window as presentation
  parameter and optionally for all of its child windows.

