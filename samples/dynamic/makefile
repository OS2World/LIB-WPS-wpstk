
# makefile for dynamic linkage sample
#
# Copyright (c) WPS Toolkit Project - Christian Langanke 2000
#
# $Id: makefile,v 1.7 2002-12-04 23:22:21 cla Exp $
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

DESCRIPTION=dynamic linkage sample makefile
!if [@echo $(STARTMSG)]
!endif

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

# define what to build
STEM=_dynamic
TARGET= $(BINDIR)\$(STEM).exe
OBJS= $(BINDIR)\$(STEM).obj
CLEANUP=$(OBJS) $(TARGET)

# required libraries to link
LIBS=$(WTK_IMPLIBRARY)

# pseudotargets (build library first, if required !)
all: $(TARGET)
        @echo $(ENDMSG)

run: $(TARGET)
        @echo ***** running sample program
        @cd $(LIBDIR)
        @$(TARGET)
        @echo $(ENDMSG)

debug: $(TARGET)
        @echo ***** loading sample program in debugger
        @cd $(LIBDIR)
        @$(DBG_SETUP);..\..\samples\dynamic;
        @$(DBG) $(TARGET) $(BINDIR)
        @echo $(ENDMSG)

clean:
        @echo About to cleanup the target files ...
        -@for %%a in ($(CLEANUP)) do @(echo - %%a & del %%a /N $(TO_NUL))
        @echo Done.
        @echo $(ENDMSG)


# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

# build the test program

$(BINDIR)\$(STEM).exe: $(OBJS) $(WTK_IMPLIBRARY)

