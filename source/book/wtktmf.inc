..******************************* Module Header *******************************
..*
..* Module Name: wtktmf.inc
..*
..* Hypertext/2 include file for programmers guide section on
..* Text Message Files
..*
..* Copyright (c) WPS Toolkit Project - Christian Langanke 2000
..*
..* $Id: wtktmf.inc,v 1.3 2009-11-17 22:00:00 cla Exp $
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
.1 Text Message Files (TMF)
.. ----------------------------------------
.an IDPNL_APITMF
.
[=TOPICS]
.su V30

.. ----------------------------------------
.2 Text Message Files (TMF) Overview
.. ----------------------------------------
.an IDPNL_APITMF_OVERVIEW
.
The *Text Message File* API of the [=TITLE]
provides an alternative to the OS/2 message files, because the OS/2 message files
have several drawbacks:
.ul compact
- they need to be compiled with the *mkmsgf* utility
- testing is complicated
- some APIs like the SysGetMessage of REXX call keep the file open until the shell
  closes, where the REXX script is running in.

For that reasons many programs use plain text files, but also this has a drawback,
as the complete file has to be searched for every message again and again.
Here the *Text Message File* (TMF) concept jumps in, combining the
convenience of having editable text files while avoiding to search the whole file
for every message request.

Text message files complying to the [.IDPNL_APITMF_FORMAT format]
can be used with the [.WtkGetTextMessage WtkGetTextMessage],
call. When a message is requested by this, the following actions are performed:
.ul compact
- the text message file is checked for being modified (compared with a previously saved timestamp)
- if modified,
  - an internal message id table is compiled and saved as extended attribute
  - the current timestamp is saved as an extended attribute
- the requested message is located by the internal message id table and returned
  to the caller

So in most of the times the internal message id table is used to determine the
location of a message within the text message file, which decreases the amount of
required file handling drastically.

Further releases of the [=TITLE] will be extended by a
similar API, either holding the file open and/or reading the file into memory,
once more gaining quicker access to messages.

.. ----------------------------------------
.2 text message file format
.. ----------------------------------------
.an IDPNL_APITMF_FORMAT
.
The *Text Message Files* (TMF) must comply to the following format in order
to let [.WtkGetTextMessage WtkGetTextMessage] retrieve messages out of it:
.ul
..
- messages start with a message id in the first column of a line. The message id
  is assembled by:
.ul compact
- the string *<--*
- the alphanumeric message id
- the string *-->:*
.el
.
.
A valid message id would be: *<--MyString001-->:*
..
- before the first message comments may be included within the file, started
  with a semikolon in the first column.
.
.at fc=red
[=NOTE]
.ul compact
- After the first message id all comments are taken being part of messages !
.at
..
- messages may span several lines, where the carriage return and linefeed characters
  (except of the last line of a message) will become part of the message. So if you don't
  meant to have linefeeds in your message, make sure that they don't span lines,
  no matter how long the line will ever get
..
- placeholders from *%1* to *%9* in the message text are replaced if
  values for those are handed over to [.WtkGetTextMessage WtkGetTextMessage]
  in the pointer table referred to by the *pTable* parameter.
.
  In opposite to the *DosGetMessage* call text replacement is performed even
  if not enough parameters are supplied - where values are missing, the placeholder
  remains unmodified within the message text.

