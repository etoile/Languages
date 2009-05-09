include $(GNUSTEP_MAKEFILES)/common.make

#
# Application
#
VERSION = 0.1
TOOL_NAME = test

#
# Resource files
#
test_LANGUAGES = English

test_OBJC_FILES = \
				sync.m\
				test.m

test_C_FILES = \
				runtime.c

ADDITIONAL_CFLAGS = -std=c99 -fexceptions 
ADDITIONAL_OBJCFLAGS = -fobjc-exceptions -fexceptions

#ADDITIONAL_LDFLAGS += -lgnustep-gui -lEtoileFoundation -lSmalltalkSupport#-lScriptKit

#CC=~/llvm/tools/clang/utils/ccc

test_OBJCFLAGS += -std=c99

include $(GNUSTEP_MAKEFILES)/tool.make
