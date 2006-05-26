/*   Copyright (c) 2003, Steve Dekorte
*   All rights reserved. See _BSDLicense.txt.
*/

#include "Io2Objc.h"
#include "List.h"

#ifdef GNUSTEP
#include <Foundation/Foundation.h>
#include <objc/objc.h>
#else
#import <Foundation/Foundation.h>
#import <objc/objc-runtime.h>
#endif

#define DATA(self) ((Io2ObjcData *)IoObject_dataPointer(self))

IoTag *Io2Objc_tag(void *state) 
{
	IoTag *tag = IoTag_newWithName_("Io2Objc");
	tag->state = state;
	tag->cloneFunc = (TagCloneFunc *)Io2Objc_rawClone;
	tag->freeFunc  = (TagFreeFunc *)Io2Objc_free;
	tag->markFunc  = (TagMarkFunc *)Io2Objc_mark;
	tag->performFunc = (TagPerformFunc *)Io2Objc_perform;
	return tag;
}


Io2Objc *Io2Objc_proto(void *state) 
{ 
	IoObject *self = IoObject_new(state);
	self->tag = Io2Objc_tag(state);
	
	IoObject_setDataPointer_(self, calloc(1, sizeof(Io2ObjcData)));
	DATA(self)->returnBufferSize = 128;
	DATA(self)->returnBuffer = malloc(DATA(self)->returnBufferSize);
	
	DATA(self)->object = nil;
	DATA(self)->bridge = IoObjcBridge_sharedBridge();
	assert(DATA(self)->bridge!=NULL);
	IoState_registerProtoWithFunc_(state, self, Io2Objc_proto);
	return self; 
}

Io2Objc *Io2Objc_rawClone(Io2Objc *proto) 
{ 
	IoObject *self = IoObject_rawClonePrimitive(proto);
	IoObject_setDataPointer_(self, cpalloc(IoObject_dataPointer(proto), sizeof(Io2ObjcData)));
	DATA(self)->returnBufferSize = 128;
	DATA(self)->returnBuffer = malloc(DATA(self)->returnBufferSize);
	return self; 
}

Io2Objc *Io2Objc_new(void *state)
{
	IoObject *proto = IoState_protoWithInitFunction_(state, Io2Objc_proto);
	return IOCLONE(proto);
}

Io2Objc *Io2Objc_newWithId_(void *state, id obj)
{
	Io2Objc *self = Io2Objc_new(state);
	Io2Objc_setObject(self, obj);
	return self;
}

void Io2Objc_free(Io2Objc *self)
{
	id obj = DATA(self)->object;
	if (IoObjcBridge_sharedBridge()) IoObjcBridge_removeId_(DATA(self)->bridge, obj);
	//printf("Io2Objc_free %p that referenced a %s\n", (void *)obj, [[obj className] cString]);
	[DATA(self)->object autorelease];
	free(DATA(self)->returnBuffer);
	free(IoObject_dataPointer(self));
	IoObject_dataPointer(self)=0x0;
}

void Io2Objc_mark(Io2Objc *self)
{
	IoObject_shouldMark(DATA(self)->bridge);
}

void Io2Objc_setBridge(Io2Objc *self, void *bridge)
{ 
	DATA(self)->bridge = bridge; 
}

void Io2Objc_setObject(Io2Objc *self, void *object)
{ 
	[DATA(self)->object autorelease];
	DATA(self)->object = [(id)object retain];
}

void *Io2Objc_object(Io2Objc *self)
{ 
	return DATA(self)->object; 
}

void Io2Objc_nullObjcBridge(Io2Objc *self)
{ 
	DATA(self)->bridge = 0x0; 
}

/* ----------------------------------------------------------------- */

