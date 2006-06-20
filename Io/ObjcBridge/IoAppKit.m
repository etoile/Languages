#include <AppKit/AppKit.h>
#include "IoAppKit.h"

#define AS_NUMBER(x) \
        [appkit appendFormat: @#x":=%d\n", x];

void IoAppKit_Init(IoState *self, IoObject *context)
{
  NSMutableString *appkit = [[NSMutableString alloc] init];

  /* NSWindow */
  AS_NUMBER(NSBorderlessWindowMask);
  AS_NUMBER(NSTitledWindowMask);
  AS_NUMBER(NSClosableWindowMask);
  AS_NUMBER(NSMiniaturizableWindowMask);
  AS_NUMBER(NSResizableWindowMask);
 
  AS_NUMBER(NSBackingStoreRetained);
  AS_NUMBER(NSBackingStoreNonretained);
  AS_NUMBER(NSBackingStoreBuffered);

  IoState_on_doCString_withLabel_(self, context, [appkit cString], "AppKit.io");
  [appkit dealloc];
}

