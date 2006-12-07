include $(GNUSTEP_MAKEFILES)/common.make

# Only for Darwin
#ADDITIONAL_CFLAGS += -mdynamic-no-pic -falign-loops=16
ADDITIONAL_LDFLAGS +=
ADDITIONAL_INCLUDE_DIRS += \
	-I../basekit \
	-I../coroutine \
	-I../skipdb \
	-I../garbagecollector

SUBPROJECT_NAME = iovm

iovm_C_FILES = \
	IoBlock.c \
	IoCall.c \
	IoCFunction.c \
	IoCollector.c \
	IoCompiler.c \
	IoCoroutine.c \
	IoDate.c \
	IoDebugger.c \
	IoDirectory.c \
	IoDuration.c \
	IoDynLib.c \
	IoFile.c \
	IoFile_stat.c \
	IoLexer.c \
	IoList.c \
	IoMap.c \
	IoMessage.c \
	IoMessage_opShuffle.c \
	IoMessage_parser.c \
	IoNumber.c \
	IoObject.c \
	IoObject_flow.c \
	IoObject_persistence.c \
	IoRandom.c \
	IoRange.c \
	IoSandbox.c \
	IoSeq.c \
	IoSeq_immutable.c \
	IoSeq_mutable.c \
	IoState.c \
	IoState_callbacks.c \
	IoState_coros.c \
	IoState_debug.c \
	IoState_eval.c \
	IoState_exceptions.c \
	IoState_flowControl.c \
	IoState_symbols.c \
	IoStore.c \
	IoSystem.c \
	IoTag.c \
	IoToken.c \
	IoVMInit.c \
	IoWeakLink.c 

iovm_CFLAGS += -DINSTALL_PREFIX=\"$(GNUSTEP_LOCAL_ROOT)/Tools\"

include $(GNUSTEP_MAKEFILES)/subproject.make

