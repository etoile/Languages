/*#io
docCopyright("Steve Dekorte", 2004)
docLicense("BSD revised")
docObject("SkipDB")    
docDescription("A sorted key/value pair database implemented with skip lists on top of UDB.")
*/

#ifndef SkipDB_DEFINED
#define SkipDB_DEFINED 1

#include "SkipDBRecord.h"
#include "Hash.h"
#include "RandomGen.h"

#ifdef __cplusplus
extern "C" {
#endif

// if prob. dist = 0.5, then max level 32 is enough for 2^32 records 

#define SKIPDB_MAX_LEVEL 32
#define SKIPDB_PROBABILITY_DISTRIBUTION 0.5 

typedef void (SkipDBObjectMarkFunc)(void *);
typedef void (SkipDBFreeObjectFunc)(void *);
    
typedef struct
{
    int refCount;
    void *dbm;
    PID_TYPE headerPid;
    SkipDBRecord *header; 
    SkipDBRecord *youngestRecord; // most recently accessed 
    
    SkipDBRecord *update[SKIPDB_MAX_LEVEL];
    float p;

    BStream *stream;
    SkipDBObjectMarkFunc *objectMarkFunc;
    SkipDBFreeObjectFunc *objectFreeFunc;
    List *cursors;
    
    List *dirtyRecords;
    List *pidsToRemove;
    
    size_t cachedRecordCount; 
    size_t cacheHighWaterMark;
    size_t cacheLowWaterMark;
    unsigned char mark; // current record mark identifier 
    Hash *pidToRecord;
    RandomGen *randomGen;
} SkipDB;

#include "SkipDBCursor.h"

SkipDB *SkipDB_newWithDBM_(void *dbm);
SkipDB *SkipDB_newWithDBM_atPid_(void *dbm, PID_TYPE pid);

void SkipDB_retain(SkipDB *self);
void SkipDB_release(SkipDB *self);
void SkipDB_dealloc(SkipDB *self); // private 

BStream *SkipDB_tmpStream(SkipDB *self);

void SkipDB_headerPid_(SkipDB *self, PID_TYPE pid);
PID_TYPE SkipDB_headerPid(SkipDB *self);
SkipDBRecord *SkipDB_headerRecord(SkipDB *self);

UDB *SkipDB_udb(SkipDB *self);
int SkipDB_isOpen(SkipDB *self);
int SkipDB_delete(SkipDB *self);

// notifications 

void SkipDB_noteNewRecord_(SkipDB *self, SkipDBRecord *r);
void SkipDB_noteAccessedRecord_(SkipDB *self, SkipDBRecord *r);
void SkipDB_noteDirtyRecord_(SkipDB *self, SkipDBRecord *r);
void SkipDB_noteAssignedPidToRecord_(SkipDB *self, SkipDBRecord *r);
void SkipDB_noteWillFreeRecord_(SkipDB *self, SkipDBRecord *r);

// cache 

void SkipDB_setCacheHighWaterMark_(SkipDB *self, size_t recordCount);
size_t SkipDB_cacheHighWaterMark(SkipDB *self);

void SkipDB_setCacheLowWaterMark_(SkipDB *self, size_t recordCount);
size_t SkipDB_cacheLowWaterMark(SkipDB *self);

void SkipDB_clearCache(SkipDB *self);
void SkipDB_freeAllCachedRecords(SkipDB *self);
int SkipDB_headerIsEmpty(SkipDB *self);

// transactions

void SkipDB_sync(SkipDB *self);
void SkipDB_saveDirtyRecords(SkipDB *self);
void SkipDB_deleteRecordsToRemove(SkipDB *self);

// record api 

SkipDBRecord *SkipDB_recordAt_(SkipDB *self, Datum k);
SkipDBRecord *SkipDB_recordAt_put_(SkipDB *self, Datum k, Datum v);

// bdb style api

void SkipDB_at_put_(SkipDB *self, Datum k, Datum v);
Datum SkipDB_at_(SkipDB *self, Datum k);
void SkipDB_removeAt_(SkipDB *self, Datum k);

// compact

int SkipDB_compact(SkipDB *self);

// debugging 

void SkipDB_showUpdate(SkipDB *self);
void SkipDB_show(SkipDB *self);

// private

void SkipDB_updateAt_put_(SkipDB *self, int level, SkipDBRecord *r);
SkipDBRecord *SkipDB_recordAtPid_(SkipDB *self, PID_TYPE pid);

// objects

void SkipDB_objectMarkFunc_(SkipDB *self, SkipDBObjectMarkFunc *func);
void SkipDB_freeObjectCallback_(SkipDB *, SkipDBFreeObjectFunc *func);

// cursor

int SkipDB_count(SkipDB *self);

SkipDBRecord *SkipDB_firstRecord(SkipDB *self);
SkipDBRecord *SkipDB_lastRecord(SkipDB *self);
SkipDBRecord *SkipDB_goto_(SkipDB *self, Datum key);

SkipDBCursor *SkipDB_createCursor(SkipDB *self);
void SkipDB_freeCursor_(SkipDB *self, SkipDBCursor *cursor);

// moving from in-memory to on-disk 

void SkipDB_mergeInto_(SkipDB *self, SkipDB *other);

#ifdef __cplusplus
}
#endif
#endif
