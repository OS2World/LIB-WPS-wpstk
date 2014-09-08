
# GNUmakefile for NLS functions related sample code
#
# Copyright (c) WPS Toolkit Project - Christian Langanke 2000
#
# $Id: GNUmakefile,v 1.10 2008-12-14 22:46:00 cla Exp $
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
# DLLs are to be named .nls
APPTYPE=VIO

# --- list of available source for separate NLS modules

NLSSUPPORTED=eng deu

# define what to build
STEM=_nls
RESDIR=$(BINDIR)

ifdef NLS
# - create resource DLL
DLL=1
OBJS=_resource.obj
RES=$(STEM)$(NLS).res
DLLTARGET=$(STEM)$(NLS)$(DLLEXT)
INFTARGET=$(STEM)$(NLS).inf
DEF=_resource.def
DLLEXT=.nls
endif

ifdef NLSDLL
 # - create DLL
DLL=1
TARGET=$(STEM)dll.dll
OBJS=$(STEM)dll.obj
LIBS=$(WTK_LIBRARY_NAME)
DEF=$(STEM)dll.def
endif

ifdef NLSEXE
# - create EXE
TARGET=$(STEM).exe
OBJS=$(STEM).obj
LIBS=$(WTK_LIBRARY_NAME)
endif

TESTTARGET=$(STEM).exe

# include configuration (compiler options),
# also defines $(BINDIR), $(INCDIR) etc.
BASEDIR=../..
PRJINC=$(BASEDIR)/config/GNUproject.in
include $(BASEDIR)/config/GNUrules.in

DESCRIPTION=NLS related functions sample code makefile
ifndef CALLED
$(shell cmd /c @echo $(STARTMSG))
endif

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

# --- implicit rule for ipf source

.SUFFIXES: .ipf .inf

.ipf.inf:
	ipfc $*.ipf $(BINDIR)/$*.inf /INF

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

CLEANUP=\
$(addprefix $(BINDIR_BS)\,$(STEM).exe)  \
$(addprefix $(BINDIR_BS)\,$(STEM).obj)  \
$(addprefix $(BINDIR_BS)\,_resource.obj)  \
$(addprefix $(BINDIR_BS)\,$(STEM)???.obj)  \
$(addprefix $(BINDIR_BS)\,$(STEM)???.nls)  \
$(addprefix $(BINDIR_BS)\,$(STEM)???.dll)  \
$(addprefix $(BINDIR_BS)\,$(STEM)???.inf)  \
$(addprefix $(BINDIR_BS)\,$(STEM)???.map)  \
$(addprefix $(BINDIR_BS)\,$(STEM)???.res)


# pseudotargets
all: buildnls
	@for %a in (NLSDLL NLSEXE) do @$(MAKE) target %a=1 CALLED=1 MAKELEVEL=1
	@echo $(ENDMSG)

nls: buildnls
	@echo $(ENDMSG)

buildnls:
	@for %a in ($(NLSSUPPORTED)) do @$(MAKE) target NLS=%a CALLED=1 MAKELEVEL=1

target: $(TARGET) $(DLLTARGET) $(INFTARGET)

run: all
	@echo ***** running sample program
	@cmd /C $(TESTTARGET) $(BINDIR)
	@echo $(ENDMSG)


debug: all
	@echo ***** loading sample program in debugger
	@$(DBG) $(BINDIR)/$(TARGET) $(BINDIR)
	@echo $(ENDMSG)


clean:
	@echo About to cleanup the target files ...
	-@for %a in ($(CLEANUP)) do @(echo - %a & del %a /N $(TO_NUL))
	@echo Done.
	@echo $(ENDMSG)



# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

# build the test program


ifdef NLS

# build the resource modules and INF files

$(STEM)$(NLS)$(DLLEXT): $(STEM)$(NLS).obj $(STEM)$(NLS).res $(DEF)

$(STEM)$(NLS).obj: _resource.obj
	copy $(BINDIR_BS)\_resource.obj $(BINDIR_BS)\$(STEM)$(NLS).obj >NUL 2>&1

$(STEM)$(NLS).inf: $(STEM)$(NLS).ipf

endif


ifdef NLSDLL
# build the test program DLL

$(STEM)dll.dll: $(OBJS) $(DEF) $(WTK_LIBRARY)

$(STEM)dll.obj: $(STEM)dll.c $(STEM)testinc.c

endif


ifdef NLSEXE

# build the test program

$(STEM).exe: $(OBJS) $(WTK_LIBRARY)

$(STEM).obj: $(STEM).c $(STEM)testinc.c

endif

