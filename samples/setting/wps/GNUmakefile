
# GNU makefile for settings and details manager WPS sample code
#
# Copyright (c) WPS Toolkit Project - Christian Langanke 2000
#
# $Id: GNUmakefile,v 1.16 2008-12-14 23:11:49 cla Exp $
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
RES=settdlg.res
SCPDIR=$(BINDIR)
RESDIR=$(BINDIR)
PMPRINTF=1

# for using SOM with Innotek GCC, certain
# headers and symbols are required
SOMDEF=1

BASEDIR=../../..
PRJINC=$(BASEDIR)/config/GNUproject.in
include $(BASEDIR)/config/GNUrules.in

DESCRIPTION=settings and details manager WPS sample code makefile
$(shell cmd /c @echo $(STARTMSG))


# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

# define what to build
TARGET=settcls.ih settcls.dll
OBJS=settcls.obj \
     settdlg.obj \
     settcb.obj
DEF=settcls.def
CLEANUP=$(addprefix $(BINDIR_BS)\,$(OBJS))   \
        $(BINDIR_BS)\settdlg.res             \
        $(BINDIR_BS)\settcls.def             \
        $(BINDIR_BS)\settcls.h               \
        $(BINDIR_BS)\settcls.ih              \
        $(BINDIR_BS)\settcls.dll


# required libraries to link
LIBS=$(WTK_LIBRARY_NAME) somtk os2386

# pseudotargets (build library first, if required !)
all: $(TARGET)
	@echo $(ENDMSG)


som: settcls.ih settcls.h settcls.def
	@echo $(ENDMSG)


clean:
	@echo About to cleanup the target files ...
	-@for %a in ($(CLEANUP)) do @(echo - %a & del %a /N $(TO_NUL))
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

settcls.dll: settcls.ih settcls.c settcls.def settdlg.res $(OBJS) $(WTK_LIBRARY)

settcls.ih: settcls.idl

# this one to let "make -n" run through without existing files
settcls.def: settcls.idl

# this one to let "make -n" run through without existing files
settcls.h: settcls.idl

# explicit rule
settcls.obj: settcls.c const.h settcls.h settcls.ih

settdlg.obj: settdlg.h settdlg.rch const.h

settcb.obj: settcb.h const.h

settdlg.res: settdlg.rc settdlg.rch settdlg.dlg settcls.ico welcome.txt

$(WTK_LIBRARY):

