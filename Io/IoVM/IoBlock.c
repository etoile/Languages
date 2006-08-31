/*#io
getSlot("Block") ioDoc(
            docCopyright("Steve Dekorte", 2002)
            docLicense("BSD revised")
            docDescription("Blocks are anonymous functions (messages with their own locals object). They are typically used to represent object methods.")
		  docCategory("Core")
*/

#include "IoBlock.h"
#include "IoState.h"
#include "IoObject.h"
#include "IoMessage.h"
#include "IoMessage_parser.h"
#include "IoCFunction.h"
#include "IoSeq.h"
#include "IoNumber.h"
#include "IoList.h"
#include "ByteArray.h"

#define DATA(self) ((IoBlockData *)IoObject_dataPointer(self))

IoTag *IoBlock_tag(void *state)
{
	IoTag *tag = IoTag_newWithName_("Block");
	tag->state = state;
	tag->cloneFunc = (TagCloneFunc *)IoBlock_rawClone;
	tag->markFunc  = (TagMarkFunc *)IoBlock_mark;
	tag->freeFunc  = (TagFreeFunc *)IoBlock_free;
	tag->activateFunc = (TagActivateFunc *)IoBlock_activate;
	tag->writeToStoreOnStreamFunc  = (TagWriteToStoreOnStreamFunc *)IoBlock_writeToStore_stream_;
	tag->readFromStoreOnStreamFunc = (TagReadFromStoreOnStreamFunc *)IoBlock_readFromStore_stream_;
	return tag;
}

void IoBlock_copy_(IoBlock *self, IoBlock *other)
{
	DATA(self)->message = IOREF(DATA(other)->message);
	
	{
		List *l1 = DATA(self)->argNames;
		List_removeAll(l1);
		LIST_FOREACH(DATA(other)->argNames, i, v, List_append_(l1, IOREF(v)); );
	}
	
	if (DATA(other)->scope) 
	{
		IOREF(DATA(other)->scope);
	}
	
	DATA(self)->scope = DATA(other)->scope;
}

void IoBlock_writeToStore_stream_(IoBlock *self, IoStore *store, BStream *stream)
{
	ByteArray *ba = IoBlock_justCode(self);
	BStream_writeTaggedByteArray_(stream, ba);
	//printf("write block '%s'\n", ByteArray_asCString(ba));
	ByteArray_free(ba);
	
	if (DATA(self)->scope)
	{
		BStream_writeTaggedInt32_(stream, IoStore_pidForObject_(store, DATA(self)->scope));
	}
	else
	{
		BStream_writeTaggedInt32_(stream, 0);
	}
}

void IoBlock_readFromStore_stream_(IoBlock *self, IoStore *store, BStream *stream)
{
	IoBlock *newBlock = 0x0;
	ByteArray *ba = BStream_readTaggedByteArray(stream);
	
	//printf("read block [[[%s]]]]\n", ByteArray_asCString(ba));
	newBlock = IoState_on_doCString_withLabel_(IOSTATE, IoState_lobby(IOSTATE), ByteArray_asCString(ba), "Block readFromStore");
	
	if (!newBlock || !ISBLOCK(newBlock))
	{
		IoState_error_(IOSTATE, 0x0, "Store found bad block code: %s", (char *)ByteArray_bytes(ba));
	}
	
	IoBlock_copy_(self, newBlock);
	
	{
		PID_TYPE pid = BStream_readTaggedInt32(stream);
		
		if (pid)
		{
			DATA(self)->scope = IoStore_objectWithPid_(store, pid);
		}
		else
		{
			DATA(self)->scope = 0x0;
		}
	}
}

