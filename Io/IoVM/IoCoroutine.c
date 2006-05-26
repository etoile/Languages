/*#io
Map ioDoc(
          docCopyright("Steve Dekorte", 2002)
          docLicense("BSD revised")    
          docDescription("Object wrapper for an Io coroutine.")
		docCategory("Core")
         */

#include "IoCoroutine.h"
#include "IoObject.h"
#include "IoState.h"
#include "IoCFunction.h"
#include "IoSeq.h"
#include "IoNumber.h"
#include "IoList.h"
#include "IoBlock.h"

//#define DEBUG

#define DATA(self) ((IoCoroutineData *)IoObject_dataPointer(self))

IoCoroutine *IoMessage_locals_coroutineArgAt_(IoMessage *self, void *locals, int n)
{
	IoObject *v = IoMessage_locals_valueArgAt_(self, (IoObject *)locals, n);
	if (!ISCOROUTINE(v)) IoMessage_locals_numberArgAt_errorForType_(self, (IoObject *)locals, n, "Coroutine");
	return v;
}

void IoCoroutine_writeToStore_stream_(IoCoroutine *self, IoStore *store, BStream *stream)
{
	//IoCoroutineData *data = DATA(self);
}

void IoCoroutine_readFromStore_stream_(IoCoroutine *self, IoStore *store, BStream *stream)
{
	//IoCoroutineData *data = DATA(self);
}

IoTag *IoCoroutine_tag(void *state)
{
	IoTag *tag = IoTag_newWithName_("Coroutine");
	tag->state = state;
	tag->freeFunc  = (TagFreeFunc *)IoCoroutine_free;
	tag->cloneFunc = (TagCloneFunc *)IoCoroutine_rawClone;
	tag->markFunc  = (TagMarkFunc *)IoCoroutine_mark;
	tag->writeToStoreOnStreamFunc  = (TagWriteToStoreOnStreamFunc *)IoCoroutine_writeToStore_stream_;
	tag->readFromStoreOnStreamFunc = (TagReadFromStoreOnStreamFunc *)IoCoroutine_readFromStore_stream_;
	return tag;
}

IoCoroutine *IoCoroutine_proto(void *state)
{	
	IoObject *self = IoObject_new(state);
	
	self->tag = IoCoroutine_tag(state);
	IoObject_setDataPointer_(self, calloc(1, sizeof(IoCoroutineData)));
	DATA(self)->ioStack = Stack_new();
	
	IoState_registerProtoWithFunc_((IoState *)state, self, IoCoroutine_proto);
	
	// init Coroutine proto's coro as the main one
	{
	Coro *coro = Coro_new();
	DATA(self)->cid = coro;
	Coro_initializeMainCoro(coro);
	}

	return self;
}

void IoCoroutine_protoFinish(IoCoroutine *self)
{
	IoMethodTable methodTable[] = {
	{"ioStack", IoCoroutine_ioStack},
	{"run", IoCoroutine_run},
	{"main", IoCoroutine_main},
	{"resume", IoCoroutine_resume},
	{"isCurrent", IoCoroutine_isCurrent},
	{"currentCoroutine", IoCoroutine_currentCoroutine},
	{"implementation", IoCoroutine_implementation},
	{"setMessageDebugging", IoCoroutine_setMessageDebugging},
	{NULL, NULL},
	};
	
	IoObject_addMethodTable_(self, methodTable);
}

IoCoroutine *IoCoroutine_rawClone(IoCoroutine *proto) 
{ 
	IoObject *self = IoObject_rawClonePrimitive(proto);
	IoObject_setDataPointer_(self, calloc(1, sizeof(IoCoroutineData)));
	DATA(self)->ioStack = Stack_new();
	DATA(self)->cid = (Coro *)0x0;
	return self; 
}

IoCoroutine *IoCoroutine_new(void *state)
{
	IoObject *proto = IoState_protoWithInitFunction_((IoState *)state, IoCoroutine_proto);
	IoObject *self = IOCLONE(proto);
	return self;
}

void IoCoroutine_free(IoCoroutine *self) 
{
	Coro *coro = DATA(self)->cid;
	if (coro) Coro_free(coro);
	Stack_free(DATA(self)->ioStack);
	free(DATA(self));
}

