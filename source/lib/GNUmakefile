
# GNUmakefile for the WPS Toolkit library code
#
# Copyright (c) WPS Toolkit Project - Christian Langanke 2000
#
# $Id: GNUmakefile,v 1.41 2008-12-14 22:46:01 cla Exp $
#
# ===========================================================================
#
# This file is part of the WPS Toolkit package and is free software.  You can
# redistribute it and/or modify it under the terms of the GNU Library General
# Public License as published by the Free Software Foundation, in version 2
# as it comes in the "COPYING.LIB" file of the WPS Toolkit main distribution.
# This library is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
# License for more details.

IDEF=wpstki.def
CDEF=wpstk.def

# select what to compile
ifdef DLL
IMPLIB=1
LIBS=somtk os2386
DEF=$(BINDIR)/$(IDEF)
else
DEF=$(CDEF)
endif

# use std switch to avoid problems with MD5 code
#C_FLAGS_USER+=-std=c89            # does not work at all
                                   # also : -ansi, iso9899:1990
#C_FLAGS_USER+=-std=iso9899:199409 # does not work at all
#C_FLAGS_USER+=-std=c99            # wtkset: case type warnings, several implicit declarations
                                   # also: iso9899:1999, iso9899:199x, c9x
#C_FLAGS_USER+=-std=gnu89          # wtkumd5 (default): errors
C_FLAGS_USER+=-std=gnu99           # wtkset: case type errors, printf.c:mplicit declaration
                                   # no problem with md5
                                   # also: gnu9x

# for using SOM with Innotek GCC, certain
# headers and symbols are required
SOMDEF=1

# include configuration (compiler options),
# also defines $(BINDIR), $(INCDIR) etc.
BASEDIR=../..
PRJINC=$(BASEDIR)/config/GNUproject.in
include $(BASEDIR)/config/GNUrules.in

DESCRIPTION=WPS Toolkit library code makefile
ifndef DLL
$(shell cmd /c @echo $(STARTMSG))
endif

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

# --- don't print directories on recursive calls

MAKE:=$(MAKE) --no-print-directory

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

# define what to build

# define what to build
ILIBTARGET=wpstki.lib
DLLTARGET=wpstk.dll
LSTTARGET=wpstk.lst
LIBTARGET=wpstk.lib

OBJS=\
wpstk.obj \
wtkset.obj \
wtkset1.obj \
wtkeas.obj \
wtkumod.obj \
wtkulmd.obj \
wtkurgx.obj \
wtkuctl.obj \
wtkupm.obj \
wtkupm1.obj \
wtkupm2.obj \
wtkusys.obj \
wtkuioc.obj \
wtkuerr.obj \
wtkufil.obj \
wtkufil2.obj \
wtkufil3.obj \
wtkufil4.obj \
wtkufil5.obj \
wtkufil6.obj \
wtkufil7.obj \
wtkutim.obj \
wtkutim2.obj \
wtkutim3.obj \
wtkutim4.obj \
wtkucrc.obj \
wtkumd5.obj \
wtkmmf.obj  \
wtktmf.obj  \
wtkfslvl.obj \
wtkfinit.obj \
wtkfcfgs.obj \
wtkfbmp.obj \
wtkfbldl.obj \
wtkproc.obj \
wtkbbh.obj  \
wtkuloc.obj  \

OBJS_DEB=printf.obj

CLEANUP1= \
$(addprefix $(BINDIR_BS)\,wtk*.obj $(OBJS_DEB))
CLEANUP2= \
$(addprefix $(BINDIR_BS)\,$(ILIBTARGET))       \
$(addprefix $(BINDIR_BS)\,$(DLLTARGET))        \
$(addprefix $(BINDIR_BS)\,$(LSTTARGET))        \
$(addprefix $(BINDIR_BS)\,$(LIBTARGET))        \
$(addprefix $(BINDIR_BS)\,$(IDEF))             \

# pseudotargets

.PHONY: all lib ilib run checkenv clean

ifdef DLL
all: $(ILIBTARGET) $(DLLTARGET)
else
all:  lib ilib
	@echo $(ENDMSG)
	@cmd /c echo.
endif

lib: $(LSTTARGET)

ilib:
	@$(MAKE) MAKELEVEL=1 DLL=1


run: all

clean:
	@echo About to cleanup the target files ...
	-@for %a in ($(CLEANUP1)) do @(echo - %a & del %a /N $(TO_NUL))
	-@for %a in ($(CLEANUP2)) do @(echo - %a & del %a /N $(TO_NUL))
	@echo Done.
	@echo $(ENDMSG)

# include global targets
include $(BASEDIR)/config/GNUcommon.in

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

ifdef DLL

# ----- build the import library
$(ILIBTARGET): $(DEF)

$(DLLTARGET): $(OBJS) $(DEF)

$(DEF): wpstk.def
	@copy wpstk.def $(BINDIR_BS)\$(IDEF) $(TO_NUL)
	@echo EXPORTS>>$(BINDIR_BS)\$(IDEF)
	@xemxexp $(BINDIR_BS)\wpstk.lib >>$(BINDIR_BS)\$(IDEF)

else

# ----- build the code library
$(LSTTARGET): $(LIBTARGET) $(OBJS_DEB)

$(LIBTARGET): $(OBJS)

endif

# include global targets
include $(BASEDIR)/config/GNUcommon.in

