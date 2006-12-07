/*#io
docCopyright("Steve Dekorte", 2002)
docLicense("BSD revised")
*/

IOVM_API void IoState_setupCachedNumbers(IoState *self);
IOVM_API IoObject *IoState_numberWithDouble_(IoState *self, double n);

IOVM_API IoSymbol *IoState_symbolWithByteArray_copy_(IoState *self, ByteArray *ba, int copy);
IOVM_API IoSymbol *IoState_symbolWithCString_(IoState *self, const char *s);
IOVM_API IoSymbol *IoState_symbolWithCString_length_(IoState *self, const char *s, int length);
IOVM_API IoSymbol *IoState_symbolWithDatum_(IoState *self, Datum *d);

IOVM_API IoSymbol *IoState_addSymbol_(IoState *self, IoSymbol *s);
IOVM_API void IoState_removeSymbol_(IoState *self, IoSymbol *aString);

