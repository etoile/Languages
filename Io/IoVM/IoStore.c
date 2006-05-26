/*#io
Store ioDoc(
		  docCopyright("Steve Dekorte", 2002)
		  docLicense("BSD revised")
		  docObject("Store")
		  docDescription("This object can be used to store the Io state in a way similar to a Smalltalk system image. However, objects are stored as individual indexed key/value records, so it opens the possibility of lazy loading and using the Io namespace itself as a high performance, transactional database.")
		  */

#include "IoStore.h"
#include "IoObject_persistence.h"
#include "IoState.h"
#include "IoNumber.h"
#include "IoList.h"
//#include "IoPObject.h"
#include "ByteArray.h"

#define DATA(self) ((IoStoreData *)IoObject_dataPointer(self))

IoTag *IoStore_tag(void *state)
{
	IoTag *tag = IoTag_newWithName_("Store");
	tag->state = state;
	tag->cloneFunc = (TagCloneFunc *)IoStore_rawClone;
	tag->freeFunc  = (TagFreeFunc *)IoStore_free;
	tag->markFunc  = (TagMarkFunc *)IoStore_mark;
	return tag;
}

IoStore *IoStore_proto(void *state)
{
	IoObject *self = IoObject_new(state);
	self->tag = IoStore_tag(state);
	
	IoObject_setDataPointer_(self, calloc(1, sizeof(IoStoreData)));
	
	DATA(self)->path = IoState_symbolWithCString_((IoState *)state, "DefaultStore");
	DATA(self)->pidToObject = Hash_new();
	DATA(self)->objectsToSave = List_new();
	DATA(self)->tmpKeyStream   = BStream_new();
	DATA(self)->tmpValueStream = BStream_new();
	DATA(self)->debug = 0;
	
	IoState_registerProtoWithFunc_((IoState *)state, self, IoStore_proto);
	
	IoStore_addMethods(self);
	return self;
}

IoStore *IoStore_rawClone(IoStore *self) 
{
	// singleton 
	return self; 
} 

void IoStore_addMethods(IoStore *self) 
{ 
	IoMethodTable methodTable[] = {
	{"path", IoStore_path},
	{"setPath", IoStore_setPath},
	{"store", IoStore_store},
	{"load", IoStore_load},
	{NULL, NULL},
	};
	
	IoObject_addMethodTable_(self, methodTable);
} 

// ----------------------------------------------------------- 

IoStore *IoStore_new(void *state)
{
	IoObject *proto = IoState_protoWithInitFunction_((IoState *)state, IoStore_proto);
	return IOCLONE(proto);
}

void IoStore_free(IoStore *self)
{ 
	IoStoreData *data = DATA(self);
	
	Hash_free(data->pidToObject);
	List_free(data->objectsToSave);
	BStream_free(data->tmpKeyStream);
	BStream_free(data->tmpValueStream);
	free(data); 
}

void IoStore_mark(IoStore *self) 
{ 
	IoObject_shouldMark(DATA(self)->path);
}

int IoStore_debugIsOn(IoStore *self)
{
	return DATA(self)->debug;
}

BStream *IoStore_valueStream(IoStore *self)
{
	return DATA(self)->tmpValueStream;
}

IoObject *IoStore_path(IoStore *self, IoObject *locals, IoMessage *m)
{
	return DATA(self)->path;
}

IoObject *IoStore_setPath(IoStore *self, IoObject *locals, IoMessage *m)
{
	IoSymbol *p = IoMessage_locals_seqArgAt_(m, locals, 0);
	DATA(self)->path = IOREF(p);
	return self;
}

// store exec ---------------------------------------------------- 

SkipDBM *IoStore_sdbm(IoStore *self)
{
	return IOSTATE->sdbm;
}

UDB *IoStore_udb(IoStore *self)
{
	return SkipDBM_udb(IOSTATE->sdbm);
}

void IoStore_beginTransaction(IoStore *self)
{
	SkipDBM_beginTransaction(IoStore_sdbm(self));
}

void IoStore_commitTransaction(IoStore *self)
{
	SkipDBM_commitTransaction(IoStore_sdbm(self));
}

void IoStore_openIfNeeded(IoStore *self)
{
	SkipDBM *sdbm = IoStore_sdbm(self);
	
	if (!SkipDBM_isOpen(sdbm))
	{
		SkipDBM_setPath_(sdbm, CSTRING(DATA(self)->path));
		SkipDBM_open(sdbm);
		/*
		 {
			 IoState_error_(IOSTATE, m, "unable to open Store");
			 return self;
		 }
		 */
	}
}

