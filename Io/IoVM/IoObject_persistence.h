/*#io
docCopyright("Steve Dekorte", 2002)
docLicense("BSD revised")
*/

#ifndef IOOBJECT_PERSISTENCE_DEFINED 
#define IOOBJECT_PERSISTENCE_DEFINED 1

#include "IoStore.h"
#include "IoObject.h"

#ifdef __cplusplus
extern "C" {
#endif

void IoObject_writeToStore_stream_(IoObject *self, IoStore *store, BStream *stream);
void IoObject_writeProtosToStore_stream_(IoObject *self, IoStore *store, BStream *stream);
void IoObject_writeSlotsToStore_stream_(IoObject *self, IoStore *store, BStream *stream);

IoObject *IoObject_allocFromStore_stream_(IoObject *self, IoStore *store, BStream *stream);
void IoObject_readFromStore_stream_(IoObject *self, IoStore *store, BStream *stream);
void IoObject_readProtosFromStore_stream_(IoObject *self, IoStore *store, BStream *stream);
void IoObject_readSlotsFromStore_stream_(IoObject *self, IoStore *store, BStream *stream);

PID_TYPE IoObject_persistentId(IoObject *self);
void IoObject_setPersistentId_(IoObject *self, PID_TYPE pid);

#ifdef __cplusplus
}
#endif
#endif
