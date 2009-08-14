include $(GNUSTEP_MAKEFILES)/common.make

# We reset PROJECT_DIR provided by etoile.make to match the subproject since 
# etoile.make doesn't detect and handle such embedded project
PROJECT_DIR = $(CURDIR)

VERSION = 1.0
BUNDLE_NAME = LowerIfResponds
BUNDLE_EXTENSION = .ast
BUNDLE_INSTALL_DIR = $(GNUSTEP_BUNDLES)/LanguageKit

${BUNDLE_NAME}_PRINCIPAL_CLASS = LKLowerIfRespondsTransform

${BUNDLE_NAME}_OBJC_FILES = \
	transform.m

${BUNDLE_NAME}_OBJCFLAGS = -std=c99 -g -Wno-unused-value
${BUNDLE_NAME}_LDFLAGS += -g -lEtoileFoundation -lLanguageKit 
${BUNDLE_NAME}_CFLAGS += -Wno-implicit -g

include $(GNUSTEP_MAKEFILES)/bundle.make
-include ../../../etoile.make

