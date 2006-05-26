/*
docCopyright("Steve Dekorte", 2002)
*/

#ifndef IODIRECTORY_DEFINED
#define IODIRECTORY_DEFINED 1

#include "IoObject.h"
#include "IoSeq.h"

#define ISDIRECTORY(self) IoObject_hasCloneFunc_(self, (TagCloneFunc *)IoDirectory_rawClone)

typedef IoObject IoDirectory;

typedef struct
{
    IoSymbol *path;
} IoDirectoryData;

IoDirectory *IoDirectory_rawClone(IoObject *self);
IoDirectory *IoDirectory_proto(void *state);
IoDirectory *IoDirectory_new(void *state);
IoDirectory *IoDirectory_newWithPath_(void *state, IoSymbol *path);
IoDirectory *IoDirectory_cloneWithPath_(IoObject *self, IoSymbol *path);

void IoDirectory_free(IoObject *self);
void IoDirectory_mark(IoObject *self);

// ----------------------------------------------------------- 

IoObject *IoDirectory_path(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoDirectory_setPath(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoDirectory_name(IoObject *self, IoObject *locals, IoMessage *m);

IoObject *IoDirectory_at(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoDirectory_size(IoObject *self, IoObject *locals, IoMessage *m);

IoObject *IoDirectory_exists(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoDirectory_items(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoDirectory_create(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoDirectory_createSubdirectory(IoObject *self, IoObject *locals, IoMessage *m);

ByteArray *IoDirectory_CurrentWorkingDirectoryAsByteArray(void);
int IoDirectory_SetCurrentWorkingDirectory(const char *path);

IoObject *IoDirectory_currentWorkingDirectory(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoDirectory_setCurrentWorkingDirectory(IoObject *self, IoObject *locals, IoMessage *m);

#endif
