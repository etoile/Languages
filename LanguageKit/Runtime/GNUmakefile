include $(GNUSTEP_MAKEFILES)/common.make

# We reset PROJECT_DIR provided by etoile.make to match the subproject since 
# etoile.make doesn't detect and handle such embedded project
PROJECT_DIR = $(CURDIR)
# We redefine the project name since the framework name doesn't match the name
# of the project directory (Runtime)
PROJECT_NAME = LanguageKitRuntime

FRAMEWORK_NAME = ${PROJECT_NAME}

LIBRARIES_DEPEND_UPON += -lgmp $(FND_LIBS) $(OBJC_LIBS) $(SYSTEM_LIBS)

${FRAMEWORK_NAME}_CPPFLAGS += -D_POSIX_C_SOURCE=199309 -D_BSD_SOURCE
${FRAMEWORK_NAME}_OBJCFLAGS += -std=c99 -fexceptions -fobjc-exceptions
${FRAMEWORK_NAME}_CFLAGS += -std=c99 -fexceptions


${FRAMEWORK_NAME}_OBJC_FILES = \
	BigInt.m\
	BlockClosure.m\
	BoxedFloat.m\
	NSValue+structs.m\
	LanguageKitExceptions.m\
	OverflowHandler.m\
	Symbol.m

${FRAMEWORK_NAME}_OBJ_FILES = MsgSendSmallInt.o

${FRAMEWORK_NAME}_HEADER_FILES = \
	BigInt.h\
	LKObject.h\
	BlockClosure.h

include $(GNUSTEP_MAKEFILES)/framework.make
-include ../../../etoile.make
-include ../../../documentation.make

MsgSendSmallInt.o:
	@echo " Compiling small int messages..."
	@clang -c -ftrapv `gnustep-config --objc-flags` ${${FRAMEWORK_NAME}_OBJCFLAGS} MsgSendSmallInt.m -o MsgSendSmallInt.o

before-clean::
	@rm -f MsgSendSmallInt.o MsgSendSmallInt.d

before-distclean:: before-clean
