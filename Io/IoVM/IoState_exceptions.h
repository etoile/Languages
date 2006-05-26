/*#io
docCopyright("Steve Dekorte", 2002)
docLicense("BSD revised")
*/

void IoState_fatalError_(IoState *self, char *error);

void IoState_error_(IoState *self, IoMessage *m, const char *format, ...);
