
# makefile for NLS functions related sample code
#
# Copyright (c) WPS Toolkit Project - Christian Langanke 2000
#
# $Id: makefile,v 1.7 2006-08-18 23:32:47 cla Exp $
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

# tell we would like to have a VIO application
APPTYPE=VIO

# --- list of available source for separate NLS modules

NLSSUPPORTED=eng deu

# define what to build
STEM=_nls

!ifdef NLS
# - create resource DLL
DLL=1
OBJS=$(BINDIR)\_resource.obj
RES=$(BINDIR)\$(STEM)$(NLS).res
DLLTARGET=$(BINDIR)\$(STEM)$(NLS)$(DLLEXT)
INFTARGET=$(BINDIR)\$(STEM)$(NLS).inf
DEF=_resource.def
DLLEXT=.nls
!endif

!ifdef NLSDLL
# - create DLL
DLL=1
TARGET=$(BINDIR)\$(STEM)dll.dll
OBJS=$(BINDIR)\$(STEM)dll.obj
DEF=$(STEM)dll.def
!endif

!ifdef NLSEXE
# - create EXE
TARGET= $(BINDIR)\$(STEM).exe
OBJS=$(BINDIR)\$(STEM).obj
!endif

TESTTARGET=$(BINDIR)\$(STEM).exe


# include configuration (compiler options),
# also defines $(BINDIR), $(INCDIR) etc.
BASEDIR=..\..
PRJINC=$(BASEDIR)\config\project.in
!include $(BASEDIR)\config\rules.in

DESCRIPTION=NLS related sample code makefile
!ifndef CALLED
!if [@echo $(STARTMSG)]
!endif
!endif


# define libs to link - value is set by include file !
LIBS=$(WTK_LIBRARY)

# !if [@echo ----- NLSEXE=$(NLSEXE) NLSDLL=$(NLSDLL) NLS=$(NLS) CALLED=$(CALLED) MAKELEVEL=$(MAKELEVEL) -----------]
# !endif

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

# --- implicit rule for ipf source

.SUFFIXES: .ipf .inf

.ipf{$(BINDIR)}.inf:
    ipfc $(@B).ipf $*.inf /INF

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

CLEANUP=\
$(BINDIR)\$(STEM).exe \
$(BINDIR)\$(STEM).obj \
$(BINDIR)\_resource.obj \
$(BINDIR)\$(STEM)???.obj \
$(BINDIR)\$(STEM)???.nls \
$(BINDIR)\$(STEM)???.dll \
$(BINDIR)\$(STEM)???.inf \
$(BINDIR)\$(STEM)???.map \
$(BINDIR)\$(STEM)???.res

# pseudotargets
all: buildnls
        @for %%a in (NLSDLL NLSEXE) do @$(MAKE) /nologo target %%a=1 CALLED=1 MAKELEVEL=1
        @echo $(ENDMSG)

nls: buildnls
        @echo $(ENDMSG)

buildnls:
    @for %%a in ($(NLSSUPPORTED)) do @$(MAKE) /nologo target NLS=%%a CALLED=1 MAKELEVEL=1

target: $(TARGET) $(DLLTARGET) $(INFTARGET)

run: all
        @echo ***** running sample program
        @$(TESTTARGET) $(BINDIR)
        @echo $(ENDMSG)

debug: all
        @echo ***** loading sample program in debugger
        @$(DBG_SETUP)
        @$(DBG) $(TESTTARGET) $(BINDIR)
        @echo $(ENDMSG)

clean:
        @echo About to cleanup the target files ...
        -@for %%a in ($(CLEANUP)) do @(echo - %%a & del %%a /N $(TO_NUL))
        @echo Done.
        @echo $(ENDMSG)


# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

!ifdef NLS
# build the resource modules and INF files
$(BINDIR)\$(STEM)$(NLS)$(DLLEXT): $(BINDIR)\$(STEM)$(NLS).obj $(RES) $(DEF)

$(BINDIR)\$(STEM)$(NLS).obj: $(BINDIR)\_resource.obj
     @copy $(BINDIR)\_resource.obj $(BINDIR)\$(STEM)$(NLS).obj >NUL 2>&1

$(BINDIR)\$(STEM)$(NLS).inf: $(STEM)$(NLS).ipf
!endif

!ifdef NLSDLL
# build the test program DLL
$(BINDIR)\$(STEM)dll.dll: $(OBJS) $(WTK_LIBRARY)

$(BINDIR)\$(STEM)dll.obj: $(STEM)dll.c $(STEM)testinc.c
!endif

!ifdef NLSEXE
# build the test program
$(BINDIR)\$(STEM).exe: $(OBJS) $(WTK_LIBRARY)

$(BINDIR)\$(STEM).obj: $(STEM).c $(STEM)testinc.c
!endif

