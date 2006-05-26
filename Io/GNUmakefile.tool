include $(GNUSTEP_MAKEFILES)/common.make

ifeq ($(FOUNDATION_LIB), apple)
  # For AAVector
  ADDITIONAL_LDFLAGS += -framework Accelerate -faltivec
  # Only for Darwin 
  ADDITIONAL_CFLAGS += -mdynamic-no-pic -falign-loops=16
endif

ADDITIONAL_INCLUDE_DIRS += -IIoVM -IIoVM/base -IIoVM/SkipDB -IAAVector -IObjcBridge

#
# Runtime tool (aka ioVM)
#

TOOL_NAME = ioobjc

io_SUBPROJECTS += \
	IoVM \
	AAVector \
	ObjcBridge

#
# Files
#

io_OBJC_FILES = \
	main.m

ifeq ($(FOUNDATION_LIB), apple)
else
  io_objc_OBJC_LIBS += -lgnustep-gui
endif

# MyLanguage_BUNDLE_LIBS += -lmylanguage

include $(GNUSTEP_MAKEFILES)/tool.make
include $(GNUSTEP_MAKEFILES)/aggregate.make
