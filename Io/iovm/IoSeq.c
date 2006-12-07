

#define IOSEQ_C
#include "IoSeq.h"
#undef IOSEQ_C 
#include "IoSeq_mutable.h"
#include "IoSeq_immutable.h"
#include "IoState.h"
#include "IoCFunction.h"
#include "IoObject.h"
#include "IoNumber.h"
#include "IoMessage.h"
#include "IoList.h"
#include "IoSeq.h"
#include <ctype.h>

#define BIVAR(self) ((ByteArray *)(IoObject_dataPointer(self)))
//#define HASHIVAR(self) ((self)->extraData)

int ISMUTABLESEQ(IoObject *self)
{
	return ISSEQ(self) && !(self->isSymbol);
}

int ioSeqCompareFunc(void *s1, void *s2)
{
    ByteArray *ba1 = BIVAR((IoSeq *)s1);
    ByteArray *ba2 = BIVAR((IoSeq *)s2);
    
    if (s1 == s2 || ba1 == ba2) 
    {
        return 0;
    }
    
    // this depends on the encoding! 
    
    if (ISSYMBOL(((IoSymbol *)s1)) && ISSYMBOL(((IoSymbol *)s2)))
    {
        return strcmp(ByteArray_asCString(ba1), ByteArray_asCString(ba2));
    }
    
    return ByteArray_compare_(ba1, ba2);
}

int ioSymbolFindFunc(void *s, void *ioSymbol)
{ 
    return strcmp((char *)s, (char *)(BIVAR((IoObject *)ioSymbol)->bytes)); 
}

int IoObject_isStringOrBuffer(IoObject *self)
{
    return ISSEQ(self);
}

int IoObject_isNotStringOrBuffer(IoObject *self)
{
    return !(ISSEQ(self));
}

void IoSeq_rawPrint(IoSeq *self)
{
    IoState_justPrintba_(IOSTATE, BIVAR(self));
}

/*
 void IoSymbol_writeToStore_stream_(IoSymbol *self, IoStore *store, BStream *stream)
 {
     BStream_writeTaggedByteArray_(stream, BIVAR(self));
 }
 
 IoSymbol *IoSymbol_allocFromStore_stream_(IoSymbol *self, IoStore *store, BStream *stream)
 {
     ByteArray *ba = BStream_readTaggedByteArray(stream);
     
     if (!ba)
     {
         printf("String read error: missing byte array");
         IoState_exit(IOSTATE, -1);
     }
     
     return IoState_symbolWithByteArray_copy_(IOSTATE, ba, 1);  
 }
 */

void IoSeq_writeToStore_stream_(IoSeq *self, IoStore *store, BStream *stream)
{
    BStream_writeTaggedByteArray_(stream, BIVAR(self));
}

void IoSeq_readFromStore_stream_(IoSeq *self, IoStore *store, BStream *stream)
{
    BStream_readTaggedByteArray_(stream, BIVAR(self));
}


IoTag *IoSeq_tag(void *state)
{
    IoTag *tag = IoTag_newWithName_("Sequence");
    tag->state = state;
    tag->freeFunc    = (TagFreeFunc *)IoSeq_free;
    tag->cloneFunc   = (TagCloneFunc *)IoSeq_rawClone;
    tag->compareFunc = (TagCompareFunc *)IoSeq_compare;
    tag->writeToStoreOnStreamFunc  = (TagWriteToStoreOnStreamFunc *)IoSeq_writeToStore_stream_;
    tag->readFromStoreOnStreamFunc = (TagReadFromStoreOnStreamFunc *)IoSeq_readFromStore_stream_;
    return tag;
}

IoSeq *IoSeq_proto(void *state)
{
    IoObject *self = IoObject_new(state);
    
    self->tag = IoSeq_tag(state);
    IoObject_setDataPointer_(self, ByteArray_new());
    
    IoState_registerProtoWithFunc_((IoState *)state, self, IoSeq_proto);
    return self;
}

