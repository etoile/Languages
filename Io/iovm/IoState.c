/*
 docCopyright("Steve Dekorte", 2002)
 docLicense("BSD revised")
 */

#define IOSTATE_C 1
#include "IoState.h"
#undef IOSTATE_C 

#include "IoObject.h"
#include "IoCall.h"
#include "IoCoroutine.h"
#include "IoSeq.h"
#include "IoNumber.h"
#include "IoCFunction.h" 
#include "IoBlock.h"
#include "IoList.h"
#include "IoMap.h"
#include "IoRange.h"
#include "IoFile.h"
#include "IoDate.h"
#include "IoDuration.h"
#include "IoSeq.h"
#include "IoMessage_parser.h"
#include "IoDynLib.h"
#include "IoWeakLink.h"

#include "IoSystem.h"
#include "IoCompiler.h"
#include "IoDebugger.h"
#include "IoCollector.h"
#include "IoRandom.h"
#include "IoSandbox.h"
#include "IoDirectory.h"

#include <stdlib.h>

void IoVMCodeInit(IoObject *context);

void IoState_setupQuickAccessSymbols(IoState *self)
{
	self->activateSymbol     = IoState_retain_(self, SIOSYMBOL("activate"));
	self->forwardSymbol	     = IoState_retain_(self, SIOSYMBOL("forward"));
	self->initSymbol	     = IoState_retain_(self, SIOSYMBOL("init"));
	self->selfSymbol	     = IoState_retain_(self, SIOSYMBOL("self"));
	self->setSlotSymbol	     = IoState_retain_(self, SIOSYMBOL("setSlot"));
	self->setSlotWithTypeSymbol  = IoState_retain_(self, SIOSYMBOL("setSlotWithType"));
	self->updateSlotSymbol   = IoState_retain_(self, SIOSYMBOL("updateSlot"));
	self->callSymbol         = IoState_retain_(self, SIOSYMBOL("call"));  
	self->typeSymbol         = IoState_retain_(self, SIOSYMBOL("type"));  
	self->opShuffleSymbol         = IoState_retain_(self, SIOSYMBOL("opShuffle"));  
	self->noShufflingSymbol       = IoState_retain_(self, SIOSYMBOL("__noShuffling__"));
}

