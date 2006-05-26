/*   Copyright (c) 2003, Steve Dekorte
*   All rights reserved. See _BSDLicense.txt.
*/

#include "ObjcSubclass.h"
#include "List.h"
#include "IoState.h"
#include "IoMessage.h"

#ifdef GNUSTEP
#include <GNUstepBase/GSObjCRuntime.h>
#include <objc/objc-list.h>
#include <objc/objc.h>
#else
#import <objc/objc-class.h>
#import <objc/objc-runtime.h>
#import <objc/objc.h>
#endif
#include <stdlib.h>
#include "Objc2Io.h"

/*
 * globals are evil, but Objective-C classes are globals already,
 * so who cares if we have these globals for tracking the ones we add?
 *
 * IMPORTANT: can't use this addon with multiple Io states.
 */

static IoState *state = 0x0;
static Hash *classProtos = 0x0;

@implementation ObjcSubclass

+ new
{
    return [[self alloc] init];
}

+ (Hash *)classProtos
{
    if (!classProtos) classProtos = Hash_new();
    return classProtos;
}

+ (void)mark
{
    if (classProtos)
    {
	Hash *h = classProtos;
	IoObject *k = Hash_firstKey(h);
	
	while (k)
	{
	    IoObject *v = Hash_at_(h, k);
	    IoObject_shouldMark(v);
	    k = Hash_nextKey(h);
	}
	
    }
}

+ (Class)newClassNamed:(IoSymbol *)ioName proto:(IoObject *)proto
{
    Class a = [ObjcSubclass class];
    Class c = malloc(sizeof(struct objc_class));
    memcpy(c, a, sizeof(struct objc_class));
    c->name = strdup(CSTRING(ioName));
#ifdef GNUSTEP
    NSValue *c_ptr = GSObjCMakeClass([NSString stringWithCString: c->name encoding: NSASCIIStringEncoding], NSStringFromClass(a), nil);
    GSObjCAddClasses([NSArray arrayWithObject: c_ptr]);
#else
    //FIXME: Cannot find the GNUstep equivalent (seb)
    objc_addClass(c);
#endif
    state = proto->tag->state;
    if (c) Hash_at_put_([self classProtos], ioName, proto);
    return c;
}

/*
 - (BOOL)respondsToSelector:(SEL)sel
 {
     BOOL r = [super respondsToSelector:sel];
     //printf("ObjcSubclass respondsToSelector:\"%s\"] = %i\n", (char *)sel_getName(sel), r);
     return r;
 }
 */

+ (id)allocWithZone:(NSZone *)zone
{
    id v = [super allocWithZone:zone];
    //printf("ObjcSubclass allocWithZone\n");
    [v setProto];
    return v;
}

+ alloc
{
    id v = [super alloc];
    //printf("[ObjcSubclass alloc]\n");
    //[v setProto];
    return v;
}

- (void)setProto
{
    const char *s = [[self className] cString];
    printf("classname = %s state = %p\n", s, state);
    
    if (state)
    {
	IoSymbol *className = IoState_symbolWithCString_(state, (char *)s);
	IoObject *proto = (IoObject *)Hash_at_((void *)[[self class] classProtos], className);
	[super init];
	ioValue = (IoObject *)IOCLONE(proto);
	bridge = IoObjcBridge_sharedBridge();
	IoObjcBridge_addValue_(bridge, ioValue, self);
    }
}

@end
