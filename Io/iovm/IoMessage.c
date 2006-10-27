/*#io
Message ioDoc(
              docCopyright("Steve Dekorte", 2002)
              docLicense("BSD revised")
              docObject("Message")
              docDescription("""A Message object encapsulates the action of a message send. Blocks are composed of a Message and its children.

Terminology

Example;
A B(C D); E F
In the above example:
"A" is the "name" of the first message
the B message is "attached" to the A message
the C message is the first argument of the B message
the D message is attached the C message
the E message is the "next" message of the A message
the F message is attached the E message


Important; Modifying the message tree of a block currently in use may cause 
a crash if a garbage collection cycle occurs. If the implementation were 
changed to retain every called message, this could be avoided. 
But the cost to performance seems to outweigh the need to cover this case for now.
""")
docCategory("Core")
*/

#include "IoObject.h"
#define IOMESSAGE_C 
#include "IoMessage.h"
#undef IOMESSAGE_C
#include "IoSeq.h"
#include "IoMap.h"
#include "IoNumber.h"
#include "IoState.h"
#include "IoCFunction.h"
#include "IoBlock.h"
#include "IoList.h"
#include "IoDate.h"
#include "IoSeq.h"
#include <ctype.h>
#include <stdarg.h>
#include "IoMessage_parser.h"

#define DATA(self) ((IoMessageData *)IoObject_dataPointer(self))

void IoMessage_writeToStore_stream_(IoMessage *self, IoStore *store, BStream *stream)
{
	ByteArray *ba = IoMessage_description(self);
	BStream_writeTaggedByteArray_(stream, ba);
	ByteArray_free(ba);
}

void IoMessage_readFromStore_stream_(IoMessage *self, IoStore *store, BStream *stream)
{
	const char *code = BStream_readTaggedCString(stream);
	IoMessage *m = IoMessage_newFromText_label_(IOSTATE, (char *)code, "[from store]");
	IoMessage_copy_(self, m);
}

IoObject *IoMessage_activate(IoMessage *self, IoObject *target, IoObject *locals, IoMessage *m, IoObject *slotContext)
{
	//printf("activating self %s\n", CSTRING(IoMessage_name(self)));
	//printf("activating m %s\n", CSTRING(IoMessage_name(m)));

	return IoMessage_locals_performOn_(self, locals, locals);
	//return IoObject_perform(locals, locals, self);
}

IoTag *IoMessage_tag(void *state)
{
	IoTag *tag = IoTag_newWithName_("Message");
	tag->state = state;
	tag->freeFunc  = (TagFreeFunc *)IoMessage_free;
	tag->cloneFunc = (TagCloneFunc *)IoMessage_rawClone;
	tag->markFunc  = (TagMarkFunc *)IoMessage_mark;
	tag->writeToStoreOnStreamFunc  = (TagWriteToStoreOnStreamFunc *)IoMessage_writeToStore_stream_;
	tag->readFromStoreOnStreamFunc = (TagReadFromStoreOnStreamFunc *)IoMessage_readFromStore_stream_;
	tag->activateFunc = (TagActivateFunc *)IoMessage_activate;
	return tag;
}

IoMessage *IoMessage_proto(void *state)
{
	IoMethodTable methodTable[] = {
	{"clone", IoMessage_clone},
		
	{"name", IoMessage_protoName},
	{"setName", IoMessage_protoSetName},
		
	{"next", IoMessage_nextMessage},
	{"setNext", IoMessage_setNextMessage},
		
	{"attached", IoMessage_attachedMessage},
	{"setAttached", IoMessage_setAttachedMessage},
		
	{"nextMessage", IoMessage_nextMessage},
	{"setNextMessage", IoMessage_setNextMessage},
		
	{"attachedMessage", IoMessage_attachedMessage},
	{"setAttachedMessage", IoMessage_setAttachedMessage},
		
	{"argAt", IoMessage_argAt},
	{"arguments", IoMessage_arguments},
	{"setArguments", IoMessage_setArguments},
	{"appendArg", IoMessage_appendArg},
	{"argCount", IoMessage_argCount_},
		
	{"cachedResult", IoMessage_cachedResult},
	{"setCachedResult", IoMessage_setCachedResult},
	{"removeCachedResult", IoMessage_removeCachedResult},
		
	{"lineNumber", IoMessage_lineNumber},
	{"setLineNumber", IoMessage_setLineNumber},
		
	{"characterNumber", IoMessage_characterNumber},
	{"setCharacterNumber", IoMessage_setCharacterNumber},
		
	{"label", IoMessage_label},
	{"setLabel", IoMessage_setLabel},
		
	{"code", IoMessage_descriptionString},
	{"doInContext", IoMessage_doInContext},
	{"fromString", IoMessage_fromString},
	{"argsEvaluatedIn", IoMessage_argsEvaluatedIn},
	{"asString", IoMessage_asString},
		
	{"asMessageWithEvaluatedArgs", IoMessage_asMessageWithEvaluatedArgs},

	{"opShuffle", IoMessage_opShuffle},
	{"opShuffleC", IoMessage_opShuffle},
		
	{NULL, NULL},
	};
	
	IoObject *self = IoObject_new(state);
	IoMessageData *d;
	IoObject_setDataPointer_(self, calloc(1, sizeof(IoMessageData)));
	d = IoObject_dataPointer(self);

	self->tag = IoMessage_tag(state);  
	d->args  = List_new();
	d->name  = IOSYMBOL("[unnamed]");
	d->label = IOSYMBOL("[unlabeled]");
	//d->charNumber = -1;
	d->lineNumber = -1;
	IoState_registerProtoWithFunc_((IoState *)state, self, IoMessage_proto);
	
	IoObject_addMethodTable_(self, methodTable);
	return self;
}

