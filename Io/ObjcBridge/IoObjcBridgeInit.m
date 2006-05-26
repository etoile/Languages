/*   Copyright (c) 2003, Steve Dekorte
*   All rights reserved. See _BSDLicense.txt.
*/

#include "IoState.h"
#include "IoObjcBridge.h"
#include "Io2Objc.h"
#include "IoGNUstep.h"

void IoObjcBridgeInit(IoState *self, IoObject *context)
{
    IoObject *sharedBridge = IoObjcBridge_sharedBridge();
    
    if (sharedBridge) 
    {
	IoObject_setSlot_to_(context, 
	    IoState_symbolWithCString_(self, "ObjcBridge"),
	    sharedBridge);
    }
    else 
    {
	IoObject_setSlot_to_(context, IoState_symbolWithCString_(self, "ObjcBridge"),
	IoObjcBridge_proto(self));
	Io2Objc_proto(self);
	[[NSAutoreleasePool alloc] init]; /* hack */
	// we need a toplevel autorelease pool to have memory-safe exception handling
	// in Cocoa (pools need not to be released before throwing an NSException
	// (this according to the Cocoa documentation....) 
    }
}

