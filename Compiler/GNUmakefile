include $(GNUSTEP_MAKEFILES)/common.make

ifeq ($(test), yes)
SUBPROJECTS += Test
endif

#
# Application
#
VERSION = 0.1
TOOL_NAME = edlc

${TOOL_NAME}_LANGUAGES = English

${TOOL_NAME}_OBJC_FILES = \
	main.m

MAN1_PAGES = edlc.1

${TOOL_NAME}_OBJCFLAGS = -std=c99 -g -Wno-unused-value
${TOOL_NAME}_LDFLAGS += -g -lgmp -lEtoileFoundation -lgnustep-gui\
   	-L/usr/local/lib
# Hacks to compile without installing the frameworks first.
${TOOL_NAME}_LDFLAGS += \
	-lSmalltalkSupport \
	-lLanguageKit

${TOOL_NAME}_CFLAGS += -Wno-implicit -g

include $(GNUSTEP_MAKEFILES)/aggregate.make
-include ../../etoile.make
include $(GNUSTEP_MAKEFILES)/tool.make
