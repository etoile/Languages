/*#io
Sequence ioDoc(
docCopyright("Steve Dekorte", 2002)
docLicense("BSD revised")
*/

#ifndef IOSEQ_DEFINED
#define IOSEQ_DEFINED 1

#include "Common.h"
#include "ByteArray.h"
#include "IoObject_struct.h"
#include "IoMessage.h"

#ifdef __cplusplus
extern "C" {
#endif

int ISMUTABLESEQ(IoObject *self);

#define IOSEQ(data, size)  IoSeq_newWithData_length_((IoState*)IOSTATE, data, size)
#define IOSEQ_LENGTH(self) ByteArray_size((ByteArray *)(IoObject_dataPointer(self)))
#define IOSEQ_BYTES(self)  ByteArray_bytes((ByteArray *)(IoObject_dataPointer(self)))
#define ISSEQ(self)	   IoObject_hasCloneFunc_(self, (TagCloneFunc *)IoSeq_rawClone)

#define WHITESPACE         " \t\n\r"

// Symbol defines 

#define IOSYMBOL(s)         IoState_symbolWithCString_((IoState*)IOSTATE, (char *)(s))
#define IOSYMBOLID(s)       (IoObject_dataPointer(self))
#define CSTRING(uString)    IoSeq_asCString(uString)
#define ISSYMBOL(self)      (self->isSymbol)
#define ISBUFFER(self)	    ISMUTABLESEQ(self)

#if !defined(IoSymbol_DEFINED) 
  #define IoSymbol_DEFINED 
  typedef IoObject IoSymbol;
  typedef IoObject IoSeq;
#endif

//#define IOSYMBOL_HASHCODE(self) ((unsigned int)(self->extraData))
#define IOSYMBOL_LENGTH(self)   ByteArray_size(((ByteArray *)(IoObject_dataPointer(self))))
#define IOSYMBOL_BYTES(self)    ByteArray_bytes(((ByteArray *)(IoObject_dataPointer(self))))

typedef IoObject *(IoSplitFunction)(void *, ByteArray *, int);

typedef IoObject IoSeq;

int ioSeqCompareFunc(void *s1, void *s2);
int ioSymbolFindFunc(void *s, void *ioSymbol);

int IoObject_isStringOrBuffer(IoObject *self);
int IoObject_isNotStringOrBuffer(IoObject *self);

IoSeq *IoSeq_proto(void *state);
IoSeq *IoSeq_protoFinish(IoSeq *self);

IoSeq *IoSeq_rawClone(IoSeq *self);
IoSeq *IoSeq_new(void *state);
IoSeq *IoSeq_newWithByteArray_copy_(void *state, ByteArray *ba, int copy);
IoSeq *IoSeq_newWithData_length_(void *state, const unsigned char *s, size_t length);
IoSeq *IoSeq_newWithDatum_(void *state, Datum *d);
IoSeq *IoSeq_newWithCString_length_(void *state, const char *s, size_t length);
IoSeq *IoSeq_newWithCString_(void *state, const char *s);
IoSeq *IoSeq_newFromFilePath_(void *state, const char *path);
IoSeq *IoSeq_rawMutableCopy(IoSeq *self);

// these Symbol creation methods should only be called by IoState 

IoSymbol *IoSeq_newSymbolWithCString_(void *state, const char *s);
IoSymbol *IoSeq_newSymbolWithData_length_(void *state, const char *s, size_t length);
IoSymbol *IoSeq_newSymbolWithByteArray_copy_(void *state, ByteArray *ba, int copy);

// these Symbol creation methods can be called by anyone

IoSymbol *IoSeq_newSymbolWithFormat_(void *state, const char *format, ...);

//

void IoSeq_free(IoSeq *self);
int IoSeq_compare(IoSeq *self, IoSeq *v);

char *IoSeq_asCString(IoSeq *self);
unsigned char *IoSeq_rawBytes(IoSeq *self);
ByteArray *IoSeq_rawByteArray(IoSeq *self);

size_t IoSeq_rawSize(IoSeq *self);
size_t IoSeq_rawSizeInBytes(IoSeq *self);
void IoSeq_rawSetSize_(IoSeq *self, size_t size);
void IoSeq_setIsSymbol_(IoSeq *self, int i);

// conversion 

double IoSeq_asDouble(IoSeq *self);
Datum IoSeq_asDatum(IoSeq *self);
IoSymbol *IoSeq_rawAsSymbol(IoSeq *self);

IoSymbol *IoSeq_rawAsUnquotedSymbol(IoObject *self);
IoSymbol *IoSeq_rawAsUnescapedSymbol(IoObject *self);

int IoSeq_rawEqualsCString_(IoObject *self, const char *s);
double IoSeq_rawAsDoubleFromHex(IoObject *self);
double IoSeq_rawAsDoubleFromOctal(IoObject *self);

#include "IoSeq_immutable.h"
#include "IoSeq_mutable.h"
#include "IoSeq_inline.h"

#ifdef __cplusplus
}
#endif
#endif
