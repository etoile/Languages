/*#io
docCopyright("Steve Dekorte", 2002)
docLicense("BSD revised")
*/

#ifndef IOWEAKLINK_DEFINED 
#define IOWEAKLINK_DEFINED 1

#include "Common.h"
#include "IoObject_struct.h"
#include "IoStore.h"
#include "BStream.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ISWEAKLINK(self) IoObject_hasCloneFunc_(self, (TagCloneFunc *)IoWeakLink_rawClone)

typedef IoObject IoWeakLink;

typedef struct 
{
    IoObject *link;
} IoWeakLinkData;

IoObject *IoWeakLink_proto(void *state);
IoObject *IoWeakLink_new(void *state);

void IoWeakLink_writeToStore_stream_(IoWeakLink *self, IoStore *store, BStream *stream);
void IoWeakLink_readFromStore_stream_(IoWeakLink *self, IoStore *store, BStream *stream);

IoObject *IoWeakLink_rawClone(IoWeakLink *self);
void IoWeakLink_free(IoWeakLink *self);
void IoWeakLink_mark(IoWeakLink *self);

IoObject *IoWeakLink_rawLink(IoWeakLink *self);

void IoObject_collectorNotification(IoWeakLink *self);

IoObject *IoWeakLink_setLink(IoWeakLink *self, IoObject *locals, IoMessage *m);
void IoWeakLink_rawSetLink(IoObject *self, IoObject *v);
IoObject *IoWeakLink_link(IoWeakLink *self, IoObject *locals, IoMessage *m);

void IoWeakLink_notification(IoObject *self, void *notification);

#ifdef __cplusplus
}
#endif
#endif
