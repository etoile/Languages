/*
docCopyright("Steve Dekorte", 2004)
docLicense("BSD revised")
docDescription("A Binary Stream, supports tagged items.")
*/

#ifndef BSTREAM_DEFINED
#define BSTREAM_DEFINED 1

#include "Common.h"
#include "ByteArray.h"
#include "BStreamTag.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    ByteArray *ba;
    size_t index;
    unsigned char ownsByteArray;
    ByteArray *tmp;
    ByteArray *errorBa;
    int flipEndian;
    unsigned char *typeBuf;
} BStream;

BStream *BStream_new(void);
BStream *BStream_clone(BStream *self);
void BStream_free(BStream *self);

char *BStream_errorString(BStream *self);
void BStream_setByteArray_(BStream *self, ByteArray *ba);
void BStream_setData_length_(BStream *self, unsigned char *data, size_t length);
ByteArray *BStream_byteArray(BStream *self);
Datum BStream_datum(BStream *self);
void BStream_empty(BStream *self);
int BStream_isEmpty(BStream *self);

// writing -------------------------------------- 

void BStream_writeByte_(BStream *self, unsigned char v);

void BStream_writeUint8_(BStream *self, uint8_t v);
void BStream_writeUint32_(BStream *self, uint32_t v);
void BStream_writeInt32_(BStream *self, int32_t v);
#if !defined(__SYMBIAN32__)
void BStream_writeInt64_(BStream *self, int64_t v);
#endif
void BStream_writeDouble_(BStream *self, double v);
void BStream_writeData_length_(BStream *self, const unsigned char *data, size_t length);
void BStream_writeCString_(BStream *self, const char *s);
void BStream_writeByteArray_(BStream *self, ByteArray *ba);

// reading -------------------------------------- 

unsigned char BStream_readByte(BStream *self);

uint8_t BStream_readUint8(BStream *self);
uint32_t BStream_readUint32(BStream *self);
int32_t BStream_readInt32(BStream *self);
#if !defined(__SYMBIAN32__)
int64_t BStream_readInt64(BStream *self);
#endif
double BStream_readDouble(BStream *self);
uint8_t *BStream_readDataOfLength_(BStream *self, size_t length);
void BStream_readByteArray_(BStream *self, ByteArray *b);
ByteArray *BStream_readByteArray(BStream *self);
const char *BStream_readCString(BStream *self);

// tagged writing -------------------------------------- 

void BStream_writeTaggedUint8_(BStream *self, uint8_t v);
void BStream_writeTaggedUint32_(BStream *self, uint32_t v);
void BStream_writeTaggedInt32_(BStream *self, int32_t v);
#if !defined(__SYMBIAN32__)
void BStream_writeTaggedInt64_(BStream *self, int64_t v);
#endif
void BStream_writeTaggedDouble_(BStream *self, double v);
void BStream_writeTaggedData_length_(BStream *self, const unsigned char *data, size_t length);
void BStream_writeTaggedCString_(BStream *self, const char *s);
void BStream_writeTaggedByteArray_(BStream *self, ByteArray *ba);

// tagged reading --------------------------------------

uint8_t BStream_readTaggedUint8(BStream *self);
uint32_t BStream_readTaggedUint32(BStream *self);
int32_t BStream_readTaggedInt32(BStream *self);
#if !defined(__SYMBIAN32__)
int64_t BStream_readTaggedInt64(BStream *self);
#endif
double BStream_readTaggedDouble(BStream *self);
void BStream_readTaggedByteArray_(BStream *self, ByteArray *b);
ByteArray *BStream_readTaggedByteArray(BStream *self);
const char *BStream_readTaggedCString(BStream *self);

void BStream_show(BStream *self);

#ifdef __cplusplus
}
#endif
#endif

