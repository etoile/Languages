/*   Copyright (c) 2003, Steve Dekorte
docLicense("BSD revised")
 *
 *  An Objective-C proxy to an Io value
 */

#ifndef __IO_OBJC2IO__
#define __IO_OBJC2IO__

#include "IoState.h"
#include "IoObject.h"
#include "IoObjcBridge.h"
#ifdef GNUSTEP
  #include <Foundation/Foundation.h>
#else
  #import <Foundation/Foundation.h>
#endif

@interface Objc2Io : NSObject // NSProxy
{
  IoObjcBridge *bridge;
  IoObject *ioValue;
}

- (void)setIoObject:(IoObject *)v;
- (IoObject *)ioValue;
- (void)setBridge:(IoObjcBridge *)b;

- (void)mark;

//- (BOOL)respondsToSelector:(SEL)aSelector;
//- (void)forwardInvocation:(NSInvocation *)anInvocation;

@end

#endif
