/*#io
docCopyright("Steve Dekorte", 2004)
docLicense("BSD revised")
docObject("SkipDBCursor")    
docDescription("A cursor for a skipdb.")
*/


#ifndef SkipDBCursor_DEFINED
#define SkipDBCursor_DEFINED 1

typedef struct SkipDBCursor SkipDBCursor;

#include "SkipDB.h"
#include <stdio.h>
#include <sys/types.h> 

#ifdef __cplusplus
extern "C" {
#endif

struct SkipDBCursor
{
    SkipDB *sdb;
    SkipDBRecord *record;
};

SkipDBCursor *SkipDBCursor_new(void);
SkipDBCursor *SkipDBCursor_newWithDB_(SkipDB *sdb);
void SkipDBCursor_free(SkipDBCursor *self);
void SkipDBCursor_mark(SkipDBCursor *self);

SkipDBRecord *SkipDBCursor_goto_(SkipDBCursor *self, Datum key);

SkipDBRecord *SkipDBCursor_first(SkipDBCursor *self);
SkipDBRecord *SkipDBCursor_last(SkipDBCursor *self);

SkipDBRecord *SkipDBCursor_previous(SkipDBCursor *self);
SkipDBRecord *SkipDBCursor_current(SkipDBCursor *self);
SkipDBRecord *SkipDBCursor_next(SkipDBCursor *self);

#ifdef __cplusplus
}
#endif
#endif
