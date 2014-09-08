..******************************* Module Header *******************************
..*
..* Module Name: wtkmmf.inc
..*
..* Hypertext/2 include file for programmers guide section on
..* Memory Mapped Files Manager
..*
..* Copyright (c) WPS Toolkit Project - Christian Langanke 2000
..*
..* $Id: wtkmmf.inc,v 1.7 2008-10-22 14:15:04 cla Exp $
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
.1 Memory Mapped Files Manager
.. ----------------------------------------
.an IDPNL_APIMMF
.
[=TOPICS]
.su V30 1

.. ----------------------------------------
.2 Memory Mapped Files Manager Overview
.. ----------------------------------------
.an IDPNL_APIMMF_OVERVIEW
.
The *Memory Mapped Files Manager* API of the [=TITLE]
provides an API for reading and writing files by simply reading from and
writing to memory. For that files are mapped to memory and the
[.IDPNL_APIMMF_XHANDLER MMF exception handler]
takes care for that only the required parts of the file are being read from
disk and only the modified parts are saved to disk, if requested.

With memory mapped files opening a file is as easy as allocating a chunk of
memory, and saving all changes is done by a simple API call, there is no need
for intermediate files or for fiddeling around with file pointers.

Whenever available, the MMF API will allocate memory in the High Memory Arena
above 512 MB. Otherwise, memory in the conventional address space is allocated.

.. ----------------------------------------
.2 initialize MMF support
.. ----------------------------------------
.an IDPNL_APIMMF_INIT
.
When using the *Memory Mapped Files Manager* API, it must be
initialized by a call to [.WtkInitializeMmf WtkInitializeMmf].
If the API is used in several threads within a process, this
must be done per thread. If an instance of the MMF Manager was
aleady created, the API returns with an error, but still reports
the handle to the current instance.

Beside initializing the internal data this call will register the
[.IDPNL_APIMMF_XHANDLER MMF exception handler].

.at fc=red
[=NOTE]
.ul compact
- When leaving the function that called WtkInitializeMmf, WtkTerminateMmf
  must be called to deregister the exception handler again, especially if
  other exception handlers are used beside the one for MMF support. Otherwise
  unpredictable results (most often simply protection violations) may occurr.

Please see also:
.ul compact
- [.IDPNL_APIMMF_MULTITHREAD]
- [.IDPNL_APIMMF_LINKCODE]


.. ----------------------------------------
.2 get MMF Manager information
.. ----------------------------------------
.an IDPNL_APIMMF_GETINFO
.
Once a MMF Manager has been created for the current by a call to 
[.WtkInitializeMmf WtkInitializeMmf], information can be retrieved
about it by a call to [.WtkGetMmfInfo WtkGetMmfInfo].

This fills a buffer with the structure [.MMINFO MMINFO], which 
contains the handle to the MMF Manager as well as the current
and maximum amount of file buffers and MMF MAnager instances.

Please see also:
.ul compact
- [.IDPNL_APIMMF_MULTITHREAD]
- [.IDPNL_APIMMF_LINKCODE]

.. ----------------------------------------
.2 allocating a memory mapped file buffer
.. ----------------------------------------
.an IDPNL_APIMMF_ALLOCFILE
.
Allocating a file with the *Memory Mapped Files Manager* API
does not really already allocate memory. Instead, it only reserves
address space for the requested memory object so that the new and/or
existant file #could# fit into the memory. However, the maximum size
of the memory object must be specified on allocation, because it
cannot be extended later.

All further access to the file can simply be done by accessing the memory,
everything else is being taken care by the
[.IDPNL_APIMMF_XHANDLER MMF exception handler].

Use one of the following APIs to allocate a file
.pl tsize=1
- [.WtkAllocMmf WtkAllocMmf]
= open a file and allocate a buffer for it
- [.WtkAllocMmfFile WtkAllocMmfFile]
= allocate a buffer for an already opened file
.
.
Make sure that you don't modify the file with the original filehandle
while having it mapped by WtkAllocMmfFile, otherwise this will
lead to unpredictable results, if you use [.WtkUpdateMmf WtkUpdateMmf]
to update it later.

