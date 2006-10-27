/*#io
Sandbox ioDoc(
		    docCopyright("Steve Dekorte", 2002)
		    docLicense("BSD revised")
		    docObject("Sandbox")
		    docDescription("Sandbox can be used to run separate instances of Io within the same process.")
		    docCategory("Core")
		    */

#include "IoSandbox.h"
#include "IoSeq.h"
#include "IoState.h"
#include "IoCFunction.h"
#include "IoObject.h"
#include "IoList.h"
#include "IoSeq.h"
#include "ByteArray.h"
#include "PortableTruncate.h"
#include <errno.h>
#include <stdio.h>

#define DATA(self) ((IoSandboxData *)IoObject_dataPointer(self))

IoTag *IoSandbox_tag(void *state)
{
	IoTag *tag = IoTag_newWithName_("Sandbox");
	tag->state = state;
	tag->cloneFunc = (TagCloneFunc *)IoSandbox_rawClone;
	tag->freeFunc  = (TagFreeFunc *)IoSandbox_free;
	tag->writeToStoreOnStreamFunc  = (TagWriteToStoreOnStreamFunc *)IoSandbox_writeToStore_stream_;
	tag->readFromStoreOnStreamFunc = (TagReadFromStoreOnStreamFunc *)IoSandbox_readFromStore_stream_;
	return tag;
}

IoSandbox *IoSandbox_proto(void *state)
{
	IoMethodTable methodTable[] = {
	{"messageCount", IoSandbox_messageCount},
	{"setMessageCount", IoSandbox_setMessageCount},
	{"timeLimit", IoSandbox_timeLimit},
	{"setTimeLimit", IoSandbox_setTimeLimit},
	{"doSandboxString", IoSandbox_doSandboxString},
	{NULL, NULL},
	};
	
	IoObject *self = IoObject_new(state);
	self->tag = IoSandbox_tag(state);
	
	IoState_registerProtoWithFunc_((IoState *)state, self, IoSandbox_proto);
	
	IoObject_addMethodTable_(self, methodTable);
	
	return self;
}

#define BOXSTATE(self) ((IoState *)(IoObject_dataPointer(self)))

IoState *IoSandbox_boxState(IoSandbox *self)
{
	if (!IoObject_dataPointer(self))
	{
		IoObject_setDataPointer_(self, IoState_new());
		IoSandbox_addPrintCallback(self);
	}
	
	return (IoState *)(IoObject_dataPointer(self));
}

void IoSandbox_addMakeSecureMethod(IoSandbox *self)
{
	/*#io
	docSlot("makeSecure", 
		   "Removes Collector DynLib File Directory Debugger Store Sandbox objects.")
	*/
	
	char *s = "makeSecure := method("
	"Object removeSlots := method(\n"
	"    stackLoop := block(\n"
	"        stackName := thisMessage argAt(0) name\n"
	"        indexName := thisMessage argAt(1) name\n"
	"        stack := list(sender doMessage(thisMessage argAt(2)))\n"
	"        body := thisMessage argAt(3)\n"
	"\n"
	"        sender setSlot(stackName, stack)\n"
	"        while(stack count > 0,\n"
	"            sender setSlot(indexName, stack pop)\n"
	"            sender doMessage(body)\n"
	"        )\n"
	"    )\n"
	"\n"
	"    stackLoop(stack, m, thisMessage arguments first,\n"
	"        self removeSlot(m name)\n"
	"        if (m nextMessage != Nil, stack add(m nextMessage))\n"
	"        if (m attachedMessage != Nil, stack add(m attachedMessage))\n"
	"    )\n"
	")\n"
	"\n"
	"# Remove some of the Objects from IoVM\n"
	"IoVM removeSlots(Collector DynLib File Directory Debugger Store Sandbox)\n"
	"\n"
	"# Remove the dangerous CFunctions\n"
	"System removeSlots(getenv system exit)\n"
	"Object removeSlots(doFile launchFile turnOnMessageDebugging turnOffMessageDebugging)\n"
	"\n"
//    "# Remove the printing CFunctions\n"
//    "(?keepPrintingCFunctions) ifNil(\n"
//    "    Number removeSlots(print linePrint)\n"
//    "    Duration removeSlots(print)\n"
//    "    String removeSlots(print linePrint)\n"
//    "    Nil removeSlots(print)\n"
//    "    Date removeSlots(print)\n"
//    "    Buffer removeSlots(print)\n"
//    "    Locals removeSlots(print write writeln)\n"
//    "    Block removeSlots(print)\n"
//    "    Object removeSlots(print write writeln)\n"
//    ")\n"
//    "\n"
	"# Remove the Protos we don't need\n"
	"#Protos removeSlots(IoServer IoDesktop)\n"
	"\n"
	"# Clean up\n"
	"Object removeSlot(\"removeSlots\")\n"
	")"
	;
	IoObject_rawDoString_label_(self, IOSYMBOL(s), IOSYMBOL("[Sandbox]"));
}

