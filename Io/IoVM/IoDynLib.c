/*#io
DynLib ioDoc(
		   docCopyright("Steve Dekorte", 2002)
		   docLicense("BSD revised")
		   docDescription("A DLL Loader by Kentaro A. Kurahone <kurahone@sigusr1.org>.")
		   docCategory("Core")
		   */

#include "IoObject.h"
#include "IoState.h"
#include "IoSeq.h"
#include "IoBlock.h"
#include "IoNumber.h"
#include "IoList.h"
#include "IoMessage.h"
#include "IoDynLib.h"
#include "DynLib.h"

#define DATA(self) ((DynLib *)IoObject_dataPointer(self))

static IoTag *IoDynLib_tag(void *state)
{
	IoTag *tag = IoTag_newWithName_("DynLib");
	tag->state = state;
	tag->cloneFunc = (TagCloneFunc *) IoDynLib_rawClone;
	tag->freeFunc  = (TagFreeFunc *)  IoDynLib_free;
	return tag;
}

IoObject *IoDynLib_proto(void *state)
{
	IoMethodTable methodTable[] = {
	{"setPath", IoDynLib_setPath},
	{"path", IoDynLib_path},
	{"setInitFuncName", IoDynLib_setInitFuncName},
	{"initFuncName", IoDynLib_initFuncName},
	{"setFreeFuncName", IoDynLib_setFreeFuncName},
	{"freeFuncName", IoDynLib_freeFuncName},
	{"open", IoDynLib_open},
	{"close", IoDynLib_close},
	{"isOpen", IoDynLib_isOpen},
	{"call", IoDynLib_call},
	{"callPluginInit", IoDynLib_callPluginInitFunc},
	{"returnsString", IoDynLib_returnsString},
	{NULL, NULL},
	};
	
	IoObject *self = IoObject_new(state);
	self->tag = IoDynLib_tag(state);
	IoObject_setDataPointer_(self, DynLib_new());
	DynLib_setInitArg_(DATA(self), state);
	DynLib_setFreeArg_(DATA(self), state);
	IoState_registerProtoWithFunc_((IoState *)state, self, IoDynLib_proto);
	
	IoObject_addMethodTable_(self, methodTable);
	return self;
}

IoDynLib *IoDynLib_new(void *state)
{ 
	IoDynLib *proto = IoState_protoWithInitFunction_((IoState *)state, IoDynLib_proto);
	return IOCLONE(proto); 
}

IoDynLib *IoDynLib_rawClone(IoDynLib *proto)
{
	/* 
	Note that due to the nature of this object, a clone will *NOT* inherit
	 it's parent's dynamically loaded object.
	 */
	
	IoObject *self = IoObject_rawClonePrimitive(proto);
	IoObject_setDataPointer_(self, DynLib_new());
	DynLib_setInitArg_(DATA(self), IOSTATE);
	DynLib_setFreeArg_(DATA(self), IOSTATE);
	return self;
}

void IoDynLib_free(IoDynLib *self)
{
	DynLib_free(DATA(self));
}

IoDynLib *IoDynLib_setPath(IoDynLib *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("setPath(aString)", 
		   "Sets the path to the dynamic library. Returns self.")
	*/
	
	DynLib_setPath_(DATA(self), 
				 CSTRING(IoMessage_locals_symbolArgAt_(m, locals, 0)));
	return self;
}

IoDynLib *IoDynLib_path(IoDynLib *self, IoObject *locals, IoMessage *m)
{ 
	/*#io
	docSlot("path", "Returns the path to the dynamic library.")
	*/
	
	return IOSYMBOL(DynLib_path(DATA(self))); 
}

IoDynLib *IoDynLib_setInitFuncName(IoDynLib *self, IoObject *locals, IoMessage *m)
{ 
	/*#io
	docSlot("setInitFuncName(aString)", 
		   "Sets the initialization function name for the dynamic library. Returns self.")
	*/
	
	DynLib_setInitFuncName_(DATA(self), 
					    CSTRING(IoMessage_locals_symbolArgAt_(m, locals, 0))); 
	return self;
}

IoDynLib *IoDynLib_initFuncName(IoDynLib *self, IoObject *locals, IoMessage *m)
{ 
	/*#io
	docSlot("initFuncName", "Returns the initialization function name.")
	*/
	
	return IOSYMBOL(DynLib_initFuncName(DATA(self))); 
}

IoDynLib *IoDynLib_setFreeFuncName(IoDynLib *self, IoObject *locals, IoMessage *m)
{ 
	/*#io
	docSlot("setFreeFuncName(aString)", "Sets the free function name. Returns self.")
	*/
	
	DynLib_setFreeFuncName_(DATA(self), 
					    CSTRING(IoMessage_locals_symbolArgAt_(m, locals, 0)));
	return self;
}

