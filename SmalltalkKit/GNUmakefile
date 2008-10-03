include $(GNUSTEP_MAKEFILES)/common.make

# We reset PROJECT_DIR provided by etoile.make to match the subproject since 
# etoile.make doesn't detect and handle such embedded project
PROJECT_DIR = $(CURDIR)

SUBPROJECTS = Support
#
# Library
#
VERSION = 0.6
FRAMEWORK_NAME = SmalltalkKit

${FRAMEWORK_NAME}_OBJC_FILES = \
	smalltalk.m\
	SmalltalkCompiler.m\
	SmalltalkParser.m

${FRAMEWORK_NAME}_HEADER_FILES = \
	SmalltalkCompiler.h\
	SmalltalkKit.h\
	SmalltalkParser.h


${FRAMEWORK_NAME}_OBJCFLAGS = -std=c99 -g -Wno-unused-value
${FRAMEWORK_NAME}_LDFLAGS += -g -lEtoileFoundation -lLanguageKit \
	-lSmalltalkSupport
${FRAMEWORK_NAME}_CFLAGS += -Wno-implicit -g

# LLVM flags
LLVM_LIBS=all
${FRAMEWORK_NAME}_CCFLAGS += `llvm-config --cxxflags  ${LLVM_LIBS}` -g
LIBRARIES_DEPEND_UPON += `llvm-config --ldflags --libfiles ${LLVM_LIBS}` 
LD = g++

${FRAMEWORK_NAME}_RESOURCE_FILES += MsgSendSmallInt.bc\
	ObjCConstants.plist

include $(GNUSTEP_MAKEFILES)/aggregate.make
include $(GNUSTEP_MAKEFILES)/framework.make
-include ../../etoile.make

smalltalk.m: smalltalk.y
	@echo Generating parser...
	@lemon smalltalk.y ; mv smalltalk.c smalltalk.m
