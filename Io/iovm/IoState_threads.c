/*
 docCopyright("Steve Dekorte", 2002)
 docLicense("BSD revised")
 */

#include "IoState.h"
#include "IoObject.h"
#include "Thread.h"
#include "List.h"

void IoState_initThreads(IoState *self)
{
	Thread_Init();
}

void IoState_shutdownThreads(IoState *self)
{
	Thread_Shutdown();
}

List *IoState_threadIds(IoState *self)
{
	return Thread_Threads();
}

// --------------------------------------------

typedef struct
{
	IoState *state;
	Thread *thread;
	char *evalString;
} IoThreadInfo;

IoThreadInfo *IoThreadInfo_new(void) 
{
	IoThreadInfo *self = (IoThreadInfo *)calloc(1, sizeof(IoThreadInfo));
	return self;
}

void IoThreadInfo_free(IoThreadInfo *self) 
{
	if(self->evalString) free(self->evalString);
	free(self);
}

void IoThreadInfo_setState_(IoThreadInfo *self, IoState *state)
{
	self->state = state;
}

IoState *IoThreadInfo_state(IoThreadInfo *self)
{
	return self->state; 
}

void IoThreadInfo_setThread_(IoThreadInfo *self, Thread *thread)
{
	self->thread = thread;
}

Thread *IoThreadInfo_thread(IoThreadInfo *self)
{
	return self->thread; 
}

void IoThreadInfo_setEvalString_(IoThreadInfo *self, char *s)
{
	self->evalString = strcpy(malloc(strlen(s) + 1), s);
}

char *IoThreadInfo_evalString(IoThreadInfo *self)
{
	return self->evalString; 
}

// ----------------------------------------------------

void *IoState_beginThread(IoThreadInfo *ti)
{
	Thread *t = IoThreadInfo_thread(ti);
	IoState *state = IoThreadInfo_state(ti);
	
	Thread_setUserData_(t, state);
	IoState_doCString_(state, IoThreadInfo_evalString(ti));
	IoThreadInfo_free(ti);
	IoState_free(state);
	Thread_destroy(t);
	return 0x0;
}

void *IoState_createThreadAndEval(IoState *self, char *s)
{
	IoState *newState = IoState_new();
	Thread *t;
	
	Thread_Init();
	
	t = Thread_new();
	
	IoThreadInfo *ti = IoThreadInfo_new();
	IoThreadInfo_setState_(ti, newState);
	IoThreadInfo_setThread_(ti, t);
	IoThreadInfo_setEvalString_(ti, s);
	
	Thread_setFunc_(t, IoState_beginThread);
	Thread_setFuncArg_(t, (void *)ti);
	Thread_start(t);
	
	return (void *)t;
}

void IoState_endThisThread(IoState *self)
{
	Thread_destroy(Thread_CurrentThread());
}

int IoState_threadCount(IoState *self)
{
	List *threads;
	int count;
	
	threads = IoState_threadIds(self);
	count = List_size(threads);
	List_free(threads);
	
	return count;	
}
