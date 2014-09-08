
# GNUmakefile for device I/O control related functions sample code
#
# Copyright (c) WPS Toolkit Project - Christian Langanke 2000
#
# $Id: GNUmakefile,v 1.6 2006-08-15 17:15:28 cla Exp $
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

# tell we would like to have a PM application
APPTYPE=PM

# the PM console requires a separate thread
MT=1

# include configuration (compiler options),
# also defines $(BINDIR), $(INCDIR) etc.
BASEDIR=../..
PRJINC=$(BASEDIR)/config/GNUproject.in
include $(BASEDIR)/config/GNUrules.in

DESCRIPTION=PM control related functions sample code makefile
$(shell cmd /c @echo $(STARTMSG)) 


# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

# define what to build
STEM=_ctl
SRC=$(STEM).c
OBJS=$(STEM).obj
TARGET= $(STEM).exe
HLPTARGET=$(STEM).hlp
CLEANUP= $(addprefix $(BINDIR_BS)\,$(OBJS))   \
         $(addprefix $(BINDIR_BS)\,$(TARGET)) \
         $(addprefix $(BINDIR_BS)\,$(HLPTARGET)) \

# required libraries to link
LIBS=$(WTK_LIBRARY_NAME)

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

# --- implicit rule for ipf source

.SUFFIXES: .ipf .hlp

.ipf.hlp:
	ipfc $*.ipf $(BINDIR)/$*.hlp

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

# pseudotargets (build library first, if required !)
all: $(TARGET) $(HLPTARGET)
	@echo $(ENDMSG)


run: $(TARGET)
	@echo ***** running sample program
	@cmd /C $(TARGET)
	@echo $(ENDMSG)


debug: $(TARGET)
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

$(TARGET): $(OBJ) $(WTK_LIBRARY)

$(STEM).hlp: $(STEM).ipf

$(WTK_LIBRARY):