IoState *IoState_new(void)
{
	IoCFunction *cFunctionProto;
	IoSeq *seqProto;
	IoState *self = (IoState *)calloc(1, sizeof(IoState));
	
	// collector
	
	self->collector = Collector_new();
	IoState_pushCollectorPause(self);
	
	Collector_setFreeFunc_(self->collector, (CollectorFreeFunc *)IoObject_free);
	Collector_setMarkFunc_(self->collector, (CollectorMarkFunc *)IoObject_mark);
	
	self->mainArgs   = MainArgs_new();
	self->primitives = Hash_new();
	
	self->recycledObjects = List_new();
	
	// Sandbox 
	
	self->messageCount = 0;
	self->messageCountLimit = 0;
	self->endTime = 0;
	
	self->sdbm = SkipDBM_new();
	self->symbols = SkipDBM_rootSkipDB(self->sdbm);
		
	/* 
     Problem:
	 - there are some interdependencies here:
	 - creating instances requires a retain stack
	 - we need a Coroutine to use for our retainStack
	 - defining any primitive methods requires Strings and CFunctions
	 
	 Solution:
	 - create a temporary fake stack
	 - create Object, CFunction and String protos sans methods. 
	 - then add methods to Object, CFunction and String
	 */
	
	self->currentIoStack = Stack_new(); // temp retain stack until coro is up
	
	self->objectProto = IoObject_proto(self); // need to do this first, so we have a retain stack 
	//IoState_retain_(self, self->objectProto);
	
	self->mainCoroutine = IoCoroutine_proto(self);
	Stack_free(self->currentIoStack);
	self->currentIoStack = 0x0;
	
	IoState_setCurrentCoroutine_(self, self->mainCoroutine);
	IoState_retain_(self, self->mainCoroutine);
	
	IoState_addValue_(self, self->objectProto); // to put objectProto into an ObjectGroup
	
	seqProto = IoSeq_proto(self);
	
	IoState_setupQuickAccessSymbols(self);
	
	IoObject_rawSetProto_(seqProto, self->objectProto); 
	
	cFunctionProto = IoCFunction_proto(self);
	self->localsUpdateSlotCFunc = IoState_retain_(self, 
										 IoCFunction_newWithFunctionPointer_tag_name_(self, IoObject_localsUpdateSlot, 0x0, "localsUpdate"));
	
	IoSeq_protoFinish(seqProto);
	IoObject_protoFinish(self);
	IoCFunction_protoFinish(self);
	IoCoroutine_protoFinish(self->mainCoroutine);
	
	self->setSlotBlock  = IoState_retain_(self, IoObject_getSlot_(self->objectProto, SIOSYMBOL("setSlot")));  
	
	// setup lobby
	
	{
		IoObject *objectProto = self->objectProto; 
		IoObject *protos = IOCLONE(objectProto);
		IoObject *vm     = IOCLONE(objectProto);
		
		self->lobby      = IOCLONE(objectProto);
		IoState_retain_(self, self->lobby);
		
		// setup namespace
		
		IoObject_setSlot_to_(self->lobby, SIOSYMBOL("Lobby"), self->lobby);
		IoObject_setSlot_to_(self->lobby, SIOSYMBOL("Protos"), protos);  
		IoObject_setSlot_to_(protos, SIOSYMBOL("IoVM"), vm); 
		
		IoObject_setSlot_to_(vm, SIOSYMBOL("Compiler"),  IoCompiler_proto(self));
		IoObject_setSlot_to_(vm, SIOSYMBOL("Collector"), IoCollector_proto(self));
		IoObject_setSlot_to_(vm, SIOSYMBOL("Exception"), IOCLONE(objectProto));
		
		// setup proto chain
		
		IoObject_rawSetProto_(objectProto, self->lobby); 
		IoObject_rawSetProto_(self->lobby, protos); 
		IoObject_rawSetProto_(protos, vm); 
		
		// add protos to namespace
		
		IoObject_setSlot_to_(vm, SIOSYMBOL("Object"), objectProto);
		IoObject_setSlot_to_(vm, SIOSYMBOL("Sequence"), seqProto);
		IoObject_setSlot_to_(vm, SIOSYMBOL("Number"), IoNumber_proto(self)); 
		
		IoState_setupCachedNumbers(self);

		IoObject_setSlot_to_(vm, SIOSYMBOL("Random"), IoRandom_proto(self)); 
		
		{
			IoObject *systemProto = IoSystem_proto(self);
			IoObject_setSlot_to_(vm, SIOSYMBOL("System"), systemProto);
			IoObject_setSlot_to_(systemProto, SIOSYMBOL("installPrefix"), SIOSYMBOL(INSTALL_PREFIX));
		}
		
		// nil
		
		self->ioNil = IOCLONE(objectProto);
		IoObject_setSlot_to_(vm, SIOSYMBOL("Nil"), self->ioNil);
		IoObject_setSlot_to_(vm, SIOSYMBOL("nil"), self->ioNil);
		IoObject_setSlot_to_(vm, self->noShufflingSymbol, self->ioNil);
		IoObject_setSlot_to_(vm, SIOSYMBOL("Message"), IoMessage_proto(self));
		IoObject_setSlot_to_(vm, SIOSYMBOL("Call"),  IoCall_proto(self));
		
		self->nilMessage  = IoMessage_newWithName_(self, SIOSYMBOL("nil"));
		IoMessage_cachedResult_(self->nilMessage, self->ioNil);
		IoState_retain_(self, self->nilMessage);
		
		// true 
		
		self->ioTrue = IoObject_new(self);
		IoObject_setSlot_to_(vm, SIOSYMBOL("true"), self->ioTrue);
		IoObject_setSlot_to_(self->ioTrue, SIOSYMBOL("type"), SIOSYMBOL("true"));
		IoState_retain_(self, self->ioTrue);
		
		// false
		
		self->ioFalse = IoObject_new(self);
		IoObject_setSlot_to_(vm, SIOSYMBOL("false"), self->ioFalse);
		IoObject_setSlot_to_(self->ioFalse, SIOSYMBOL("type"), SIOSYMBOL("false"));
		IoState_retain_(self, self->ioFalse);
		
		// cached messages

		self->collectedLinkMessage  = IoMessage_newWithName_(self, SIOSYMBOL("collectedLink"));
		IoState_retain_(self, self->collectedLinkMessage);
				
		self->printMessage  = IoMessage_newWithName_(self, SIOSYMBOL("print"));
		IoState_retain_(self, self->printMessage);
		
		self->initMessage   = IoMessage_newWithName_(self, SIOSYMBOL("init"));
		IoState_retain_(self, self->initMessage);
		
		self->compareMessage = IoMessage_newWithName_(self, SIOSYMBOL("compare"));
		IoState_retain_(self, self->compareMessage);
		
		self->runMessage = IoMessage_newWithName_(self, SIOSYMBOL("run"));
		IoState_retain_(self, self->runMessage);
		
		self->mainMessage = IoMessage_newWithName_(self, SIOSYMBOL("main"));
		IoState_retain_(self, self->mainMessage);

                self->opShuffleMessage = IoMessage_newWithName_(self, self->opShuffleSymbol);
                IoState_retain_(self, self->opShuffleMessage);
		
		{
			self->debugger = IoState_retain_(self, IoDebugger_proto(self));
			IoObject_setSlot_to_(vm, SIOSYMBOL("Debugger"), self->debugger);
			
			self->vmWillSendMessage  = IoMessage_newWithName_(self, SIOSYMBOL("vmWillSendMessage"));
			IoMessage_cachedResult_(self->nilMessage, self->ioNil);
			IoState_retain_(self, self->vmWillSendMessage);
		}
		
		IoObject_setSlot_to_(vm, SIOSYMBOL("Block"),      IoBlock_proto(self));
		IoObject_setSlot_to_(vm, SIOSYMBOL("List"),       IoList_proto(self));
		IoObject_setSlot_to_(vm, SIOSYMBOL("Map"),        IoMap_proto(self));
        IoObject_setSlot_to_(vm, SIOSYMBOL("Range"),      IoRange_proto(self));
		IoObject_setSlot_to_(vm, SIOSYMBOL("Coroutine"),  self->mainCoroutine);
		IoObject_setSlot_to_(vm, SIOSYMBOL("File"),       IoFile_proto(self));
		IoObject_setSlot_to_(vm, SIOSYMBOL("Directory"),  IoDirectory_proto(self));
		IoObject_setSlot_to_(vm, SIOSYMBOL("Date"),       IoDate_proto(self));
		IoObject_setSlot_to_(vm, SIOSYMBOL("Duration"),   IoDuration_proto(self));
		IoObject_setSlot_to_(vm, SIOSYMBOL("WeakLink"),   IoWeakLink_proto(self));
		IoObject_setSlot_to_(vm, SIOSYMBOL("Sandbox"),    IoSandbox_proto(self));
		
#if !defined(__SYMBIAN32__)
		IoObject_setSlot_to_(vm, SIOSYMBOL("DynLib"),     IoDynLib_proto(self));
#endif
		
		self->store = IoStore_proto(self);		
		IoObject_setSlot_to_(vm, SIOSYMBOL("Store"),      self->store);
		IoObject_setSlot_to_(vm, SIOSYMBOL("CFunction"),  cFunctionProto);
		
		self->localsProto = IoState_retain_(self, IoObject_localsProto(self));
		IoObject_setSlot_to_(vm, SIOSYMBOL("Locals"),  self->localsProto);
		
		self->stopStatus = MESSAGE_STOP_STATUS_NORMAL;
		self->returnValue = (void *)0x0;
						
		IoState_clearRetainStack(self);
		IoState_popCollectorPause(self);
		
		IoVMCodeInit(vm);
		IoState_clearRetainStack(self);
	}
	
	return self;
}