IoBlock *IoBlock_proto(void *vState)
{
	IoState *state = (IoState *)vState;
	IoMethodTable methodTable[] = {
	{"print", IoBlock_print},
	{"code", IoBlock_code},
	{"message", IoBlock_message},
	{"setMessage", IoBlock_setMessage},
	{"argumentNames", IoBlock_argumentNames},
	{"setArgumentNames", IoBlock_argumentNames_},
	{"setScope", IoBlock_setScope_},
	{"scope", IoBlock_scope},
	{"performOn", IoBlock_performOn},
	{"call", IoBlock_call},
	{NULL, NULL},
	};
	
	IoObject *self = IoObject_new(state);
	
	IoObject_setDataPointer_(self, calloc(1, sizeof(IoBlockData)));
	self->tag = IoBlock_tag(state);
	DATA(self)->message  = ((IoState *)state)->nilMessage;
	DATA(self)->argNames = List_new();
	DATA(self)->scope    = 0x0;
	IoState_registerProtoWithFunc_((IoState *)state, self, IoBlock_proto);

	IoObject_addMethodTable_(self, methodTable);
	return self;
}

IoBlock *IoBlock_rawClone(IoBlock *proto)
{
	IoBlockData *protoData = DATA(proto);
	IoObject *self = IoObject_rawClonePrimitive(proto);
	IoObject_setDataPointer_(self, calloc(1, sizeof(IoBlockData)));
	self->tag = proto->tag;
	DATA(self)->message  = IoMessage_deepCopyOf_(protoData->message);
	DATA(self)->argNames = List_clone(protoData->argNames);
	DATA(self)->scope    = protoData->scope;
	self->isActivatable = 1;
	return self;
}

IoBlock *IoBlock_new(IoState *state)
{
	IoObject *proto = IoState_protoWithInitFunction_((IoState *)state, IoBlock_proto);
	return IOCLONE(proto);
}

void IoBlock_rawPrint(IoBlock *self)
{ 
	ByteArray *ba = IoBlock_justCode(self);
	printf("%s\n", (char *)ByteArray_bytes(ba));
}

void IoBlock_mark(IoBlock *self)
{ 
	IoBlockData *bd = DATA(self);
	IoObject_shouldMark(bd->message);
	IoObject_shouldMarkIfNonNull(bd->scope);
	LIST_DO_(bd->argNames, IoObject_shouldMark);
}

void IoBlock_free(IoBlock *self)
{
	List_free(DATA(self)->argNames);;
	free(IoObject_dataPointer(self));
}

void IoBlock_message_(IoBlock *self, IoMessage *m) 
{ 
	DATA(self)->message = IOREF(m); 
}

// calling -------------------------------------------------------- 

IoObject *IoBlock_activate(IoBlock *self, IoObject *target, IoObject *locals, IoMessage *m, IoObject *slotContext)
{
	IoState *state = IOSTATE;	
	ptrdiff_t poolMark = IoState_pushRetainPool(state);
		
	IoBlockData *selfData = DATA(self);
	List *argNames  = selfData->argNames;
	IoObject *scope = selfData->scope;
	
	IoObject *blockLocals = IOCLONE(state->localsProto);
	IoObject *result;

	blockLocals->isLocals = 1;
	
	if (!scope)
	{ 
		scope = target;
	}
	
	IoObject_createSlotsIfNeeded(blockLocals);
	
	{
		IoObject *ac = IoCall_with(state, 
							  locals, 
							  target, 
							  m, 
							  slotContext, 
							  self, 
							  state->currentCoroutine);
		IoObject_setSlot_to_(blockLocals, state->callSymbol, ac);
	}

	IoObject_setSlot_to_(blockLocals, state->selfSymbol, scope);
	IoObject_setSlot_to_(blockLocals, state->updateSlotSymbol, state->localsUpdateSlotCFunc);
	
	LIST_FOREACH(argNames, i, name,
		IoObject *arg = IoMessage_locals_valueArgAt_(m, locals, i);
		// gc may kick in while evaling locals, so we need to be safe
		IoObject_setSlot_to_(blockLocals, name, arg);
	);
	
	if (Coro_stackSpaceAlmostGone(IoCoroutine_cid(state->currentCoroutine))) 
	{ 
		/*
		IoCoroutine *currentCoroutine = state->currentCoroutine;
		Coro *coro = IoCoroutine_cid(currentCoroutine);
		
		printf("%p-%p block overflow %i/%i\n", 
			  (void *)currentCoroutine, (void *)coro, Coro_bytesLeftOnStack(coro), Coro_stackSize(coro));
		printf("message = %s\n", CSTRING(IoMessage_name(selfData->message)));
		*/
		{
			IoCoroutine *newCoro = IoCoroutine_new(state);
			IoCoroutine_try(newCoro, blockLocals, blockLocals, selfData->message);
			result = IoCoroutine_rawResult(newCoro);
		}
	}
	else
	{
		result = IoMessage_locals_performOn_(selfData->message, blockLocals, blockLocals);  
	}
	
	state->stopStatus = MESSAGE_STOP_STATUS_NORMAL;
	
	IoState_popRetainPool_(state, poolMark);
	//IoState_popRetainPool(state);
	IoState_stackRetain_(state, result);
	return result;
}

