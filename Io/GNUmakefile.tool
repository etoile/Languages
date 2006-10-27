ifeq ($(FOUNDATION_LIB), apple)
  # For Vector
  ADDITIONAL_LDFLAGS += -framework Accelerate -faltivec
  # Only for Darwin 
  ADDITIONAL_CFLAGS += -mdynamic-no-pic -falign-loops=16
endif

#
# Runtime tool (aka ioVM)
#

ADDITIONAL_INCLUDE_DIRS += \
        -Iiovm \
        -Ibasekit \
        -Iskipdb \
        -Igarbagecollector \
        -Icoroutine \
        -Ithread \
        -IVector \
        -IObjcBridge

TOOL_NAME = ioobjc

ioobjc_SUBPROJECTS += \
	basekit \
	coroutine \
        garbagecollector \
	skipdb \
	thread \
	iovm \
	Vector \
	ObjcBridge

ioobjc_OBJC_FILES = \
	main.m

ifeq ($(FOUNDATION_LIB), apple)
else
  # in order to access gui classes from tool.
  ioobjc_OBJC_LIBS += -lgnustep-gui
endif

include $(GNUSTEP_MAKEFILES)/tool.make

