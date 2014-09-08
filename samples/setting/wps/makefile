
# makefile for settings and details manager WPS sample code
#
# Copyright (c) WPS Toolkit Project - Christian Langanke 2000
#
# $Id: makefile,v 1.11 2006-08-16 00:27:23 cla Exp $
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

# include configuration (compiler options),
# also defines $(BINDIR), $(INCDIR) etc.
DLL=1
PMPRINTF=1
DEF=$(BINDIR)\settcls.def
RES=$(BINDIR)\settdlg.res
SCPDIR=$(BINDIR)

BASEDIR=..\..\..
PRJINC=$(BASEDIR)\config\project.in
!include $(BASEDIR)\config\rules.in


DESCRIPTION=settings and details manager WPS sample code makefile
!if [@echo $(STARTMSG)]
!endif


# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

# define what to build
TARGET=$(BINDIR)\settcls.ih $(DEF) $(BINDIR)\settcls.dll
OBJS=$(BINDIR)\settcls.obj \
     $(BINDIR)\settdlg.obj \
     $(BINDIR)\settcb.obj
CLEANUP=$(OBJS) $(RES) $(BINDIR)\settcls.h $(BINDIR)\settcls.def $(TARGET)

# required libraries to link
LIBS=$(WTK_LIBRARY) somtk.lib

# pseudotargets (build library first, if required !)
all: $(TARGET)
        @echo $(ENDMSG)

som: $(BINDIR)\settcls.ih $(BINDIR)\settcls.h $(BINDIR)\settcls.def
        @echo $(ENDMSG)

clean:
        @echo About to cleanup the target files ...
        -@for %%a in ($(CLEANUP)) do @(echo - %%a & del %%a /N $(TO_NUL))
        @echo Done.
        @echo $(ENDMSG)

run:
        @echo Not running a WPS class. Execute "make install" instead.
        @echo $(ENDMSG)

debug:
        @echo This sample is not intended to be executed in a debugger.
        @echo Execute "make install" instead and watch the PM Printf output.
        @echo For other debugging purposes use the VIO sample.
        @echo $(ENDMSG)


install: $(WTK_LIBRARY) $(TARGET)
        @call on $(BINDIR)
        @echo $(ENDMSG)

deinstall:
        @call off
        @echo $(ENDMSG)


# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

# build the som code

$(BINDIR)\settcls.dll: $(OBJS) $(RES) $(DEF) settcls.idl $(WTK_LIBRARY)

$(BINDIR)\settcls.ih: settcls.idl

# this one to let "make -n" run through without existing files
$(BINDIR)\settcls.def: settcls.idl

# this one to let "make -n" run through without existing files
$(BINDIR)\settcls.h: settcls.idl

# explicit rule
$(BINDIR)\settcls.obj: settcls.c $(BINDIR)\settcls.h  $(BINDIR)\settcls.ih const.h

$(BINDIR)\settdlg.obj: settdlg.h settdlg.rch const.h

$(BINDIR)\settcb.obj: settcb.h const.h

$(BINDIR)\settdlg.res: settdlg.rc settdlg.rch settdlg.dlg settcls.ico welcome.txt

