/*   
docCopyright("Steve Dekorte", 2002)
docLicense("BSD revised")
*/

#ifdef IOOBJECT_C 
#define IO_IN_C_FILE
#endif
#include "Common_inline.h"
#ifdef IO_DECLARE_INLINES

#include "IoState.h"

IOINLINE int IoObject_isWhite(IoObject *self)
{
	return Collector_markerIsWhite_(IOCOLLECTOR, &(self->marker));
}

IOINLINE int IoObject_isGray(IoObject *self)
{
	return Collector_markerIsGray_(IOCOLLECTOR, &(self->marker));
}

IOINLINE int IoObject_isBlack(IoObject *self)
{
	return Collector_markerIsBlack_(IOCOLLECTOR, &(self->marker));
}

IOINLINE void IoObject_createSlotsIfNeeded(IoObject *self)
{
	if (!self->ownsSlots)
	{ 
		/*printf("creating slots for %s %p\n", self->tag->name, (void *)self);*/
		IoObject_createSlots(self); 
	}
}

IOINLINE void IoObject_rawRemoveAllProtos(IoObject *self)
{
	IoObject_createSlotsIfNeeded(self);
	{
	int count = IoObject_rawProtosCount(self);
	memset(self->protos, 0x0, count * sizeof(IoObject *));
	}
}

IOINLINE void IoObject_shouldMark(IoObject *self)
{	
	Collector_shouldMark_(IOCOLLECTOR, self);
}

IOINLINE void IoObject_shouldMarkIfNonNull(IoObject *self)
{	
	if (self) 
	{
		IoObject_shouldMark(self);
	}
}

IOINLINE IoObject *IoObject_addingRef_(IoObject *self, IoObject *ref)
{	 
	Collector_value_addingRefTo_(IOCOLLECTOR, self, ref);
	//self->isDirty = 1;
	return ref;
}

IOINLINE void IoObject_inlineSetSlot_to_(IoObject *self, 
                                         IoSymbol *slotName, 
                                         IoObject *value)
{ 
	IoObject_createSlotsIfNeeded(self);
	/*
	 if (!slotName->isSymbol) 
	 { 
		 printf("Io System Error: setSlot slotName not symbol\n"); 
		 exit(1); 
	 }
	 */
	
	/*
	 if (PHash_count(self->slots) > 200)
	 { 
		 printf("PHash %p slots %i\n", (void *)(self->slots), PHash_count(self->slots));
		 IoState_error_(IOSTATE, 0x0, "too many slots");
	 }
	 */
	
	PHash_at_put_(self->slots, IOREF(slotName), IOREF(value));
	
	/*
	 if(PHash_at_put_(self->slots, IOREF(slotName), IOREF(value)))
	 { 
		 self->isDirty = 1; 
	 }
	 */ 
}

IOINLINE IoObject *IoObject_rawGetSlot_context_(IoObject *self, 
                                                IoSymbol *slotName, 
                                                IoObject **context)
{
	register IoObject *v = (IoObject *)0x0;
	
	if (self->ownsSlots)
	{
		v = (IoObject *)PHash_at_(self->slots, slotName);
		
		if (v) 
		{
			*context = self;
			return v; 
		}
	}
	
	IoObject_setHasDoneLookup_(self, 1);
	
	{
		register IoObject **protos = self->protos;
		
		for (; *protos; protos ++)
		{
			if (IoObject_hasDoneLookup((*protos))) 
			{
				continue;
			}
			
			v = IoObject_rawGetSlot_context_(*protos, slotName, context);
			
			if (v) 
			{
				break;
			}
		}
	}
	
	IoObject_setHasDoneLookup_(self, 0);
	
	return v;
}

