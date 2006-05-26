
#include "IoSeq.h"

void IoSeq_addMutableMethods(IoSeq *self);
void IoSeq_rawPreallocateToSize_(IoSeq *self, size_t size);
int IoSeq_byteCompare(const void *a, const void *b);

IoObject *IoSeq_copy(IoSeq *self, IoObject *locals, IoMessage *m);
IoObject *IoSeq_appendSeq(IoSeq *self, IoObject *locals, IoMessage *m);
IoObject *IoSeq_append(IoSeq *self, IoObject *locals, IoMessage *m);
IoObject *IoSeq_atInsertSeq(IoSeq *self, IoObject *locals, IoMessage *m);

// removing 

IoObject *IoSeq_removeSlice(IoSeq *self, IoObject *locals, IoMessage *m);
IoObject *IoSeq_removeLastByte(IoSeq *self, IoObject *locals, IoMessage *m);
IoObject *IoSeq_setSize(IoSeq *self, IoObject *locals, IoMessage *m);
IoObject *IoSeq_preallocateToSize(IoSeq *self, IoObject *locals, IoMessage *m);
IoObject *IoSeq_replaceSeq(IoSeq *self, IoObject *locals, IoMessage *m);
IoObject *IoSeq_replaceFirstSeq(IoSeq *self, IoObject *locals, IoMessage *m);
IoObject *IoSeq_atPut(IoSeq *self, IoObject *locals, IoMessage *m);
IoObject *IoSeq_lowercase(IoSeq *self, IoObject *locals, IoMessage *m);
IoObject *IoSeq_uppercase(IoSeq *self, IoObject *locals, IoMessage *m);

// clip 

IoObject *IoSeq_clipBeforeSeq(IoSeq *self, IoObject *locals, IoMessage *m);
IoObject *IoSeq_clipAfterSeq(IoSeq *self, IoObject *locals, IoMessage *m);
IoObject *IoSeq_clipBeforeEndOfSeq(IoSeq *self, IoObject *locals, IoMessage *m);
IoObject *IoSeq_clipAfterStartOfSeq(IoSeq *self, IoObject *locals, IoMessage *m);
IoObject *IoSeq_empty(IoSeq *self, IoObject *locals, IoMessage *m);

// sort 

IoObject *IoSeq_sort(IoSeq *self, IoObject *locals, IoMessage *m);

// removing indexwise

IoObject *IoSeq_removeOddIndexes(IoSeq *self, IoObject *locals, IoMessage *m);
IoObject *IoSeq_removeEvenIndexes(IoSeq *self, IoObject *locals, IoMessage *m);
IoObject *IoSeq_duplicateIndexes(IoSeq *self, IoObject *locals, IoMessage *m);
IoObject *IoSeq_replaceMap(IoSeq *self, IoObject *locals, IoMessage *m);

// strip

IoObject *IoSeq_strip(IoSeq *self, IoObject *locals, IoMessage *m);
IoObject *IoSeq_lstrip(IoSeq *self, IoObject *locals, IoMessage *m);
IoObject *IoSeq_rstrip(IoSeq *self, IoObject *locals, IoMessage *m);
IoObject *IoSeq_setFloat32At(IoSeq *self, IoObject *locals, IoMessage *m);

// escape 

IoObject *IoSeq_escape(IoSeq *self, IoObject *locals, IoMessage *m);
IoObject *IoSeq_unescape(IoSeq *self, IoObject *locals, IoMessage *m);

IoObject *IoSeq_removePrefix(IoSeq *self, IoObject *locals, IoMessage *m);
IoObject *IoSeq_removeSuffix(IoSeq *self, IoObject *locals, IoMessage *m);
IoObject *IoSeq_capitalize(IoSeq *self, IoObject *locals, IoMessage *m);
IoObject *IoSeq_appendPathSeq(IoObject *self, IoObject *locals, IoMessage *m);

IoObject *IoSeq_asCapitalized(IoObject *self, IoObject *locals, IoMessage *m);


