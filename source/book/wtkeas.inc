..******************************* Module Header *******************************
..*
..* Module Name: wtkeas.inc
..*
..* Hypertext/2 include file for pprogrammers guide section on
..* Extended Attributes Manager
..*
..* Copyright (c) WPS Toolkit Project - Christian Langanke 2000
..*
..* $Id: wtkeas.inc,v 1.2 2005-02-24 20:55:35 cla Exp $
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
.1 Extended Attributes Manager
.. ----------------------------------------
.an IDPNL_APIEAS
.
[=TOPICS]
.su V30

.. ----------------------------------------
.2 Extended Attributes Manager Overview
.. ----------------------------------------
.an IDPNL_APIEAS_OVERVIEW
.
The Extended Attributes Manager API of the [=TITLE]
is designed to greatly ease the handling of extended attributes.
While the attribute data structures that can be read and/or written with
several OS/2 control programs, provide great flexibility, the setup of these
structures require time consuming coding and testing, and such each time that you
are required to handle extended attributes.

In order to have a simplified API, the API of the [=TITLE]
differs from conventional extended attribute handling in several ways:

.ul
- You can only read or write one extended attribute (either with a single or a list
  of multiple values) at a time. Since this is, what you will do in most cases anyway,
  this will not really restrict you a lot and this step alone makes the following point
  possible:
..
- You never have to deal with the binary structures again, if you use the
  extended attributes API of the [=TITLE]. All this handling
  is completely done for you, while you just provide values for a given extended attribute
  in a source buffer to let it be written or appended, or you ask for values to be returned
  in a target buffer for a specific type of extended attributes. The *EAT__*** type
  values and some knowledge about organization of extended attributes are the only things
  that are still required in order to setup extended attributes for a given file or
  directory.
..
- Normally, you are required to read, modify and write an extended attribute, if you
  do not even just replace it with a completely new value. Having said this,
  the [=TITLE] provides you both with three APIs to let you do
  also these single steps one after another, but also with a single step API to accomplish
  this task in one step. Since writing exactly one value to an extended attribute is
  exactly, what most Workplace Shell classes need to do, this simplifies your coding a lot.
  And even if you call these APIs for several extended attributes one after another
  (compared to setup a complex structure and write it with only one call to a control
  program API), the effective use of the OS/2 filesystem cache will reduce the overhead
  of subsequent disk I/O accesses.

.. ----------------------------------------
.2 Reading an extended attribute
.. ----------------------------------------
.an id=IDPNL_APIEAS_READEA
.
The following APIs are provided to read an extended attribute of
a given file or directory:

.pl
..
- [.WtkQueryEaSize WtkQueryEaSize]
= determines the size of all values of the given EA.
..
- [.WtkReadEa WtkReadEa]
= reads the value (list) of a given extended attribute into memory.
  Use the following APIs to query all or certain of these values:
.ul compact
- [.WtkFindFirstEaValue WtkFindFirstEaValue]
- [.WtkFindNextEaValue WtkFindNextEaValue]
.
.
[=NOTE]
.ul compact
- Call [.WtkCloseEa WtkCloseEa] in order to
  release the memory occupied by the value (list) of the extended attribute afterwards !
- an in-memory value (list) of an extended attribute being read by
  [.WtkReadEa WtkReadEa] can be extended by
  calling [.WtkAppendEaValue WtkAppendEaValue]
  and saved to disk by [.WtkSaveEa WtkSaveEa].
.el
.el
..
- [.WtkGetNextEaValue WtkGetNextEaValue]
= In order to easily query all values of an extended attribute matching a given type.
  call this API in a simple while loop, until it returns *ERROR__NO__MORE__FILES*.
  *WtkGetNextEaValue* will call
.ul compact
- [.WtkFindNextEaValue WtkFindNextEaValue],
- [.WtkFindNextEaValue WtkFindNextEaValue] and
- [.WtkCloseEa WtkCloseEa]
.el
.
  for you to accomplish this task.
  Make sure to either call *WtkGetNextEaValue*, until it returns
  *ERROR__NO__MORE__FILES*, or call [.WtkCloseEa WtkCloseEa] yourself to free up
  the memory occupied by the temporary in-memory value (list) for the extended
  attribute.
..
- [.WtkReadStringEa WtkReadStringEa]
= This API is even simpler to use compared to *WtkGetNextEaValue* when
  it comes to read a string extended attribute.
  While being able to read exactly one value and only a string value,
  no knowledge of extended attributes is required at all to use it.

.. ----------------------------------------
.2 Writing an extended attribute
.. ----------------------------------------
.an id=IDPNL_APIEAS_WRTITEEA
.
The following APIs are provided to write an extended attribute of a given file or directory:

.pl
..
- [.WtkCreateEa WtkCreateEa]
= creates an empty in-memory valuelist for an extended attribute.
..
- [.WtkAppendEaValue WtkAppendEaValue]
= appends a value to an in-memory valuelist for an extended attribute that
  has either been created by [.WtkCreateEa WtkCreateEa]
  or read from disk by [.WtkReadEa WtkReadEa].
  The value can only be appended, if the type matches the current value (list).
..
- [.WtkSaveEa WtkSave]
= writes a modified value (list) of a given extended attribute to disk.
..
- [.WtkPutEaValue WtkPutEaValue]
= reads the current value (list) of an extended attribute from disk,
  appends the given value to the list, if the type matches the current
  value (list), and saves the newly extended valuelist to disk.
..
- [.WtkWriteStringEa WtkWriteStringEa]
= This API is even simpler to use compared to *WtkPutEaValue* when
  it comes to write an string extended attribute.
  While being able to write exactly one value and only a string value,
  no knowledge of extended attributes is required at all to use it.

