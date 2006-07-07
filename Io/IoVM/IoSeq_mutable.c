/*#io
Sequence ioDoc(
			docCopyright("Steve Dekorte", 2002)
			docLicense("BSD revised")
			docObject("Sequence")
			docDescription("""A Sequence is a container for a list of data elements. Typically these elements are each 1 byte in size. A Sequence can be either mutable or immutable. When immutable, only the read-only methods can be used. 
<p>
Terminology
<ul>
<li> Buffer: A mutable Sequence of single byte elements, typically in a binary encoding
<li> Symbol or String: A unique immutable Sequence, typically in a character encoding
</ul>
""")
			docCategory("Core")
			*/

#include "IoSeq.h"
#include "IoState.h"
#include "IoCFunction.h"
#include "IoObject.h"
#include "IoNumber.h"
#include "IoMessage.h"
#include "IoList.h"
#include "IoMap.h"
#include <ctype.h>
#include <errno.h>

#define BIVAR(self) ((ByteArray *)IoObject_dataPointer(self))

#define IO_ASSERT_NOT_SYMBOL(self) \
if (ISSYMBOL(self)) \
{ \
	IoState_error_(IOSTATE, m,  \
						   "'%s' cannot be called on an immutable Sequence", CSTRING(IoMessage_name(m)));\
}

void IoSeq_addMutableMethods(IoSeq *self)
{
	IoMethodTable methodTable[] = {
	{"copy", IoSeq_copy},
	{"appendSeq", IoSeq_appendSeq},
	{"append", IoSeq_append},
	{"atInsertSeq", IoSeq_atInsertSeq},
	{"removeSlice", IoSeq_removeSlice},
	{"removeLast", IoSeq_removeLast},
	{"setSize", IoSeq_setSize},
	{"preallocateToSize", IoSeq_preallocateToSize},
	{"replaceSeq", IoSeq_replaceSeq},
	{"replaceFirstSeq", IoSeq_replaceFirstSeq},
	{"atPut", IoSeq_atPut},
	{"lowercase", IoSeq_lowercase},
	{"uppercase", IoSeq_uppercase},
		
	{"clipBeforeSeq", IoSeq_clipBeforeSeq},
	{"clipAfterSeq",  IoSeq_clipAfterSeq},
	{"clipBeforeEndOfSeq",  IoSeq_clipBeforeEndOfSeq},
	{"clipAfterStartOfSeq", IoSeq_clipAfterStartOfSeq},
		
	{"empty", IoSeq_empty},
	{"sort", IoSeq_sort},
		
	{"removeOddIndexes",  IoSeq_removeOddIndexes},
	{"removeEvenIndexes", IoSeq_removeEvenIndexes},
	{"duplicateIndexes",  IoSeq_duplicateIndexes},
	{"replaceMap", IoSeq_replaceMap},
		
	{"strip",  IoSeq_strip},
	{"lstrip", IoSeq_lstrip},
	{"rstrip", IoSeq_rstrip},
		
	{"float32ArrayAtPut", IoSeq_float32ArrayAtPut},
	{"float32ArrayAdd", IoSeq_float32ArrayAdd},
	{"float32ArrayMultiplyScalar", IoSeq_float32ArrayMultiplyScalar},
	{"zero", IoSeq_zero},
	
	{"escape",   IoSeq_escape},
	{"unescape", IoSeq_unescape},
	{"removePrefix", IoSeq_removePrefix},
	{"removeSuffix", IoSeq_removeSuffix},
	{"capitalize", IoSeq_capitalize},
	{"appendPathSeq", IoSeq_appendPathSeq},
		
	{NULL, NULL},
	};
	
	IoObject_addMethodTable_(self, methodTable);    
}

IoObject *IoSeq_copy(IoSeq *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("copy(aSequence)", "Replaces the bytes of the receiver with a copy of those in aSequence. Returns self. ")
	*/
	
	IoObject *other;
	
	IO_ASSERT_NOT_SYMBOL(self);
	
	other = IoMessage_locals_seqArgAt_(m, locals, 0);
	
	ByteArray_copy_(BIVAR(self), BIVAR(other)); 
	
	return self;
}