void IoCoroutine_mark(IoCoroutine *self) 
{
	//printf("Coroutine_%p mark\n", (void *)self);
	Stack_do_(DATA(self)->ioStack, (ListDoCallback *)IoObject_shouldMark);
}

// raw

Stack *IoCoroutine_rawIoStack(IoCoroutine *self)
{ 
	return DATA(self)->ioStack; 
}

void IoCoroutine_rawShow(IoCoroutine *self)
{
	Stack_do_(DATA(self)->ioStack, (StackDoCallback *)IoObject_show);
	printf("\n");
}

void *IoCoroutine_cid(IoObject *self)
{
	return DATA(self)->cid;
}

// runTarget

void IoCoroutine_rawSetRunTarget_(IoObject *self, IoObject *v)
{
	IoObject_setSlot_to_(self, IOSYMBOL("runTarget"), v);
}

IoObject *IoCoroutine_rawRunTarget(IoObject *self)
{
	return IoObject_rawGetSlot_(self, IOSYMBOL("runTarget"));
}

// runMessage

void IoCoroutine_rawSetRunMessage_(IoObject *self, IoObject *v)
{
	IoObject_setSlot_to_(self, IOSYMBOL("runMessage"), v);
}

IoObject *IoCoroutine_rawRunMessage(IoObject *self)
{
	return IoObject_rawGetSlot_(self, IOSYMBOL("runMessage"));
}

// runLocals

void IoCoroutine_rawSetRunLocals_(IoObject *self, IoObject *v)
{
	IoObject_setSlot_to_(self, IOSYMBOL("runLocals"), v);
}

IoObject *IoCoroutine_rawRunLocals(IoObject *self)
{
	return IoObject_rawGetSlot_(self, IOSYMBOL("runLocals"));
}

// parent

void IoCoroutine_rawSetParentCoroutine_(IoObject *self, IoObject *v)
{
	IoObject_setSlot_to_(self, IOSYMBOL("parentCoroutine"), v);
}

IoObject *IoCoroutine_rawParentCoroutine(IoObject *self)
{
	return IoObject_getSlot_(self, IOSYMBOL("parentCoroutine"));
}

// result

void IoCoroutine_rawSetResult_(IoObject *self, IoObject *v)
{
	IoObject_setSlot_to_(self, IOSYMBOL("result"), v);
}

IoObject *IoCoroutine_rawResult(IoObject *self)
{
	return IoObject_getSlot_(self, IOSYMBOL("result"));
}

// exception

void IoCoroutine_rawRemoveException(IoObject *self)
{
	IoObject_removeSlot_(self, IOSYMBOL("exception"));
}

void IoCoroutine_rawSetException_(IoObject *self, IoObject *v)
{
	IoObject_setSlot_to_(self, IOSYMBOL("exception"), v);
}

IoObject *IoCoroutine_rawException(IoObject *self)
{
	return IoObject_getSlot_(self, IOSYMBOL("exception"));
}

// ioStack

IoObject *IoCoroutine_ioStack(IoCoroutine *self, IoObject *locals, IoMessage *m)
{ 
	/*#io
	docSlot("ioStack", "Returns List of values on this coroutine's stack.")
	*/
	
	return IoList_newWithList_(IOSTATE, Stack_asList(DATA(self)->ioStack));
}

void IoCoroutine_rawReturnToParent(IoCoroutine *self)
{
	IoCoroutine *parent = IoCoroutine_rawParentCoroutine(self);
		  	
	if (parent && ISCOROUTINE(parent))
	{
		IoCoroutine_rawResume(parent);
	}
	else
	{
		if (self == IOSTATE->mainCoroutine)
		{
			printf("IoCoroutine error: attempt to return from main coro\n");
			exit(-1);
		}
	}

	if (!ISNIL(IoCoroutine_rawException(self)))
	{
		IoCoroutine_rawPrintBackTrace(self);
	}
	
	printf("IoCoroutine error: unable to auto abort coro %p\n", (void *)self);
	exit(-1);
}

