/*#io
docCopyright("Steve Dekorte", 2002)
docLicense("BSD revised")
*/

void IoState_setupCachedNumbers(IoState *self);
IoObject *IoState_numberWithDouble_(IoState *self, double n);

IoSymbol *IoState_symbolWithByteArray_copy_(IoState *self, ByteArray *ba, int copy);
IoSymbol *IoState_symbolWithCString_(IoState *self, const char *s);
IoSymbol *IoState_symbolWithCString_length_(IoState *self, const char *s, int length);
IoSymbol *IoState_symbolWithDatum_(IoState *self, Datum *d);

IoSymbol *IoState_addSymbol_(IoState *self, IoSymbol *s);
void IoState_removeSymbol_(IoState *self, IoSymbol *aString);

