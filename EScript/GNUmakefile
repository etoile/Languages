include $(GNUSTEP_MAKEFILES)/common.make

# We reset PROJECT_DIR provided by etoile.make to match the subproject since 
# etoile.make doesn't detect and handle such embedded project
PROJECT_DIR = $(CURDIR)

VERSION = 0.1
FRAMEWORK_NAME = EScriptKit

${FRAMEWORK_NAME}_OBJC_FILES = \
	escript.m\
	EScriptCompiler.m\
	EScriptParser.m

${FRAMEWORK_NAME}_HEADER_FILES = \
	EScriptCompiler.h\
	EScriptKit.h\
	EScriptParser.h


${FRAMEWORK_NAME}_OBJCFLAGS = -std=c99 -g -Wno-unused-value
${FRAMEWORK_NAME}_LDFLAGS += -g -lEtoileFoundation -lLanguageKit
${FRAMEWORK_NAME}_CFLAGS += -Wno-implicit -g

include $(GNUSTEP_MAKEFILES)/framework.make
-include ../../etoile.make

escript.m: escript.y
	@echo Generating parser...
	@lemon escript.y ; mv escript.c escript.m

clean::
	@rm -f escript.h escript.m escript.out