IoObject *IoSeq_appendSeq(IoSeq *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("appendSeq(aSequence1, aSequence2, ...)", 
		   "Appends aSequence arguments to the receiver. Returns self. ")
	*/
	
	int i;
	
	IO_ASSERT_NOT_SYMBOL(self);
	IOASSERT(IoMessage_argCount(m), "requires at least one argument");
	
	for (i = 0; i < IoMessage_argCount(m); i ++)
	{
		IoObject *other = IoMessage_locals_valueArgAt_(m, locals, i);
		
		if (ISNUMBER(other)) 
		{ 
			double d = IoNumber_asDouble(other);
			char s[24];
			memset(s, 0x0, 24);
			
			if (d == (long)d)
			{ 
				snprintf(s, 24, "%ld", (long)d); 
			}
			else
			{ 
				snprintf(s, 24, "%g", d); 
			}
			
			ByteArray_appendCString_(BIVAR(self), s);
		}
		else if (ISSEQ(other))
		{ 
			ByteArray_append_(BIVAR(self), BIVAR(other)); 
		}
		else  
		{
			IoState_error_(IOSTATE, m, 
						   "argument 0 to method '%s' must be a number, string or buffer, not a '%s'",
								  CSTRING(IoMessage_name(m)), IoObject_name(other));
		}
	}
	return self;
}

