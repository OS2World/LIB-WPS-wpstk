
# makefile for the WPS Toolkit documentation
#
# Copyright (c) WPS Toolkit Project - Christian Langanke 2000
#
# $Id: makefile,v 1.12 2005-11-01 14:48:21 cla Exp $
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
BASEDIR=..\..
PRJINC=$(BASEDIR)\config\project.in
!include $(BASEDIR)\config\rules.in


DESCRIPTION=WPS Toolkit documentation makefile
!if [@echo $(STARTMSG)]
!endif

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

# define what to build
TARGET= $(BOOKDIR)\wtkref.inf
TMPFILES=$(CMPDIR)\functions.ipf $(CMPDIR)\datatypes.ipf $(BOOKDIR)\EPM\*
CLEANUP=$(TARGET)                  \
        $(CMPDIR)\functions.ipf    \
        $(CMPDIR)\datatypes.ipf    \
        $(CMPDIR)\epmkwds.c__      \
        $(CMPDIR)\wpstk.ndx        \
        $(BOOKDIR)\EPM\epmkwds.c__ \
        $(BOOKDIR)\EPM\wpstk.ndx

# pseudotargets
all:  $(TARGET)
        @echo $(ENDMSG)

clean:
        @echo About to cleanup the target files ...
        -@for %%a in ($(CLEANUP)) do @(echo - %%a & del %%a /N $(TO_NUL))
        @echo Done.
        @echo $(ENDMSG)

view: $(TARGET)
        start view $(TARGET) $(PANEL)
        @echo $(ENDMSG)

show: view

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

# build the book

$(BOOKDIR)\wtkref.inf:  wtkref.txt *.inc $(CMPDIR)\functions.ipf
        htext /N wtkref.txt $(CMPDIR)\wtkref.ipf $(BOOKDIR)\wtkref.inf

# build generated source

$(CMPDIR)\functions.ipf: $(INCDIR)\wtk.h $(LIBSRCDIR)\*.c $(INCDIR)\*.h $(EXECDIR)\srcscan.cmd datatypes\* makefile
        srcscan $(INCDIR)\wtk.h $(LIBSRCDIR) $(CMPDIR) $(INCDIR) warp4def.h /V /L
        @copy $(CMPDIR)\*.ndx $(NDXDIR) $(TO_NUL)
        @copy $(CMPDIR)\*.c__ $(NDXDIR) $(TO_NUL)

