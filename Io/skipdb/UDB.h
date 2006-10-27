/*#io
docCopyright("Steve Dekorte", 2004)
docLicense("BSD revised")
docObject("UDB")    
docDescription("An unordered value database. (sort of like malloc for disk space)
    It compacts the data like a single space copying garbage collector.")
*/

#ifndef UDB_DEFINED
#define UDB_DEFINED 1

#include "List.h"
#include "ByteArray.h"
#include "UDBRecord.h"
#include "UDBIndex.h"
#include "UDBRecords.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    char *path;
    UDBIndex *index;
    UDBRecords *records;
    int withinTransaction;
    int isOpen;
} UDB;

UDB *UDB_new(void);
void UDB_free(UDB *self);

void UDB_setPath_(UDB *self, const char *s);
void UDB_setLogPath_(UDB *self, const char *s);
char *UDB_path(UDB *self);

void UDB_delete(UDB *self);
void UDB_open(UDB *self);
int UDB_isOpen(UDB *self);
void UDB_close(UDB *self);

// transactions --------------------------------------------------- 

void UDB_beginTransaction(UDB *self);
void UDB_commitTransaction(UDB *self);

// ops -------------------------------------------------- 

PID_TYPE UDB_nextPid(UDB *self);
PID_TYPE UDB_allocPid(UDB *self);

PID_TYPE UDB_put_(UDB *self, Datum d);
void UDB_at_put_(UDB *self, PID_TYPE pid, Datum d);
Datum UDB_at_(UDB *self, PID_TYPE pid);
void UDB_removeAt_(UDB *self, PID_TYPE id);

int UDB_compact(UDB *self);
int UDB_compactStep(UDB *self);
int UDB_compactStepFor_(UDB *self, double maxSeconds);

void UDB_show(UDB *self);
void UDB_showIndex(UDB *self);

#ifdef __cplusplus
}
#endif
#endif