IoDynLib *IoDynLib_freeFuncName(IoDynLib *self, IoObject *locals, IoMessage *m)
{ 
	/*#io
	docSlot("freeFuncName", "Returns the free function name.")
	*/
	
	return IOSYMBOL(DynLib_freeFuncName(DATA(self))); 
}

IoDynLib *IoDynLib_open(IoDynLib *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("open", 
		   "Opens the dynamic library and returns self or raises a DynLoad.open Error if there is an error. ")
	*/
	
	if (IoMessage_argCount(m)) 
	{
		IoDynLib_setPath(self, locals, m);
	}
	
	DynLib_open(DATA(self));
	
	if (DynLib_error(DATA(self)))
	{
		IoState_error_(IOSTATE, m, "Error loading object '%s': '%s'", 
					DynLib_path(DATA(self)), DynLib_error(DATA(self)));
	}
	
	return self;
}

IoDynLib *IoDynLib_close(IoDynLib *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("close", 
		   "Closes the library. Returns self.")
	*/
	
	DynLib_close(DATA(self));
	return self;
}

IoDynLib *IoDynLib_isOpen(IoDynLib *self, IoObject *locals, IoMessage *m)
{ 
	/*#io
	docSlot("isOpen", "Returns true if the library is open, or false otherwise.")
	*/
	
	return IOBOOL(self, DynLib_isOpen(DATA(self)));
}

int bouncer(IoBlock *self, int ret, int a, int b, int c, int d, int e)
{
	IoObject *lobby = IoState_lobby(IOSTATE);
	IoNumber *n;
	static IoMessage *m = NULL;
	List *argNames = ((IoBlockData*)IoObject_dataPointer(self))->argNames;
	
	if (m == NULL)
		m = IoMessage_new(IOSTATE);
	if (0 < argNames->size)
		IoMessage_setCachedArg_toInt_(m, 0, a);
	if (1 < argNames->size)
		IoMessage_setCachedArg_toInt_(m, 1, b);
	if (2 < argNames->size)
		IoMessage_setCachedArg_toInt_(m, 2, c);
	if (3 < argNames->size)
		IoMessage_setCachedArg_toInt_(m, 3, d);
	if (4 < argNames->size)
		IoMessage_setCachedArg_toInt_(m, 4, e);
	
	n = IoBlock_activate(self, lobby, lobby, m, lobby);
	
	if (ISNUMBER(n))
	{
		return IoNumber_asInt(n);
	}
	
	return 0;
}

unsigned int marshal(IoDynLib *self, IoObject *arg)
{
	unsigned int n = 0;
	
	if (ISNUMBER(arg))
	{
		n = IoNumber_asInt(arg);
	}
	else if (ISSYMBOL(arg))
	{
		n = (unsigned int)CSTRING(arg);
	}
	else if (ISLIST(arg)) 
	{
		int i;
		unsigned int *l = malloc(IoList_rawSize(arg) * sizeof(unsigned int));
		for (i = 0; i < IoList_rawSize(arg); i ++)
			l[i] = marshal(self, List_at_(IoList_rawList(arg), i));
		n = (unsigned int)l;
	} 
	else if (ISBUFFER(arg)) 
	{
		n = (unsigned int)IoSeq_rawBytes(arg);
	} 
	else if (ISBLOCK(arg)) 
	{
		unsigned char *blk = malloc(20), *p = blk;
		// FIXME: need trampoline code for other architectures
		*p++ = 0x68;
		*((int *)p) = (int)arg;
		p += sizeof(int);
		*p++ = 0xb8;
		*((int *)p) = (int)bouncer;
		p += sizeof(int);
		*p++ = 0xff;
		*p++ = 0xd0;
		*p++ = 0x83;
		*p++ = 0xc4;
		*p++ = 0x04;
		*p++ = 0xc3;
		n = (unsigned int)blk;
	} 
	else 
	{
		return (unsigned int)IONIL(self);
	}
	
	return n;
}

IoObject *demarshal(IoObject *self, IoObject *arg, unsigned int n)
{
	if (ISNUMBER(arg)) 
	{
		return IONUMBER(n);
	} 
	else if (ISSYMBOL(arg)) 
	{
		if (n == 0)
			return IOSYMBOL("");
		return IOSYMBOL((char*)n);
	} 
	else if (ISLIST(arg)) 
	{
		unsigned int *values = (unsigned int *)n;
		int i;
		
		for (i = 0; i < IoList_rawSize(arg); i ++) 
		{
			IoObject *value = List_at_(IoList_rawList(arg), i);
			List_at_put_(IoList_rawList(arg), i, demarshal(self, value, values[i]));
		}
		
		free(values);
		return arg;
	} 
	else if (ISBUFFER(arg)) 
	{
		return arg;
	} 
	else if (ISBLOCK(arg)) 
	{
		return arg;
	}
	
	return IONIL(self);
}

