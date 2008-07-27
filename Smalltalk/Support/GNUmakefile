include $(GNUSTEP_MAKEFILES)/common.make

# We reset PROJECT_DIR provided by etoile.make to match the subproject since 
# etoile.make doesn't detect and handle such embedded project
PROJECT_DIR = $(CURDIR)
# We redefine the project name since the target name doesn't match the name
# of the project directory (Support)
PROJECT_NAME = SmalltalkSupport

LIBRARY_NAME = SmalltalkSupport

LIBRARIES_DEPEND_UPON += -lgmp $(FND_LIBS) $(OBJC_LIBS) $(SYSTEM_LIBS)

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
-include ../../../etoile.make
-include GNUmakefile.postamble
