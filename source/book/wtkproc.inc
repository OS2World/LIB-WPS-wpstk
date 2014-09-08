..******************************* Module Header *******************************
..*
..* Module Name: wtkproc.inc
..*
..* Hypertext/2 include file for programmers guide section on
..* Process related functions
..*
..* Copyright (c) WPS Toolkit Project - Christian Langanke 2000
..*
..* $Id: wtkproc.inc,v 1.4 2009-07-03 20:45:26 cla Exp $
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
.1 Process related functions
.. ----------------------------------------
.an IDPNL_APIPROC
.
[=TOPICS]
.su V30

.. ----------------------------------------
.2 Process related functions Overview
.. ----------------------------------------
.an IDPNL_APIPROC_OVERVIEW
.
The Process related functions of the [=TITLE] allow easy access to
all informations related to the process status of the system.

The functions take a snapshot of the process status tree in the background,
and detailed information can be retrieved from that snapshot.

At every time, the snapshot can be refreshed, returning a flag
indicating wether the process status has changed since the last snapshot.

As an exception to the above, the API
[.WtkQueryProcessDetails WtkQueryProcessDetails]
can also query process details without taking a snapshot before.

.. ----------------------------------------
.2 initialize process API
.. ----------------------------------------
.an IDPNL_APIPROC_INIT
.
Usage of the Process related functions must be
initialized by a call to [.WtkInitializeProcessStatus WtkInitializeProcessStatus].

*WtkInitializeProcessStatus* performs the following actions:
.ul compact
- allocates memory and takes a first
  [.IDPNL_APIPROC_SNAPSHOT snapshot of the process status]
- if desired, allocates [.IDPNL_APIPROC_PRIVATEMEN private memory per process],
  where a pointer to that memory can be retrieved per process by a call to
  [.WtkQueryProcessDetails WtkQueryProcessDetails]

[=NOTE]
.ul compact
- the memory being allocated must be freed
  by a call to [.WtkTerminateProcessStatus WtkTerminateProcessStatus],
  once the process status is not longer required.

.. ----------------------------------------
.2 retrieving a process status snapshot
.. ----------------------------------------
.an IDPNL_APIPROC_SNAPSHOT
.
A first snapshot of the process status is being taken by the call to
[.WtkInitializeProcessStatus WtkInitializeProcessStatus].
At every time, the snapshot can be refreshed by a call to
[.WtkRefreshProcessStatus WtkRefreshProcessStatus].

This API optionally returns a flag indicating wether the process status has
changed since the last snapshot.

.. ----------------------------------------
.2 querying process status information
.. ----------------------------------------
.an IDPNL_APIPROC_QUERYINFO
.
From a previously taken
[.IDPNL_APIPROC_SNAPSHOT snapshot of the process status]
information can be queried by use of several APIs.

The process identifiers of all child processes of a given process tree
can be obtained with [.WtkEnumProcessChilds WtkEnumProcessChilds],
specifiying *WTK__PROCESS__ENUMALL* starts enumeration at the root
of the process tree.

Detailed information on processes and threads can be obtained with
[.WtkQueryProcessDetails WtkQueryProcessDetails], informations for threads
are returned with [.WtkEnumProcessThreads WtkEnumProcessThreads].
If (only) the executable name of a process is known, all running instances
of that program can be queried with [.WtkQueryProcess WtkQueryProcess].

If a display of the process tree is to be displayed, it is of importance
to know the level depth of the process tree - this is returned by
[.WtkQueryProcessStatusMaxLevel WtkQueryProcessStatusMaxLevel].

.. ----------------------------------------
.2 waiting for a process
.. ----------------------------------------
.an IDPNL_APIPROC_WAITPROCESS
.
By calling [.WtkWaitProcess WtkWaitProcess] a process can be waited for.
For this either a previously taken snapshot of the process status
can be used, or the AI can be called without it, having a one line call to the API
without requiring to [.IDPNL_APIPROC_SNAPSHOT initialize] and
[.IDPNL_APIPROC_TERMINATE terminate] a snapshot.

With the parameters ulWait and pulMaxWait the amount of time in milliseconds
per wait period and the maximum wait time can be specified. If so, the
maximum wait value will be decreased with each wait, and set to zero,if
time is exhausted - just in case that the maximul value is not an exact
multiple of ulWait.

WtkWaitProcess can also be used as well to query if a process is in memory,
by not waiting for it, setting ulWait to zero and pulMaxWait to NULL.

.. ----------------------------------------
.2 using private data in process status tree snapshot
.. ----------------------------------------
.an IDPNL_APIPROC_PRIVATEMEN
.
When taking [.IDPNL_APIPROC_SNAPSHOT snapshots of the process tree],
private memory can be reserved per process.
The size of that private memory per process must be specified on the call
to [.WtkInitializeProcessStatus WtkInitializeProcessStatus].

When querying process details with [.WtkQueryProcessDetails WtkQueryProcessDetails],
a pointer to the private memory for that specific memory can be returned.
This private memory can be filled by the application and used later
by querying the pointer to that memory again with *WtkQueryProcessDetails*.
The private memory per process will not be reset
until the next time that the snapshot is being refreshed with
[.WtkRefreshProcessStatus WtkRefreshProcessStatus].

The private memory must not be freed by the application, but will be freed
automatically when the usage of the process API is terminated
by a call to [.WtkTerminateProcessStatus WtkTerminateProcessStatus].

.. ----------------------------------------
.2 terminate process API
.. ----------------------------------------
.an IDPNL_APIPROC_TERMINATE
.
Usage of the Process related functions must be
terminated by a call to [.WtkTerminateProcessStatus WtkTerminateProcessStatus]
in order to free up the [.IDPNL_APIPROC_SNAPSHOT snapshot memory] as well as
the [.IDPNL_APIPROC_PRIVATEMEN private memory].

