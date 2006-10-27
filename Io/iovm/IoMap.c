/*#io
Map ioDoc(
          docCopyright("Steve Dekorte", 2002)
          docLicense("BSD revised")    
          docObject("Map")
          docDescription("A key/value dictionary appropriate for holding large key/value collections.")
		docCategory("DataStructures")
          */

#include "IoMap.h"
#include "IoObject.h"
#include "IoState.h"
#include "IoCFunction.h"
#include "IoSeq.h"
#include "IoState.h"
#include "IoNumber.h"
#include "IoList.h"
#include "IoBlock.h"

#define HASHIVAR(self) ((Hash *)IoObject_dataPointer(self))

IoTag *IoMap_tag(void *state)
{
	IoTag *tag = IoTag_newWithName_("Map");
	tag->state = state;
	tag->freeFunc  = (TagFreeFunc *)IoMap_free;
	tag->cloneFunc = (TagCloneFunc *)IoMap_rawClone;
	tag->markFunc  = (TagMarkFunc *)IoMap_mark;
	tag->writeToStoreOnStreamFunc = (TagWriteToStoreOnStreamFunc *)IoMap_writeToStore_stream_;
	tag->readFromStoreOnStreamFunc = (TagReadFromStoreOnStreamFunc *)IoMap_readFromStore_stream_;
	return tag;
}

void IoMap_writeToStore_stream_(IoMap *self, IoStore *store, BStream *stream)
{
	Hash *hash = HASHIVAR(self);
	IoObject *k = Hash_firstKey(hash);
	
	while (k)
	{
		IoObject *v = (IoObject *)Hash_at_(hash, k);
		BStream_writeTaggedInt32_(stream, IoStore_pidForObject_(store, k));
		BStream_writeTaggedInt32_(stream, IoStore_pidForObject_(store, v));
		k = Hash_nextKey(hash);
	}
	
	BStream_writeTaggedInt32_(stream, 0);
}

void IoMap_readFromStore_stream_(IoMap *self, IoStore *store, BStream *stream)
{
	Hash *hash = HASHIVAR(self);
	
	for (;;)
	{
		int k, v;
		
		k = BStream_readTaggedInt32(stream);
		
		if (k == 0) 
		{
			break;
		}
		
		v = BStream_readTaggedInt32(stream);
		Hash_at_put_(hash, IoStore_objectWithPid_(store, k), IoStore_objectWithPid_(store, v));
	}  
}


IoMap *IoMap_proto(void *state)
{
	IoMethodTable methodTable[] = {
	{"empty",    IoMap_empty}, 
	{"at",       IoMap_at}, 
	{"atPut",    IoMap_atPut}, 
	{"atIfAbsentPut", IoMap_atIfAbsentPut}, 
	{"size",     IoMap_size}, 
	{"keys",     IoMap_keys}, 
	{"values",   IoMap_values}, 
	{"foreach",  IoMap_foreach}, 
	{"hasKey",   IoMap_hasKey}, 
	{"hasValue", IoMap_hasValue}, 
	{"removeAt", IoMap_removeAt}, 
	{NULL, NULL},
	};

    IoObject *self = IoObject_new(state);

    self->tag = IoMap_tag(state);
    IoObject_setDataPointer_(self, Hash_new());

    IoState_registerProtoWithFunc_((IoState *)state, self, IoMap_proto);

    IoObject_addMethodTable_(self, methodTable);
    return self;
}

IoMap *IoMap_rawClone(IoMap *proto) 
{ 
	IoObject *self = IoObject_rawClonePrimitive(proto);
	IoObject_setDataPointer_(self, Hash_clone(HASHIVAR(proto)));
	return self; 
}

IoMap *IoMap_new(void *state)
{
	IoObject *proto = IoState_protoWithInitFunction_((IoState *)state, IoMap_proto);
	return IOCLONE(proto);
}

void IoMap_free(IoMap *self) 
{
	Hash_free(HASHIVAR(self));
}

void IoMap_mark(IoMap *self) 
{ 
	Hash_doOnKeyAndValue_(HASHIVAR(self), (ListDoCallback *)IoObject_shouldMark); 
}

void IoMap_rawAtPut(IoMap *self, IoSymbol *k, IoObject *v)
{
	Hash_at_put_(HASHIVAR(self), IOREF(k), IOREF(v));
}

Hash *IoMap_rawHash(IoMap *self)
{ 
	return HASHIVAR(self); 
}

// ----------------------------------------------------------- 

IoObject *IoMap_empty(IoMap *self, IoObject *locals, IoMessage *m)
{ 
	/*#io
	docSlot("empty", "Removes all keys from the receiver. Returns self.")
	*/
	
	Hash_clean(HASHIVAR(self)); 
	return self; 
}

IoObject *IoMap_rawAt(IoMap *self, IoSymbol *k)
{
	return Hash_at_(HASHIVAR(self), k);
}

