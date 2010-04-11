include $(GNUSTEP_MAKEFILES)/common.make


#
# Application
#
VERSION = 0.1
TOOL_NAME = sttool

${TOOL_NAME}_LANGUAGES = English

${TOOL_NAME}_OBJC_FILES = \
	tst.m\

MAN1_PAGES = edlc.1

${TOOL_NAME}_OBJCFLAGS = -std=c99 -g -Wno-unused-value
${TOOL_NAME}_LDFLAGS += -g -lgmp -lEtoileFoundation -lgnustep-gui\
   	-L/usr/local/lib\
	-lSmalltalkSupport\
	smalltalk.optimised.o

all:: smalltalk.optimised.o

${TOOL_NAME}_CFLAGS += -Wno-implicit -g

include $(GNUSTEP_MAKEFILES)/aggregate.make
include $(GNUSTEP_MAKEFILES)/tool.make

test.bc: test.st
	@echo Compiling test.st...
	@edlc -c -f test.st

smalltalk.bc: test.bc
	@echo Linking Smalltalk...
	@llvm-link $(GNUSTEP_LOCAL_ROOT)/Library/Frameworks/LanguageKitCodeGen.framework/Versions/0/Resources/MsgSendSmallInt.bc test.bc -o smalltalk.bc

smalltalk.optimised.bc: smalltalk.bc
	@echo Optimising Smalltalk...
	cp smalltalk.bc smalltalk.optimised.bc
	#@opt -O3 smalltalk.bc -o smalltalk.optimised.bc

smalltalk.optimised.s: smalltalk.optimised.bc
	@echo Generative code from Smalltalk...
	@llc smalltalk.optimised.bc

smalltalk.optimised.o: smalltalk.optimised.s
	@echo Assembling Smalltalk...
	@gcc -c smalltalk.optimised.s -g
