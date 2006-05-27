/*   Copyright (c) 2003, Steve Dekorte
*   All rights reserved. See _BSDLicense.txt.
*/

#include <Objc2Io.h>
#include "List.h"
#import "MethodDeclarations.h"

#ifdef GNUSTEP
#include <objc/objc.h>
#include <objc/objc-api.h>
#else
#import <objc/objc-runtime.h>
#import <objc/objc-api.h>
#endif

static inline BOOL OBClassIsSubclassOfClass(Class subClass, Class superClass)
{
	while (subClass) 
	{
		if (subClass == superClass) 
		{ 
			return YES; 
		}
		else 
		{ 
			subClass = subClass->super_class; 
		}
	}
	return NO;
}

@interface NSMethodSignature (stuff) /* declare to avoid compiler warning */
+ (NSMethodSignature *)signatureWithObjCTypes:(const char *)encoding;
@end

@implementation Objc2Io

- init
{
	id obj = [super init];
	//[obj retain]; // debug test
	return obj;
}

- (void)dealloc
{
	//if (IoObjcBridge_rawDebugOn(bridge)) IoState_print_(bridge->tag->state, "[Objc2Io %p dealloc]\n", self);
	IoObjcBridge_removeValue_(bridge, ioValue);
	[super dealloc];
}

- (void)setIoObject:(IoObject *)v 
{ 
	ioValue = v; 
}

- (IoObject *)ioValue 
{ 
	return ioValue; 
}

- (void)setBridge:(IoObjcBridge *)b 
{ 
	bridge = b; 
}

- (void)mark
{
	if (bridge) IoObject_shouldMark((IoObject *)bridge);
	if (ioValue) IoObject_shouldMark((IoObject *)ioValue);
}

- (BOOL)respondsToSelector:(SEL)sel
{
	char *objcMethodName = (char *)sel_getName(sel);
	char *ioMethodName = IoObjcBridge_ioMethodFor_(bridge, objcMethodName);
	
	if (IoObjcBridge_rawDebugOn(bridge))
		IoState_print_(bridge->tag->state,
					"[Objc2Io respondsToSelector:\"%s\"] ", ioMethodName);
	
	{
		IoSymbol *methodName = IoState_symbolWithCString_(bridge->tag->state, ioMethodName);
		int r = IoObject_rawGetSlot_((IoObject *)ioValue, methodName) ? YES : NO;
		
		if (IoObjcBridge_rawDebugOn(bridge)) 
		{
			printf("= %i\n", r);
		}
		
		return r;
	}
}

/*
 - (void)doesNotRecognizeSelector:(SEL)aSelector
 {
	 printf("-------------------------- Objc2Io doesNotRecognizeSelector:\\n");
 }
 */

const char *get_selector_encoding(Class theClass, SEL sel)
{
#ifdef GNUSTEP
	Method_t m = class_get_instance_method(theClass, sel); 
#else
	Method m = class_getInstanceMethod(theClass, sel); 
	if (!m) 
	{
		m = class_getClassMethod(theClass, sel);
	}
#endif
	
	if (!m) 
	{ 
		return NULL; 
	} 
	else 
	{ 
		return m->method_types; 
	}
}