.. ----------------------------------------
.2 allocating a memory only buffer
.. ----------------------------------------
.an IDPNL_APIMMF_ALLOCMEMONLY
.
The *Memory Mapped Files Manager* API [.WtkAllocMmf WtkAllocMmf]
can also be used to allocate a memory object without mapping a file to it.

Such a memory object has still the advantage that it can grow dynamically,
so subsequent handling of memory (sub)allocation within the uncommitted memory
object is done automatically by the [.IDPNL_APIMMF_XHANDLER MMF exception handler].

.. ----------------------------------------
.2 allocating high memory
.. ----------------------------------------
.an IDPNL_APIMMF_ALLOCHIGHMEM
.res 2000
.
The *Memory Mapped Files Manager* API allocates the file buffer
in high memory above 512 MB in the so-called High Memory Arena,
when the flag MMF__ALLOC__HIGHMEM is specified on a call to 
[.WtkAllocMmf WtkAllocMmf] or
[.WtkAllocMmfFile WtkAllocMmfFile].

However, a high memory buffer mostly cannot be accessed from 16-bit code.
[=TOPICS]
.su H50 -1

.. ----------------------------------------
.3 16- to 32-bit address conversion
.. ----------------------------------------
.an IDPNL_APIMMF_ALLOCHIGHMEM_THUNKING
.
Up to 4 GB memory can be adressed by 32-bit code, where 16-bit code
can address only 1 GB of memory.

This is the reason why OS/2 first was designed to use memory up to 512 MB
only (not 1 GB though), so that 16- and 32-bit code can access the same
memory by translating 32:0 memory adresses to 16:16 addresses (this is
called thunking), if a 16-bit API is called from 32-bit code, and vice
versa.

As a result high memory cannot be accessed by a 16-bit API, if the
address lies higher than 1 GB. If this is tried, the converted 16:16
address will be invalid, and, when being accessed, lead to an
access violation.

.. ----------------------------------------
.3 problematic operating system APIs
.. ----------------------------------------
.an IDPNL_APIMMF_ALLOCHIGHMEM_PROBLEMAPIS
.
As some considerable parts of the operating system are still implemented
in 16-bit code, these cannot be used to access high memory, such as:
.ul compact
- Vio** calls
- Kbd** calls
- Mou** calls

Also certain Dos** APIs, even if provided as a 32-bit interface to
applications, are only forwarded to an old 16-bit implementation,
such as DosMapCase. This particular one can easily be avoided by
using the C runtime function strupr().

However, there may also be trouble with some 32-bit Dos ** APIs,
if they in special cases  call 16-bit APIs. Eamples for this would
be *DosRead* and *DosWrite*. While working perfectly with high memory
when reading from or writing to a file, in textmode applications
.ul compact
- calling *DosRead* with the standard input handle 0 will read the
  keyboard with Kbd** calls
- calling *DosWrite* either with the
  - standard output handle 1 
  - standard error handle 2
.el
  will produce textmode screen output 

In such cases also an otherwise 32-bit implemented Dos** API will
as will fail when using a high memory buffer.

.. ----------------------------------------
.3 workarounds for 16-bit access
.. ----------------------------------------
.an IDPNL_APIMMF_ALLOCHIGHMEM_WORKAROUNDS
.
When direct or indirect access by a 16-bit API cannot be avoided
there are the following alternatives:
.ul
- allocate low memory instead, if that is not leading to
  memory constraints in the low memory arena
- use C runtime functions instead - hopefully they don't call
  the same 16-bit APIs...
- where applicable, copy small portions of high memory to low 
  memory and process it from there, if that is not leading to
  performance degradions

.. ----------------------------------------
.2 update a file from a memory mapped file buffer
.. ----------------------------------------
.an IDPNL_APIMMF_UPDATE
.
After having modified a memory mapped file buffer all changes can
be written back to disk by a call to [.WtkUpdateMmf WtkUpdateMmf].
When doing so, only the memory pages being marked as modified by the
[.IDPNL_APIMMF_XHANDLER MMF exception handler]
are written to disk.

