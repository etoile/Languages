/*#io
Object ioDoc(
		   
*/

#include "IoObject_persistence.h"
#include "IoState.h"
#include "IoCFunction.h"
#include "Datum.h"

void IoObject_writeToStore_stream_(IoObject *self, IoStore *store, BStream *stream)
{
    if (self->tag->writeToStoreOnStreamFunc) 
    { 
        (*(self->tag->writeToStoreOnStreamFunc))(self, store, stream); 
    }
    
    IoObject_writeSlotsToStore_stream_(self, store, stream);
}

IoObject *IoObject_allocFromStore_stream_(IoObject *self, IoStore *store, BStream *stream)
{
    if (self->tag->allocFromStoreOnStreamFunc) 
    { 
        return (*(self->tag->allocFromStoreOnStreamFunc))(self, store, stream); 
    }
    
    return self;
}

void IoObject_readFromStore_stream_(IoObject *self, IoStore *store, BStream *stream)
{
    if (self->tag->readFromStoreOnStreamFunc) 
    { 
        (*(self->tag->readFromStoreOnStreamFunc))(self, store, stream); 
    }
    
    IoObject_readSlotsFromStore_stream_(self, store, stream);
}

int IoObject_nonCFunctionSlotCount(IoObject *self)
{
    int count = 0;
    IoSymbol *key = PHash_firstKey(self->slots);
    
    while (key)
    {
        IoObject *value = PHash_at_(self->slots, key);
        
        if (!ISCFUNCTION(value)) 
        { 
            count ++; 
        }
	   
        key = PHash_nextKey(self->slots);
    }
    
    return count;
}

void IoObject_writeProtosToStore_stream_(IoObject *self, IoStore *store, BStream *stream)
{
    int i, protosCount = IoObject_rawProtosCount(self);
    
    BStream_writeTaggedInt32_(stream, protosCount);
    
    for (i = 0; i < protosCount; i ++)
    {
        IoObject *proto = IoObject_rawProtoAt_(self, i);
        int protoId = IoStore_pidForObject_(store, proto);
        BStream_writeTaggedInt32_(stream, protoId); 
    }
}

void IoObject_writeSlotsToStore_stream_(IoObject *self, IoStore *store, BStream *stream)
{
    IoObject_writeProtosToStore_stream_(self, store, stream);
    
    if (self->ownsSlots)
    {
        BStream_writeTaggedInt32_(stream, IoObject_nonCFunctionSlotCount(self));
        
        if (self->slots)
        {
            IoSymbol *key = PHash_firstKey(self->slots);
            
            while (key)
            {
                IoObject *value = PHash_at_(self->slots, key);
                
                if (!ISCFUNCTION(value))
                {
                    BStream_writeTaggedInt32_(stream, IoStore_pidForObject_(store, key));
                    BStream_writeTaggedInt32_(stream, IoStore_pidForObject_(store, value));
                    if (IoStore_debugIsOn(store))
                    {
                        printf("%s%" PID_FORMAT " %s %s%" PID_FORMAT "\n", 
                               IoObject_name(self), IoObject_persistentId(self), 
                               CSTRING(key), IoObject_name(value), IoStore_pidForObject_(store, value));
                    }
                }
                key = PHash_nextKey(self->slots);
            }
        }
    }
}

void IoObject_readProtosFromStore_stream_(IoObject *self, IoStore *store, BStream *stream)
{
    int i, max = BStream_readTaggedInt32(stream);
    
    IoObject_rawRemoveAllProtos(self);
    
    for (i = 0; i < max; i ++)
    {
        int protoId = BStream_readTaggedInt32(stream);  
        IoObject *proto = IoStore_objectWithPid_(store, protoId);
        IoObject_rawAppendProto_(self, proto);
    }
}

void IoObject_readSlotsFromStore_stream_(IoObject *self, IoStore *store, BStream *stream)
{
    int k, v;
    IoSymbol *key;
    IoObject *value;
    int i, max;
    
    IoObject_readProtosFromStore_stream_(self, store, stream);
    
    max = BStream_readTaggedInt32(stream);
    
    for (i = 0; i < max; i ++)
    {
        k = BStream_readTaggedInt32(stream);  
        v = BStream_readTaggedInt32(stream); 
        
        if (IoStore_debugIsOn(store))
        {
            printf("%s%" PID_FORMAT " load slot %i, %i\n", 
                   IoObject_name(self), IoObject_persistentId(self), k, v);
        }
        
        key   = IoStore_objectWithPid_(store, k);
        value = IoStore_objectWithPid_(store, v);
        IoObject_setSlot_to_(self, key, value);
    }
}

PID_TYPE IoObject_persistentId(IoObject *self) 
{
    return self->persistentId; 
}

void IoObject_setPersistentId_(IoObject *self, PID_TYPE pid) 
{ 
    self->persistentId = pid; 
}