IoObject *IoObject_initBindings(IoObject *self, IoObject *locals, IoMessage *m)
{
	IOSTATE->bindingsInitCallback(IOSTATE, self);
	return self;
}

void IoState_init(IoState *self)
{
	if (self->bindingsInitCallback)
	{
		IoObject *vm = IoObject_getSlot_(self->lobby, SIOSYMBOL("IoVM"));
		
		IoState_pushCollectorPause(self);
		self->bindingsInitCallback(self, vm);
		IoState_popCollectorPause(self);
		IoState_clearRetainStack(self);
	}
}

void IoState_registerProtoWithFunc_(IoState *self, IoObject *proto, IoStateProtoFunc *func)
{ 
	if (Hash_at_(self->primitives, (void *)func))
	{
		IoState_fatalError_(self, "IoState_registerProtoWithFunc_() Error: attempt to add the same proto twice");
	}
	
	IoState_retain_(self, proto);
	Hash_at_put_(self->primitives, (void *)func, proto); 
	//printf("registered %s\n", IoObject_name(proto));
}

IoObject *IoState_protoWithName_(IoState *self, const char *name)
{
	IoObject *proto = Hash_firstValue(self->primitives);
	
	while (proto)
	{
		if (!strcmp(IoObject_name(proto), name)) 
		{
			return proto;
		}
		
		proto = Hash_nextValue(self->primitives);
	}
	
	return 0x0;
}

