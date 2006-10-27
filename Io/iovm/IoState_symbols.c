/*
 docCopyright("Steve Dekorte", 2002)
 docLicense("BSD revised")
 */

#include "IoState.h"
#include "IoObject.h"
#include "IoSeq.h"
#include "IoNumber.h"

#define MIN_CACHED_NUMBER -10
#define MAX_CACHED_NUMBER 256

void IoState_setupCachedNumbers(IoState *self)
{
	int i;
	
	self->cachedNumbers = List_new();
	
	for (i = MIN_CACHED_NUMBER; i < MAX_CACHED_NUMBER + 1; i ++)
	{
		IoNumber *number = IoNumber_newWithDouble_(self, i);
		List_append_(self->cachedNumbers, number);
		IoState_retain_(self, number);
	}
}

IoObject *IoState_numberWithDouble_(IoState *self, double n)
{
	long i = (long)n;
	
	if (self->cachedNumbers && i == n && i >= MIN_CACHED_NUMBER && i <= MAX_CACHED_NUMBER)
	{
		return List_at_(self->cachedNumbers, i - MIN_CACHED_NUMBER);
	}
	
	return IoNumber_newWithDouble_(self, n);
}

// ----------------------------------


int ioStateStringCmp(const unsigned char *data, size_t size, IoSymbol *s)
{
	Datum d1 = Datum_FromData_length_((unsigned char *)data, size);
	Datum d2 = IoSeq_asDatum(s);
	return Datum_compare_(&d1, &d2);
}

IoSymbol *IoState_symbolWithByteArray_copy_(IoState *self, ByteArray *ba, int copy)
{
	Datum k = ByteArray_asDatum(ba);
	SkipDBRecord *r = SkipDB_recordAt_(self->symbols, k);
	
	if (!r)
	{
		IoSymbol *ioSymbol = IoSeq_newSymbolWithByteArray_copy_(self, ba, copy);
		return IoState_addSymbol_(self, ioSymbol);
	}
	
	if (!copy) 
	{ 
		ByteArray_free(ba); 
	}
	
	{
		IoObject *symbol = (IoSymbol *)SkipDBRecord_object(r);
		IoState_stackRetain_(self, symbol);
		return symbol;
	}
}

IoSymbol *IoState_symbolWithCString_length_(IoState *self, const char *s, int length)
{
	Datum k = Datum_FromData_length_((unsigned char *)s, length);
	SkipDBRecord *r = SkipDB_recordAt_(self->symbols, k);
	IoSymbol *ioSymbol;
	
	if (!r)
	{
		// the new string is automatically stack retained 
		ioSymbol = IoSeq_newSymbolWithData_length_(self, s, length);
		return IoState_addSymbol_(self, ioSymbol);
	}
	
	ioSymbol = (IoSymbol *)SkipDBRecord_object(r);
	IoState_stackRetain_(self, ioSymbol);
	return ioSymbol;
}

IoSymbol *IoState_symbolWithDatum_(IoState *self, Datum *d)
{
	return IoState_symbolWithCString_length_(self, (char *)d->data, d->size);
}

IoSymbol *IoState_symbolWithCString_(IoState *self, const char *s)
{
	return IoState_symbolWithCString_length_(self, s, strlen(s));
}

IoSymbol *IoState_addSymbol_(IoState *self, IoSymbol *s)
{
	Datum k = IoSeq_asDatum(s);
	Datum v = Datum_Empty();   
	
	SkipDBRecord *r = SkipDB_recordAt_put_(self->symbols, k, v);
	
	SkipDBRecord_object_(r, s);
	IoSeq_setIsSymbol_(s, 1);
	return s;
}

void IoState_removeSymbol_(IoState *self, IoSymbol *aString)
{ 
	SkipDB_removeAt_(self->symbols, IoSeq_asDatum(aString)); 
}
