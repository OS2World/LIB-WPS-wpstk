
# makefile for the WPS Toolkit library code
#
# Copyright (c) WPS Toolkit Project - Christian Langanke 2000
#
# $Id: makefile,v 1.37 2008-02-03 22:45:03 cla Exp $
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

IDEF=$(BINDIR)\wpstki.def
CDEF=wpstk.def

# select what to compile
!ifdef DLL
IMPLIB=1
LIBS=somtk
DEF=$(IDEF)
!else
DEF=$(CDEF)
!endif

# include configuration (compiler options),
# also defines $(BINDIR), $(INCDIR) etc.
BASEDIR=..\..
PRJINC=$(BASEDIR)\config\project.in
!include $(BASEDIR)\config\rules.in

DESCRIPTION=WPS Toolkit library code makefile
!ifndef DLL
!if [@echo $(STARTMSG)]
!endif
!endif

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

# define what to build
ILIBTARGET=$(BINDIR)\wpstki.lib
DLLTARGET=$(BINDIR)\wpstk.dll
LSTTARGET=$(BINDIR)\wpstk.lst
LIBTARGET=$(BINDIR)\wpstk.lib
MAP=$(BINDIR)\wpstk.map

OBJS=$(BINDIR)\wpstkdll.obj

OBJS_DEB= $(BINDIR)\printf.obj

OBJS1= \
$(BINDIR)\wpstk.obj \
$(BINDIR)\wtkset.obj \
$(BINDIR)\wtkset1.obj \
$(BINDIR)\wtkeas.obj  \
$(BINDIR)\wtkumod.obj \
$(BINDIR)\wtkulmd.obj \
$(BINDIR)\wtkuctl.obj \

OBJS2= \
$(BINDIR)\wtkupm.obj \
$(BINDIR)\wtkupm1.obj \
$(BINDIR)\wtkupm2.obj \
$(BINDIR)\wtkusys.obj \
$(BINDIR)\wtkuioc.obj \
$(BINDIR)\wtkuerr.obj \
$(BINDIR)\wtkufil.obj \


OBJS3= \
$(BINDIR)\wtkufil2.obj \
$(BINDIR)\wtkufil3.obj \
$(BINDIR)\wtkufil4.obj \
$(BINDIR)\wtkufil5.obj \
$(BINDIR)\wtkufil6.obj \
$(BINDIR)\wtkufil7.obj \

OBJS4= \
$(BINDIR)\wtkutim.obj \
$(BINDIR)\wtkutim2.obj \
$(BINDIR)\wtkutim3.obj \
$(BINDIR)\wtkutim4.obj \
$(BINDIR)\wtkucrc.obj \
$(BINDIR)\wtkumd5.obj \
$(BINDIR)\wtkurgx.obj \

OBJS5= \
$(BINDIR)\wtkmmf.obj \
$(BINDIR)\wtktmf.obj \
$(BINDIR)\wtkfslvl.obj \
$(BINDIR)\wtkfinit.obj \
$(BINDIR)\wtkfcfgs.obj \
$(BINDIR)\wtkfbmp.obj \
$(BINDIR)\wtkfbldl.obj \

OBJS6= \
$(BINDIR)\wtkproc.obj \
$(BINDIR)\wtkbbh.obj \
$(BINDIR)\wtkuloc.obj \

CLEANUP1= \
$(OBJS) $(OBJS1) $(OBJS2) $(OBJS3) $(OBJS4)

CLEANUP2= \
$(OBJS5) $(OBJS6) $(OBJS_DEB)

CLEANUP3= \
$(ILIBTARGET) $(DLLTARGET) $(LSTTARGET) $(LIBTARGET) $(MAP) \
$(IDEF)

# pseudotargets

!ifdef DLL
all: $(ILIBTARGET) $(DLLTARGET)
!else
all:  lib ilib
        @echo $(ENDMSG)
!endif

lib: $(LSTTARGET)

ilib:
     @$(MAKE) MAKELEVEL=1 DLL=1

run: all

clean:
        @echo About to cleanup the target files ...
        -@for %%a in ($(CLEANUP1)) do @(echo - %%a & del %%a /N $(TO_NUL))
        -@for %%a in ($(CLEANUP2)) do @(echo - %%a & del %%a /N $(TO_NUL))
        -@for %%a in ($(CLEANUP3)) do @(echo - %%a & del %%a /N $(TO_NUL))
        @echo Done.
        @echo $(ENDMSG)

# include global targets
!include $(BASEDIR)\config\common.in

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

!ifdef DLL

# ----- build the import library
$(ILIBTARGET): $(DEF)

$(DLLTARGET): $(OBJS) $(OBJS1) $(OBJS2) $(OBJS3) $(OBJS4) $(OBJS5) $(OBJS6) $(DEF)

$(DEF): wpstk.def
        @copy wpstk.def $(DEF) $(TO_NUL)
        @echo EXPORTS>>$(DEF)
        @lstexp $(BINDIR)\wpstk.lst Wtk>>$(DEF)

!else

# ----- build the code library
$(LSTTARGET): $(LIBTARGET) $(OBJS_DEB)

$(LIBTARGET): $(OBJS1) $(OBJS2) $(OBJS3) $(OBJS4) $(OBJS5) $(OBJS6)

!endif

