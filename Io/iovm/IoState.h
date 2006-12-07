/*#io
docCopyright("Steve Dekorte", 2002)
docLicense("BSD revised")
*/

#ifndef IOSTATE_DEFINED 
#define IOSTATE_DEFINED 1

#include "IoVMApi.h"

#include "Collector.h"
#include "Stack.h"
#include "Hash.h"
#include "MainArgs.h"
#include "IoObject_struct.h"
#include "IoSeq.h"
#include "IoVersion.h"
#include "IoStore.h"
#include "IoCoroutine.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct IoState IoState;

#include "IoState_callbacks.h"

typedef IoObject *(IoStateProtoFunc)(void *);

struct IoState
{
	// primitives

	Hash *primitives;

	// symbols

	SkipDBM *sdbm;
	SkipDB *symbols;

	// coroutines

	IoObject *objectProto;
	IoObject *mainCoroutine;	  // the object that represents the main "thread"
	IoObject *currentCoroutine; // the object whose coroutine is active 
	Stack *currentIoStack;      // quick access to current coro's retain stack 

	// quick access objects 

	IoSymbol *activateSymbol;
	IoSymbol *forwardSymbol;
	IoSymbol *initSymbol;
	IoSymbol *selfSymbol;
	IoSymbol *setSlotSymbol;
	IoSymbol *setSlotWithTypeSymbol;
	IoSymbol *callSymbol;
	IoSymbol *updateSlotSymbol;
	IoSymbol *typeSymbol;
	IoSymbol *opShuffleSymbol;
	IoSymbol *noShufflingSymbol;
	IoSymbol *semicolonSymbol;

	IoObject *setSlotBlock;
	IoObject *localsUpdateSlotCFunc;
	IoObject *localsProto;

	IoMessage *collectedLinkMessage;
	IoMessage *compareMessage;
	IoMessage *initMessage;
	IoMessage *mainMessage;
	IoMessage *nilMessage;
	IoMessage *runMessage;
	IoMessage *printMessage;
	IoMessage *opShuffleMessage;

	List *cachedNumbers;
	
	// singletons

	IoObject *ioNil;
	IoObject *ioTrue;
	IoObject *ioFalse;

	// garbage collection

	Collector *collector;
	IoObject *lobby;
	IoObject *core;

	// recycling 

	List *recycledObjects;

	// startup environment

	MainArgs *mainArgs;

	// current execution state 

	int stopStatus;
	void *returnValue;

	// embedding

	void *callbackContext;
	IoStateBindingsInitCallback *bindingsInitCallback;
	IoStatePrintCallback        *printCallback;
	IoStateExceptionCallback    *exceptionCallback;
	IoStateExitCallback         *exitCallback;
	IoStateActiveCoroCallback   *activeCoroCallback;
	//IoStateThreadLockCallback   *threadLockCallback;
	//IoStateThreadUnlockCallback *threadUnlockCallback;

	// debugger

	IoObject *debugger;
	IoMessage *vmWillSendMessage;

	// SandBox limits 

	int messageCountLimit;
	int messageCount;
	double timeLimit;
	double endTime;

	// tail calls

	IoMessage *tailCallMessage;

	// exiting

	int shouldExit;
	int exitResult;
	
	// persistence
	
	IoStore *store; 
	
	// thread message queue 
	
	//List *threadMessageQueue;
};

#define IOSTATE_STRUCT_DEFINED

// setup

IOVM_API IoState *IoState_new(void);
IOVM_API void IoState_init(IoState *self);

void IoState_setupQuickAccessSymbols(IoState *self);
void IoState_setupCachedMessages(IoState *self);
void IoState_setupSingletons(IoState *self);

// setup tags 

IOVM_API void IoState_registerProtoWithFunc_(IoState *self, IoObject *proto, IoStateProtoFunc *func);
IOVM_API IoObject *IoState_protoWithInitFunction_(IoState *self, IoStateProtoFunc *func);
IOVM_API IoObject *IoState_protoWithName_(IoState *self, const char *name);

IOVM_API void IoState_free(IoState *self);

// lobby

IOVM_API IoObject *IoState_lobby(IoState *self);
IOVM_API void IoState_setLobby_(IoState *self, IoObject *obj);

// command line 

IOVM_API void IoState_argc_argv_(IoState *st, int argc, const char *argv[]);
IOVM_API void IoState_runCLI(IoState *self);

// store

IOVM_API IoStore *IoState_store(IoState *self);

#include "IoState_coros.h"
#include "IoState_debug.h"
#include "IoState_eval.h"
#include "IoState_symbols.h"
#include "IoState_exceptions.h"
#include "IoState_inline.h"
#include "IoState_flowControl.h"

#ifdef __cplusplus
}
#endif
#endif
