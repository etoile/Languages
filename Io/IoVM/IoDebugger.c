/*#io
Debugger ioDoc(
			docCopyright("Steve Dekorte", 2002)
			docLicense("BSD revised")
			docDescription("Contains methods related to the IoVM debugger.")
			docCategory("Core")
*/

#include "IoDebugger.h"
#include "IoNumber.h"
#include "IoMessage_parser.h"

IoObject *IoDebugger_proto(void *state)
{
    IoMethodTable methodTable[] = 
    { 
	{"vmWillSendMessage", IoObject_self},
	{0x0, 0x0},
    };
    
    IoObject *self = IoObject_new(state);
    IoObject_addMethodTable_(self, methodTable);
    return self;
}
    
/*#io
    docSlot("vmWillSendMessage", 
    "When debugging for one or more actors is turned on 
    (by sending an actor a turnOnMessageDebugging message), a 
    vmWillSendMessage message will be sent to the Debugger object 
    for each message send within the coroutines of the actors being 
    debugged. The first argument will contain the actor and the 
    second argument will contain the message that is about to be 
    sent. You will need to send a resume message to that actor in 
    order for it to continue execution.")
*/
