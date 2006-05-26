/*#io
docCopyright("Steve Dekorte", 2002)
docLicense("BSD revised")
*/


IoObject *IoState_currentCoroutine(IoState *self);
void IoState_setCurrentCoroutine_(IoState *self, IoObject *coroutine);

void IoState_yield(IoState *self);


