/*#io
docCopyright("Steve Dekorte", 2002)
docLicense("BSD revised")
*/

void IoState_break(IoState *self, IoObject *v);

void IoState_continue(IoState *self);

void IoState_return(IoState *self, IoObject *v);

void IoState_resetStopStatus(IoState *self);

int IoState_handleStatus(IoState *self);
