include $(GNUSTEP_MAKEFILES)/common.make

SUBPROJECT_NAME = libSmalltalkSupport

# At a future point, we could use libosspuuid (see ../GNUmakefile):
#${SUBPROJECT_NAME}_INCLUDE_DIRS += $(shell uuid-config --cflags)
${SUBPROJECT_NAME}_CPPFLAGS += -D_POSIX_C_SOURCE=2
${SUBPROJECT_NAME}_OBJCFLAGS += -std=c99 
${SUBPROJECT_NAME}_CFLAGS += -std=c99
${SUBPROJECT_NAME}_LDFLAGS += -lgmp -L/usr/local/lib

#
# Class files
#
${SUBPROJECT_NAME}_OBJC_FILES = \
	BigInt.m\
	NSObject+log.m\
	BlockClosure.m\
	NSArray+map.m

-include GNUmakefile.preamble
include $(GNUSTEP_MAKEFILES)/subproject.make
-include GNUmakefile.postamble
