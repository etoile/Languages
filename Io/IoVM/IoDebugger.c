/*#io
Debugger ioDoc(
			docCopyright("Steve Dekorte", 2002)
			docLicense("BSD revised")
			docDescription("Contains methods related to the IoVM debugger.")
			docCategory("Core")
*/

#include "IoDebugger.h"
#include "IoMessage_parser.h"

IoObject *IoDebugger_proto(void *state)
{
    IoMethodTable methodTable[] = 
    { 
    {0x0, 0x0},
    };
    
    IoObject *self = IoObject_new(state);
    IoObject_addMethodTable_(self, methodTable);
    return self;
}

