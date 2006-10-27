/*#io
docCopyright("Steve Dekorte", 2004)
docLicense("BSD revised")
docObject("SkipDBM")    
docDescription("A SkipDB Manager.")
*/

#ifndef SkipDBM_DEFINED
#define SkipDBM_DEFINED 1

#include "SkipDB.h"
#include "Hash.h"
#include <stdio.h>
#include <sys/types.h> 

#ifdef __cplusplus
extern "C" {
#endif

typedef void (SkipDBMThreadLockFunc)(void *);
typedef void (SkipDBMThreadUnlockFunc)(void *);

typedef struct
{
	UDB *udb;
	Hash *pidToDB;
	List *dbs;
	SkipDB *rootDB;
	void *callbackContext;
	SkipDBMThreadLockFunc *threadLockCallback;
	SkipDBMThreadUnlockFunc *threadUnlockCallback;
} SkipDBM;

SkipDBM *SkipDBM_new(void);
void SkipDBM_free(SkipDBM *self);

UDB *SkipDBM_udb(SkipDBM *self);

void SkipDBM_setPath_(SkipDBM *self, const char *path);
char *SkipDBM_path(SkipDBM *self);

// open/close

void SkipDBM_open(SkipDBM *self);
int SkipDBM_isOpen(SkipDBM *self);
void SkipDBM_close(SkipDBM *self);
void SkipDBM_delete(SkipDBM *self);

// databases 

SkipDB *SkipDBM_newSkipDB(SkipDBM *self);
SkipDB *SkipDBM_rootSkipDB(SkipDBM *self);
SkipDB *SkipDBM_skipDBAtPid_(SkipDBM *self, PID_TYPE pid);
void SkipDBM_willFreeDB_(SkipDBM *self, SkipDB *sdb); // private 

// transactions

void SkipDBM_beginTransaction(SkipDBM *self);
void SkipDBM_commitTransaction(SkipDBM *self);

// cache 

void SkipDBM_clearCaches(SkipDBM *self);

// compact 

int SkipDBM_compact(SkipDBM *self);

// thread locking

void SkipDBM_setCallbackContext_(SkipDBM *self, void *calbackContext);

void SkipDBM_setThreadLockCallback_(SkipDBM *self, SkipDBMThreadLockFunc *calback);
void SkipDBM_setThreadUnlockCallback_(SkipDBM *self, SkipDBMThreadUnlockFunc *calback);

void SkipDBM_lockThread(SkipDBM *self);
void SkipDBM_unlockThread(SkipDBM *self);

#ifdef __cplusplus
}
#endif
#endif
