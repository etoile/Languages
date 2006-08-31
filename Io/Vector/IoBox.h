/*
docCopyright("Steve Dekorte", 2002)
docLicense("BSD revised")
*/

#ifndef IOBOX_DEFINED
#define IOBOX_DEFINED 1

#include "IoObject.h"
#include "IoVector.h"

#define ISBOX(self) IoObject_hasCloneFunc_(self, (TagCloneFunc *)IoBox_rawClone)
void *IoMessage_locals_boxArgAt_(IoMessage *self, void *locals, int n);

typedef IoObject IoBox;

typedef struct
{
    IoVector *origin;
    IoVector *size;
} IoBoxData;

IoBox *IoBox_rawClone(IoBox *self);
IoBox *IoBox_proto(void *state);
IoBox *IoBox_new(void *state);
void IoBox_rawCopy(IoBox *self, IoBox *other);
void IoBox_rawSet(IoBox *self, NUM_TYPE x, NUM_TYPE y, NUM_TYPE z, NUM_TYPE w, NUM_TYPE h, NUM_TYPE d);
IoBox *IoBox_newSet(void *state, NUM_TYPE x, NUM_TYPE y, NUM_TYPE z, NUM_TYPE w, NUM_TYPE h, NUM_TYPE d);

IoVector *IoBox_rawOrigin(IoBox *self);
IoVector *IoBox_rawSize(IoBox *self);

void IoBox_free(IoBox *self);
void IoBox_mark(IoBox *self);

/* ----------------------------------------------------------- */  
IoObject *IoBox_origin(IoBox *self, IoObject *locals, IoMessage *m);
IoObject *IoBox_size(IoBox *self, IoObject *locals, IoMessage *m);

IoObject *IoBox_width(IoBox *self, IoObject *locals, IoMessage *m);
IoObject *IoBox_height(IoBox *self, IoObject *locals, IoMessage *m);
IoObject *IoBox_depth(IoBox *self, IoObject *locals, IoMessage *m);

IoObject *IoBox_set(IoBox *self, IoObject *locals, IoMessage *m);
IoObject *IoBox_setOrigin(IoBox *self, IoObject *locals, IoMessage *m);
IoObject *IoBox_setSize(IoBox *self, IoObject *locals, IoMessage *m);

void IoBox_rawUnion(IoBox *self, IoBox *other);
IoObject *IoBox_Union(IoBox *self, IoObject *locals, IoMessage *m);
IoObject *IoBox_containsPoint(IoBox *self, IoObject *locals, IoMessage *m);
IoObject *IoBox_intersectsBox(IoBox *self, IoObject *locals, IoMessage *m);

IoObject *IoBox_print(IoBox *self, IoObject *locals, IoMessage *m);
/*
IoObject *IoBox_asString(IoBox *self, IoObject *locals, IoMessage *m);

IoObject *IoBox_Min(IoBox *self, IoObject *locals, IoMessage *m);
IoObject *IoBox_Max(IoBox *self, IoObject *locals, IoMessage *m);
*/

#endif