IoObject *Io2Objc_perform(Io2Objc *self, IoObject *locals, IoMessage *m)
{
	/* --- get the method signature ------------ */
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	void *state = IOSTATE;
	NSInvocation *invocation = nil;
	NSMethodSignature *methodSignature;
	NSString *methodName = IoObjcBridge_objcMethodFor_(DATA(self)->bridge, CSTRING(IoMessage_name(m)));
	SEL selector = NSSelectorFromString(methodName);
	id object = DATA(self)->object;
	char debug = IoObjcBridge_rawDebugOn(DATA(self)->bridge);
	IoObject *result;
	
	//NSLog(@"[%@<%i> %s]", NSStringFromClass( [object class] ), object, CSTRING(m->method));
	
	// see if receiver can handle message ------------- 
	
	if (![object respondsToSelector:selector])
	{
		IoState_error_(state, m,
					"Io Io2Objc perform %s does not respond to message '%s'",
					[[object className] cString], 
					methodName);
		return IONIL(self);
	}
	
	methodSignature = [object methodSignatureForSelector:selector];
	
	/* --- create an invocation ------------- */
	invocation = [NSInvocation invocationWithMethodSignature:methodSignature];
	[invocation setTarget:object];
	[invocation setSelector:selector];
	
	if (debug) 
	{
		char *cType = (char *)[methodSignature methodReturnType];
		IoState_print_(IOSTATE, "Io -> Objc (%s)", 
					IoObjcBridge_nameForTypeChar_(DATA(self)->bridge, *cType));
		IoState_print_(IOSTATE, "%s(", methodName);
	}
	
	/* --- attach arguments to invocation --- */
	{
		int n, max = [methodSignature numberOfArguments];
		
		for (n = 2; n < max; n++)
		{
			char *cType = (char *)[methodSignature getArgumentTypeAtIndex:n];
			IoObject *ioValue = IoMessage_locals_valueArgAt_(m, locals, n-2);
			char *error;
			void *cValue = IoObjcBridge_cValueForIoObject_ofType_error_(DATA(self)->bridge, ioValue, cType, &error);
			
			if (debug) 
			{
				printf("%s", IoObjcBridge_nameForTypeChar_(DATA(self)->bridge, *cType));
				
				if (n < max - 1) 
				{
					printf(", ");
				}
			}
			
			if (error)
			{
				IoState_error_(state, m, "Io Io2Objc perform %s argtype:'%s' argnum:%i ", error, cType, n-2);
			}
			
			[invocation setArgument:cValue atIndex:n]; /* copies the contents of value as a buffer of the appropriate size */
		}
	}
	
	if (debug) 
	{
		IoState_print_(IOSTATE, ")\n");
	}
	
	/* --- invoke --------------------------- */
	{ 
		NS_DURING
			[invocation invoke];
		NS_HANDLER
			IoState_error_(state, m, "Io Io2Objc perform while sending '%s' %s - %s",
						methodName, [[localException name] cString], [[localException reason] cString]);
		NS_ENDHANDLER
	}
	/* --- return result --------------------------- */
	{
		char *cType = (char *)[methodSignature methodReturnType];
		unsigned int length = [methodSignature methodReturnLength];
		
		if (*cType == 'v') 
		{
			return IONIL(self); /* void */
		}
		
		if (length > (unsigned int)DATA(self)->returnBufferSize)
		{
			DATA(self)->returnBuffer = realloc(DATA(self)->returnBuffer, length);
			DATA(self)->returnBufferSize = length;
		}
		/*printf("length = %i\n", length);*/
				
		if (*cType == '@')
		{
			id obj;
			[invocation getReturnValue:&obj];
			result = IoObjcBridge_ioValueForCValue_ofType_(DATA(self)->bridge, obj, cType);
		}
		else
		{
			[invocation getReturnValue:(DATA(self)->returnBuffer)]; // &?
			result = IoObjcBridge_ioValueForCValue_ofType_(DATA(self)->bridge, (DATA(self)->returnBuffer), cType);
		}
	}
	[pool release];
	return result;
}

