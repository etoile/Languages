/*#io
WeakLink ioDoc(
               docCopyright("Steve Dekorte", 2002)
               docLicense("BSD revised")
			docCategory("Core")
               docDescription("""A WeakLink is a primitive that can hold a reference to an object without preventing the garbage collector from collecting it. The link reference is set with the setLink() method. After the garbage collector collects an object, it informs any (uncollected) WeakLink objects whose link value pointed to that object by calling their "collectedLink" method.
""")
               */

#include "IoWeakLink.h"
#include "IoSeq.h"
#include "IoState.h"
#include "IoObject.h"
#include "IoNumber.h"

#define DATA(self) ((IoWeakLinkData *)IoObject_dataPointer(self))

IoTag *IoWeakLink_tag(void *state)
{
	IoTag *tag = IoTag_newWithName_("WeakLink");
	tag->state = state;
	tag->cloneFunc   = (TagCloneFunc *)IoWeakLink_rawClone;
	tag->freeFunc    = (TagFreeFunc *)IoWeakLink_free;
	tag->notificationFunc          = (TagNotificationFunc *)IoWeakLink_notification;
	tag->writeToStoreOnStreamFunc  = (TagWriteToStoreOnStreamFunc *)IoWeakLink_writeToStore_stream_;
	tag->readFromStoreOnStreamFunc = (TagReadFromStoreOnStreamFunc *)IoWeakLink_readFromStore_stream_;
	return tag;
}

void IoWeakLink_writeToStore_stream_(IoWeakLink *self, IoStore *store, BStream *stream)
{
	if (DATA(self)->link)
	{
		BStream_writeTaggedInt32_(stream, IoStore_pidForObject_(store, DATA(self)->link));
	}
	else
	{
		BStream_writeTaggedInt32_(stream, 0);	
	}
}

void IoWeakLink_readFromStore_stream_(IoWeakLink *self, IoStore *store, BStream *stream)
{
	PID_TYPE linkid = BStream_readTaggedInt32(stream);
	
	if (linkid != 0)
	{
		IoObject *link = IoStore_objectWithPid_(store, linkid);
		IoWeakLink_rawSetLink(self, link);
	}
}

IoObject *IoWeakLink_proto(void *state) 
{ 
	IoMethodTable methodTable[] = {
	{"setLink", IoWeakLink_setLink},
	{"link", IoWeakLink_link},
	{NULL, NULL},
	};
	
	IoObject *self = IoObject_new(state);
	
	IoObject_setDataPointer_(self, calloc(1, sizeof(IoWeakLinkData)));
	self->tag = IoWeakLink_tag(state);
	DATA(self)->link = 0x0;
	IoState_registerProtoWithFunc_((IoState *)state, self, IoWeakLink_proto);
		
	IoObject_addMethodTable_(self, methodTable);
	return self;
}

IoObject *IoWeakLink_rawClone(IoObject *proto) 
{ 
	IoObject *self = IoObject_rawClonePrimitive(proto);
	IoObject_setDataPointer_(self, calloc(1, sizeof(IoWeakLinkData)));
	DATA(self)->link = 0x0;
	return self;
}

IoObject *IoWeakLink_new(void *state)
{ 
	IoObject *proto = IoState_protoWithInitFunction_((IoState *)state, IoWeakLink_proto);
	return IOCLONE(proto);
}

void IoWeakLink_rawStopListening(IoObject *self)
{
	if (DATA(self)->link) IoObject_removeListener_(DATA(self)->link, self);
}

void IoWeakLink_free(IoObject *self)
{
	IoWeakLink_rawStopListening(self);
	free(IoObject_dataPointer(self));
}

IoObject *IoWeakLink_newWithValue_(void *state, IoObject *v)
{
	IoObject *self = IoWeakLink_new(state);
	DATA(self)->link = v;
	return self;
}

void IoWeakLink_notification(IoObject *self, void *notification) // called when link is freed
{
	DATA(self)->link = 0x0;
	//IoMessage_locals_performOn_(IOSTATE->collectedLinkMessage, self, self);
}

// ----------------------------------------------------------- 

IoObject *IoWeakLink_setLink(IoObject *self, IoObject *locals, IoMessage *m)
{ 
	/*#io
	docSlot("setLink(aValue)", "Sets the link pointer. Returns self.")
	*/
	
	IoWeakLink_rawSetLink(self, IoMessage_locals_valueArgAt_(m, locals, 0));
	return self; 
}

void IoWeakLink_rawSetLink(IoObject *self, IoObject *v)
{
	IoWeakLink_rawStopListening(self);
			
	if (ISNIL(v)) 
	{
		DATA(self)->link = 0x0;
	}
	else
	{
		DATA(self)->link = v; // no IOREF needed since this is a weak link 
		IoObject_addListener_(v, self);
	}
}

IoObject *IoWeakLink_link(IoObject *self, IoObject *locals, IoMessage *m)
{ 
	/*#io
	docSlot("link", "Returns the link pointer or Nil if none is set.")
	*/
	
	IoObject *v = DATA(self)->link;
	return v ? v : IONIL(self); 
}

