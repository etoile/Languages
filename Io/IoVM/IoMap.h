/*#io
docCopyright("Steve Dekorte", 2002)
docLicense("BSD revised")
*/

#ifndef IoMap_DEFINED
#define IoMap_DEFINED 1

#include "Common.h"
#include "IoObject.h"
#include "IoList.h"
#include "Hash.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ISMAP(self) IoObject_hasCloneFunc_(self, (TagCloneFunc *)IoMap_rawClone)

typedef IoObject IoMap;

IoMap *IoMap_proto(void *state);
IoMap *IoMap_rawClone(IoMap *self);
IoMap *IoMap_new(void *state);
void IoMap_free(IoMap *self);
void IoMap_mark(IoMap *self);
Hash *IoMap_rawHash(IoMap *self);

void IoMap_writeToStore_stream_(IoMap *self, IoStore *store, BStream *stream);
void IoMap_readFromStore_stream_(IoMap *self, IoStore *store, BStream *stream);

// ----------------------------------------------------------- 

IoObject *IoMap_empty(IoMap *self, IoObject *locals, IoMessage *m);

IoObject *IoMap_rawAt(IoMap *self, IoSymbol *k);
IoObject *IoMap_at(IoMap *self, IoObject *locals, IoMessage *m);

void IoMap_rawAtPut(IoMap *self, IoSymbol *k, IoObject *v);
IoObject *IoMap_atPut(IoMap *self, IoObject *locals, IoMessage *m);
IoObject *IoMap_atIfAbsentPut(IoMap *self, IoObject *locals, IoMessage *m);
IoObject *IoMap_removeAt(IoMap *self, IoObject *locals, IoMessage *m);
IoObject *IoMap_size(IoMap *self, IoObject *locals, IoMessage *m);

IoObject *IoMap_hasKey(IoMap *self, IoObject *locals, IoMessage *m);
IoObject *IoMap_hasValue(IoMap *self, IoObject *locals, IoMessage *m);

IoList *IoMap_rawKeys(IoMap *self);
IoObject *IoMap_keys(IoMap *self, IoObject *locals, IoMessage *m);

IoObject *IoMap_values(IoMap *self, IoObject *locals, IoMessage *m);
IoObject *IoMap_foreach(IoMap *self, IoObject *locals, IoMessage *m);

#ifdef __cplusplus
}
#endif
#endif
