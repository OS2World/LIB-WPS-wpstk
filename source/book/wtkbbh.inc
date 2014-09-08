..******************************* Module Header *******************************
..*
..* Module Name: wtkbbh.inc
..*
..* Hypertext/2 include file for programmers guide section on
..* Bubble Help Manager
..*
..* Copyright (c) WPS Toolkit Project - Christian Langanke 2000
..*
..* $Id: wtkbbh.inc,v 1.1 2004-08-17 17:45:12 cla Exp $
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
.1 Bubble Help Manager
.. ----------------------------------------
.an IDPNL_APIBBBH
.
[=TOPICS]
.su V30

.. ----------------------------------------
.2 Bubble Help Manager Overview
.. ----------------------------------------
.an IDPNL_APIBBBH_OVERVIEW
.
The *Bubble Help* API of the [=TITLE]
provides an API for easily implementing a bubble help function.

.. ----------------------------------------
.2 initialize Bubble Help support
.. ----------------------------------------
.an IDPNL_APIBBBH_INIT
.
When using the *Bubble Help* API, it must be
initialized by a call to [.WtkInitializeMmf WtkInitializeMmf],
this must be done for each window requesting bubble help support.

.. ----------------------------------------
.2 activating/deactivating bubble help
.. ----------------------------------------
.an IDPNL_APIBBBH_ACTIVATE
.
On certain events it is useful to deactivate the bubble help, i.e.
when a mouse button is pressed, while a context menu is active etc..
Afterwards the bubble help needs to be activated again.

For that the APIs [.WtkDeactivateBubbleHelp WtkDeactivateBubbleHelp]
and [.WtkActivateBubbleHelp WtkActivateBubbleHelp] are available.

.. ----------------------------------------
.2 updating the bubble help text
.. ----------------------------------------
.an IDPNL_APIBBBH_UPDATETEXT
.
The API [.WtkUpdateBubbleHelp WtkUpdateBubbleHelp] allows to update the
bubble help text, if necessary. It must at least be called once, as on 
initialization the default text is an empty string.

.. ----------------------------------------
.2 terminate Bubble Help Support
.. ----------------------------------------
.an IDPNL_APIBBBH_TERMINATE
.
When using the *Bubble Help* API, it must be
terminated by a call to [.WtkTerminateBubbleHelp WtkTerminateBubbleHelp]
in order to release all resources used by the Bubble Help Manager.

