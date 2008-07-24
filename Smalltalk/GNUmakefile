include $(GNUSTEP_MAKEFILES)/common.make

SUBPROJECTS += SmalltalkKit

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
   	-lSmalltalkKit -L./SmalltalkKit/SmalltalkKit.framework/Versions/Current\
   	-L/usr/local/lib
${TOOL_NAME}_CFLAGS += -Wno-implicit -g
${TOOL_NAME}_CPPFLAGS += -I./SmalltalkKit

LD = g++

${TOOL_NAME}_SUBPROJECTS += Support 
include $(GNUSTEP_MAKEFILES)/aggregate.make
include $(GNUSTEP_MAKEFILES)/tool.make