- (NSMethodSignature *)methodSignatureForSelector:(SEL)sel
{
	/*char debug = IoObjcBridge_rawDebugOn(bridge);*/
	char encoding[32];  
	List *classes = IoObjcBridge_allClasses(bridge);
	int i, max = List_size(classes);
	
	for (i = 0; i < max; i ++)
	{
		Class class = List_at_(classes, i);
		const char *m; 
		
		if (OBClassIsSubclassOfClass(class, [NSObject class]))
		{
			NSMethodSignature *ms = [class instanceMethodSignatureForSelector:sel];
			if (ms) return ms;
		}
		
		m = get_selector_encoding(class, sel);
		
		if (m)
		{
			strcpy(encoding, m);
			goto done;
		}
		
	}
	
	//if (debug) printf("Objc2Io: unable to find signature for selector: '%s'\n", sel_getName(sel));
	
	// Note: some methods are dynamically generated, e.g. setter/getters by InterfaceBuilder
	// they are in the form setVariable:, ecc... 
	// the following code provide a generic signature of the form @:@@@@ 
	// the result will be an id and all the arguments are id
	
	{
		int argcount = 0;
		const char *s = sel_getName(sel);
		
		while (*s) 
		{ 
			if (*s == ':') 
			{
				argcount++; 
			} 
			s++; 
		}
		
		memset(encoding, '@', argcount+3);
		//encoding[0] = 'v';
		encoding[2] = ':';
		encoding[argcount+3] = 0x0;
	}
	//printf("encoding  = '%s'\n", encoding);
done:
	{
		NSMethodSignature *sig = [NSMethodSignature signatureWithObjCTypes:encoding];
		/*
		 char *cType = (char *)[sig getArgumentTypeAtIndex:2];
		 printf("argcount = %i\n", [sig numberOfArguments]-2);
		 printf("first arg = %s\n", cType);
		 */
		return sig;
	}
}

- (void)forwardInvocation:(NSInvocation *)invocation
{
	//if (!bridge) { return; } else {
	char debug = IoObjcBridge_rawDebugOn(bridge);
	IoState *state = bridge->tag->state;
	char *methodName = IoObjcBridge_ioMethodFor_(bridge, (char *)sel_getName([invocation selector]));
	
	NSMethodSignature *methodSignature = [invocation methodSignature];
	int n, max = [methodSignature numberOfArguments];
	IoMessage *m = IoMessage_newWithName_(state, IoState_symbolWithCString_(state, methodName)); /* cache this instead??? */
	unsigned char buffer[128];
	const char *returnType = [methodSignature methodReturnType];
	
	if (!*returnType) 
	{
		returnType = "?";
	}
	
	if (debug) 
	{
		IoState_print_(bridge->tag->state, "Objc -> Io (%s)", IoObjcBridge_nameForTypeChar_(bridge, *returnType));
		IoState_print_(bridge->tag->state, " %s(", methodName);
	}
	
	/* -- set the io message arguments --- */
	for (n = 2; n < max; n++)
	{
		IoObject *ioArgValue;
		char *cType = (char *)[methodSignature getArgumentTypeAtIndex:n];
		
		memset(buffer, 0x0, 128);
		
		if (debug) 
		{
			printf("%s", IoObjcBridge_nameForTypeChar_(bridge, *cType));
			if (n < max-1) printf(", ");
		}
		
		if (*cType == '@')
		{
			id obj;
			[invocation getArgument:&obj atIndex:n];
			ioArgValue = IoObjcBridge_ioValueForCValue_ofType_(bridge, obj, cType);
		}
		else
		{
			[invocation getArgument:&buffer atIndex:n];
			ioArgValue = IoObjcBridge_ioValueForCValue_ofType_(bridge, buffer, cType);
		}
		
		IoMessage_setCachedArg_to_(m, n-2, ioArgValue);
	}
	
	if (debug) 
	{
		printf(")\n");
	}
	
	{
		/* -- perform io message --- */
		IoObject *result;
		char *cType = (char *)[methodSignature methodReturnType];
		//result = IoObject_perform(ioValue, ioValue, m);
		result = ioValue->tag->performFunc(ioValue, ioValue, m);
		
		// convert and return result if not void 
		
		if (*cType != 'v')
		{
			char *error;
			void *cResult = IoObjcBridge_cValueForIoObject_ofType_error_(bridge, result, cType, &error);
			
			if (error)
			{
				IoState_error_(state, m, "Io Objc2Io %s - return type:%s", error, cType);
			}
			
			//printf("Objc2Io return type = %s\ufffc cType);
			[invocation setReturnValue:(void *)cResult];
	}
    }
}

@end
