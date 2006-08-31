/*   Copyright (c) 2003, Steve Dekorte
*   All rights reserved. See _BSDLicense.txt.
*/

#include "IoState.h"
#include "IoObjcBridge.h"
#include "Io2Objc.h"

void IoObjcBridgeInit(IoObject *context)
{
	IoState *self = ((IoObject *)context)->tag->state;
	IoObject *sharedBridge = IoObjcBridge_sharedBridge();

	if (sharedBridge)
	{
		IoObject_setSlot_to_(context, IoState_symbolWithCString_(self, "ObjcBridge"), sharedBridge);
	}
	else
	{
		IoObject_setSlot_to_(context, IoState_symbolWithCString_(self, "ObjcBridge"), IoObjcBridge_proto(self));
		Io2Objc_proto(self);
		NSLog(@"ObjcBridge"); /* without that line GNUstep goes mad, do not remove it */
		[[NSAutoreleasePool alloc] init]; /* hack */
		// we need a toplevel autorelease pool to have memory-safe exception handling
		// in Cocoa (pools need not to be released before throwing an NSException
		// (this according to the Cocoa documentation....)
	}
}

@implementation NSBundle(Io)
+ (NSBundle *)mainBundle
{
	char *path = CSTRING(IoState_doCString_(IoObjcBridge_sharedBridge()->state, "Lobby launchPath"));
	return [NSBundle bundleWithPath:[NSString stringWithCString:path]];
}
@end
