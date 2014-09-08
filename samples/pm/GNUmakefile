
# GNUmakefile for PM helper functions sample code
#
# Copyright (c) WPS Toolkit Project - Christian Langanke 2004
#
# $Id: GNUmakefile,v 1.10 2008-12-22 18:47:01 cla Exp $
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
OBJS=$(STEM).obj _pmnb.obj _pmfddlg.obj _pmcnr.obj
TARGET=$(STEM).exe
HLPTARGET=$(STEM).hlp
RES=$(STEM).res
CLEANUP= $(addprefix $(BINDIR_BS)\,$(OBJS))   \
         $(addprefix $(BINDIR_BS)\,$(TARGET)) \
         $(addprefix $(BINDIR_BS)\,$(STEM).res) \
         $(addprefix $(BINDIR_BS)\,$(STEM).map) \
         $(addprefix $(BINDIR_BS)\,$(HLPTARGET)) \

# required libraries to link
LIBS=$(WTK_LIBRARY_NAME)

# include configuration (compiler options),
# also defines $(BINDIR), $(INCDIR) etc.
BASEDIR=../..
PRJINC=$(BASEDIR)/config/GNUproject.in
include $(BASEDIR)/config/GNUrules.in

DESCRIPTION=PM helper functions sample code makefile
$(shell cmd /c @echo $(STARTMSG))


# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

# --- implicit rule for ipf source

.SUFFIXES: .ipf .hlp

.ipf.hlp:
	ipfc $*.ipf $(BINDIR)/$*.hlp

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


# pseudotargets (build library first, if required !)
all: $(TARGET) $(HLPTARGET)
	@echo $(ENDMSG)


run: $(TARGET) $(HLPTARGET)
	@echo ***** running sample program
	@cmd /C $(TARGET)
	@echo $(ENDMSG)


debug: $(TARGET) $(HLPTARGET)
	@echo ***** loading sample program in debugger
	@$(DBG) $(BINDIR)/$(TARGET)
	@echo $(ENDMSG)


clean:
	@echo About to cleanup the target files ...
	-@for %a in ($(CLEANUP)) do @(echo - %a & del %a /N $(TO_NUL))
	@echo Done.
	@echo $(ENDMSG)



# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

# build the test program

$(TARGET): $(OBJS) $(WTK_LIBRARY)  $(STEM).res

$(STEM).res: $(STEM).rc $(STEM).dlg $(STEM).men $(STEM).rch $(STEM)eng.rch mletext.txt item.bmp

$(STEM).hlp: $(STEM).ipf

$(WTK_LIBRARY):

