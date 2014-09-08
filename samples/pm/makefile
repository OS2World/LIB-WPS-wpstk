
# makefile for PM helper functions sample code
#
# Copyright (c) WPS Toolkit Project - Christian Langanke 2004
#
# $Id: makefile,v 1.6 2008-12-22 18:47:02 cla Exp $
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

# tell we would like to have a PM application with debug support
APPTYPE=PM
PMPRINTF=1

# define what to build
STEM=_pm
OBJS=$(BINDIR)\$(STEM).obj $(BINDIR)\_pmnb.obj \
     $(BINDIR)\_pmfddlg.obj $(BINDIR)\_pmcnr.obj
TARGET=$(BINDIR)\$(STEM).exe
HLPTARGET=$(BINDIR)\$(STEM).hlp
RES=$(BINDIR)\$(STEM).res
CLEANUP=$(OBJS) $(TARGET) $(RES) $(BINDIR)\$(STEM).map $(HLPTARGET)

# required libraries to link
LIBS=$(WTK_LIBRARY)

# include configuration (compiler options),
# also defines $(BINDIR), $(INCDIR) etc.
BASEDIR=..\..
PRJINC=$(BASEDIR)\config\project.in
!include $(BASEDIR)\config\rules.in

DESCRIPTION=PM helper functions sample code makefile
!if [@echo $(STARTMSG)]
!endif

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

# --- implicit rule for ipf source

.SUFFIXES: .ipf .hlp

.ipf{$(BINDIR)}.hlp:
    ipfc $(@B).ipf $*.hlp

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

# pseudotargets (build library first, if required !)
all: $(TARGET) $(HLPTARGET)
        @echo $(ENDMSG)

run: $(TARGET) $(HLPTARGET)
        @echo ***** running sample program
        @$(TARGET)
        @echo $(ENDMSG)

debug: $(TARGET) $(HLPTARGET)
        @echo ***** loading sample program in debugger
        @$(DBG_SETUP)
        @$(DBG) $(TARGET)
        @echo $(ENDMSG)

clean:
        @echo About to cleanup the target files ...
        -@for %%a in ($(CLEANUP)) do @(echo - %%a & del %%a /N $(TO_NUL))
        @echo Done.
        @echo $(ENDMSG)


# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

# build the test program

$(BINDIR)\$(STEM).exe: $(OBJS) $(WTK_LIBRARY) $(RES)

$(RES): $(STEM).rc $(STEM).dlg $(STEM).men $(STEM).rch $(STEM)eng.rch mletext.txt item.bmp

$(BINDIR)\$(STEM).hlp: $(STEM).ipf