IoMessage *IoMessage_rawClone(IoMessage *proto) 
{ 
	IoObject *self = IoObject_rawClonePrimitive(proto);
	
	IoObject_setDataPointer_(self, calloc(1, sizeof(IoMessageData)));  
	DATA(self)->args = List_new(); 
	DATA(self)->name = DATA(proto)->name; 
	DATA(self)->label = DATA(proto)->label; 
	/* any clone really needs to be a deep copy */
	return self;
}

IoMessage *IoMessage_new(void *state)
{
	IoObject *proto = IoState_protoWithInitFunction_((IoState *)state, IoMessage_proto);
	IoObject *self = IOCLONE(proto);
	return self;
}

// Message shallowCopy := method(Message clone setName(name) setArguments(arguments))

void IoMessage_copy_(IoMessage *self, IoMessage *other)
{ 
	DATA(self)->name = IOREF(DATA(other)->name);
	
	{
		List *l1 = DATA(self)->args;
		List *l2 = DATA(other)->args;
		int i, max = List_size(l2);
		List_removeAll(l1);
		
		for (i = 0; i < max; i ++)
		{
			List_append_(l1, IOREF(List_at_(l2, i)));
		}
	}
	
	if (DATA(other)->attachedMessage) IOREF(DATA(other)->attachedMessage);
	DATA(self)->attachedMessage = DATA(other)->attachedMessage;
	
	if (DATA(other)->nextMessage) IOREF(DATA(other)->nextMessage);
	DATA(self)->nextMessage = DATA(other)->nextMessage;
	
	if (DATA(other)->cachedResult) IOREF(DATA(other)->cachedResult);
	DATA(self)->cachedResult = DATA(other)->cachedResult;
	
	//DATA(self)->charNumber = DATA(other)->charNumber;
	DATA(self)->lineNumber = DATA(other)->lineNumber;
	
	if (DATA(other)->label) IOREF(DATA(other)->label);
	DATA(self)->label = DATA(other)->label;
}

IoMessage *IoMessage_deepCopyOf_(IoMessage *self)
{
	IoMessage *child = IoMessage_new(IOSTATE);
	int i;
	
	/*printf("deep copying: %s\n", ByteArray_asCString(IoMessage_description(self)));*/
	for (i = 0; i < IoMessage_argCount(self); i ++)
	{ 
		List_append_(DATA(child)->args, 
				   IOREF(IoMessage_deepCopyOf_(LIST_AT_(DATA(self)->args, i))));
	}
	
	DATA(child)->name = IOREF(DATA(self)->name);
	IoMessage_cachedResult_(child, (IoObject *)DATA(self)->cachedResult);
	
	if (DATA(self)->nextMessage) 
	{
		DATA(child)->nextMessage = IOREF(IoMessage_deepCopyOf_(DATA(self)->nextMessage));
	}
	
	if (DATA(self)->attachedMessage) 
	{
		DATA(child)->attachedMessage = IOREF(IoMessage_deepCopyOf_(DATA(self)->attachedMessage));
	}
	/*printf("deep copy result: %s\n", ByteArray_asCString(IoMessage_description(child)));*/
	return child;
}

IoMessage *IoMessage_newWithName_(void *state, IoSymbol *symbol)
{
	IoMessage *self = IoMessage_new(state);
	DATA(self)->name = IOREF(symbol);
	return self;
}

IoMessage *IoMessage_newWithName_label_(void *state, IoSymbol *symbol, IoSymbol *label)
{
	IoMessage *self = IoMessage_new(state);
	DATA(self)->name  = IOREF(symbol);
	DATA(self)->label = IOREF(label);
	return self;
}

IoMessage *IoMessage_newWithName_returnsValue_(void *state, IoSymbol *symbol, IoObject *v)
{
	IoMessage *self = IoMessage_new(state);
	DATA(self)->name         = IOREF(symbol);
	DATA(self)->cachedResult = IOREF(v);
	return self;
}

IoMessage *IoMessage_newWithName_andCachedArg_(void *state, IoSymbol *symbol, IoObject *arg)
{
	IoMessage *self = IoMessage_newWithName_(state, symbol);
	IoMessage_addCachedArg_(self, arg);
	return self;
}