.. ----------------------------------------
.2 freeing memory mapped file buffers
.. ----------------------------------------
.an IDPNL_APIMMF_DEALLOCFILE
.
When not longer needed, a file buffer previously allocated with
[.WtkAllocMmf WtkAllocMmf] or [.WtkAllocMmfFile WtkAllocMmfFile]
must be freed. If a file needs an update with changes having been written
to the file buffer before, update the file with [.WtkUpdateMmf WtkUpdateMmf]
before freeing the buffer.

For freeing file buffers, use either [.WtkFreeMmf WtkFreeMmf] or
[.WtkFreeMmfFile WtkFreeMmfFile]. The two calls are identical, however
each call was implemented as a counterpart to the WtkAllocMMf-APIs.

[=NOTE]
.ul compact
- If a buffer has been allocated with [.WtkAllocMmfFile WtkAllocMmfFile]
  to use an already opened file, this file is not closed on deallocation of the buffer

.. ----------------------------------------
.2 terminate MMF support
.. ----------------------------------------
.an IDPNL_APIMMF_TERMINATE
.
When using the *Memory Mapped Files Manager* API, it must be
terminated when leaving the function that has initialized MMF support.

Beside freeing internal resources this call will deregister the
[.IDPNL_APIMMF_XHANDLER.MMF exception handler], which has been
registered on the [.IDPNL_APIMMF_INIT initialization of the MMF support].
If that is not done, unpredictable results (most often simply
protection violations) may occurr.

.. ----------------------------------------
.2 MMF exception handler
.. ----------------------------------------
.an IDPNL_APIMMF_XHANDLER
.
The exception handler used by the *Memory Mapped Files Manager* API
is responsible for keeping track of the file being mapped to memory.

On PC platforms memory is reserved and or allocated in 4 Kb memory pages
and every page can among others have the attributes
*committed for read access* and/or *committed for write access*.

Having reserved the memory address space only at first, none of the commit
attributes are set, this is called #uncommitted memory#. Every access
now of uncommitted memory (either for read or write access) will normally
cause an access violation. 
The MMF exception handler intercepts all exceptions for allocated memory 
mapped file buffers and performs the following actions:

.pl tsize=3
- *on a read access to a memory page not committed for read*
= read the 4 Kb portion of the mapped file and mark this memory page
  committed for read access
..
- *on a write access to a memory page not committed for write*
=
.ul compact
- perform the action for memory not being committed for read, if
  that memory page was not read before
- mark this memory page committed for write access, so that a call to
  [.WtkUpdateMmf WtkUpdateMmf] later can write the modified pages back to disk.

Please see also:
.ul compact
- [.IDPNL_APIMMF_MULTITHREAD]
- [.IDPNL_APIMMF_LINKCODE]

.. ----------------------------------------
.2 MMF multithreading considerations
.. ----------------------------------------
.an IDPNL_APIMMF_MULTITHREAD
.
While MMF handles are only valid in the thread where created,
allocated file buffers can be accessed by all threads of a process
as long as the thread creating the buffers is alive.

.. ----------------------------------------
.2 MMF code linking considerations
.. ----------------------------------------
.an IDPNL_APIMMF_LINKCODE
.res 2001
.
The MMF API can create manager instances to a maximum number of
128, where one must be created for each thread and process
using the MMF API. Depending on how an application is or parts of
an application are linked to the Workplace Shell Toolkit
.ul compact
- this number is available to one application, or must be shared
  by multiple applications at a time
- the MMF handles are valid within all or only some parts of an
  application

_*Linked dynamically to the Workplace Shell Toolkit runtime DLL*_
.ul
- All applications linked to the runtime DLL share the amount of 128
  possible instances of MMF Managers.
- Within the same thread of an application the MMF handles can be
  interchanged freely between all parts of an application being linked
  to the runtime DLL.

_*Linked statically to the Workplace Shell Toolkit library*_
.ul
- Each part of the application linked to the library can instanciate
  128 possible instances of MMF Managers. This appies to all application
  parts carrying an own runtime, such as the main executable and all DLLs.
- Each MMF handle is only valid in the application part where it was
  created.
.
  It would not be possible to e.g. create a MMF handle in the main
  executable, and pass it to a DLL, and with this allocate file
  buffers. The API would reject the handle as being invalid.
.
  However the main application part can allocate file buffers, which
  can be used by DLL modules, or vice versa.