void IoCoroutine_coroStart(void *context) // Called by Coro_Start()
{
	IoCoroutine *self = (IoCoroutine *)context;
	IoObject *result;

	IoState_setCurrentCoroutine_(IOSTATE, self);
	//printf("IoCoroutine starting %i\n", (int)self);
	
	result = IoMessage_locals_performOn_(IOSTATE->mainMessage, self, self);
	
	IoCoroutine_rawSetResult_(self, result);
	IoCoroutine_rawReturnToParent(self);
}

IoObject *IoCoroutine_main(IoObject *self, IoObject *locals, IoMessage *m)
{
	IoObject *runTarget  = IoCoroutine_rawRunTarget(self);
	IoObject *runLocals  = IoCoroutine_rawRunLocals(self);
	IoObject *runMessage = IoCoroutine_rawRunMessage(self);
	
	if (runTarget && runLocals && runMessage)
	{
		return IoMessage_locals_performOn_(runMessage, runLocals, runTarget);
	}
	else
	{
		printf("IoCoroutine_main()  missing needed parameters\n");
	}

	return IONIL(self);
}

Coro *IoCoroutine_rawCoro(IoObject *self)
{
	return DATA(self)->cid;
}


void IoCoroutine_clearStack(IoObject *self)
{
   Stack_clear(DATA(self)->ioStack);
}

void IoCoroutine_rawRun(IoObject *self)
{	
	Coro *coro = DATA(self)->cid;
	//Stack_clear(DATA(self)->ioStack);

	if (coro)
	{
		//printf("error: attempt to run same coro %p twice\n", (void *)self);
		//exit(-1);
	}
	else
	{
		coro = Coro_new();
		DATA(self)->cid = coro;	
	}
	
	{
		IoCoroutine *current = IoState_currentCoroutine(IOSTATE);
		Coro *currentCoro = IoCoroutine_rawCoro(current);
		Coro_startCoro_(currentCoro, coro, self, (CoroStartCallback *)IoCoroutine_coroStart);
		//IoState_setCurrentCoroutine_(IOSTATE, current);
	}
}

IoObject *IoCoroutine_run(IoObject *self, IoObject *locals, IoMessage *m)
{ 
	IoCoroutine_rawRun(self);
	return IoCoroutine_rawResult(self);
}

void IoCoroutine_try(IoObject *self, IoObject *target, IoObject *locals, IoMessage *message)
{
	IoCoroutine *currentCoro = (IoCoroutine *)IoState_currentCoroutine((IoState *)IOSTATE);
	IoCoroutine_rawSetRunTarget_(self, target);
	IoCoroutine_rawSetRunLocals_(self, locals);
	IoCoroutine_rawSetRunMessage_(self, message);
	IoCoroutine_rawSetParentCoroutine_(self, currentCoro);
	IoCoroutine_rawRun(self); // this will pause the current coro
}

IoCoroutine *IoCoroutine_newWithTry(void *state, 
							 IoObject *target, 
							 IoObject *locals, 
							 IoMessage *message)
{
	IoCoroutine *self = IoCoroutine_new(state);
	IoCoroutine_try(self, target, locals, message);
	return self;
}

void IoCoroutine_raiseError(IoCoroutine *self, IoSymbol *description, IoMessage *m)
{
	IoObject *e = IoObject_rawGetSlot_(self, IOSYMBOL("Exception"));
	
	if (e)
	{
		e = IOCLONE(e);
		IoObject_setSlot_to_(e, IOSYMBOL("error"), description);
		if (m) IoObject_setSlot_to_(e, IOSYMBOL("caughtMessage"), m);
		IoObject_setSlot_to_(e, IOSYMBOL("coroutine"), self);
		IoCoroutine_rawSetException_(self, e);
	}
	
	IoCoroutine_rawReturnToParent(self);
}

// methods

IoObject *IoCoroutine_rawResume(IoObject *self)
{ 
	IoCoroutine *current = IoState_currentCoroutine(IOSTATE);
	IoState_setCurrentCoroutine_(IOSTATE, self);
	//printf("IoCoroutine resuming %i\n", (int)self);
	Coro_switchTo_(IoCoroutine_rawCoro(current), IoCoroutine_rawCoro(self));
	
	//IoState_setCurrentCoroutine_(IOSTATE, current);
	return self;
}