List *IoState_tagList(IoState *self) // caller must free returned List 
{
	List *tags = List_new();
	void *k = Hash_firstKey(self->primitives);
	
	while (k)
	{
		IoObject *proto = Hash_at_(self->primitives, k);
		List_append_(tags, proto->tag);
		k = Hash_nextKey(self->primitives);
	}
	
	return tags;
}

void IoState_free(IoState *self)
{	
	// this should only be called from the main coro from outside of Io
	
	/*
	Collector_removeAllRetainedValues(self->collector);
	Collector_setMarkBeforeSweepValue_(self->collector, 0x0);
	Collector_collect(self->collector);
	Collector_collect(self->collector); // needed?
	*/
	
	Collector_freeAllValues(self->collector); // free all object known to the collector
	Collector_free(self->collector);
	
	{
	List *tags = IoState_tagList(self);
	List_do_(tags, (ListDoCallback *)IoTag_free);
	List_free(tags);
	}
	
	Hash_free(self->primitives);

	SkipDBM_free(self->sdbm);
	self->sdbm    = 0x0;
	self->symbols = 0x0;
	
	List_free(self->recycledObjects);
	List_free(self->cachedNumbers);
	
	MainArgs_free(self->mainArgs);
	self->mainArgs = 0x0;
	
	free(self);
}

IoObject *IoState_lobby(IoState *self) 
{ 
	return self->lobby; 
}

void IoState_setLobby_(IoState *self, IoObject *obj) 
{ 
	self->lobby = obj;
}

IoObject *IoState_protoWithInitFunction_(IoState *self, IoStateProtoFunc *func)
{ 
	IoObject *proto = Hash_at_(self->primitives, (void *)func); 
	
	if (!proto)
	{
		IoState_fatalError_(self, "IoState_protoWithInitFunction_() Error: missing proto");
	}
	
	return proto;
}


// command line ------------------------------------------------

void IoState_argc_argv_(IoState *self, int argc, const char *argv[])
{
	IoList *args = IoList_new(self);
	int i;
	
	for (i = 1; i < argc; i ++)
	{
		IoList_rawAppend_(args, SIOSYMBOL(argv[i]));
	}
	
	IoObject_setSlot_to_(self->lobby, SIOSYMBOL("args"), args);
	
	MainArgs_argc_argv_(self->mainArgs, argc, argv); 
}

// store -------------------------------------------------------

IoStore *IoState_store(IoState *self)
{ 
	return self->store; 
}

IoObject *IoState_rawOn_doCString_withLabel_(IoState *self, 
									IoObject *target,
									const char *s,
									const char *label)
{
	IoMessage *m = IoMessage_newFromText_label_(self, s, label);
	return IoMessage_locals_performOn_(m, self->lobby, self->lobby);
}

// CLI ---------------------------------------------------------

void IoState_rawPrompt(IoState *self)
{
	int max = 1024 * 16;
	char *s = malloc(max);
	IoObject *result;
	
	for (;;)
	{
		fputs("Io> ", stdout);
		fflush(stdout);
		fgets(s, max, stdin);
		
		if (feof(stdin)) 
		{
			break;
		}
		
		result = IoState_rawOn_doCString_withLabel_(self, self->lobby, s, "IoState_rawPrompt()");
		
		fputs("==> ", stdout);
		IoObject_print(result);
		fputs("\n", stdout);
	}
	
	free(s);
}

void IoState_runCLI(IoState *self)
{
	//IoState_rawPrompt(self);
	IoState_on_doCString_withLabel_(self, self->lobby, "CLI run", "IoState_runCLI()");
}