IoSeq *IoSeq_protoFinish(IoSeq *self)
{
    IoSeq_addImmutableMethods(self);
    IoSeq_addMutableMethods(self);
    return self;
}

IoSeq *IoSeq_rawClone(IoSeq *proto)
{
    if (ISSYMBOL(proto))
    {
		return proto;
    }
    else
    {
		IoSeq *self = IoObject_rawClonePrimitive(proto);
		IoObject_setDataPointer_(self, ByteArray_clone(BIVAR(proto)));
		return self;
    }
}

// ----------------------------------------------------------- 

IoSeq *IoSeq_new(void *state)
{
    IoSeq *proto = IoState_protoWithInitFunction_((IoState *)state, IoSeq_proto);
    return IOCLONE(proto);
}

IoSeq *IoSeq_newWithData_length_(void *state, const unsigned char *s, size_t length)
{
    IoSeq *self = IoSeq_new(state);
    ByteArray_setData_size_(BIVAR(self), s, length);
    return self;
}

IoSeq *IoSeq_newWithDatum_(void *state, Datum *d)
{
	return IoSeq_newWithCString_length_(state, (char *)Datum_data(d), Datum_size(d));
}

IoSeq *IoSeq_newWithCString_(void *state, const char *s)
{ 
    return IoSeq_newWithData_length_(state, (unsigned char *)s, strlen(s)); 
}

IoSeq *IoSeq_newWithCString_length_(void *state, const char *s, size_t length)
{ 
    return IoSeq_newWithData_length_(state, (unsigned char *)s, length); 
}

IoSeq *IoSeq_newWithByteArray_copy_(void *state, ByteArray *ba, int copy)
{
    IoSeq *self = IoSeq_new(state);
    
    if (copy)
    { 
        ByteArray_copy_(BIVAR(self), ba); 
    }
    else
    {
        ByteArray_free(BIVAR(self));
        IoObject_setDataPointer_(self, ba);
    }
    
    return self;
}

IoSeq *IoSeq_newFromFilePath_(void *state, const char *path)
{
    IoSeq *self = IoSeq_new(state);
    ByteArray_readFromFilePath_(BIVAR(self), path);
    return self;
}

IoSeq *IoSeq_rawMutableCopy(IoSeq *self)
{ 
    return IoSeq_newWithByteArray_copy_(IOSTATE, BIVAR(self), 1); 
}

// these Symbol creation methods should only be called by IoState ------ 

IoSymbol *IoSeq_newSymbolWithData_length_(void *state, const char *s, size_t length)
{
    IoObject *self = IoSeq_new(state);
    ByteArray_setData_size_(BIVAR(self), (unsigned char *)s, length);
    return self;
}

IoSymbol *IoSeq_newSymbolWithCString_(void *state, const char *s)
{ 
    return IoSeq_newSymbolWithData_length_(state, s, strlen(s)); 
}

IoSymbol *IoSeq_newSymbolWithByteArray_copy_(void *state, ByteArray *ba, int copy)
{
    IoObject *self = IoSeq_new(state);
    
    if (copy)
    { 
        ByteArray_copy_(BIVAR(self), ba); 
    }
    else
    {
        ByteArray_free(BIVAR(self));
        IoObject_setDataPointer_(self, ba);
    }
    
    return self;
}

// these Symbol creation methods can be called by anyone 

IoSymbol *IoSeq_newSymbolWithFormat_(void *state, const char *format, ...)
{
    ByteArray *ba;
    va_list ap;
    va_start(ap, format);
    ba = ByteArray_newWithVargs_(format, ap);
    va_end(ap);
    return IoState_symbolWithByteArray_copy_(state, ba, 0);
}

// ----------------------------------------------------- 

void IoSeq_free(IoSeq *self) 
{
    if (self->isSymbol) 
    { 
        IoState_removeSymbol_(IOSTATE, self); 
    }
    
    if (BIVAR(self) != NULL)
    {
        ByteArray_free(BIVAR(self));
    }
}

