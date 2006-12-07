/*#io
docCopyright("Steve Dekorte", 2002)
docLicense("BSD revised") 
*/

#ifndef IOTAG_DEFINED 
#define IOTAG_DEFINED 1

#include "Common.h"
#include "Stack.h"

#include "IoVMApi.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void * (TagCloneFunc)(void *); /* self */
typedef void   (TagFreeFunc)(void *); /* self */
typedef void   (TagCleanupFunc)(void *); /* self */
typedef void   (TagMarkFunc)(void *); /* self */
typedef void * (TagPerformFunc)(void *, void *, void *); /* self, locals, message */
typedef void * (TagActivateFunc)(void *, void *, void *, void *, void *); /* self, target, locals, message, slotContext */
typedef int    (TagCompareFunc)(void *, void *); /* self and another IoObject */
typedef void   (TagPrintFunc)(void *); /* self */
typedef size_t (TagMemorySizeFunc)(void *); /* self */
typedef void   (TagCompactFunc)(void *); /* self */
typedef void   (TagNotificationFunc)(void *, void *); /* self, notification */

typedef void   (TagWriteToStoreOnStreamFunc)(void *, void *, void *);   /* self, store, stream */
typedef void * (TagAllocFromStoreOnStreamFunc)(void *, void *, void *); /* self, store, stream */
typedef void   (TagReadFromStoreOnStreamFunc)(void *, void *, void *);  /* self, store, stream */

typedef struct
{
    void *state;
    char *name;
    TagCloneFunc *cloneFunc;
    TagCleanupFunc *tagCleanupFunc;
    /* 
    perform means: look up the slot specificed by 
    the message and activate it with the message, return result 
    */    
    TagPerformFunc *performFunc; 

    
    TagActivateFunc *activateFunc; /* return the receiver or compute and return a value */
    TagFreeFunc *freeFunc;
    TagMarkFunc *markFunc;
    TagCompareFunc *compareFunc;
    TagNotificationFunc *notificationFunc;
    
    TagWriteToStoreOnStreamFunc   *writeToStoreOnStreamFunc;
    TagAllocFromStoreOnStreamFunc *allocFromStoreOnStreamFunc;
    TagReadFromStoreOnStreamFunc  *readFromStoreOnStreamFunc;

    TagMemorySizeFunc *memorySizeFunc;
    TagCompactFunc *compactFunc;
    /*
    Stack *recyclableInstances;
    int maxRecyclableInstances;
    */
} IoTag;

IOVM_API IoTag *IoTag_new(void);
IOVM_API IoTag *IoTag_newWithName_(char *name);
IOVM_API void IoTag_free(IoTag *self);

IOVM_API void IoTag_name_(IoTag *self, const char *name);
IOVM_API const char *IoTag_name(IoTag *self);

IOVM_API void IoTag_mark(IoTag *self);

#include "IoTag_inline.h"

#ifdef __cplusplus
}
#endif
#endif
