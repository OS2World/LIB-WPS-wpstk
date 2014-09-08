
# makefile for file and directory functions sample code
#
# Copyright (c) WPS Toolkit Project - Christian Langanke 2000
#
# $Id: makefile,v 1.5 2002-12-04 23:22:23 cla Exp $
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
BASEDIR=..\..
PRJINC=$(BASEDIR)\config\project.in
!include $(BASEDIR)\config\rules.in

DESCRIPTION=file and directory functions sample code makefile
!if [@echo $(STARTMSG)]
!endif


# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

# define what to build
STEM=_file
TARGET= $(BINDIR)\$(STEM).exe
OBJS= $(BINDIR)\$(STEM).obj
CLEANUP=$(OBJS) $(TARGET)

# required libraries to link
LIBS=$(WTK_LIBRARY)

# pseudotargets (build library first, if required !)
all: $(TARGET)
        @echo $(ENDMSG)

run: $(TARGET)
        @echo ***** running sample program
        @$(TARGET) $(BINDIR)
        @echo $(ENDMSG)

debug: $(TARGET)
        @echo ***** loading sample program in debugger
        @$(DBG_SETUP)
        @$(DBG) $(TARGET) $(BINDIR)
        @echo $(ENDMSG)

clean:
        @echo About to cleanup the target files ...
        -@for %%a in ($(CLEANUP)) do @(echo - %%a & del %%a /N $(TO_NUL))
        @echo Done.
        @echo $(ENDMSG)


# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

# build the test program

$(BINDIR)\$(STEM).exe: $(OBJS) $(WTK_LIBRARY)

