/*#io
docCopyright("Steve Dekorte", 2002)
docLicense("BSD revised")
*/

#ifndef IOSTORE_DEFINED 
#define IOSTORE_DEFINED 1

#include "IoObject_struct.h"

typedef IoObject IoStore;

#include "IoMessage.h"
#include "IoSeq.h"
#include <stdarg.h>
#include "Hash.h"
#include "BStream.h"
#include "SkipDB/SkipDBM.h"
#include "IoContext.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ISSTORE(self) IoObject_hasCloneFunc_(self, (TagCloneFunc *)IoStore_rawClone)

typedef struct
{
    // hmm, should the State or the Store own the SDBM? 
    
    IoObject *path;
    BStream *tmpKeyStream;
    BStream *tmpValueStream;

    IoContext context;

    Hash *pidToObject;
    List *objectsToSave;
    int debug;
} IoStoreData;


IoStore *IoStore_rawClone(IoStore *self);
IoStore *IoStore_proto(void *state);
IoStore *IoStore_new(void *state);
IoStore *IoStore_newWithPath_(void *state, IoSymbol *path);

void IoStore_free(IoStore *self);
void IoStore_mark(IoStore *self);

BStream *IoStore_valueStream(IoStore *self); 
void IoStore_addMethods(IoStore *self);

// -------------------------------------------------------- 

IoObject *IoStore_clone(IoStore *self, IoObject *locals, IoMessage *m);

IoObject *IoStore_path(IoStore *self, IoObject *locals, IoMessage *m);
IoObject *IoStore_setPath(IoStore *self, IoObject *locals, IoMessage *m);

int IoStore_debugIsOn(IoStore *self);
/*
IoObject *IoStore_debugOn(IoStore *self, IoObject *locals, IoMessage *m);
IoObject *IoStore_debugOff(IoStore *self, IoObject *locals, IoMessage *m);
*/

IoObject *IoStore_store(IoStore *self, IoObject *locals, IoMessage *m);
IoObject *IoStore_load(IoStore *self, IoObject *locals, IoMessage *m);

// Store exec

IoObject *IoStore_store(IoStore *self, IoObject *locals, IoMessage *m);
void IoStore_finishSave(IoStore *self);

// at / put

void IoStore_atPid_put_(IoStore *self, PID_TYPE pid, ByteArray *vb);
BStream *IoStore_atPid_(IoStore *self, PID_TYPE pid);
void IoStore_removeAtPid_(IoStore *self, PID_TYPE pid);

// cleanup

void IoStore_willFreePersistentObject_(IoStore *self, IoObject *obj);

// save

PID_TYPE IoStore_pidForObject_(IoStore *self, IoObject *obj);
PID_TYPE IoStore_saveDataForObject_(IoStore *self, IoObject *obj);
void IoStore_saveObject_(IoStore *self, IoObject *obj);

// load

IoObject *IoStore_objectWithPid_(IoStore *self, PID_TYPE pid);
IoObject *IoStore_loadObjectWithPid_(IoStore *self, PID_TYPE pid);

#ifdef __cplusplus
}
#endif
#endif


