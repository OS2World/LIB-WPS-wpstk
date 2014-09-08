
# makefile for text initialization file access functions sample code
#
# Copyright (c) WPS Toolkit Project - Christian Langanke 2000
#
# $Id: makefile,v 1.3 2003-11-02 15:04:21 cla Exp $
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
BASEDIR=..\..\..
PRJINC=$(BASEDIR)\config\project.in
!include $(BASEDIR)\config\rules.in

DESCRIPTION=text initialization file access functions sample makefile
!if [@echo $(STARTMSG)]
!endif


# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

# define what to build
STEM=_init
TARGET= $(BINDIR)\$(STEM).exe
OBJS= $(BINDIR)\$(STEM).obj
CLEANUP=$(OBJS) $(TARGET) $(TESTFILE)
TESTFILE=$(CMPDIR)\test.ini

# required libraries to link
LIBS=$(WTK_LIBRARY)

# pseudotargets (build library first, if required !)
all: $(TARGET)
        @echo $(ENDMSG)

run: $(TARGET)
        @echo ***** running sample program
        @copy test.ini $(TESTFILE) >NUL 2>&1
        @$(TARGET) $(TESTFILE)
        @echo $(ENDMSG)

debug: $(TARGET)
        @echo ***** loading sample program in debugger
        @copy test.ini $(TESTFILE) >NUL 2>&1
        @$(DBG_SETUP)
        @$(DBG) $(TARGET) $(TESTFILE)
        @echo $(ENDMSG)

clean:
        @echo About to cleanup the target files ...
        -@for %%a in ($(CLEANUP)) do @(echo - %%a & del %%a /N $(TO_NUL))
        @echo Done.
        @echo $(ENDMSG)


# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

# build the test program

$(BINDIR)\$(STEM).exe: $(OBJS) $(WTK_LIBRARY)

