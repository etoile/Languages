
// Embedding callback function types

typedef void (IoStateBindingsInitCallback)(void *, void *);
typedef void (IoStatePrintCallback)(void *, size_t count, const char *);
typedef void (IoStateExceptionCallback)(void *, IoObject *);
typedef void (IoStateExitCallback)(void *, int);
typedef void (IoStateActiveCoroCallback)(void *, int);
typedef void (IoStateThreadLockCallback)(void *);
typedef void (IoStateThreadUnlockCallback)(void *);

// context

void IoState_callbackContext_(IoState *self, void *context);
void *IoState_callbackContext(IoState *self);

// bindings

void IoState_setBindingsInitCallback(IoState *self, IoStateBindingsInitCallback *callback);

// print

void IoState_print_(IoState *self, const char *format, ...);
void IoState_justPrint_(IoState *self, const size_t count, const char *s);
void IoState_justPrintln_(IoState *self);
void IoState_justPrintba_(IoState *self, ByteArray *ba);
void IoState_printCallback_(IoState *self, IoStatePrintCallback *callback);

// exceptions

void IoState_exceptionCallback_(IoState *self, IoStateExceptionCallback *callback);
void IoState_exception_(IoState *self, IoObject *e);

// exit

void IoState_exitCallback_(IoState *self, IoStateExitCallback *callback);
void IoState_exit(IoState *self, int returnCode);

// coros - called when coro count changes

void IoState_activeCoroCallback_(IoState *self, IoStateActiveCoroCallback *callback);
void IoState_schedulerUpdate(IoState *self, int count);

