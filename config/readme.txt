
# makefile configuration readme
#
# Copyright (c) WPS Toolkit Project - Christian Langanke 2000
#
# $Id: readme.txt,v 1.6 2006-12-03 20:56:46 cla Exp $
#
# ===========================================================================
#
# This file is part of the WPS Toolkit package and is free software.
# You can redistribute it and/or modify it under the terms of the GNU General
# Public License as published by the Free Software Foundation, in version 2
# as it comes in the "COPYING" file of the WPS Toolkit main distribution.
# This library is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
# or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# for more details.


Overview per make/compiler
==========================

Files   : rules.in (external source !), project.in
Included: rules.in by makefile in each directory
make    : nmake
compiler: IBM C Set 2, IBM Visual Age V3 for OS/2, Open Watcom

Files   : GNUrules.in (external source !), GNUproject.in
Included: GNUrules.in by GNUmakefile in each directory
make    : GNU make
compiler: EMX for OS/2, Innotek GCC


Files   : config.cmd
used by : project.in / GNUproject.in
task    : determine current compiler and tookit.
          If used with emx, creates some patched headers for
          usage of header files of the Toolkit for OS/2 WARP 3/4.

Files   : wdef2lib.cmd
used by : rules.in
task    : add support for creating a library out of a definition file
          with Open Watcom

Files   : wdef2lnk.cmd
used by : rules.in
task    : add support for adding directives and options to a Open Watcom
          wlink file from the directives of a module definition file

