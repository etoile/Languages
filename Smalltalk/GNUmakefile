include $(GNUSTEP_MAKEFILES)/common.make

SUBPROJECTS += SmalltalkKit

#
# Application
#
VERSION = 0.1
TOOL_NAME = st

#
# Resource files
#
${TOOL_NAME}_LANGUAGES = English

${TOOL_NAME}_OBJC_FILES = \
	main.m

${TOOL_NAME}_OBJCFLAGS = -std=c99 -g -Wno-unused-value
# FIXME: The -gui requirement is only there because EtoileXML needs it, which it shouldn't
${TOOL_NAME}_LDFLAGS += -g -lEtoileFoundation -lgnustep-gui\
   	-lSmalltalkKit -L./SmalltalkKit/obj
${TOOL_NAME}_CFLAGS += -Wno-implicit -g
${TOOL_NAME}_CPPFLAGS += -I./SmalltalkKit


${TOOL_NAME}_SUBPROJECTS += Support 
include $(GNUSTEP_MAKEFILES)/aggregate.make
include $(GNUSTEP_MAKEFILES)/tool.make
