#*************************************************************************
# Copyright (c) 2002 The University of Chicago, as Operator of Argonne
#     National Laboratory.
# Copyright (c) 2002 The Regents of the University of California, as
#     Operator of Los Alamos National Laboratory.
# EPICS BASE is distributed subject to a Software License Agreement found
# in file LICENSE that is included with this distribution. 
#*************************************************************************

TOP=../../../..

include $(TOP)/configure/CONFIG

INC += gdd.h
INC += gddI.h
INC += gddContainer.h
INC += gddContainerI.h
INC += gddArray.h
INC += gddArrayI.h
INC += gddScalar.h
INC += gddScalarI.h
INC += gddNewDel.h
INC += gddUtils.h
INC += gddUtilsI.h
INC += gddErrorCodes.h
INC += aitTypes.h
INC += aitConvert.h
INC += aitHelpers.h
INC += dbMapper.h
INC += gddAppTable.h
INC += gddAppFuncTable.h
INC += smartGDDPointer.h
INC += gddApps.h
INC += gddEnumStringTable.h

HTMLS += gdd.html
HTMLS += gddref.html
HTMLS += gddref2.html

GDDSRCS = gdd.cc gddTest.cc gddAppTable.cc gddNewDel.cc \
    gddAppDefs.cc aitTypes.c aitConvert.cc aitHelpers.cc  \
    gddArray.cc gddContainer.cc gddErrorCodes.cc gddUtils.cc \
    gddEnumStringTable.cc

LIBRARY = gdd

gdd_SRCS = dbMapper $(GDDSRCS)

gdd_LIBS = Com

gdd_RCS = gdd.rc

PROD_HOST += genApps
genApps_SRCS = genApps.cc $(GDDSRCS)
genApps_SYS_LIBS_WIN32 = ws2_32

PROD_LIBS = Com

PROD_HOST += aitGen
aitGen_SRCS = aitTypes.c aitGen.c

# aitGen.c doesn't compile for linux-arm at -O3 when using gcc-3.4.5
aitGen_CFLAGS_linux-arm = -O2

CLEANS += $(COMMON_DIR)/aitConvertGenerated.cc

USR_CXXFLAGS_Linux = -fno-strict-aliasing
USR_CXXFLAGS_RTEMS = -fno-strict-aliasing

include $(TOP)/configure/RULES

# Manual dependencies
#
aitConvert$(OBJ): $(COMMON_DIR)/aitConvertGenerated.cc

dbMapper$(OBJ):	$(COMMON_DIR)/gddApps.h 

# Rules for generated files
#
$(COMMON_DIR)/aitConvertGenerated.cc: $(TOOLS)/aitGen$(HOSTEXE)
	$(call PATH_FILTER, $(TOOLS)/aitGen$(HOSTEXE)) $@

$(COMMON_DIR)/gddApps.h : $(TOOLS)/genApps$(HOSTEXE)
	$(call PATH_FILTER, $(TOOLS)/genApps$(HOSTEXE)) $@