IOINLINE IoObject *IoObject_rawGetSlot_(IoObject *self, IoSymbol *slotName)
{
	register IoObject *v = (IoObject *)0x0;
	
	if (self->ownsSlots)
	{
		v = (IoObject *)PHash_at_(self->slots, slotName);
		
		if (v) return v; 
	}
	
	IoObject_setHasDoneLookup_(self, 1);
	
	{
		register IoObject **protos = self->protos;
		
		for (; *protos; protos ++)
		{
			if (IoObject_hasDoneLookup((*protos))) 
			{
				continue;
			}
			
			v = IoObject_rawGetSlot_(*protos, slotName);
			
			if (v) break;
		}
	}
	
	IoObject_setHasDoneLookup_(self, 0);
	
	return v;
}

IOINLINE IoObject *IoObject_firstProto(IoObject *self)
{
	return self->protos[0];
}

IOINLINE IoObject *IoObject_rawProtoAt_(IoObject *self, int i)
{
	return self->protos[i];
}

IOINLINE void IoObject_mark(IoObject *self)
{	
	/*
	if (self->isLocals)
	{
		printf("mark %p locals\n", (void *)self);
	}
	else
	{
		printf("mark %p %s\n", (void *)self, IoObject_name(self));
	}
	*/
	
	if (self->ownsSlots)
	{ 
		PHash_doOnKeyAndValue_(self->slots, (PHashDoCallback *)IoObject_shouldMark); 
	}
	
	// mark protos 
	
	{ 
		IoObject **proto = self->protos;
		
		while (*proto != 0x0)
		{
			IoObject_shouldMark(*proto);
			proto ++;
		}
	}
	
	{
		TagMarkFunc *func = self->tag->markFunc;
		
		if (func) 
		{
			(func)(self);
		}
	}
}

IoObject *IoObject_addingRef_(IoObject *self, IoObject *ref);
int IoObject_hasCloneFunc_(IoObject *self, TagCloneFunc *func);

IOINLINE IoObject *IoObject_activate(IoObject *self, 
                                     IoObject *target, 
                                     IoObject *locals, 
                                     IoMessage *m,
                                     IoObject *slotContext)
{
	//TagActivateFunc *act = self->tag->activateFunc;
	//return act ? (IoObject *)((*act)(self, target, locals, m, slotContext)) : self;
	//printf("activate %s %i\n", self->tag->name, self->isActivatable);

	return self->isActivatable ? (IoObject *)((self->tag->activateFunc)(self, target, locals, m, slotContext)) : self;
}

IOINLINE IoObject *IoObject_forward(IoObject *self, IoObject *locals, IoMessage *m)
{
	IoState *state = IOSTATE;
	IoObject *context;
	IoObject *forwardSlot = IoObject_rawGetSlot_context_(self, state->forwardSymbol, &context);
	
	if (Coro_stackSpaceAlmostGone((Coro*)IoCoroutine_cid(state->currentCoroutine))) 
	{ 

		IoState_error_(IOSTATE, m, "stack overflow in forward while sending '%s' message to a '%s' object", 
					CSTRING(IoMessage_name(m)), IoObject_name(self)); 
	}
		
	if (forwardSlot) 
	{
		return IoObject_activate(forwardSlot, self, locals, m, context);
	}
	
	IoState_error_(state, m, "'%s' does not respond to message '%s'", 
				IoObject_name(self), CSTRING(IoMessage_name(m)));
	return self;
}

IOINLINE IoObject *IoObject_perform(IoObject *self, IoObject *locals, IoMessage *m)
{
	IoObject *context;
	IoObject *slotValue = IoObject_rawGetSlot_context_(self, IoMessage_name(m), &context);
/*	
	if (Coro_stackSpaceAlmostGone(IoCoroutine_cid(IOSTATE->currentCoroutine))) 
	{ 
		IoState_error_(IOSTATE, m, "stack overflow while sending '%s' message to a '%s' object", 
					CSTRING(IoMessage_name(m)), IoObject_name(self)); 
	}
*/
	if (slotValue) 
	{
		return IoObject_activate(slotValue, self, locals, m, context);
	}

	if (self->isLocals)
	{
		return IoObject_localsForward(self, locals, m);
	}

	return IoObject_forward(self, locals, m);
}

#undef IO_IN_C_FILE
#endif
