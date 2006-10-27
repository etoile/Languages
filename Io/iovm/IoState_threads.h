/*#io
docCopyright("Steve Dekorte", 2002)
docLicense("BSD revised")
*/

#include "IoState.h"

void IoState_initThreads(IoState *self);
void IoState_shutdownThreads(IoState *self);

List *IoState_threadIds(IoState *self);

void *IoState_createThreadAndEval(IoState *self, char *s);

void *IoState_beginThread(void *state);
void IoState_endThisThread(IoState *self);

int IoState_threadCount(IoState *self);