IoObject *IoCoroutine_resume(IoObject *self, IoObject *locals, IoMessage *m)
{ 
	//printf("IoCoroutine_resume()\n");
	return IoCoroutine_rawResume(self);
}

IoObject *IoCoroutine_implementation(IoObject *self, IoObject *locals, IoMessage *m)
{ 
	return IOSYMBOL(CORO_IMPLEMENTATION);
}

IoObject *IoCoroutine_isCurrent(IoObject *self, IoObject *locals, IoMessage *m)
{ 
	IoObject *v = IOBOOL(self, self == IoState_currentCoroutine(IOSTATE));
	return v;
}

IoObject *IoCoroutine_currentCoroutine(IoObject *self, IoObject *locals, IoMessage *m)
{ 
	return IoState_currentCoroutine(IOSTATE);
}

// stack trace

int IoCoroutine_rawIoStackSize(IoObject *self)
{
	return Stack_count(DATA(self)->ioStack);
}

void IoCoroutine_rawPrint(IoObject *self)
{
	Coro *coro = DATA(self)->cid;
	
	if (coro)
	{
		printf("Coroutine_%p with cid %p ioStackSize %i\n", 
			  (void *)self, 
			  (void *)coro, 
			  (int)Stack_count(DATA(self)->ioStack));
	}
}

// debugging

int IoCoroutine_rawDebuggingOn(IoObject *self)
{
	return DATA(self)->debuggingOn;
}

IoObject *IoCoroutine_setMessageDebugging(IoObject *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("setMessageDebugging(aBoolean)", "Turns on message level debugging for this coro. When on, this 
coro will send a vmWillSendMessage message to the Debugger object before 
each message send and pause itself. See the Debugger object documentation 
for more information. ")
	*/
	IoObject *v = IoMessage_locals_valueArgAt_(m, locals, 0);
	
	DATA(self)->debuggingOn = ISTRUE(v);
	IoState_updateDebuggingMode(IOSTATE);
	
	return self;
}

IoObject *IoObject_performWithDebugger(IoObject *self, IoObject *locals, IoMessage *m)
{
	IoState *state = IOSTATE;
	IoObject *currentCoroutine = IoState_currentCoroutine(state);
	
	if (IoCoroutine_rawDebuggingOn(currentCoroutine))
	{
		if (state->debugger)
		{
			/*
			 printf("IoObject_performWithDebugger(%i, %p, %s)\n", 
				   (int)(void *)self, (void *)locals, CSTRING(IoMessage_name(m)));
			 */
			IoMessage_setCachedArg_to_(state->vmWillSendMessage, 0, currentCoroutine);
			IoMessage_setCachedArg_to_(state->vmWillSendMessage, 1, m);
			IoMessage_setCachedArg_to_(state->vmWillSendMessage, 2, self);
			IoMessage_setCachedArg_to_(state->vmWillSendMessage, 3, locals);
			//IoObject_actorPerform(state->debugger, locals, state->vmWillSendMessage, 0);
			//IoCoroutine_rawPause(currentCoroutine);
		}
	}
	
	return IoObject_perform(self, locals, m);
}

void IoCoroutine_rawPrintBackTrace(IoObject *self)
{
	IoObject *e = IoCoroutine_rawException(self);
	IoMessage *caughtMessage = IoObject_rawGetSlot_(e, IOSYMBOL("caughtMessage"));
	
	if (IoObject_rawGetSlot_(e, IOSYMBOL("showStack"))) // sanity check
	{
		IoState_on_doCString_withLabel_(IOSTATE, e, "showStack", "[Coroutine]");
	}
	else
	{
		IoSymbol *error = IoObject_rawGetSlot_(e, IOSYMBOL("error"));
		
		if (error)
		{
			fputs(CSTRING(error), stderr); 
			fputs("\n", stderr); 
		}
		else
		{
			fputs("error: [missing error slot in Exception object]\n", stderr); 
		}
		
		if (caughtMessage)
		{
			ByteArray *ba = IoMessage_asMinimalStackEntryDescription(caughtMessage);
			fputs(ByteArray_asCString(ba), stderr); 
			fputs("\n", stderr); 
			ByteArray_free(ba);
		}
	}
}

