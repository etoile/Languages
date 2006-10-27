/*#io
docCopyright("Steve Dekorte", 2002)
docLicense("BSD revised")
*/

IoObject *IoState_tryToPerform(IoState *self, 
							  IoObject *target, 
							  IoObject *locals, 
							  IoMessage *m);

IoObject *IoState_rawOn_doCString_withLabel_(IoState *self, 
									IoObject *target,
									const char *s,
									const char *label);

IoObject *IoState_doCString_(IoState *self, const char *s);


IoObject *IoState_on_doCString_withLabel_(IoState *self, 
								  IoObject *target, 
								  const char *s, 
								  const char *label);

//IoObject *IoState_on_doPackedCString_withLabel_(IoState *self, IoObject *target, const char *s, const char *label);

// sandbox

void IoState_zeroSandboxCounts(IoState *self);
void IoState_resetSandboxCounts(IoState *self);

IoObject *IoState_doSandboxCString_(IoState *self, const char *s);

double IoState_endTime(IoState *self);

// file

IoObject *IoState_doFile_(IoState *self, const char *path);

