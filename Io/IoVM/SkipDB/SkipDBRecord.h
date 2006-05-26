/*#io
docCopyright("Steve Dekorte", 2004)
docLicense("BSD revised")
docObject("SkipDBRecord")    
docDescription("A skip list record.")
*/

#ifndef SkipDBRecord_DEFINED
#define SkipDBRecord_DEFINED 1

#include "List.h"
#include "BStream.h"
#include "UDB.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SkipDBRecord  SkipDBRecord;
typedef struct SkipDBPointer SkipDBPointer;

#define SKIPDBRECORD_UNMARKED  0
#define SKIPDBRECORD_MARKED    1

struct SkipDBPointer
{
    PID_TYPE pid;
    unsigned char matchingPrefixSize;
    SkipDBRecord *record;
};

struct SkipDBRecord
{
    int level;
    SkipDBPointer *pointers;
    
    PID_TYPE previousPid;
    SkipDBRecord *previousRecord;

    ByteArray *key;
    ByteArray *value;

    void *sdb;
    PID_TYPE pid;
    void *object; // extra pointer for user to make use of - not stored 
    int ownsKey;
    uint8_t isDirty;

    // cache related 

    SkipDBRecord *youngerRecord; // accessed before self 
    SkipDBRecord *olderRecord;   // accessed after self 
    unsigned char mark;
};

int SkipDBRecord_pointersAreEmpty(SkipDBRecord *self);

SkipDBRecord *SkipDBRecord_new(void);
SkipDBRecord *SkipDBRecord_newWithDB_(void *db);

void SkipDBRecord_setOlderRecord_(SkipDBRecord *self, SkipDBRecord *r);
SkipDBRecord *SkipDBRecord_olderRecord(SkipDBRecord *self);

void SkipDBRecord_setYoungerRecord_(SkipDBRecord *self, SkipDBRecord *r);
SkipDBRecord *SkipDBRecord_youngerRecord(SkipDBRecord *self);

void SkipDBRecord_removeFromAgeList(SkipDBRecord *self);

void SkipDBRecord_removeReferencesToUnmarked(SkipDBRecord *self);

void SkipDBRecord_dealloc(SkipDBRecord *self);

void SkipDBRecord_db_(SkipDBRecord *self, void *db);

void SkipDBRecord_pid_(SkipDBRecord *self, PID_TYPE pid);
PID_TYPE SkipDBRecord_pid(SkipDBRecord *self);
PID_TYPE SkipDBRecord_pidAllocIfNeeded(SkipDBRecord *self);

int SkipDBRecord_level(SkipDBRecord *self);
void SkipDBRecord_level_(SkipDBRecord *self, int level);

void SkipDBRecord_mark(SkipDBRecord *self);

uint8_t SkipDBRecord_isDirty(SkipDBRecord *self);
void SkipDBRecord_markAsDirty(SkipDBRecord *self);
void SkipDBRecord_markAsClean(SkipDBRecord *self);

void SkipDBRecord_atLevel_setPid_(SkipDBRecord *self, int level, PID_TYPE pid);
PID_TYPE SkipDBRecord_pidAtLevel_(SkipDBRecord *self, int level);

void SkipDBRecord_atLevel_setRecord_(SkipDBRecord *self, int level, SkipDBRecord *r);
SkipDBRecord *SkipDBRecord_recordAtLevel_(SkipDBRecord *self, int level);

void SkipDBRecord_copyLevel_from_(SkipDBRecord *self, int level, SkipDBRecord *other);

//-------------------------------------------- 

void SkipDBRecord_keyDatum_(SkipDBRecord *self, Datum k);
Datum SkipDBRecord_keyDatum(SkipDBRecord *self);
ByteArray *SkipDBRecord_key(SkipDBRecord *self);

void SkipDBRecord_valueDatum_(SkipDBRecord *self, Datum v);
Datum SkipDBRecord_valueDatum(SkipDBRecord *self);
ByteArray *SkipDBRecord_value(SkipDBRecord *self);

// serialization ----------------------------- 

void SkipDBRecord_toStream_(SkipDBRecord *self, BStream *s);
void SkipDBRecord_fromStream_(SkipDBRecord *self, BStream *s);
void SkipDBRecord_save(SkipDBRecord *self);

// search ------------------------------------ 

SkipDBRecord *SkipDBRecord_find_quick_(SkipDBRecord *self, Datum key, int quick);
SkipDBRecord *SkipDBRecord_findLastRecord(SkipDBRecord *self);

void SkipDBRecord_willRemove_(SkipDBRecord *self, SkipDBRecord *other);

void SkipDBRecord_show(SkipDBRecord *self);

// object ------------------------------------ 

void SkipDBRecord_object_(SkipDBRecord *self, void *object);
void *SkipDBRecord_object(SkipDBRecord *self);

// next ------------------------------------ 

SkipDBRecord *SkipDBRecord_nextRecord(SkipDBRecord *self);

// previous ------------------------------------ 

void SkipDBRecord_previousPid_(SkipDBRecord *self, PID_TYPE pid);
PID_TYPE SkipDBRecord_previousPid(SkipDBRecord *self);

void SkipDBRecord_previousRecord_(SkipDBRecord *self, SkipDBRecord *r);
SkipDBRecord *SkipDBRecord_previousRecord(SkipDBRecord *self);

#ifdef __cplusplus
}
#endif
#endif
