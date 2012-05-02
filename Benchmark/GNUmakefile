include $(GNUSTEP_MAKEFILES)/common.make


#
# Application
#
VERSION = 0.1
TOOL_NAME = benchmark

${TOOL_NAME}_LANGUAGES = English

${TOOL_NAME}_OBJC_FILES = \
	main.m\

${TOOL_NAME}_OBJCFLAGS = -std=c99 -g -Wno-unused-value\
	-fno-objc-legacy-dispatch
${TOOL_NAME}_LDFLAGS += -g -lgmp -lEtoileFoundation -lgnustep-gui\
   	-L/usr/local/lib\
	-lSmalltalkSupport\
	-lLanguageKitRuntime\
	smalltalk.optimised.o 

#ADDITIONAL_OBJCFLAGS +=  -march=i686

${TOOL_NAME}_CFLAGS += -Wno-implicit -g -O0

#SMALLTALK_FILES += prototype.st
SMALLTALK_FILES += fib.st

all:: smalltalk.optimised.o

clean::
	rm -f smalltalk.optimised.* *.bc

include $(GNUSTEP_MAKEFILES)/aggregate.make
include $(GNUSTEP_MAKEFILES)/tool.make

smalltalk.optimised.o: ${SMALLTALK_FILES}
	@sh compile.sh ${SMALLTALK_FILES}
