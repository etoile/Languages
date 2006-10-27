/*#io
docCopyright("Steve Dekorte", 2002)
docLicense("BSD revised")
*/

#ifndef IOLIST_DEFINED
#define IOLIST_DEFINED 1

#include "Common.h"
#include "IoState.h"
#include "IoObject.h"
#include "List.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ISLIST(self) \
  IoObject_hasCloneFunc_(self, (TagCloneFunc *)IoList_rawClone)

typedef IoObject IoList;

IoList *IoList_proto(void *state);
IoList *IoList_rawClone(IoObject *self);
IoList *IoList_new(void *state);
IoList *IoList_newWithList_(void *state, List *list);
void IoList_free(IoObject *self);
void IoList_mark(IoObject *self);
int IoList_compare(IoObject *self, IoList *otherList);

void IoList_writeToStore_stream_(IoObject *self, IoStore *store, BStream *stream);
void IoList_readFromStore_stream_(IoObject *self, IoStore *store, BStream *stream);

List *IoList_rawList(IoObject *self);
IoObject *IoList_rawAt_(IoObject *self, int i);
void IoList_rawAt_put_(IoObject *self, int i, IoObject *v);
void IoList_rawAppend_(IoObject *self, IoObject *v);
void IoList_rawRemove_(IoObject *self, IoObject *v);
void IoList_rawAddIoList_(IoObject *self, IoList *other);
void IoList_rawAddBaseList_(IoObject *self, List *other);
size_t IoList_rawSize(IoObject *self);

// immutable 

IoObject *IoList_with(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoList_indexOf(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoList_contains(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoList_containsIdenticalTo(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoList_capacity(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoList_size(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoList_at(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoList_first(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoList_last(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoList_slice(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoList_sliceInPlace(IoObject *self, IoObject *locals, IoMessage *m);

IoObject *IoList_mapInPlace(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoList_map(IoObject *self, IoObject *locals, IoMessage *m);

IoObject *IoList_select(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoList_detect(IoObject *self, IoObject *locals, IoMessage *m);

IoObject *IoList_foreach(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoList_reverseForeach(IoObject *self, IoObject *locals, IoMessage *m);

// mutable

IoObject *IoList_preallocateToSize(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoList_append(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoList_prepend(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoList_appendIfAbsent(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoList_appendSeq(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoList_remove(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoList_push(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoList_pop(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoList_removeAll(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoList_atInsert(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoList_removeAt(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoList_atPut(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoList_removeAll(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoList_swapIndices(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoList_reverse(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoList_sortInPlace(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoList_sortInPlaceBy(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoList_selectInPlace(IoObject *self, IoObject *locals, IoMessage *m);\

#ifdef __cplusplus
}
#endif
#endif