IoObject *IoStore_store(IoStore *self, IoObject *locals, IoMessage *m)
{ 
	//IoObject *obj = IoMessage_locals_valueArgAt_(m, locals, 0);
	IoObject *obj = IOSTATE->lobby;
	UDB *udb = IoStore_udb(self);
	
	DATA(self)->context.self    = self;
	DATA(self)->context.locals  = locals;
	DATA(self)->context.message = m;
	
	IoStore_openIfNeeded(self);
	
	IoStore_beginTransaction(self);
	// create a place holder for the lobby record, if needed 
	
	if (UDB_at_(udb, 2).size == 0)
	{
		PID_TYPE pid = UDB_put_(udb, Datum_Empty()); 
		
		if (pid != 2)
		{
			printf("wrong pid - expected 2 but got %" PID_FORMAT "\n", pid);
		}
	}
	
	obj->isDirty = 1;
	
	{
		PID_TYPE pid;
		IoObject_setPersistentId_(obj, 2);
		pid = IoStore_pidForObject_(self, obj);
		
		if (pid != 2)
		{
			printf("wrong pid for Lobby - expected 2 but got %" PID_FORMAT "\n", pid);
			exit(1);
		}
	}
	
	IoStore_finishSave(self);
	IoStore_commitTransaction(self);
	
	return self; 
}

IoObject *IoStore_load(IoStore *self, IoObject *locals, IoMessage *m)
{ 
	IoObject *root = IONIL(self);
	
	DATA(self)->context.self = self;
	DATA(self)->context.locals = locals;
	DATA(self)->context.locals = m;
	
	IoState_pushCollectorPause(IOSTATE);
	IoStore_openIfNeeded(self);
	root = IoStore_objectWithPid_(self, 2); 
	IoState_setLobby_(IOSTATE, root);
	IoState_popCollectorPause(IOSTATE);
	return root;
}

/*
 void IoStore_setValueStreamByteData_length_(IoStore *self, unsigned char *data, size_t size)
 {
	 BStream *vs = DATA(self)->tmpValueStream;
	 ByteArray_setData_size_(BStream_byteArray(vs), data, size);
 }
 */

// db ops ---------------------------------- 

void IoStore_atPid_put_(IoStore *self, PID_TYPE pid, ByteArray *vb)
{
	UDB *udb = IoStore_udb(self);
	UDB_at_put_(udb, pid, Datum_FromByteArray_(vb));
}

void IoStore_atPid_intoStream_(IoStore *self, PID_TYPE pid, BStream *stream)
{
	UDB *udb = IoStore_udb(self);
	Datum v = UDB_at_(udb, pid);
	BStream_setData_length_(stream, v.data, v.size);
}

BStream *IoStore_atPid_(IoStore *self, PID_TYPE pid)
{
	UDB *udb = IoStore_udb(self);
	BStream *stream = DATA(self)->tmpValueStream;
	Datum v = UDB_at_(udb, pid);
	BStream_setData_length_(stream, v.data, v.size);
	return stream;
}

void IoStore_removeAtPid_(IoStore *self, PID_TYPE pid)
{
	UDB *udb = IoStore_udb(self);
	UDB_removeAt_(udb, pid);
}

// cleanup ---------------------------------------------- 

void IoStore_willFreePersistentObject_(IoStore *self, IoObject *obj)
{
	PID_TYPE pid = IoObject_persistentId(obj);
	Hash_removeKey_(DATA(self)->pidToObject, (void *)pid);
}

// save ---------------------------------------------- 

PID_TYPE IoStore_pidForObject_(IoStore *self, IoObject *obj)
{
	UDB *udb = IoStore_udb(self);
	PID_TYPE pid = IoObject_persistentId(obj);
	
	if (!pid)
	{
		pid = UDB_put_(udb, Datum_Empty());
		IoObject_setPersistentId_(obj, pid);
		obj->isDirty = 1;
		
		if (!obj->isDirty)
		{
			printf("error, no pid on %s and not dirty\n", IoObject_name(obj));
			exit(1);
		}
		
		Hash_at_put_(DATA(self)->pidToObject, (void *)pid, (void *)obj);
	}
	
	if (obj->isDirty)
	{
		List_append_(DATA(self)->objectsToSave, obj);
		obj->isDirty = 0;
	}
	
	return pid;
}