IoDynLib *IoDynLib_call(IoDynLib *self, IoObject *locals, IoMessage *m)
{
	int n, rc = 0;
	unsigned int *params = NULL;
	void *f = DynLib_pointerForSymbolName_(DATA(self), 
								    CSTRING(IoMessage_locals_symbolArgAt_(m, locals, 0)));
	if (f == NULL) 
	{
		IoState_error_(IOSTATE, m, "Error resolving call '%s'.", 
					CSTRING(IoMessage_locals_symbolArgAt_(m, locals, 0)));
		return IONIL(self);
	}
	
	if (IoMessage_argCount(m) > 9) 
	{
		IoState_error_(IOSTATE, m, "Error, too many arguments (%i) to call '%s'.", 
					IoMessage_argCount(m) - 1,
					CSTRING(IoMessage_locals_symbolArgAt_(m, locals, 0)));
		return IONIL(self);
	}
	
	if (IoMessage_argCount(m) > 1)
		params = malloc(IoMessage_argCount(m) * sizeof(unsigned int));
	
	for (n = 0; n < IoMessage_argCount(m) - 1; n++) 
	{
		IoObject *arg = IoMessage_locals_valueArgAt_(m, locals, n + 1);
		params[n] = marshal(self, arg);
		
		if ((IoObject*)params[n] == IONIL(self)) 
		{
			IoState_error_(IOSTATE, m, "Error marshalling argument (%i) to call '%s'.", 
						n + 1,
						CSTRING(IoMessage_locals_symbolArgAt_(m, locals, 0)));
			// FIXME this can leak memory.
			free(params);
			return IONIL(self);
		}
	}
	
#if 0
	printf("calling %s with %i arguments\n", 
		  CSTRING(IoMessage_locals_symbolArgAt_(m, locals, 0)),
		  IoMessage_argCount(m) - 1);
#endif
	
	switch(IoMessage_argCount(m) - 1) {
		case 0:
			rc = ((int (*)())f)();
			break;
		case 1:
			rc = ((int (*)(int))f)(params[0]);
			break;
		case 2:
			rc = ((int (*)(int,int))f)(params[0], params[1]);
			break;
		case 3:
			rc = ((int (*)(int,int,int))f)(params[0], params[1], params[2]);
			break;
		case 4:
			rc = ((int (*)(int,int,int,int))f)(params[0], params[1], params[2], params[3]);
			break;
		case 5:
			rc = ((int (*)(int,int,int,int,int))f)(params[0], params[1], params[2], params[3], params[4]);
			break;
		case 6:
			rc = ((int (*)(int,int,int,int,int,int))f)(params[0], params[1], params[2], params[3], params[4], params[5]);
			break;
		case 7:
			rc = ((int (*)(int,int,int,int,int,int,int))f)(params[0], params[1], params[2], params[3], params[4], params[5], params[6]);
			break;
		case 8:
			rc = ((int (*)(int,int,int,int,int,int,int,int))f)(params[0], params[1], params[2], params[3], params[4], params[5], params[6], params[7]);
			break;
	}
	
	for (n = 0; n < IoMessage_argCount(m) - 1; n ++) 
	{
		IoObject *arg = IoMessage_locals_valueArgAt_(m, locals, n + 1);
		demarshal(self, arg, params[n]);
	}
	
	free(params);
	
	return IONUMBER(rc);
}

IoDynLib *IoDynLib_callPluginInitFunc(IoDynLib *self, IoObject *locals, IoMessage *m)
{
	int rc = 0;
	unsigned int *params = NULL;
	void *f = DynLib_pointerForSymbolName_(DATA(self), 
								    CSTRING(IoMessage_locals_symbolArgAt_(m, locals, 0)));
	if (f == NULL) 
	{
		IoState_error_(IOSTATE, m, "Error resolving call '%s'.", 
					CSTRING(IoMessage_locals_symbolArgAt_(m, locals, 0)));
		return IONIL(self);
	}
	
	if (IoMessage_argCount(m) < 1) 
	{
		IoState_error_(IOSTATE, m, "Error, you must give an init function name to check for.");
		return IONIL(self);
	}
	
	params = malloc(sizeof(unsigned int) * 2);

	params[0] = (unsigned int)IOSTATE;
	params[1] = (unsigned int)IOSTATE->lobby;
	rc = ((int (*)(int,int))f)(params[0], params[1]);
		
	return IONUMBER(rc);
}

IoSeq *IoDynLib_returnsString(IoDynLib *self, IoObject *locals, IoMessage *m)
{
	int n = IoNumber_asInt(IoMessage_locals_numberArgAt_(m, locals, 0));
	
	if (n == 0)
	{
		return IOSYMBOL("");
	}
	
	return IOSYMBOL((char*)n);
}