void IoMessage_mark(IoMessage *self)
{
	IoObject_shouldMarkIfNonNull(DATA(self)->name); 
	
	if (DATA(self)->args) 
	{ 
		LIST_FOREACH(DATA(self)->args, i, v, IoObject_shouldMark(v)); 
	}

	IoObject_shouldMarkIfNonNull(DATA(self)->cachedResult); 
	IoObject_shouldMarkIfNonNull((IoObject *)DATA(self)->attachedMessage); 
	IoObject_shouldMarkIfNonNull((IoObject *)DATA(self)->nextMessage); 
	IoObject_shouldMarkIfNonNull((IoObject *)DATA(self)->label); 
}

void IoMessage_free(IoMessage *self)
{ 
	if (DATA(self)->args) 
	{ 
		List_free(DATA(self)->args); 
	}
	
	free(IoObject_dataPointer(self));
}

List *IoMessage_args(IoMessage *self) 
{ 
	return DATA(self)->args; 
}

void IoMessage_cachedResult_(IoMessage *self, IoObject *v)
{ 
	DATA(self)->cachedResult = v ? IOREF(v) : 0x0; 
}

void IoMessage_label_(IoMessage *self, IoSymbol *ioSymbol) /* sets label for children too */
{
	DATA(self)->label = IOREF(ioSymbol);
	List_do_with_(DATA(self)->args, (ListDoWithCallback *)IoMessage_label_, ioSymbol);
	
	if (DATA(self)->attachedMessage) 
	{
		IoMessage_label_(DATA(self)->attachedMessage, ioSymbol); 
	}
	
	if (DATA(self)->nextMessage) 
	{
		IoMessage_label_(DATA(self)->nextMessage, ioSymbol); 
	}
}

int IoMessage_rawLineNumber(IoMessage *self)
{ 
	return DATA(self)->lineNumber; 
}

void IoMessage_rawSetLineNumber_(IoMessage *self, int n)
{
	DATA(self)->lineNumber = n;
}

void IoMessage_rawSetCharNumber_(IoMessage *self, int n)
{ 
	//DATA(self)->charNumber = n; 
}

int IoMessage_rawCharNumber(IoMessage *self)
{ 
	return 0; //DATA(self)->charNumber; 
}

List *IoMessage_rawArgList(IoMessage *self)
{ 
	return DATA(self)->args; 
}

unsigned char IoMessage_isNotCached(IoMessage *self)
{ 
	return !(DATA(self)->cachedResult); 
}

unsigned char IoMessage_needsEvaluation(IoMessage *self)
{
	List *args = DATA(self)->args;
	int a = List_detect_(args, (ListDetectCallback *)IoMessage_isNotCached) != 0x0;
	
	if (a) 
	{
		return 1;
	}
	
	if (DATA(self)->attachedMessage && IoMessage_needsEvaluation(DATA(self)->attachedMessage))
	{
		return 1;
	}
	
	if (DATA(self)->nextMessage && IoMessage_needsEvaluation(DATA(self)->nextMessage)) 
	{
		return 1;
	}
	
	return 0;
}

void IoMessage_addCachedArg_(IoMessage *self, IoObject *v)
{
	IoMessage *m = IoMessage_new(IOSTATE);
	IoMessage_cachedResult_(m, v);
	IoMessage_addArg_(self, m);
}

void IoMessage_setCachedArg_to_(IoMessage *self, int n, IoObject *v)
{
	IoMessage *arg;
	
	while (!(arg = List_at_(DATA(self)->args, n)))
	{ 
		IoMessage_addArg_(self, IoMessage_new(IOSTATE)); 
	}
	
	IoMessage_cachedResult_(arg, v);
}

void IoMessage_setCachedArg_toInt_(IoMessage *self, int n, int anInt)
{
	// optimized to avoid creating a number unless necessary 
	
	IoMessage *arg = 0x0;
	
	while (!(arg = List_at_(DATA(self)->args, n)))
	{ 
		List_append_(DATA(self)->args, IOREF(IoMessage_new(IOSTATE))); 
	}
	
	DATA(arg)->cachedResult = IOREF(IONUMBER(anInt));
}