IoSandbox *IoSandbox_rawClone(IoSandbox *proto)
{
	IoObject *self = IoObject_rawClonePrimitive(proto);
	return self;
}

void IoSandbox_addPrintCallback(IoSandbox *self)
{
	IoState *boxState = IoSandbox_boxState(self);
	IoState_callbackContext_(boxState, self);
	IoState_printCallback_(boxState, (IoStatePrintCallback *)IoSandbox_printCallback);  
}

void IoSandbox_printCallback(IoSandbox *self, const char *s)
{
	IoState *state = IOSTATE;
	IoSeq *buf = IOSEQ((const unsigned char *)s, (unsigned int)strlen(s));
	IoMessage *m = IoMessage_newWithName_(state, IOSYMBOL("printCallback"));
	IoMessage *arg = IoMessage_newWithName_returnsValue_(state, IOSYMBOL("buffer"), buf);
	IoMessage_addArg_(m, arg);
	IoMessage_locals_performOn_(m, state->lobby, self);
}

IoSandbox *IoSandbox_new(void *state)
{
	IoObject *proto = IoState_protoWithInitFunction_((IoState *)state, IoSandbox_proto);
	return IOCLONE(proto);
}

void IoSandbox_free(IoSandbox *self)
{
	if (IoObject_dataPointer(self)) 
	{
		IoState_free(IoSandbox_boxState(self));
	}
}

void IoSandbox_writeToStore_stream_(IoSandbox *self, IoStore *store, BStream *stream)
{
}

void *IoSandbox_readFromStore_stream_(IoSandbox *self, IoStore *store, BStream *stream)
{
	return self;
}

/* ----------------------------------------------------------- */

IoNumber *IoSandbox_messageCount(IoSandbox *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("messageCount", 
		   "Returns a number containing the messageCount limit of the Sandbox. ")
	*/
	
	IoState *boxState = IoSandbox_boxState(self);
	return IONUMBER(boxState->messageCountLimit);
}

IoObject *IoSandbox_setMessageCount(IoSandbox *self, IoObject *locals, IoMessage *m)
{ 
	/*#io
	docSlot("setMessageCount(anInteger)", 
		   "Sets the messageCount limit of the receiver. ")
	*/
	
	IoState *boxState = IoSandbox_boxState(self);
	boxState->messageCountLimit = IoMessage_locals_intArgAt_(m, locals, 0);
	return self; 
}

IoNumber *IoSandbox_timeLimit(IoSandbox *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("timeLimit", 
		   "Returns a number containing the time limit of calls made to the Sandbox. ")
	*/
	
	IoState *boxState = IoSandbox_boxState(self);
	return IONUMBER(boxState->timeLimit);
}

IoObject *IoSandbox_setTimeLimit(IoSandbox *self, IoObject *locals, IoMessage *m)
{ 
	/*#io
	docSlot("setTimeLimit(aDouble)", 
		   "Sets the time limit of the Sandbox. ")
	*/
	
	IoState *boxState = IoSandbox_boxState(self);
	boxState->timeLimit = IoMessage_locals_doubleArgAt_(m, locals, 0);
	return self; 
}

IoObject *IoSandbox_doSandboxString(IoSandbox *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("doSandboxString(aString)", 
		   "Evaluate aString instead the Sandbox. ")
	*/
	
	IoState *boxState = IoSandbox_boxState(self);
	char *s = IoMessage_locals_cStringArgAt_(m, locals, 0);
	
	IoObject *result = IoState_doSandboxCString_(boxState, s);
	
	if (ISSYMBOL(result))
	{
		return IOSYMBOL(CSTRING(result));
	}
	
	if (ISNUMBER(result))
	{
		return IONUMBER(CNUMBER(result));
	}    
	
	return IONIL(self);
}