// ------------------------------------------------------------------------ 

IoObject *IoBlock_method(IoObject *target, IoObject *locals, IoMessage *m)
{
	IoBlock *const self = IoBlock_new((IoState *)(target->tag->state));
	const int nargs = IoMessage_argCount(m);
	IoMessage *const message = (nargs > 0) ? IoMessage_rawArgAt_(m, nargs - 1) : IOSTATE->nilMessage;
	int i;

	DATA(self)->message = IOREF(message);
	
	for (i = 0; i < nargs - 1; i ++)
	{
		IoMessage *argMessage = IoMessage_rawArgAt_(m, i);
		IoSymbol *name = IoMessage_name(argMessage);
		List_append_(DATA(self)->argNames, IOREF(name));
	}
	
	return self;
}

IoObject *IoObject_block(IoObject *target, IoObject *locals, IoMessage *m)
{
	IoBlock *self = (IoBlock *)IoBlock_method(target, locals, m);
	DATA(self)->scope = IOREF(locals);
	return self;
}

IoObject *IoBlock_print(IoBlock *self, IoObject *locals, IoMessage *m)
{ 
	/*#io
	docSlot("print", "prints an Io source code representation of the block/method")
	*/
	
	ByteArray *ba = IoBlock_justCode(self);
	IoState_print_(IOSTATE, ByteArray_asCString(ba));
	return IONIL(self); 
}

ByteArray *IoBlock_justCode(IoBlock *self)
{ 
	ByteArray *ba = ByteArray_new();
	
	if (DATA(self)->scope)
	{ 
		ByteArray_appendCString_(ba, "block("); 
	}
	else
	{ 
		ByteArray_appendCString_(ba, "method("); 
	}
	
	LIST_FOREACH(DATA(self)->argNames, i, argName,
		ByteArray_append_(ba, IoSeq_rawByteArray((IoSymbol *)argName));
		ByteArray_appendCString_(ba, ", ");
	);
	
	{
		ByteArray *d = IoMessage_description(DATA(self)->message);
		ByteArray_append_(ba, d);
		ByteArray_free(d);
	}
	
	ByteArray_appendCString_(ba, ")");
	return ba; 
}

IoObject *IoBlock_code(IoBlock *self, IoObject *locals, IoMessage *m)
{ 
	/*#io
	docSlot("code", "Returns a string containing the decompiled code of the receiver. ")
	*/
	
	return IoState_symbolWithByteArray_copy_(IOSTATE, IoBlock_justCode(self), 0); 
}

IoObject *IoBlock_code_(IoBlock *self, IoObject *locals, IoMessage *m)
{ 
	/*#io
	docSlot("setCode(aString)", "Set's the reciever's message to a compiled version of aString. Returns self")
	*/
	
	IoSymbol *string = IoMessage_locals_symbolArgAt_(m, locals, 0);
	char *s = CSTRING(string);
	IoMessage *newM = IoMessage_newFromText_label_(IOSTATE, s, "[IoBlock_code_]");
	
	if (newM) 
	{ 
		DATA(self)->message = IOREF(newM); 
	}
	else
	{ 
		IoState_error_(IOSTATE, m, "no messages found in compile string"); 
	}
	
	return self;
}