IoObject *IoSeq_append(IoSeq *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("append(aNumber)", 
		   "Appends aNumber (cast to a byte) to the receiver. 
Returns self. ")
	*/
	
	int i;
	
	IO_ASSERT_NOT_SYMBOL(self);
	IOASSERT(IoMessage_argCount(m), "requires at least one argument");
	
	for (i = 0; i < IoMessage_argCount(m); i ++)
	{
		unsigned char s = (unsigned char)IoMessage_locals_intArgAt_(m, locals, i);
		ByteArray_appendByte_(BIVAR(self), s);
	}
	
	return self;
}

IoObject *IoSeq_atInsertSeq(IoSeq *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("atInsertSeq(indexNumber, aSequence)", 
		   "Returns a new Sequence with aSequence inserted at 
indexNumber in the receiver.  ")
	*/
	
	int n = IoMessage_locals_intArgAt_(m, locals, 0);
	IoSeq *otherSeq = IoMessage_locals_seqArgAt_(m, locals, 1);
	
	IO_ASSERT_NOT_SYMBOL(self);
	
	ByteArray_insert_at_(BIVAR(self), BIVAR(otherSeq), n);
	return self;
}

// removing ---------------------------------------

IoObject *IoSeq_removeSlice(IoSeq *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("removeSlice(startIndex, endIndex)", 
		   "Removes the bytes from startIndex to endIndex. 
Returns self.")
	*/
     
	int start = IoMessage_locals_intArgAt_(m, locals, 0);
	int end   = IoMessage_locals_intArgAt_(m, locals, 1);
	
	IO_ASSERT_NOT_SYMBOL(self);
	
	ByteArray_removeSlice(BIVAR(self), start, end + 1);
	return self;
}

IoObject *IoSeq_removeLast(IoSeq *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("removeLast", 
		   "Removes the last element from the receiver. Returns self.")
	*/
	
	IO_ASSERT_NOT_SYMBOL(self);
	ByteArray_dropLastByte(BIVAR(self));
	return self;
}

IoObject *IoSeq_setSize(IoSeq *self, IoObject *locals, IoMessage *m)
{ 
	/*#io
	docSlot("setSize(aNumber)", 
		   "Sets the length in bytes of the receiver to aNumber. Return self.")
	*/
	
	int len = IoMessage_locals_intArgAt_(m, locals, 0);
	IO_ASSERT_NOT_SYMBOL(self);
	IOASSERT(len >= 0, "New size cannot be negative.");
	ByteArray_setSize_(BIVAR(self), len);
	return self;
}

void IoSeq_rawPreallocateToSize_(IoSeq *self, size_t size)
{
	if (ISSYMBOL(self))
	{
		IoState_error_(IOSTATE, 0x0, "attempt to resize an immutable Sequence");
	}
	
	ByteArray_sizeTo_(BIVAR(self), size);
}

IoObject *IoSeq_preallocateToSize(IoSeq *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("preallocateToSize(aNumber)", 
		   "If needed, resize the memory alloced for the receivers 
byte array to be large enough to fit the number of bytes specified by 
aNumber. This is useful for preallocating the memory so it doesn't 
keep getting allocated as the Sequence is appended to. This operation 
will not change the Sequence's length or contents. Returns self.")
	*/
	
	int newSize = IoMessage_locals_intArgAt_(m, locals, 0);
	IO_ASSERT_NOT_SYMBOL(self);
	IOASSERT(newSize >= 0, "New size cannot be negative.");
	ByteArray_sizeTo_(BIVAR(self), newSize);
	return self;
}

IoObject *IoSeq_replaceSeq(IoSeq *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("replaceSeq(aSequence, anotherSequence)", 
		   "Returns a new Sequence with all occurances of aSequence 
replaced with anotherSequence in the receiver. Returns self.")
	*/
	
	IoSeq *subSeq   = IoMessage_locals_seqArgAt_(m, locals, 0);
	IoSeq *otherSeq = IoMessage_locals_seqArgAt_(m, locals, 1);
	IO_ASSERT_NOT_SYMBOL(self);
	ByteArray_replace_with_(BIVAR(self), BIVAR(subSeq), BIVAR(otherSeq));
	return self;
}

IoObject *IoSeq_replaceFirstSeq(IoSeq *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("replaceFirstSeq(aSequence, anotherSequence, optionalStartIndex)", 
		   "Returns a new Sequence with the first occurance of aSequence 
replaced with anotherSequence in the receiver. If optionalStartIndex is 
provided, the search for aSequence begins at that index. Returns self.")
	*/
	
	IoSeq *subSeq   = IoMessage_locals_seqArgAt_(m, locals, 0);
	IoSeq *otherSeq = IoMessage_locals_seqArgAt_(m, locals, 1);
	size_t startIndex = 0;
	
	if (IoMessage_argCount(m) > 2)
	{
		IoMessage_locals_longArgAt_(m, locals, 1);
	}
	
	IO_ASSERT_NOT_SYMBOL(self);
	ByteArray_replaceFirst_from_with_(BIVAR(self), BIVAR(subSeq), startIndex, BIVAR(otherSeq));
	return self;
}

IoObject *IoSeq_atPut(IoSeq *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("atPut(aNumberIndex, aNumberByte)", 
		   "Sets the byte at the index specified by aNumberIndex 
to the byte value of aNumberByte. Returns self. ")
	*/
	
	int i = IoMessage_locals_intArgAt_(m, locals, 0);
	int v = IoMessage_locals_intArgAt_(m, locals, 1);
	
	IO_ASSERT_NOT_SYMBOL(self);
	ByteArray_at_put_(BIVAR(self), (int)i, (unsigned char)v);
	return self;
}

IoObject *IoSeq_lowercase(IoSeq *self, IoObject *locals, IoMessage *m)
{ 
	/*#io
	docSlot("Lowercase", 
		   "Returns a copy of the receiver with all characters made Lowercase. ")
	*/
	
	IO_ASSERT_NOT_SYMBOL(self);
	ByteArray_Lowercase(BIVAR(self)); 
	return self; 
}

IoObject *IoSeq_uppercase(IoSeq *self, IoObject *locals, IoMessage *m)
{ 
	/*#io
	docSlot("uppercase", 
		   "Makes all characters of the receiver uppercase. ")
	*/
	
	IO_ASSERT_NOT_SYMBOL(self);
	ByteArray_uppercase(BIVAR(self)); 
	return self; 
}

// clip -------------------------------------- 

IoObject *IoSeq_clipBeforeSeq(IoSeq *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("clipBeforeSeq(aSequence)", 
		   "Clips receiver before aSequence.")
	*/
	
	IoSeq *otherSeq = IoMessage_locals_seqArgAt_(m, locals, 0);
	IO_ASSERT_NOT_SYMBOL(self);
    ByteArray_clipBefore_(BIVAR(self), BIVAR(otherSeq));
    return self;
}

IoObject *IoSeq_clipAfterSeq(IoSeq *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("clipAfterSeq(aSequence)", 
		   "Removes the contents of the receiver after the end of 
the first occurance of aSequence. Returns true if anything was 
removed, or false otherwise.")
	*/
	
	IoSeq *otherSeq = IoMessage_locals_seqArgAt_(m, locals, 0);
	
	IO_ASSERT_NOT_SYMBOL(self);
	ByteArray_clipAfter_(BIVAR(self), BIVAR(otherSeq));
    return self;
}

IoObject *IoSeq_clipBeforeEndOfSeq(IoSeq *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("clipBeforeEndOfSeq(aSequence)", 
		   "Removes the contents of the receiver before the end of 
the first occurance of aSequence. Returns true if anything was 
removed, or false otherwise.")
	*/
	
	IoSeq *otherSeq = IoMessage_locals_seqArgAt_(m, locals, 0);
	IO_ASSERT_NOT_SYMBOL(self);
	ByteArray_clipBeforeEndOf_(BIVAR(self), BIVAR(otherSeq));
    return self;
}

IoObject *IoSeq_clipAfterStartOfSeq(IoSeq *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("clipAfterStartOfSeq(aSequence)", 
		   "Removes the contents of the receiver after the beginning of 
the first occurance of aSequence. Returns true if anything was 
removed, or false otherwise.")
	*/
	
	IoSeq *otherSeq = IoMessage_locals_seqArgAt_(m, locals, 0);
	IO_ASSERT_NOT_SYMBOL(self);
	ByteArray_clipAfterStartOf_(BIVAR(self), BIVAR(otherSeq));
    return self;
}

// ----------------------------------------- 

IoObject *IoSeq_empty(IoSeq *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("clear", 
		   "Sets all bytes in the receiver to 0x0 and sets 
it's length to 0. Returns self.")
	*/
	
	IO_ASSERT_NOT_SYMBOL(self);
	ByteArray_clear(BIVAR(self));
	ByteArray_setSize_(BIVAR(self), 0);
	return self;
}

int IoSeq_byteCompare(const void *a, const void *b)
{
	char aa = *(char *)a;
	char bb = *(char *)b;
	
	if (aa < bb) 
	{
		return -1;
	}
	
	if (aa == bb) 
	{
		return 0;
	}
	
	return 1;
}

IoObject *IoSeq_sort(IoSeq *self, IoObject *locals, IoMessage *m)
{
	IO_ASSERT_NOT_SYMBOL(self);
	qsort(ByteArray_bytes(BIVAR(self)), ByteArray_size(BIVAR(self)), 1, IoSeq_byteCompare); 
	return self;
}

IoObject *IoSeq_removeOddIndexes(IoSeq *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("removeOddIndexes", 
		   "Removes odd indexes in the receiver. 
For example, list(1,2,3) removeOddIndexes == list(2). Returns self.")
	*/
	
	int size = IoMessage_locals_intArgAt_(m, locals, 0);
	IO_ASSERT_NOT_SYMBOL(self);
	ByteArray_removeOddIndexesOfSize_(BIVAR(self), size); 
	return self;
}

IoObject *IoSeq_removeEvenIndexes(IoSeq *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("removeEvenIndexes", 
		   "Removes even indexes in the receiver. 
For example, list(1,2,3) removeEvenIndexes == list(1, 3). Returns self.")
	*/
	
	int size = IoMessage_locals_intArgAt_(m, locals, 0);
	IO_ASSERT_NOT_SYMBOL(self);
	ByteArray_removeEvenIndexesOfSize_(BIVAR(self), size); 
	return self;
}

IoObject *IoSeq_duplicateIndexes(IoSeq *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("duplicateIndexes", 
		   "Duplicates all indexes in the receiver. 
For example, list(1,2,3) duplicateIndexes == list(1,1,2,2,3,3). Returns self.")
	*/
	
	int size = IoMessage_locals_intArgAt_(m, locals, 0);
	IO_ASSERT_NOT_SYMBOL(self);
	ByteArray_duplicateIndexesOfSize_(BIVAR(self), size); 
	return self;
}

IoObject *IoSeq_replaceMap(IoObject *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("replaceMap(aMap)", 
		   "In the receiver, the keys of aMap replaced with it's values. Returns self.")
	*/
	
	IoMap *map = IoMessage_locals_mapArgAt_(m, locals, 0);
	Hash *hash = IoMap_rawHash(map);
	IoSymbol *subSeq = Hash_firstKey(hash);
	ByteArray *ba = BIVAR(self);
	
	IO_ASSERT_NOT_SYMBOL(self);
	
	while (subSeq)
	{
		IoSymbol *otherSeq = Hash_at_(hash, subSeq);
		
		if (ISSEQ(otherSeq))
		{
			ByteArray_replace_with_(ba, BIVAR(subSeq), BIVAR(otherSeq));
		}
		else
		{
			IoState_error_(IOSTATE, m,
								  "argument 0 to method '%s' must be a Map with Sequence values, not '%s'",
								  CSTRING(IoMessage_name(m)), IoObject_name(otherSeq));
		}
		subSeq = Hash_nextKey(hash);
	}
	
	return self;
}

// strip ---------------------------------------------------------- 

IoObject *IoSeq_strip(IoObject *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("strip(optionalSequence)", 
		   """Trims the whitespace (or optionalSequence) off both ends:
<pre>
"   Trim this string   \r\n" strip
==> "Trim this string"
</pre>
""")
	*/
	
	IO_ASSERT_NOT_SYMBOL(self);
	
	if (IoMessage_argCount(m) > 0) 
	{
		IoSeq *other  = IoMessage_locals_seqArgAt_(m, locals, 0);
		ByteArray_strip_(BIVAR(self), BIVAR(other));
	}
	else 
	{
		ByteArray *space = ByteArray_newWithCString_(WHITESPACE);
		ByteArray_strip_(BIVAR(self), space);
		ByteArray_free(space);
	}
	
	return self;
}

IoObject *IoSeq_lstrip(IoObject *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("lstrip(aSequence)", 
		   """Strips the characters in aSequence 
stripped from the beginning of the receiver. Example:
<pre>
"Keep the tail" lstrip(" eKp")
==> "the tail"
</pre>
""")
	*/
	
	IO_ASSERT_NOT_SYMBOL(self);
	
	if (IoMessage_argCount(m) > 0) 
	{
		IoSeq *other  = IoMessage_locals_seqArgAt_(m, locals, 0);
		ByteArray_lstrip_(BIVAR(self), BIVAR(other));
	}
	else 
	{
		ByteArray *space = ByteArray_newWithCString_(WHITESPACE);
		ByteArray_lstrip_(BIVAR(self), space);
		ByteArray_free(space);
	}
	
	return self;
}

IoObject *IoSeq_rstrip(IoObject *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("rstrip(aSequence)", 
		   """Strips the characters in 
aSequence stripped from the end of the receiver. Example:
<pre>
"Cut the tail off" rstrip(" afilot")
==> "Cut the"
</pre>
""")
	*/
	
	IO_ASSERT_NOT_SYMBOL(self);
	
	if (IoMessage_argCount(m) > 0) 
	{
		IoSeq *other  = IoMessage_locals_seqArgAt_(m, locals, 0);
		ByteArray_rstrip_(BIVAR(self), BIVAR(other));
	}
	else 
	{
		ByteArray *space = ByteArray_newWithCString_(WHITESPACE);
		ByteArray_rstrip_(BIVAR(self), space);
		ByteArray_free(space);
	}
	
	return self;
}

// float

IoObject *IoSeq_float32ArrayAtPut(IoSeq *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("float32ArrayAtPut(value, indexNumber)", 
		   "Treats the buffer as an array of 32 bit floats and 
returns the Number at the specified index.")
	*/
     
	float v = IoMessage_locals_floatArgAt_(m, locals, 0);
	int index = IoMessage_locals_intArgAt_(m, locals, 1);
	int length = ByteArray_size32(BIVAR(self));
	
	IOASSERT(index < length, "index out of bounds");
	IO_ASSERT_NOT_SYMBOL(self);
	ByteArray_setFloat32_at_(BIVAR(self), v, index);
	
	return self;
}

IoObject *IoSeq_float32ArrayAdd(IoSeq *self, IoObject *locals, IoMessage *m)
{
	IoSeq *other = IoMessage_locals_seqArgAt_(m, locals, 0);
	ByteArray_float32ArrayAdd_(IoSeq_rawByteArray(self), IoSeq_rawByteArray(other));
	return self;
}

IoObject *IoSeq_float32ArrayMultiplyScalar(IoSeq *self, IoObject *locals, IoMessage *m)
{
	float s = (float)IoMessage_locals_doubleArgAt_(m, locals, 0);
	ByteArray_float32ArrayMultiplyScalar_(IoSeq_rawByteArray(self), s);
	return self;
}

IoObject *IoSeq_zero(IoSeq *self, IoObject *locals, IoMessage *m)
{
	ByteArray_zero(IoSeq_rawByteArray(self));
	return self;
}

// ----------------------------------------------------------- 

IoObject *IoSeq_escape(IoObject *self, IoObject *locals, IoMessage *m)
{ 
	/*#io
	docSlot("escape", 
		   """Escape characters in the receiver are replaced with escape codes. 
For example a string containing a single return character would contain the 
following 2 characters after being escaped: "\n". Returns self.""")
	*/
	
	IO_ASSERT_NOT_SYMBOL(self);
	ByteArray_escape(BIVAR(self));
	return self; 
}

IoObject *IoSeq_unescape(IoObject *self, IoObject *locals, IoMessage *m)
{ 
	/*#io
	docSlot("unescape", 
		   "Escape codes replaced with escape characters. Returns self.")
	*/
	
	IO_ASSERT_NOT_SYMBOL(self);
	ByteArray_unescape(BIVAR(self));
	return self; 
}

IoObject *IoSeq_removePrefix(IoObject *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("removePrefix(aSequence)", 
		   "If the receiver begins with aSequence, it is removed. Returns self.")
	*/
	
	IoSeq *other = IoMessage_locals_seqArgAt_(m, locals, 0);
	
	IO_ASSERT_NOT_SYMBOL(self);
	
	if (ByteArray_beginsWith_(BIVAR(self), BIVAR(other)))
	{ 
		ByteArray_removeSlice(BIVAR(self), 0, ByteArray_size(BIVAR(other))); 
	}
	
	return self; 
}

IoObject *IoSeq_removeSuffix(IoObject *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("removeSuffix(aSequence)", 
		   "If the receiver end with aSequence, it is removed. Returns self.")
	*/
	
	IoSeq *other = IoMessage_locals_seqArgAt_(m, locals, 0);
	
	IO_ASSERT_NOT_SYMBOL(self);
	
	if (ByteArray_endsWith_(BIVAR(self), BIVAR(other)))
	{ 
		ByteArray *ba = BIVAR(self);
		ByteArray_removeSlice(ba, 
						  ByteArray_size(ba) - ByteArray_size(BIVAR(other)), 
						  ByteArray_size(ba)); 
	}
	
	return self; 
}

IoObject *IoSeq_capitalize(IoObject *self, IoObject *locals, IoMessage *m)
{ 
	/*#io
	docSlot("capitalize", 
		   "First charater of the receiver is made uppercase.")
	*/
	
	char firstChar = ByteArray_at_(BIVAR(self), 0);
	
	IO_ASSERT_NOT_SYMBOL(self);
	ByteArray_at_put_(BIVAR(self), 0, toupper(firstChar)); 
	return self; 
}

IoObject *IoSeq_appendPathSeq(IoObject *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("appendPathSeq(aSeq)", 
		   "Appends argument to the receiver such that there is one 
and only one path separator between the two. Returns self.")
	*/
	
	IoSeq *component = IoMessage_locals_seqArgAt_(m, locals, 0);

	IO_ASSERT_NOT_SYMBOL(self);
	ByteArray_appendPathCString_(BIVAR(self), (char *)BIVAR(component)->bytes);
	return self;
}
