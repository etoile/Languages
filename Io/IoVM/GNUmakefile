include $(GNUSTEP_MAKEFILES)/common.make

# Only for Darwin
#ADDITIONAL_CFLAGS += -mdynamic-no-pic -falign-loops=16
ADDITIONAL_LDFLAGS +=
ADDITIONAL_INCLUDE_DIRS += -Ibase -ISkipDB

SUBPROJECT_NAME = IoVM

IoVM_SUBPROJECTS = \
	base \
	SkipDB

IoVM_C_FILES = \
	IoBlock.c \
	IoMap.c \
	IoState_coros.c \
	IoCFunction.c \
	IoMessage.c \
	IoState_debug.c \
	IoCall.c \
	IoMessage_parser.c \
	IoState_eval.c \
	IoCollector.c \
	IoNumber.c \
	IoState_exceptions.c \
	IoCompiler.c \
	IoObject.c \
	IoState_flowControl.c \
	IoCoroutine.c \
	IoObject_flow.c \
	IoState_symbols.c \
	IoDate.c \
	IoObject_persistence.c \
	IoStore.c \
	IoDebugger.c \
	IoRandom.c \
	IoSystem.c \
	IoDirectory.c \
	IoRange.c \
	IoTag.c \
	IoDuration.c \
	IoSandbox.c \
	IoToken.c \
	IoDynLib.c \
	IoSeq.c \
	IoToken_parser.c \
	IoFile.c \
	IoSeq_immutable.c \
	IoVMInit.c \
	IoFile_stat.c \
	IoSeq_mutable.c \
	IoWeakLink.c \
	IoLexer.c \
	IoState.c \
	IoList.c \
	IoState_callbacks.c 

include $(GNUSTEP_MAKEFILES)/subproject.make
include $(GNUSTEP_MAKEFILES)/aggregate.make
