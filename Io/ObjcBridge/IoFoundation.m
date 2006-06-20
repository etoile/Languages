#include <Foundation/Foundation.h>
#include "IoFoundation.h"

#define AS_NUMBER(x) \
        [foundation appendFormat: @#x":=%d\n", x];

void IoFoundation_Init(IoState *self, IoObject *context)
{
  NSMutableString *foundation = [[NSMutableString alloc] init];

  AS_NUMBER(YES);
  AS_NUMBER(NO);

  IoState_on_doCString_withLabel_(self, context, [foundation cString], "Foundation.io");
  [foundation dealloc];
}

