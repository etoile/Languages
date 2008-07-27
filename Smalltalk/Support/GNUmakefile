include $(GNUSTEP_MAKEFILES)/common.make

LIBRARY_NAME = libSmalltalkSupport

# At a future point, we could use libosspuuid (see ../GNUmakefile):
#${LIBRARY_NAME}_INCLUDE_DIRS += $(shell uuid-config --cflags)
${LIBRARY_NAME}_CPPFLAGS += -D_POSIX_C_SOURCE=2
${LIBRARY_NAME}_OBJCFLAGS += -std=c99 
${LIBRARY_NAME}_CFLAGS += -std=c99

#
# Class files
#
${LIBRARY_NAME}_OBJC_FILES = \
	BigInt.m\
	BlockClosure.m\
	NSArray+map.m\
	NSObject+log.m\
	NSString+casting.m

-include GNUmakefile.preamble
include $(GNUSTEP_MAKEFILES)/library.make
-include GNUmakefile.postamble
