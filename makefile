
# top level makefile for WPS Toolkit library
#
# Copyright (c) WPS Toolkit Project - Christian Langanke 2000
#
# $Id: makefile,v 1.29 2006-12-04 21:45:08 cla Exp $
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

!ifdef 4OS2TEST_ENV
DEL_PARAM=Y	
!else
DEL_PARAM=N	  
!endif

# --- sample module list
#     module names are the subdirectory below samples directory
VIO_SAMPLES= \
crc32 dynamic eas file ioctl mmf nls process regexp sys time tmf xbin2obj \
faccess\cfgsys faccess\init faccess\syslvl setting\vio
PM_SAMPLES=ctl pm
WPS_SAMPLES=setting\wps

# --- source module lists
#     module names are the subdirectory below source directory
LIB_SOURCES=lib
DOC_SOURCES=book


# include configuration
MAIN=1
BASEDIR=.
PRJINC=$(BASEDIR)\config\project.in
!include $(BASEDIR)\config\rules.in

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

# --- don't show logos on recursive calls

MAKE=$(MAKE) /nologo

# --- default targets

# - generic default target for building a module

!ifdef SOURCE

MODULE:
  @cd source\$(SOURCE)
  @$(MAKE) $(ARG) MAKELEVEL=1
  @echo.
  @cd $(MAKEDIR)
!endif

!ifdef SAMPLE

MODULE:
  @cd samples\$(SAMPLE)
  @$(MAKE) $(ARG) MAKELEVEL=1
  @echo.
  @cd $(MAKEDIR)
!endif

# --- these targets to give help
help:
        @type $(CFG_HELPMAIN)

targets:
        @type $(CFG_HELPTARGETS)

symbols:
        @type $(CFG_HELPSYMBOL1)
        @type $(CFG_HELPSYMBOL2)


# --- compile release version
rel:
!if "$(CFG_SUFFIX)" == ""
   @$(MAKE) all NDEBUG=1 MAKELEVEL=1
!else
   @$(MAKE) all NDEBUG=1 ST=1 MAKELEVEL=1
   @$(MAKE) all NDEBUG=1 MT=1 MAKELEVEL=1
!endif

deb:
!if "$(CFG_SUFFIX)" == ""
   @$(MAKE) all DEBUG=1 MAKELEVEL=1
!else
   @$(MAKE) all DEBUG=1 ST=1 MAKELEVEL=1
   @$(MAKE) all DEBUG=1 MT=1 MAKELEVEL=1
!endif


# --- compile everything
all: lib doc samples

# --- compile developer version, no samples
dev: lib doc

# --- compile  sources
lib:
     @for %%a in ($(LIB_SOURCES)) do @$(MAKE) $(ARG) SOURCE=%%a ARG=all MAKELEVEL=1

doc:
     @for %%a in ($(DOC_SOURCES)) do @$(MAKE) $(ARG) SOURCE=%%a ARG=all MAKELEVEL=1

view:
     @for %%a in ($(DOC_SOURCES)) do @$(MAKE) $(ARG) SOURCE=%%a ARG=view MAKELEVEL=1

# --- compile all samples
samples: lib vio pm wps

vio: lib
     @for %%a in ($(VIO_SAMPLES)) do @$(MAKE) $(ARG) SAMPLE=%%a ARG=all MAKELEVEL=1

pm: lib
     @for %%a in ($(PM_SAMPLES)) do @$(MAKE) $(ARG) SAMPLE=%%a ARG=all MAKELEVEL=1

wps: lib
     @for %%a in ($(WPS_SAMPLES)) do @$(MAKE) $(ARG) SAMPLE=%%a ARG=all MAKELEVEL=1

# run all samples
run:
     @for %%a in ($(VIO_SAMPLES)) do @$(MAKE) $(ARG) SAMPLE=%%a ARG=run MAKELEVEL=1

# some redefinition of targets
docs: doc

libs: lib

sam: samples


# this target to cleanup
clean:
        @echo About to cleanup the target directories ...
        -@for %%a in ($(DIRSTOCLEAN)) do @(echo - %%a & del %%a\* /$(DEL_PARAM) $(TO_NUL) & rd %%a $(TO_NUL))
        @echo Done.

