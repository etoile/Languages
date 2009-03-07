include $(GNUSTEP_MAKEFILES)/common.make

# We reset PROJECT_DIR provided by etoile.make to match the subproject since 
# etoile.make doesn't detect and handle such embedded project
PROJECT_DIR = $(CURDIR)

VERSION = 0.1
BUNDLE_NAME = EScript
BUNDLE_EXTENSION = .language
BUNDLE_INSTALL_DIR = $(GNUSTEP_BUNDLES)/LanguageKit

${BUNDLE_NAME}_PRINCIPAL_CLASS = EScriptCompiler

${BUNDLE_NAME}_OBJC_FILES = \
	escript.m\
	EScriptCompiler.m\
	EScriptObject.m\
	EScriptParser.m\
	EScriptPreamble.m

${BUNDLE_NAME}_OBJCFLAGS = -std=c99 -g -Wno-unused-value
${BUNDLE_NAME}_LDFLAGS += -g -lEtoileFoundation -lLanguageKit
${BUNDLE_NAME}_CFLAGS += -Wno-implicit -g

include $(GNUSTEP_MAKEFILES)/bundle.make
-include ../../etoile.make

escript.m: escript.y
	@echo Generating parser...
	@lemon escript.y ; mv escript.c escript.m

clean::
	@rm -f escript.h escript.m escript.out
