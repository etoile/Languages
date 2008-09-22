include $(GNUSTEP_MAKEFILES)/common.make

SUBPROJECTS += SmalltalkKit Support

ifeq ($(test), yes)
SUBPROJECTS += Test
endif

#
# Application
#
VERSION = 0.1
TOOL_NAME = st

${TOOL_NAME}_LANGUAGES = English

${TOOL_NAME}_OBJC_FILES = \
	main.m

MAN1_PAGES = st.1

${TOOL_NAME}_OBJCFLAGS = -std=c99 -g -Wno-unused-value
${TOOL_NAME}_LDFLAGS += -g -lgmp -lEtoileFoundation -lgnustep-gui\
   	-L/usr/local/lib
# Hacks to compile without installing the frameworks first.
${TOOL_NAME}_LDFLAGS += \
	-lSmalltalkKit -L./SmalltalkKit/SmalltalkKit.framework/Versions/Current
${TOOL_NAME}_LDFLAGS += \
	-lSmalltalkSupport -L./Support/obj

${TOOL_NAME}_CFLAGS += -Wno-implicit -g
${TOOL_NAME}_CPPFLAGS += -I./SmalltalkKit/SmalltalkKit.framework

include $(GNUSTEP_MAKEFILES)/aggregate.make
-include ../../etoile.make
include $(GNUSTEP_MAKEFILES)/tool.make
