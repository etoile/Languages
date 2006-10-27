/*#io
docCopyright("Steve Dekorte", 2002)
docLicense("BSD revised")
*/

#ifndef IoCoroutine_DEFINED
#define IoCoroutine_DEFINED 1
#include "IoState.h"

#include "Common.h"
#include "Coro.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ISCOROUTINE(self) IoObject_hasCloneFunc_(self, (TagCloneFunc *)IoCoroutine_rawClone)

typedef IoObject IoCoroutine;

typedef struct
{
	Coro *cid;
	Stack *ioStack;
	int debuggingOn;
} IoCoroutineData;

IoCoroutine *IoCoroutine_proto(void *state);
void IoCoroutine_protoFinish(IoCoroutine *self);
IoCoroutine *IoCoroutine_rawClone(IoCoroutine *self);
IoCoroutine *IoCoroutine_new(void *state);

void IoCoroutine_free(IoCoroutine *self);
void IoCoroutine_mark(IoCoroutine *self);
Stack *IoCoroutine_rawIoStack(IoCoroutine *self);
void IoCoroutine_rawShow(IoCoroutine *self);

IoObject *IoCoroutine_main(IoObject *self, IoObject *locals, IoMessage *m);

void *IoCoroutine_cid(IoObject *self);

// label

//void IoCoroutine_rawSetLabel_(IoCoroutine *self, IoSymbol *s);
//IoObject *IoCoroutine_rawLabel(IoCoroutine *self);

IoObject *IoCoroutine_setLabel(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoCoroutine_label(IoObject *self, IoObject *locals, IoMessage *m);

// runTarget

void IoCoroutine_rawSetRunTarget_(IoObject *self, IoObject *v);
IoObject *IoCoroutine_rawRunTarget(IoObject *self);

// runMessage

void IoCoroutine_rawSetRunMessage_(IoObject *self, IoObject *v);
IoObject *IoCoroutine_rawRunMessage(IoObject *self);

// runLocals

void IoCoroutine_rawSetRunLocals_(IoObject *self, IoObject *v);
IoObject *IoCoroutine_rawRunLocals(IoObject *self);

// parent

void IoCoroutine_rawSetParentCoroutine_(IoObject *self, IoObject *v);
IoObject *IoCoroutine_rawParentCoroutine(IoObject *self);
// exception

void IoCoroutine_rawSetResult_(IoObject *self, IoObject *v);
IoObject *IoCoroutine_rawResult(IoObject *self);

// exception

void IoCoroutine_rawRemoveException(IoObject *self);
void IoCoroutine_rawSetException_(IoObject *self, IoObject *v);
IoObject *IoCoroutine_rawException(IoObject *self);

// ioStack

IoObject *IoCoroutine_ioStack(IoCoroutine *self, IoObject *locals, IoMessage *m);
int IoCoroutine_rawIoStackSize(IoObject *self);

// raw

void IoCoroutine_rawRun(IoObject *self);

void IoCoroutine_clearStack(IoObject *self);

void IoCoroutine_try(IoObject *self, IoObject *target, IoObject *locals, IoMessage *message);

IoCoroutine *IoCoroutine_newWithTry(void *state, 
							 IoObject *target, 
							 IoObject *locals, 
							 IoMessage *message);

void IoCoroutine_raiseError(IoCoroutine *self, IoSymbol *description, IoMessage *m);

// methods

IoObject *IoCoroutine_implementation(IoObject *self, IoObject *locals, IoMessage *m);

IoObject *IoCoroutine_run(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoCoroutine_callStack(IoObject *self, IoObject *locals, IoMessage *m);

IoObject *IoCoroutine_isCurrent(IoObject *self, IoObject *locals, IoMessage *m);

// runTarget

void IoCoroutine_rawSetRunTarget_(IoObject *self, IoObject *v);
IoObject *IoCoroutine_rawRunTarget(IoObject *self);

// message

void IoCoroutine_rawSetRunMessage_(IoObject *self, IoObject *v);
IoObject *IoCoroutine_rawRunMessage(IoObject *self);

// parent

void IoCoroutine_rawSetParentCoroutine_(IoObject *self, IoObject *v);
IoObject *IoCoroutine_rawParentCoroutine(IoObject *self);

// try

IoObject *IoCoroutine_start(IoObject *self, IoObject *locals, IoMessage *m);

IoObject *IoCoroutine_rawResume(IoObject *self);
IoObject *IoCoroutine_resume(IoObject *self, IoObject *locals, IoMessage *m);

IoObject *IoCoroutine_isCurrent(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoCoroutine_currentCoroutine(IoObject *self, IoObject *locals, IoMessage *m);

// stack trace

IoObject *IoObject_callStack(IoObject *self);

void IoObject_appendStackEntryDescription(IoObject *self, ByteArray *ba);

ByteArray *IoCoroutine_backTraceOnCallStackDescription_(IoObject *self, IoObject *calls);
void IoCoroutine_rawPrint(IoObject *self);

// debugging

int IoCoroutine_rawDebuggingOn(IoObject *self);

IoObject *IoCoroutine_setMessageDebugging(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoObject_performWithDebugger(IoObject *self, IoObject *locals, IoMessage *m);

IoObject *IoCoroutine_callStack(IoObject *self, IoObject *locals, IoMessage *m);

void IoCoroutine_rawPrintBackTrace(IoObject *self);

#ifdef __cplusplus
}
#endif
#endif
