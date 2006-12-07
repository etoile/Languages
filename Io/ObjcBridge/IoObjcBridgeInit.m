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

        char *s;

        s = "Message end := method(if(next, next end, self))\n"
        "\n"
        "Message insertNext := method(m,\n"
        "       if(next, m end setNext(next))\n"
        "       setNext(m)\n"
        ")\n"
        "\n"
        "Message makeColonUnary := method(l,\n"
        "       if(next) then(\n"
        "               if(next name == \":\") then(\n"
        "                       arg := next arguments first\n"
        "                       while(arg next and arg next next, arg := arg next)\n"
        "                       if(arg next, next insertNext(arg next); arg setNext(nil))\n"
        "               )\n"
        "               next makeColonUnary\n"
        "       )\n"
        "       self\n"
        ")\n"
        "\n"
        "Message joinColons := method(l,\n"
        "       if(next) then(\n"
        "               if(next name == \":\") then(\n"
        "                       self setName(self name .. \":\")\n"
        "                       self setArguments(next arguments)\n"
        "                       self setNext(next next)\n"
        "               )\n"
        "               next ?joinColons\n"
        "       )\n"
        "       self\n"
        ")\n"
        "\n"
        "Message unInfix := method(l,\n"
        "       if(self name endsWithSeq(\":\") and next and next name endsWithSeq(\":\")) then(\n"
        "               self setName(self name .. next name)\n"
        "               self setArguments(self arguments appendSeq(next arguments))\n"
        "               self setNext(next next)\n"
        "       )\n"
        "       next ?unInfix\n"
        "       self\n"
        ")\n"
        "\n"
        "squareBrackets := method(\n"
        "       call message argAt(0) makeColonUnary joinColons unInfix\n"
        "       call message setName(\"\")\n"
        "       call sender doMessage(call message argAt(0), call sender)\n"
        ")\n"
        "\n"
        "setSlot(\"addVariableNamed:\",\n"
        "       method(name,\n"
        "               self setSlot(name, doString(\"method(?_\" .. name .. \")\"))\n"
        "               self setSlot(\"set\" .. name asCapitalized .. \":\", doString(\"method(value, self _\" .. name .. \" := value ; self)\"))\n"
        "               nil\n"
        "       )\n"
        ")\n"
        "\n"
        "NSMakePoint := method(x, y, Point clone set(x, y))\n"
        "NSMakeSize := method(w, h, Point clone set(w, h))\n"
        "NSMakeRect := method(x, y, w, h, Box clone set(NSMakePoint(x, y), NSMakeSize(w, h)))\n"
        "\n"
        "//NSToolbarDisplayMode\n"
        "NSToolbarDisplayModeDefault      := 0\n"
        "NSToolbarDisplayModeIconAndLabel := 1\n"
        "NSToolbarDisplayModeIconOnly     := 2\n"
        "NSToolbarDisplayModeLabelOnly    := 3\n"
        "\n"
        "//NSToolbarSizeMode\n"
        "NSToolbarSizeModeDefault := 0\n"
        "NSToolbarSizeModeRegular := 1\n"
        "NSToolbarSizeModeSmall   := 2\n"
        "";

        IoState_rawOn_doCString_withLabel_(self, context, s, "ObjcBridge.io");

	if (sharedBridge)
	{
		IoObject_setSlot_to_(context, IoState_symbolWithCString_(self, "ObjcBridge"), sharedBridge);
	}
	else
	{
		IoObject_setSlot_to_(context, IoState_symbolWithCString_(self, "ObjcBridge"), IoObjcBridge_proto(self));
		Io2Objc_proto(self);
//		NSLog(@"ObjcBridge"); /* without that line the runtime goes mad, do not remove it */
		[[NSAutoreleasePool alloc] init]; /* hack */
		// we need a toplevel autorelease pool to have memory-safe exception handling
		// in Cocoa (pools need not to be released before throwing an NSException
		// (this according to the Cocoa documentation....)
	}
}

@implementation NSBundle(Io)
static NSBundle *_mainBundle;
+ (NSBundle *)mainBundle
{
	if (!_mainBundle)
	{
		char *path = CSTRING(IoState_doCString_(IoObjcBridge_sharedBridge()->state, "Lobby launchPath"));
		_mainBundle = [[self alloc] initWithPath:[NSString stringWithCString:path]];
	}
	return _mainBundle;
}
@end
