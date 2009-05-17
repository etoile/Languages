include $(GNUSTEP_MAKEFILES)/common.make

# We reset PROJECT_DIR provided by etoile.make to match the subproject since 
# etoile.make doesn't detect and handle such embedded project
PROJECT_DIR = $(CURDIR)
# We redefine the project name since the target name doesn't match the name
# of the project directory (Support)
PROJECT_NAME = SmalltalkSupport

FRAMEWORK_NAME = SmalltalkSupport

LIBRARIES_DEPEND_UPON += -lgmp -lLanguageKitRuntime $(FND_LIBS) $(OBJC_LIBS) $(SYSTEM_LIBS)

${FRAMEWORK_NAME}_CPPFLAGS += -D_POSIX_C_SOURCE=2 -I../../LanguageKit/Runtime
${FRAMEWORK_NAME}_OBJCFLAGS += -std=c99 -fexceptions 
${FRAMEWORK_NAME}_CFLAGS += -std=c99 -fexceptions 

#
# Class files
#
${FRAMEWORK_NAME}_OBJC_FILES = \
	BlockClosure.m\
	NSArray+map.m\
	NSObject+log.m\
	NSString+casting.m\
	NSString+comma.m\
	NSValue+structs.m

-include GNUmakefile.preamble
include $(GNUSTEP_MAKEFILES)/framework.make
-include ../../../etoile.make
-include GNUmakefile.postamble