IoObject *IoMap_at(IoMap *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("at(keyString, optionalDefaultValue)", 
		   "Returns the value for the key keyString. Returns nil if the key is absent. ")
	*/
	
	IoSymbol *k = IoMessage_locals_symbolArgAt_(m, locals, 0);
	void *result = Hash_at_(HASHIVAR(self), k);
	
	if (!result && IoMessage_argCount(m) > 1) 
	{ 
		return IoMessage_locals_valueArgAt_(m, locals, 1); 
	}
	
	return (result) ? result : IONIL(self);
}

IoObject *IoMap_atPut(IoMap *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("atPut(keyString, aValue)", 
		   "Inserts/sets aValue with the key keyString. Returns self. ")
	*/
	
	IoSymbol *k = IoMessage_locals_symbolArgAt_(m, locals, 0);
	IoObject *v  = IoMessage_locals_valueArgAt_(m, locals, 1);
	IoMap_rawAtPut(self, k, v);
	return self;
}

IoObject *IoMap_atIfAbsentPut(IoMap *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("atIfAbsentPut(keyString, aValue)", 
		   "Inserts/sets aValue with the key keyString if the 
key is not already present. Returns self. ")
	*/
	
	IoSymbol *k = IoMessage_locals_symbolArgAt_(m, locals, 0);
	
	if (Hash_at_(HASHIVAR(self), k) == NULL)
	{
		IoObject *v = IoMessage_locals_valueArgAt_(m, locals, 1);
		IoMap_rawAtPut(self, k, v);
	}
	return self;
}

IoObject *IoMap_size(IoMap *self, IoObject *locals, IoMessage *m)
{ 
	/*#io
	docSlot("size", 
		   "Returns the number of key/value pairs in the receiver.") 
	*/
	
	return IONUMBER(Hash_count(HASHIVAR(self))); 
}

IoObject *IoMap_hasKey(IoMap *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("hasKey(keyString)", 
		   "Returns true if the key is present or false otherwise.")
	*/
	
	IoSymbol *k = IoMessage_locals_symbolArgAt_(m, locals, 0);
	return IOBOOL(self, Hash_at_(HASHIVAR(self), k) != 0x0);
}

IoObject *IoMap_removeAt(IoMap *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("removeAt(keyString)", 
		   "Removes the specified keyString if present. Returns self. ")
	*/
	
	IoSymbol *k = IoMessage_locals_symbolArgAt_(m, locals, 0);
	Hash_removeKey_(HASHIVAR(self), k);
	return self;
}

IoObject *IoMap_hasValue(IoMap *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("hasValue(aValue)", 
		   "Returns true if the value is one of the Map's values or false otherwise.")
	*/
	
	IoList *values = IoMap_values(self, locals, m);
	return IoList_contains(values, locals, m);
}

IoList *IoMap_rawKeys(IoMap *self)
{
	IoList *list = IoList_new(IOSTATE);
	IoObject *key = (IoObject *)Hash_firstKey(HASHIVAR(self));
	
	while (key)
	{
		IoList_rawAppend_(list, key);
		key = Hash_nextKey(HASHIVAR(self));
	}
	
	return list;
}

IoList *IoMap_keys(IoMap *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("keys", "Returns a List of the receivers keys.")
	*/
	
	return IoMap_rawKeys(self);
}

IoObject *IoMap_values(IoMap *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("values", 
		   "Returns a List of the receivers values.")
	*/
	
	IoList *list = IoList_new(IOSTATE);
	IoObject *key = (IoObject *)Hash_firstKey(HASHIVAR(self));
	
	while (key)
	{
		IoList_rawAppend_(list, (IoObject *)Hash_at_(HASHIVAR(self), key));
		key = Hash_nextKey(HASHIVAR(self));
	}
	
	return list;
}

IoObject *IoMap_foreach(IoMap *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("foreach(optionalKey, value, message)", 
		   """For each key value pair, sets the locals key to 
the key and value to the value and executes message.
Example:
<pre>aMap foreach(k, v, writeln(k, " = ", v))
aMap foreach(v, write(v))</pre>

Example use with a block:

<pre>myBlock = block(k, v, write(k, " = ", v, "\n")) 
aMap foreach(k, v, myBlock(k, v))</pre>""")
	*/
	
	IoState *state = IOSTATE;
	IoSymbol *keyName, *valueName;
	IoMessage *doMessage;
	IoObject *key = (IoObject *)Hash_firstKey(HASHIVAR(self));
	IoObject *result = IONIL(self);    
	IoMessage_foreachArgs(m, self, &keyName, &valueName, &doMessage);
	IoState_pushRetainPool(state);
	
	while (key)
	{
		IoState_clearTopPool(state);
		{
			IoObject *value = (IoObject *)Hash_at_(HASHIVAR(self), key);
			
			if (keyName)
			{
				IoObject_setSlot_to_(locals, keyName, key);
			}
			
			IoObject_setSlot_to_(locals, valueName, value);
			IoMessage_locals_performOn_(doMessage, locals, locals);
			key = Hash_nextKey(HASHIVAR(self));
			
			if (IoState_handleStatus(IOSTATE)) 
			{
				goto done;
			}
		}
	}
	
done:
		IoState_popRetainPoolExceptFor_(state, result);
	return result;
}