void IoStore_saveObject_(IoStore *self, IoObject *obj)
{
	BStream *vs = DATA(self)->tmpValueStream;
	PID_TYPE pid = IoStore_pidForObject_(self, obj);
	
	const char *name = IoObject_name(obj);
	char isProto = (obj == IoState_protoWithName_(IOSTATE, name)) ? 'p' : 'i';
	
	/*
	 if (pid == 2)
	 {
		 printf("saving pid 2\n");
	 }
	 */  
     
	if (!name || !strlen(name))
	{
		printf("no proto for object %p\n", (void *)obj);
		exit(1);
	}  
	
	BStream_empty(vs);  
	BStream_writeTaggedUint8_(vs, isProto);
	BStream_writeTaggedCString_(vs, name);
	IoObject_writeToStore_stream_(obj, self, vs);
	
	IoStore_atPid_put_(self, pid, BStream_byteArray(vs));
}

void IoStore_finishSave(IoStore *self)
{
	List *objectsToSave = DATA(self)->objectsToSave;
	IoObject *obj;
	
	while ((obj = List_pop(objectsToSave)))
	{ 
		if (DATA(self)->debug)
		{ 
			printf("writing %" PID_FORMAT "\n", IoObject_persistentId(obj)); 
		}
		
		IoStore_saveObject_(self, obj); 
	}
}

// load ---------------------------------------------- 
// [pid] : [isPrimitiveProto][primitiveTypeName][data] 

IoObject *IoStore_objectWithPid_(IoStore *self, PID_TYPE pid)
{
	IoObject *obj = Hash_at_(DATA(self)->pidToObject, (void *)pid);
	
	if (obj) 
	{
		return obj;
	}
	
	return IoStore_loadObjectWithPid_(self, pid);
}

IoObject *IoStore_loadObjectWithPid_(IoStore *self, PID_TYPE pid)
{  
	IoObject *instance = IONIL(self);
	BStream *stream = BStream_new();
	IoStore_atPid_intoStream_(self, pid, stream);
	
	if (!BStream_isEmpty(stream))
	{
		char isProto = BStream_readTaggedUint8(stream);
		char *protoName = (char *)ByteArray_asCString(BStream_readTaggedByteArray(stream));
		IoObject *proto = IoState_protoWithName_(IOSTATE, protoName);
		
		//printf("%i %c %s\n", pid, isProto, protoName);
		
		if (!proto)
		{
			printf("no proto named '%s'\n", protoName);
			IoState_protoWithName_(IOSTATE, protoName);
			exit(1);
		}
		
		if (isProto == 'p') 
		{
			instance = proto;
			/*printf("%c %s\n", isProto, protoName);*/
			
			IoObject_setPersistentId_(instance, pid);
			Hash_at_put_(DATA(self)->pidToObject, (void *)pid, instance);
		}
		else if (proto->tag->allocFromStoreOnStreamFunc) 
		{
			instance = IoObject_allocFromStore_stream_(proto, self, stream);
			
			IoObject_setPersistentId_(instance, pid);
			Hash_at_put_(DATA(self)->pidToObject, (void *)pid, instance);
		}
		else
		{
			TagCloneFunc *func = proto->tag->cloneFunc;
			
			/*
			 if (strcmp(protoName, "Socket") == 0)
			 {
				 printf("reading socket\n");
				 BStream_show(stream);
			 }
			 */
			
			instance = (func)(proto);
			IoObject_rawRemoveAllProtos(instance);
			
			IoObject_setPersistentId_(instance, pid);
			Hash_at_put_(DATA(self)->pidToObject, (void *)pid, instance);
			
			IoObject_readFromStore_stream_(instance, self, stream);	    
		}
		
		if (DATA(self)->debug)
		{ 
			printf("reading %s%" PID_FORMAT "\n", protoName, pid); 
		}		
	}
	
	BStream_free(stream);
	
	return instance;
}

// PObjects ------------------------------------------------ 

/*
 IoObject *IoStore_objectWithPid_(IoStore *self, long pid)
 {
	 IoObject *obj = Hash_at_(DATA(self)->pidToObject, (void *)pid);
	 
	 if (obj) 
	 {
		 return obj;
	 }
	 
	 return IoPObject_newWithStore_pid_(IOSTATE, self, pid);
 }
 
 IoObject *IoStore_loadPObject_(IoStore *self, IoObject *pObject)
 {  
	 PID_TYPE pid = IoObject_persistentId(pObject);
	 BStream *stream = IoStore_atPid_(self, pid);
	 char isProto = BStream_readByte(stream);
	 IoObject *proto = IoState_protoWithName_(IOSTATE, (char *)BStream_readCString(stream));
	 IoObject *instance = (isProto == 'p') ? proto : IOCLONE(proto);
	 
	 IoObject_setPersistentId_(instance, pid);
	 instance = IoObject_readFromStore_stream_(instance, self);
	 Hash_at_put_(DATA(self)->pidToObject, (void *)pid, instance);
	 
	 return instance;
 }
 */