IoObject *IoBlock_message(IoBlock *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("message", "Returns the root message of the receiver. ")
	*/
	
	return DATA(self)->message ? (IoObject *)DATA(self)->message : IONIL(self); 
}

IoObject *IoBlock_setMessage(IoBlock *self, IoObject *locals, IoMessage *m)
{ 
	/*#io
	docSlot("setMessage(aMessage)", 
		   "Sets the root message of the receiver to aMessage. ")
	*/
	
	IoMessage *message = IoMessage_locals_messageArgAt_(m, locals, 0);
	DATA(self)->message = IOREF(message);
	return self;
}

IoObject *IoBlock_argumentNames(IoBlock *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("argumentNames", 
		   "Returns a List of strings containing the argument names of the receiver. ")
	*/
	
	IoList *argsList = IoList_new(IOSTATE);

	LIST_FOREACH(DATA(self)->argNames, i, v, IoList_rawAppend_(argsList, (IoObject *)v));
	
	return argsList;
}

IoObject *IoBlock_argumentNames_(IoBlock *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("setArgumentNames(aListOfStrings)", 
		   "Sets the receiver's argument names to those specified in 
aListOfStrings. Returns self.  ")
	*/
	
	IoList *newArgNames = IoMessage_locals_listArgAt_(m, locals, 0);
	List *rawNewArgNames = IoList_rawList(newArgNames);
	
	LIST_FOREACH(rawNewArgNames, i, v,
			   IOASSERT(ISSYMBOL(((IoObject *)v)), "argument names must be Strings"); 
	);
	
	List_copy_(DATA(self)->argNames, IoList_rawList(newArgNames));
	return self;
}

IoObject *IoBlock_scope(IoBlock *self, IoObject *locals, IoMessage *m)
{ 
	/*#io
	docSlot("scope", 
		   "Returns the scope used when the block is activated or 
Nil if the target of the message is the scope.   ")
	*/
	
	IoObject *scope = DATA(self)->scope;
	return scope ? scope : IONIL(self); 
}

IoObject *IoBlock_setScope_(IoBlock *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("setScope(anObjectOrNil)", 
		   "If argument is an object, when the block is activated, 
it will set the proto and self slots of it's locals to the specified 
object. If Nil, it will set them to the target of the message. ")
	*/
	
	IoObject *scope = IoMessage_locals_valueArgAt_(m, locals, 0);
	DATA(self)->scope = ISNIL(scope) ? 0x0 : IOREF(scope);
	return self;
}

IoObject *IoBlock_performOn(IoBlock *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("performOn(anObject, optionalLocals, optionalMessage, optionalSlotContext)", 
		   "Activates the receiver in the target context of anObject. 
Returns the result.")
	*/
	
	IoObject *bTarget = IoMessage_locals_valueArgAt_(m, locals, 0);
	IoObject *bLocals = locals;
	IoObject *bMessage = m;
	IoObject *bContext = bTarget;
	int argCount = IoMessage_argCount(m);
	
	if (argCount > 1) 
	{ 
		bLocals = IoMessage_locals_valueArgAt_(m, locals, 1); 
	}
	
	if (argCount > 2) 
	{ 
		bMessage = IoMessage_locals_valueArgAt_(m, locals, 2); 
	}
	
	if (argCount > 3) 
	{ 
		bContext = IoMessage_locals_valueArgAt_(m, locals, 3); 
	}
	
	return IoBlock_activate(self, bTarget, bLocals, bMessage, bContext);
}

IoObject *IoBlock_call(IoBlock *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("call(arg0, arg1, ...)", "Activates the receiver with the provided arguments.")
	*/
	
	return IoBlock_activate(self, locals, locals, m, locals);

}