IoObject *IoMessage_lineNumber(IoMessage *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("lineNumber", 
		   "Returns the line number of the message. The charcter number 
is typically the line number in the source text from with the message was read. ")
	*/
	
	return IONUMBER(DATA(self)->lineNumber);
}

IoObject *IoMessage_setLineNumber(IoMessage *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("setLineNumber(aNumber)", "Sets the line number of the message. Returns self.")
	*/
	
	DATA(self)->lineNumber = IoMessage_locals_intArgAt_(m , locals, 0);
	return self;
}

IoObject *IoMessage_characterNumber(IoMessage *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("characterNumber", 
		   "Returns the message character number. The charcter number is typically 
the beggining character index in the source text from with the message was read. ")
	*/
	
	return IONUMBER(0);
	//return IONUMBER(DATA(self)->charNumber);
}

IoObject *IoMessage_setCharacterNumber(IoMessage *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("setCharacterNumber(aNumber)", 
		   "Sets the character number of the message. Returns self.")
	*/
	
	//DATA(self)->charNumber = IoMessage_locals_intArgAt_(m , locals, 0);
	return self;
}

IoObject *IoMessage_label(IoMessage *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("label", 
		   "Returns the message label. The label is typically set the the 
name of the file from which the source code for the message was read.")
	*/
	
	return DATA(self)->label;
}

IoObject *IoMessage_setLabel(IoMessage *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("setLabel(aString)", "Sets the label of the message. Returns self.")
	*/
	
	DATA(self)->label = IOREF(IoMessage_locals_symbolArgAt_(m , locals, 0));
	return self;
}

// --- perform --------------------------------------------------------

IoObject *IoMessage_doInContext(IoMessage *self, IoObject *locals, IoMessage *m)
{
	/*
	docSlot("doInContext(anObject, locals)", "Evaluates the receiver in the context of anObject. ")
	*/
	
	IoObject *context = IoMessage_locals_valueArgAt_(m, (IoObject *)locals, 0);
	locals = IoMessage_locals_valueArgAt_(m, (IoObject *)locals, 1);
	return IoMessage_locals_performOn_(self, locals, context);
}

//#define IO_DEBUG_STACK

IoObject *IoMessage_locals_performOn_(IoMessage *self, IoObject *locals, IoObject *target)
{
	IoState *state = IOSTATE;
	IoMessage *outer;
	IoObject *result = state->ioNil;
	//int debug = 0;
	
	/*
	if (DATA(self)->cachedResult) 
	{
		result = DATA(self)->cachedResult;
		goto stop;
	}
	*/
	
#ifdef IO_DEBUG_STACK
	IoState_pushRetainPool(state); 
	IoState_stackRetain_(state, target); 
#endif

	/*
	{
		IoCoroutine *currentCoroutine = state->currentCoroutine;
		
		if (Coro_stackSpaceAlmostGone(IoCoroutine_cid(state->currentCoroutine))) 
		{ 
			Coro *coro = IoCoroutine_cid(state->currentCoroutine);
			//Coro_stackSpaceAlmostGone(IoCoroutine_cid(state->currentCoroutine));
			//IoState_error_(state, self, "stack overflow while sending '%s' message to a '%s' object", 
			//			CSTRING(IoMessage_name(self)), IoObject_name(target)); 
			printf("%p-%p overflow %i/%i\n", 
				  (void *)currentCoroutine, (void *)coro, Coro_bytesLeftOnStack(coro), Coro_stackSize(coro));
			{
				IoCoroutine *newCoro = IoCoroutine_new(state);
				IoCoroutine_try(newCoro, target, locals, self);
				result = IoCoroutine_rawResult(newCoro);
				goto stop;
			 }
		}
	}
	*/
		
	for (outer = self; outer; outer = DATA(outer)->nextMessage)
	{
		IoMessage *inner = outer;
		
		for (inner = outer; inner; inner = DATA(inner)->attachedMessage)
		{
			/*
			if (debug)
			{
				printf("%s\n", CSTRING(IoMessage_name(inner)));
				//printf("%s %i\n", CSTRING(IoMessage_name(inner)), 
				//IoCoroutine_rawIoStackSize(IoState_currentCoroutine(state)));
			}
			*/
			
#ifdef IO_SANDBOX
			if (state->messageCountLimit && state->messageCount == 0)
			{
				IoState_error_(IOSTATE, inner, "message limit exceeded (%d)", state->messageCountLimit);
			}
			
			state->messageCount --;
#endif
			
			result = DATA(inner)->cachedResult;
			
			if (!result)
			{
				// No cached value -- must perform the message. 
				//result = IoObject_perform(target, locals, inner);
				//printf("%s %s\n", IoObject_name(target), CSTRING(IoMessage_name(inner)));
				//printf("%s ",  CSTRING(IoMessage_name(inner)));
				
				IoState_pushRetainPool(state); 
				result = target->tag->performFunc(target, locals, inner);
				IoState_popRetainPoolExceptFor_(state, result); 
				
#ifdef IO_DEBUG_STACK
				IoState_stackRetain_(state, result); 
#endif
				
				if (state->stopStatus != MESSAGE_STOP_STATUS_NORMAL)
				{ 
					result = state->returnValue; 
					
					if (!result) 
					{
						result = state->ioNil; 
					}
					
					goto stop; 
				}
			}
			// The next attachedMessage uses this result as its target. 
			target = result;
		}
		// The next nextMessage targets the local environment. 
		target = locals;
	}
	
stop:
#ifdef IO_DEBUG_STACK
	IoState_popRetainPool(state); 
	IoState_stackRetain_(state, result); 
#endif 
	//printf("\n");
	return result;
}

// getting arguments --------------------------- 

int IoMessage_argCount(IoMessage *self) 
{ 
	return List_size(DATA(self)->args); 
}

void IoMessage_assertArgCount_receiver_(IoMessage *self, int n, IoObject *receiver)
{
	if (List_size(DATA(self)->args) < n)
	{
		IoState_error_(IOSTATE, self, "[%s %s] requires %i arguments\n",
							  IoObject_name(receiver), CSTRING(DATA(self)->name), n);
	}
}

void IoMessage_locals_numberArgAt_errorForType_(IoMessage *self, 
									   IoObject *locals, 
									   int n, 
									   const char *typeName)
{
	IoObject *v = IoMessage_locals_valueArgAt_(self, locals, n);
	IoState_error_(IOSTATE, self, "argument %i to method '%s' must be a %s, not a '%s'",
						  n, CSTRING(DATA(self)->name), typeName, IoObject_name(v));
}

IoObject *IoMessage_locals_numberArgAt_(IoMessage *self, IoObject *locals, int n)
{
	IoObject *v = IoMessage_locals_valueArgAt_(self, locals, n);
	
	if (!ISNUMBER(v)) 
	{
		IoMessage_locals_numberArgAt_errorForType_(self, locals, n, "Number");
	}
	
	return v;
}

int IoMessage_locals_intArgAt_(IoMessage *self, IoObject *locals, int n)
{ 
	return IoNumber_asInt(IoMessage_locals_numberArgAt_(self, locals, n)); 
}

long IoMessage_locals_longArgAt_(IoMessage *self, IoObject *locals, int n)
{ 
	return IoNumber_asLong(IoMessage_locals_numberArgAt_(self, locals, n)); 
}

double IoMessage_locals_doubleArgAt_(IoMessage *self, IoObject *locals, int n)
{ 
	return IoNumber_asDouble(IoMessage_locals_numberArgAt_(self, locals, n)); 
}

float IoMessage_locals_floatArgAt_(IoMessage *self, IoObject *locals, int n)
{ 
	return (float)IoNumber_asDouble(IoMessage_locals_numberArgAt_(self, locals, n)); 
}

char *IoMessage_locals_cStringArgAt_(IoMessage *self, IoObject *locals, int n)
{ 
	return CSTRING(IoMessage_locals_symbolArgAt_(self, locals, n)); 
}

IoObject *IoMessage_locals_seqArgAt_(IoMessage *self, IoObject *locals, int n)
{
	IoObject *v = IoMessage_locals_valueArgAt_(self, locals, n);
	
	/*
	 if (ISNUMBER(v)) 
	 {
		 return IoNumber_asString((IoNumber *)v, (IoObject *)locals, self);
	 }
	 */
	
	if (!ISSEQ(v)) 
	{
		IoMessage_locals_numberArgAt_errorForType_(self, locals, n, "Sequence");
	}
	
	return v;
}

IoObject *IoMessage_locals_symbolArgAt_(IoMessage *self, IoObject *locals, int n)
{
	IoObject *v = IoMessage_locals_valueArgAt_(self, locals, n);
	
	if (!ISSEQ(v))
	{
		IoMessage_locals_numberArgAt_errorForType_(self, locals, n, "Sequence");
	}
	
	return IoSeq_rawAsSymbol(v);
}

IoObject *IoMessage_locals_mutableSeqArgAt_(IoMessage *self, IoObject *locals, int n)
{
	IoObject *v = IoMessage_locals_valueArgAt_(self, locals, n);
	
	if (!ISMUTABLESEQ(v)) 
	{
		IoMessage_locals_numberArgAt_errorForType_(self, locals, n, "mutable Sequence");
	}
	
	return v;
}

IoObject *IoMessage_locals_blockArgAt_(IoMessage *self, IoObject *locals, int n)
{
	IoObject *v = IoMessage_locals_valueArgAt_(self, locals, n);
	if (!ISBLOCK(v)) IoMessage_locals_numberArgAt_errorForType_(self, locals, n, "Block");
	return v;
}

IoObject *IoMessage_locals_dateArgAt_(IoMessage *self, IoObject *locals, int n)
{
	IoObject *v = IoMessage_locals_valueArgAt_(self, locals, n);
	if (!ISDATE(v)) IoMessage_locals_numberArgAt_errorForType_(self, locals, n, "Date");
	return v;
}

IoObject *IoMessage_locals_messageArgAt_(IoMessage *self, IoObject *locals, int n)
{
	IoObject *v = IoMessage_locals_valueArgAt_(self, locals, n);
	if (!ISMESSAGE(v)) IoMessage_locals_numberArgAt_errorForType_(self, locals, n, "Message");
	return v;
}

IoObject *IoMessage_locals_listArgAt_(IoMessage *self, IoObject *locals, int n)
{
	IoObject *v = IoMessage_locals_valueArgAt_(self, locals, n);
	if (!ISLIST(v)) IoMessage_locals_numberArgAt_errorForType_(self, locals, n, "List");
	return v;
}

IoObject *IoMessage_locals_mapArgAt_(IoMessage *self, IoObject *locals, int n)
{
	IoObject *v = IoMessage_locals_valueArgAt_(self, locals, n);
	if (!ISMAP(v)) IoMessage_locals_numberArgAt_errorForType_(self, locals, n, "Map");
	return v;
}

// printing

void IoMessage_print(IoMessage *self)
{
	ByteArray *ba = IoMessage_description(self);
	
	IoState_print_(IOSTATE, ByteArray_asCString(ba));
	ByteArray_free(ba);
}

void IoMessage_printWithReturn(IoMessage *self)
{
	IoMessage_print(self);
	IoState_print_(IOSTATE, "\n");
}

ByteArray *IoMessage_description(IoMessage *self)
{
	ByteArray *ba = ByteArray_new();
	IoMessage_appendDescriptionTo_follow_(self, ba, 1);
	return ba;
}

ByteArray *IoMessage_descriptionJustSelfAndArgs(IoMessage *self)
{
	ByteArray *ba = ByteArray_new();
	IoMessage_appendDescriptionTo_follow_(self, ba, 0);
	return ba;
}

IoObject *IoMessage_asString(IoMessage *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("asString", "Same as code().")
	*/
	
	return IoMessage_descriptionString(self, locals, m);
}

void IoMessage_appendDescriptionTo_follow_(IoMessage *self, ByteArray *ba, int follow)
{
	IoMessageData *data = DATA(self);
	
	ByteArray_appendCString_(ba, CSTRING(data->name));
	
	{
		int i, max = List_size(DATA(self)->args);
		
		if (max > 0) 
		{
			ByteArray_appendCString_(ba, "(");
			
			for (i = 0; i < max; i ++)
			{
				IoMessage *arg = List_at_(DATA(self)->args, i);
				IoMessage_appendDescriptionTo_follow_(arg, ba, 1);
				
				if (i != max-1) 
				{ 
					ByteArray_appendCString_(ba, ", "); 
				}
			}
			
			ByteArray_appendCString_(ba, ")");
		}
	}
	
	if (!follow) 
	{
		return;
	}
	
	if (DATA(self)->attachedMessage) 
	{ 
		ByteArray_appendCString_(ba, " "); 
		IoMessage_appendDescriptionTo_follow_(DATA(self)->attachedMessage, ba, 1); 
	}
	
	if (DATA(self)->nextMessage) 
	{ 
		ByteArray_appendCString_(ba, ";\n"); 
		IoMessage_appendDescriptionTo_follow_(DATA(self)->nextMessage, ba, 1); 
	}
}

//  methods --------------------------------------------------- 

IoObject *IoMessage_clone(IoMessage *self, IoObject *locals, IoMessage *m)
{ 
	/*#io
	docSlot("clone", "Returns a Message that is a deep copy of the receiver. ")
	*/
	
	return IoMessage_deepCopyOf_(self); 
}

IoObject *IoMessage_protoName(IoMessage *self, IoObject *locals, IoMessage *m)
{ 
	/*#io
	docSlot("name", "Returns the name of the receiver. ")
	*/
	
	IoObject *s = DATA(self)->name;
	return s; 
}

IoObject *IoMessage_protoSetName(IoMessage *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("setName(aString)", "Sets the name of the receiver. Returns self. ")
	*/
	
	DATA(self)->name = IOREF(IoMessage_locals_symbolArgAt_(m, locals, 0));
    //IoMessage_cacheIfPossible(self);
	return self; 
}

IoObject *IoMessage_descriptionString(IoMessage *self, IoObject *locals, IoMessage *m)
{ 
	/*#io
	docSlot("code", 
		   "Returns a String containing a decompiled code representation of the receiver.")
	*/
	
	ByteArray *ba = IoMessage_description(self); /* me must free the returned ByteArray */
	return IoState_symbolWithByteArray_copy_(IOSTATE, ba, 0); 
}

IoObject *IoMessage_nextMessage(IoMessage *self, IoObject *locals, IoMessage *m)
{ 
	/*#io
	docSlot("nextMessage", 
		   "Returns the next message in the message chain or Nil if there is no next message. ")
	*/
	
	return DATA(self)->nextMessage ? (IoObject *)DATA(self)->nextMessage : IONIL(self); 
}

IoObject *IoMessage_setNextMessage(IoMessage *self, IoObject *locals, IoMessage *m)
{ 
	/*#io
	docSlot("setNextMessage(aMessageOrNil)", 
		   "Sets the next message in the message chain to a deep copy of 
aMessage or it removes the next message if aMessage is Nil. ")
	*/
	
	IoObject *v = IoMessage_locals_valueArgAt_(m , locals, 0);
	
	if (ISNIL(v))
	{ 
		DATA(self)->nextMessage = 0x0; 
	}
	else
	{
		IOASSERT(ISMESSAGE(v), "argument must be Message or Nil");
		DATA(self)->nextMessage = IOREF(v);
	}
	return self; 
}

IoObject *IoMessage_attachedMessage(IoMessage *self, IoObject *locals, IoMessage *m)
{ 
	/*#io
	docSlot("attachedMessage", 
		   "Returns the message attached to the receiver or Nil 
if there is no message attached. ")
	*/
	
	return DATA(self)->attachedMessage ? (IoObject *)DATA(self)->attachedMessage : IONIL(self); 
}

void IoMessage_rawSetAttachedMessage(IoMessage *self, IoMessage *m)
{ 
	if (m)
	{
		DATA(self)->attachedMessage = IOREF(m);
	}
	else
	{
		DATA(self)->attachedMessage = 0x0;
	}
}

void IoMessage_rawSetNextMessage(IoMessage *self, IoMessage *m)
{
	if (m)
	{
		DATA(self)->nextMessage = IOREF(m);
	}
	else
	{
		DATA(self)->nextMessage = 0x0;
	}
}


IoObject *IoMessage_setAttachedMessage(IoMessage *self, IoObject *locals, IoMessage *m)
{ 
	/*#io
	docSlot("setAttachedMessage(aMessage)", 
		   "Sets the message attached the message chain to a deep 
copy of aMessage. If aMessage is Nil, it removes any attached message. ")
	*/
	
	IoObject *v = IoMessage_locals_valueArgAt_(m , locals, 0);
	
	if (ISNIL(v))
	{ 
		DATA(self)->attachedMessage = 0x0; 
	}
	else
	{
		IOASSERT(ISMESSAGE(v), "argument must be Message or Nil");
		DATA(self)->attachedMessage = IOREF(v);
	}
	
	return self; 
}

IoObject *IoMessage_argAt(IoMessage *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("argAt(indexNumber)", 
		   "Returns Message object for the specified argument or Nil if none exists.")
	*/
	
	int index =  IoNumber_asInt(IoMessage_locals_numberArgAt_(m , locals, 0));
	IoObject *v = List_at_(DATA(self)->args, index);
	return v ? v : IONIL(self);
}

IoObject *IoMessage_arguments(IoMessage *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("arguments", 
		   "Returns a list of the message objects that act as the 
receiver's arguments. Modifying this list will not alter the actual 
list of arguments. Use the arguments_() method to do that. ")
	*/
	
	IoList *argsList = IoList_new(IOSTATE);
	IoList_rawAddBaseList_(argsList, DATA(self)->args);
	return argsList;
}

IoObject *IoMessage_setArguments(IoMessage *self, IoObject *locals, IoMessage *m)
{ 
	/*#io
	docSlot("setArguments(aListOfMessages)", 
		   "Sets the arguments of the receiver to deep copies of 
those contained in aListOfMessages.  Returns self.")
	*/
	
	IoList *ioList = IoMessage_locals_listArgAt_(m, locals, 0);
	List *newArgs = IoList_rawList(ioList);
	
	List_removeAll(DATA(self)->args);
	
	LIST_FOREACH(newArgs, i, argMessage, 
			   
		if (!ISMESSAGE((IoMessage *)argMessage))
		{
			IoState_error_(IOSTATE, m, "arguments_() takes a list containing only Message objects");    
		}
		
		List_append_(DATA(self)->args, IOREF((IoMessage *)argMessage)); 
	);

	return self; 
}

IoObject *IoMessage_appendArg(IoMessage *self, IoObject *locals, IoMessage *m)
{ 
	/*#io
	docSlot("appendArg(aMessage)", 
		   """Adds aMessage to the argument list of receiver. Examples,
		   <pre>
		   Io> message(a) appendArg(message(b))
		   ==> a(b)

		   Io> message(a(1,2)) appendArg(message(3))
		   ==> a(1, 2, 3)
		   </pre>""")
	*/
	
	IoMessage *msg = IoMessage_locals_messageArgAt_(m, locals, 0);
	IoMessage_addArg_(self, msg);
	return self;
}

IoObject *IoMessage_argCount_(IoMessage *self, IoObject *locals, IoMessage *m)
{ 
	/*#io
	docSlot("argCount", 
		   """Returns the number of arguments this message has. A faster way to do, msg arguments size. Examples,
		   <pre>
		   Io> message(a(1,2,3)) argCount
		   ==> 3

		   Io> message(a) argCount
		   ==> 0
		   </pre>""")
	*/
	
	return IONUMBER(IoMessage_argCount(self));
}

IoObject *IoMessage_fromString(IoMessage *self, IoObject *locals, IoMessage *m)
{ 
	/*#io
	docSlot("fromString(aString)", 
		   "Returns a new Message object for the compiled(but not executed) 
result of aString. ")
	*/
	
	IoSymbol *string = IoMessage_locals_symbolArgAt_(m, locals, 0);
	IoSymbol *label = DATA(m)->label;
	
	if (IoMessage_argCount(m) > 1) 
	{ 
		label = IoMessage_locals_symbolArgAt_(m, locals, 1); 
	}
	
	return IoMessage_newFromText_label_(IOSTATE, CSTRING(string), CSTRING(label));      
}

IoObject *IoMessage_cachedResult(IoMessage *self, IoObject *locals, IoMessage *m)
{ 
	/*#io
	docSlot("cachedResult", 
		   "Returns the cached result of the Message or Nil if there is none.")
	*/
	
	return (DATA(self)->cachedResult ? DATA(self)->cachedResult : IONIL(self)); 
}

IoObject *IoMessage_setCachedResult(IoMessage *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("setCachedResult(anObject)", 
		   "Sets the cached result of the message. Returns self.")
	*/
	
	DATA(self)->cachedResult = IOREF(IoMessage_locals_valueArgAt_(m , locals, 0));
	return self;
}

IoObject *IoMessage_removeCachedResult(IoMessage *self, IoObject *locals, IoMessage *m)
{ 
	/*#io
	docSlot("removeCachedResult", "Removes the cached result of the Message.")
	*/
	
	DATA(self)->cachedResult = 0x0;
	return self;
}

IoObject *IoMessage_argsEvaluatedIn(IoMessage *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("argsEvaluatedIn(anObject)", 
		   "Returns a List containing the argument messages evaluated in the 
context of anObject. ")
	*/
	
	IoObject *context = IoMessage_locals_valueArgAt_(m, locals, 0);
	IoList *args = IoList_new(IOSTATE);
	int i;
	
	for (i = 0; i < List_size(DATA(self)->args); i ++)
	{ 
		IoObject *arg = IoMessage_locals_valueArgAt_(self, context, i);
		IoList_rawAppend_(args, arg);
	}
	return args;
}

IoObject *IoMessage_evaluatedArgs(IoMessage *self, IoObject *locals, IoMessage *m)
{
	/*
	 docSlot("evaluatedArgs", 
		    "Returns a List containing the argument messages evaluated in the context.")
	 */
	
	IoList *args = IoList_new(IOSTATE);
	int i;
	
	for (i = 0; i < List_size(DATA(self)->args); i ++)
	{ 
		IoObject *arg = IoMessage_locals_valueArgAt_(self, locals, i);
		IoList_rawAppend_(args, arg);
	}
	
	return args;
}

// ------------------------------ 

/*
 IoSymbol *IoMessage_name(IoMessage *self) 
 { 
	 return DATA(self)->name; 
 }
 */

IoSymbol *IoMessage_rawLabel(IoMessage *self) 
{ 
	return DATA(self)->label; 
}

List *IoMessage_rawArgs(IoMessage *self) 
{ 
	return DATA(self)->args; 
}

IoMessage *IoMessage_rawArgAt_(IoMessage *self, int n)
{ 
	IoMessage *result = List_at_(DATA(self)->args, n); 
	IoState_stackRetain_(IOSTATE, result);
	return result;
}

void IoMessage_addArg_(IoMessage *self, IoMessage *m)
{ 
	List_append_(DATA(self)->args, IOREF(m)); 
}

// ------------------------------- 

ByteArray *IoMessage_asMinimalStackEntryDescription(IoMessage *self)
{
	IoSymbol *name = IoMessage_name(self);
	IoSymbol *label = IoMessage_rawLabel(self);
	int lineNumber = IoMessage_rawLineNumber(self);
	return ByteArray_newWithFormat_("%s:%i %s", CSTRING(label), lineNumber, CSTRING(name));
}

void IoMessage_foreachArgs(IoMessage *self, 
                           IoObject *receiver, 
                           IoSymbol **indexSlotName, 
                           IoSymbol **valueSlotName, 
                           IoMessage **doMessage)
{
	int offset;
	
	IoMessage_assertArgCount_receiver_(self, 2, receiver);
	
	if (IoMessage_argCount(self) > 2)
	{
		*indexSlotName = IoMessage_name(IoMessage_rawArgAt_(self, 0));
		offset = 1;
	}
	else
	{
		*indexSlotName = 0x0; //IONIL(self);
		offset = 0;
	}
	
	*valueSlotName = IoMessage_name(IoMessage_rawArgAt_(self, 0 + offset));
	*doMessage = IoMessage_rawArgAt_(self, 1 + offset);
}

IoMessage *IoMessage_asMessageWithEvaluatedArgs(IoMessage *self, IoObject *locals, IoMessage *m)
{	
	IoState *state = IOSTATE;
	IoMessage *sendMessage;
	int i, max = IoMessage_argCount(self);
	IoObject *context = locals;
	
	if (IoMessage_argCount(m) > 0)
	{
		context = IoMessage_locals_valueArgAt_(m, locals, 0); 
	}
	
	if (IoMessage_needsEvaluation(self))
	{ 
		sendMessage = IoMessage_newWithName_(state, IoMessage_name(self)); 
	}
	else 
	{ 
		sendMessage = self; 
	}
	
	for (i = 0; i < max; i ++)
	{
		IoMessage *arg = IoMessage_rawArgAt_(self, i);
		IoObject *result = IoMessage_locals_performOn_(arg, context, context);
		IoMessage_setCachedArg_to_(sendMessage, i, result);
	}
	
	return sendMessage;
}

