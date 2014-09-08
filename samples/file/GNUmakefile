
# GNUmakefile for file and directory functions sample code
#
# Copyright (c) WPS Toolkit Project - Christian Langanke 2000
#
# $Id: GNUmakefile,v 1.9 2006-08-15 17:15:31 cla Exp $
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

# include configuration (compiler options),
# also defines $(BINDIR), $(INCDIR) etc.
BASEDIR=../..
PRJINC=$(BASEDIR)/config/GNUproject.in
include $(BASEDIR)/config/GNUrules.in

DESCRIPTION=file and directory functions sample code makefile
$(shell cmd /c @echo $(STARTMSG)) 


# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

# define what to build
STEM=_file
SRC=$(STEM).c
OBJS=$(STEM).obj
TARGET=$(STEM).exe
CLEANUP= $(addprefix $(BINDIR_BS)\,$(OBJS))  \
         $(addprefix $(BINDIR_BS)\,$(TARGET))

# required libraries to link
LIBS=$(WTK_LIBRARY_NAME)

# pseudotargets (build library first, if required !)
all: $(TARGET)
	@echo $(ENDMSG)


run: $(TARGET)
	@echo ***** running sample program
	@cmd /C $(TARGET) $(BINDIR)
	@echo $(ENDMSG)


debug: $(TARGET)
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

$(TARGET): $(OBJ) $(WTK_LIBRARY)

$(WTK_LIBRARY):