int IoSeq_compare(IoSeq *self, IoSeq *v) 
{ 
    if (ISSEQ(v))
    {
        if (ISSYMBOL(v) && self == v) 
        { 
            return 0; 
        }
        
        return ioSeqCompareFunc(self, v); 
    }
    
    return ((ptrdiff_t)self->tag) - ((ptrdiff_t)v->tag);
}

ByteArray *IoSeq_rawByteArray(IoSeq *self)
{ 
    return BIVAR(self); 
}

char *IoSeq_asCString(IoSeq *self) 
{ 
    return (char *)(BIVAR(self)->bytes); 
}

unsigned char *IoSeq_rawBytes(IoSeq *self)
{ 
    return (unsigned char *)(BIVAR(self)->bytes); 
}

size_t IoSeq_rawSize(IoSeq *self)
{ 
    return (size_t)(ByteArray_size(BIVAR(self))); 
}

size_t IoSeq_rawSizeInBytes(IoSeq *self)
{ 
    return (size_t)(ByteArray_size(BIVAR(self))); 
}

double IoSeq_asDouble(IoSeq *self)
{ 
    return strtod((char *)(BIVAR(self)->bytes), NULL); 
}

Datum IoSeq_asDatum(IoSymbol *self)
{
    return ByteArray_asDatum(BIVAR(self));
}

// ----------------------------------------------------------- 

void IoSeq_rawSetSize_(IoSeq *self, size_t size)
{ 
    ByteArray_setSize_(BIVAR(self), size); 
}

size_t IoSeq_memorySize(IoObject *self)
{ 
    return sizeof(IoSeq) + ByteArray_memorySize(BIVAR(self)); 
}

void IoSeq_compact(IoObject *self) 
{ 
    ByteArray_compact(BIVAR(self)); 
}

void IoSeq_setIsSymbol_(IoSeq *self, int i)
{ 
    self->isSymbol = (unsigned char)i; 
}

// ----------------------------------------------------------- 

IoSymbol *IoSeq_rawAsUnquotedSymbol(IoObject *self)
{
    ByteArray *ba = ByteArray_clone(BIVAR(self));
    ByteArray_unquote(ba);
    return IoState_symbolWithByteArray_copy_(IOSTATE, ba, 0);
}

IoSymbol *IoSeq_rawAsUnescapedSymbol(IoObject *self)
{
    ByteArray *ba = ByteArray_clone(BIVAR(self));
    ByteArray_unescape(ba);
    return IoState_symbolWithByteArray_copy_(IOSTATE, ba, 0);
}

double IoSeq_rawAsDoubleFromHex(IoObject *self)
{
    char *s = IoSeq_asCString(self);
    unsigned int i;
    
    sscanf(s, "%x", &i);
    return (double)i;
}

double IoSeq_rawAsDoubleFromOctal(IoObject *self)
{
    char *s = IoSeq_asCString(self);
    unsigned int i;
    
    sscanf(s, "%o", &i);
    return (double)i;
}

int IoSeq_rawEqualsCString_(IoObject *self, const char *s) 
{ 
    return (strcmp((char *)BIVAR(self)->bytes, s) == 0); 
}

/*
 int IoSeq_rawIsNotAlphaNumeric(IoObject *self)
 {
     char *s = (char *)BIVAR(self)->bytes;
     
     while (!isalnum((int)*s) && *s != 0) 
     { 
         s ++; 
     }
     
     return (*s == 0);
 }
 
 unsigned int IoSeq_rawHashCode(IoSeq *self)
 { 
     uintptr_t h = (uintptr_t)HASHIVAR(self); 
     return (unsigned int)h;
 }
 
 void IoSeq_rawSetHashCode_(IoSeq *self, unsigned int h)
 {
     HASHIVAR(self) = (void *)(uintptr_t)h; 
 }
 */

