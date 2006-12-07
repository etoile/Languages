/*#io
Sequence ioDoc(
    docCopyright("Steve Dekorte", 2002)
    docLicense("BSD revised")
    docObject("ImmutableSequence")
    docDescription("""A Sequence is a container for a list of data elements. Immutable Sequences are also called "Symbols". """)
    docCategory("Core")
*/

#define _GNU_SOURCE // for NAN macro
#include "IoSeq.h"
#include "IoState.h"
#include "IoCFunction.h"
#include "IoObject.h"
#include "IoNumber.h"
#include "IoMessage.h"
#include "IoList.h"
#include <ctype.h>
#include <errno.h>
#include <math.h> // for NAN macro
#ifdef _MSC_VER
#include <xmath.h> // XXX: C99 says NAN is supposed to be defined in math.h, someone needs to give Microsoft a clue
#endif
#ifndef NAN
#define NAN 0.0/0.0
#endif

#define BIVAR(self) ((ByteArray *)IoObject_dataPointer(self))

void IoSeq_addImmutableMethods(IoSeq *self)
{
    IoMethodTable methodTable[] = {
	{"asBinaryNumber", IoSeq_asBinaryNumber},
	{"isSymbol", IoSeq_isSymbol},
	{"isMutable", IoSeq_isMutable},
	{"asSymbol", IoSeq_asSymbol},
	{"asString", IoSeq_asSymbol},
	{"asNumber", IoSeq_asNumber},
	{"whiteSpaceStrings", IoSeq_whiteSpaceStrings},
	{"print", IoSeq_print},
	{"linePrint", IoSeq_linePrint},
	{"size", IoSeq_size},
	{"isEmpty", IoSeq_isEmpty},
	{"at", IoSeq_at},
	{"slice", IoSeq_slice},
     {"between", IoSeq_between},
     {"betweenSeq", IoSeq_between},
	{"findSeq", IoSeq_findSeq},
	{"reverseFindSeq", IoSeq_reverseFindSeq},
	{"beginsWithSeq", IoSeq_beginsWithSeq},
	{"endsWithSeq", IoSeq_endsWithSeq},
	{"join", IoSeq_join},
	{"split", IoSeq_split},
	{"contains", IoSeq_contains},
	{"containsSeq", IoSeq_containsSeq},
	{"containsAnyCaseSeq", IoSeq_containsAnyCaseSeq},
	{"isLowercase", IoSeq_isLowercase},
	{"isUppercase", IoSeq_isUppercase},
	{"isEqualAnyCase", IoSeq_isEqualAnyCase},
	{"splitAt", IoSeq_splitAt},
	{"int32At", IoSeq_int32At},
	{"uint32At", IoSeq_uint32At},
	{"float32At", IoSeq_float32At},
	{"fromBase", IoSeq_fromBase},
	{"toBase", IoSeq_toBase},
	{"foreach", IoSeq_foreach},
	{"asMessage", IoSeq_asMessage},
     {"..", IoSeq_cloneAppendSeq},
     {"cloneAppendSeq", IoSeq_cloneAppendSeq},
	{"asMutable", IoSeq_asMutable},
	{"asBuffer", IoSeq_asMutable},
		
	{"fileName", IoSeq_fileName},
	{"pathExtension", IoSeq_pathExtension},
	{"lastPathComponent", IoSeq_lastPathComponent},
	{"cloneAppendPath", IoSeq_cloneAppendPath},
	{"pathComponent", IoSeq_pathComponent},
		
	{"afterSeq",  IoSeq_afterSeq},
	{"beforeSeq", IoSeq_beforeSeq},
		
	{"asCapitalized", IoSeq_asCapitalized},
	{"asUppercase", IoSeq_asUppercase},
	{"asLowercase", IoSeq_asLowercase},
	{"with", IoSeq_with},
	{"occurancesOfSeq", IoSeq_occurancesOfSeq},
		
	{NULL, NULL},
    };
    
    IoObject_addMethodTable_(self, methodTable);    
}

IoObject *IoSeq_rawAsSymbol(IoSeq *self)
{
    if (ISSYMBOL(self))
    {
		return self;
    }
    
    return IoState_symbolWithByteArray_copy_(IOSTATE, BIVAR(self), 1); 
}

IoObject *IoSeq_with(IoSeq *self, IoObject *locals, IoMessage *m)
{
    /*#io
    docSlot("with(aSequence, ...)", 
			"Returns a new Sequence which is the concatination of the arguments. 
The returned sequence will have the same mutability status as the receiver.")
    */
    
    int n, argCount = IoMessage_argCount(m);
    ByteArray *ba = ByteArray_clone(BIVAR(self));
    
    for (n = 0; n < argCount; n ++)
    {
		IoSeq *v = IoMessage_locals_seqArgAt_(m, locals, n);
		ByteArray_append_(ba, BIVAR(v));
    }
    
    if (ISSYMBOL(self))
    {
		return IoState_symbolWithByteArray_copy_(IOSTATE, ba, 0);
    }
    
    return IoSeq_newWithByteArray_copy_(IOSTATE, ba, 0);
}

IoObject *IoSeq_asBinaryNumber(IoSeq *self, IoObject *locals, IoMessage *m)
{
    /*#io
    docSlot("asBinaryNumber", 
			"Returns a Number containing the first 8 bytes of the 
receiver without casting them to a double.")
    */
    
    IoNumber *byteCount = IoMessage_locals_valueArgAt_(m, locals, 0);
    int max = ByteArray_size(BIVAR(self));
    int bc = sizeof(double);
    double d = 0;
    
    if (!ISNIL(byteCount)) 
    {
		bc = IoNumber_asInt(byteCount);
    }
    
    if (max < bc)
    {
		IoState_error_(IOSTATE, m, "requested first %i bytes, but Sequence only contians %i bytes", bc, max);
    }
    
    memcpy(&d, BIVAR(self)->bytes, bc);
    return IONUMBER(d);
}

IoObject *IoSeq_asSymbol(IoSeq *self, IoObject *locals, IoMessage *m)
{
    /*#io
    docSlot("asSymbol", 
			"Returns a immutable Sequence (aka Symbol) version of the receiver.")
    */
    
    return IoSeq_rawAsSymbol(self);
}

IoObject *IoSeq_isSymbol(IoSeq *self, IoObject *locals, IoMessage *m)
{
    /*#io
    docSlot("isSymbol", 
			"Returns true if the receiver is a 
immutable Sequence (aka, a Symbol) or false otherwise.")
    */
    
    return IOBOOL(self, ISSYMBOL(self));
}

IoObject *IoSeq_isMutable(IoSeq *self, IoObject *locals, IoMessage *m)
{
    /*#io
    docSlot("isMutable", 
			"Returns true if the receiver is a mutable Sequence or false otherwise.")
    */
    
    return IOBOOL(self, !ISSYMBOL(self));
}


IoObject *IoSeq_print(IoSeq *self, IoObject *locals, IoMessage *m)
{
    /*#io
    docSlot("print", 
			"Prints the receiver as a string. Returns self.")
    */
    
    IoState_justPrintba_(IOSTATE, BIVAR(self));
    return self;
}

IoObject *IoSeq_linePrint(IoObject *self, IoObject *locals, IoMessage *m)
{
    /*#io
    docSlot("linePrint", 
			"Prints the Sequence and a newline character.")
    */
    
    IoState_justPrintba_(IOSTATE, BIVAR(self));
    IoState_justPrintln_(IOSTATE);
    return self;
}

IoObject *IoSeq_isEmpty(IoSeq *self, IoObject *locals, IoMessage *m)
{
/*#io
docSlot("isEmpty", """Returns true if the size of the receiver is 0, false otherwise.""")
*/

	return IOBOOL(self, ByteArray_size(BIVAR(self)) == 0); 
}

IoObject *IoSeq_size(IoSeq *self, IoObject *locals, IoMessage *m)
{ 
    /*#io
    docSlot("size", """Returns the length in bytes of the receiver. For example,
<pre>
"abc" size == 3
<pre>
""")
    */
    
    return IONUMBER(ByteArray_size(BIVAR(self))); 
}

IoObject *IoSeq_at(IoSeq *self, IoObject *locals, IoMessage *m)
{
    /*#io
    docSlot("at(aNumber)", 
			"Returns a value at the index specified by aNumber. 
Raises an error if the index is out of bounds.")
    */
    
    int i = IoMessage_locals_intArgAt_(m, locals, 0);
    int l = 1; // grab one byte by default 
    long d = 0;
    
    if (IoMessage_argCount(m) == 2)
    {
		l = IoMessage_locals_intArgAt_(m, locals, 1);
		IOASSERT(l <= (int)sizeof(long), "size larger than bytes in a long");
    }
	
    if (i < 0)
    { 
		i = ByteArray_size(BIVAR(self)) + i; 
    }
    
    IOASSERT((i >= 0) && (i + l <= (int)ByteArray_size(BIVAR(self))), "index out of bounds");
    
    d = ByteArray_at_bytesCount_(BIVAR(self), i, l);
    
    return IONUMBER(d);
}

IoObject *IoSeq_slice(IoSeq *self, IoObject *locals, IoMessage *m)
{
    /*#io
    docSlot("slice(startIndex, endIndex)", 
			"Returns a new string containing the subset of the 
receiver from the startIndex to the endIndex. The endIndex argument 
is optional. If not given, it is assumed to be the end of the string. ")
    */
    
    int fromIndex = IoMessage_locals_intArgAt_(m, locals, 0);
    int last = ByteArray_size(BIVAR(self));
    ByteArray *ba;
    
    if (IoMessage_argCount(m) > 1)
    { 
		last = IoMessage_locals_intArgAt_(m, locals, 1); 
    }
    
    ba = ByteArray_newWithBytesFrom_to_(BIVAR(self), fromIndex, last);
    
    if (ISSYMBOL(self))
    {
		return IoState_symbolWithByteArray_copy_(IOSTATE, ba, 0);
    }
    
    return IoSeq_newWithByteArray_copy_(IOSTATE, ba, 0);
}

IoObject *IoSeq_between(IoSeq *self, IoObject *locals, IoMessage *m)
{
    /*#io
    docSlot("between(aSequence, anotherSequence)", 
			"Returns a new Sequence containing the bytes between the 
occurance of aSequence and anotherSequence in the receiver.")
    */
    
    int start = 0;
    int end = 0;
    IoSeq *fromSeq, *toSeq;
    
    fromSeq = (IoSeq *)IoMessage_locals_valueArgAt_(m, locals, 0);
    
    if (ISSEQ(fromSeq))
    {
		start = ByteArray_find_from_(BIVAR(self), BIVAR(fromSeq), 0) + 
		IoSeq_rawSize(fromSeq);
		
		if (start == -1) 
		{
			start = 0;
		}
    }
    else if (ISNIL(fromSeq)) 
    { 
		start = 0; 
    }
    else
    {
		IoState_error_(IOSTATE, m, "Nil or Sequence argument required for arg 0, not a %s", 
								   IoObject_name((IoObject *)fromSeq));
    }
    
    toSeq   = (IoSeq *)IoMessage_locals_valueArgAt_(m, locals, 1);
    
    if (ISSEQ(toSeq))
    {
		end = ByteArray_find_from_(BIVAR(self), BIVAR(toSeq), start); // -1;
		if (end == -1) start = ByteArray_size(BIVAR(self));
    }
    else if (ISNIL(toSeq))
    { 
		end = ByteArray_size(BIVAR(self)); 
    }
    else
    {
		IoState_error_(IOSTATE, m, "Nil or Sequence argument required for arg 1, not a %s", 
								   IoObject_name((IoObject *)toSeq));
    }  
    
    {
		ByteArray *ba = ByteArray_newWithBytesFrom_to_(BIVAR(self), start, end);
		IoSeq *result = IoSeq_newWithByteArray_copy_(IOSTATE, ba, 0);
		return result;
    }
}

// find ---------------------------------------------------------- 

IoObject *IoSeq_findSeq(IoSeq *self, IoObject *locals, IoMessage *m)
{
    /*#io
    docSlot("findSeq(aSequence, optionalStartIndex)", 
			"Returns a number with the first occurrence of aSequence in 
the receiver after the startIndex. If no startIndex is specified, 
the search starts at the beginning of the buffer. 
Nil is returned if no occurences are found. ")
    */
    
    IoSeq *otherSequence = IoMessage_locals_seqArgAt_(m, locals, 0);
    int f = 0;
    int index;
    
    if (IoMessage_argCount(m) > 1)
    { 
		f = IoMessage_locals_intArgAt_(m, locals, 1); 
    }
    
    index = ByteArray_find_from_(BIVAR(self), BIVAR(otherSequence), f);
    
    if (index == -1) 
    {
		return IONIL(self);
    }
    
    return IONUMBER(index);
}

IoObject *IoSeq_reverseFindSeq(IoObject *self, IoObject *locals, IoMessage *m)
{
    /*#io
    docSlot("reverseFindSeq(aSequence, startIndex)", 
			"Returns a number with the first occurrence of aSequence in 
the receiver before the startIndex. The startIndex argument is optional. 
By default reverseFind starts at the end of the string. Nil is 
returned if no occurrences are found. ")
    */
    
    IoSeq *other = IoMessage_locals_seqArgAt_(m, locals, 0);
    int from = ByteArray_size(BIVAR(self));
    int index;
    
    if (IoMessage_argCount(m) > 1)
    { 
		from = IoMessage_locals_intArgAt_(m, locals, 1); 
    }
    
    index = ByteArray_rFind_from_(BIVAR(self), BIVAR(other), from);
    
    if (index == -1) 
    {
		return IONIL(self);
    }
    
    return IONUMBER((double)index);
}

IoObject *IoSeq_beginsWithSeq(IoObject *self, IoObject *locals, IoMessage *m)
{
    /*#io
    docSlot("beginsWithSeq(aSequence)", 
			"Returns true if the receiver begins with aSequence, false otherwise.") 
    */
    
    IoSeq *other = IoMessage_locals_seqArgAt_(m, locals, 0);
    
    return IOBOOL(self, ByteArray_beginsWith_(BIVAR(self), BIVAR(other)));
}

IoObject *IoSeq_endsWithSeq(IoObject *self, IoObject *locals, IoMessage *m)
{
    /*#io
    docSlot("endsWithSeq(aSequence)", 
			"Returns true if the receiver ends with aSequence, false otherwise. ")
    */
    
    IoSeq *other = IoMessage_locals_seqArgAt_(m, locals, 0);
    return IOBOOL(self, ByteArray_endsWith_(BIVAR(self), BIVAR(other)));
}

IoObject *IoSeq_join(IoSeq *self, IoObject *locals, IoMessage *m)
{
    /*#io
    docSlot("join(aList)", 
			"""Joins a list of strings and returns a single string 
result. Example,

<pre>
l = List clone append("a", "b", "c")
result = Sequence join(l) // returns "abc"</pre>

If the receiver has a value, it becomes the separating string 
that is inserted between the strings in the result. Example,
<pre>
l = List clone append("a", "b", "c")
result = "." join(l) // result = "a.b.c"</pre>
""")
    */
    
    IoList *ioSymbols = IoMessage_locals_listArgAt_(m, locals, 0);
    List *strings = IoList_rawList(ioSymbols);
    int i, size = 0, stringCount = List_size(strings);
    ByteArray *ba = ByteArray_new();
    
    if (stringCount > 0) 
    {
		for (i = 0; i < stringCount; i ++)
		{
			IoSeq *string = LIST_AT_(strings, i);
			IOASSERT(ISSEQ(string), "Sequence join() requires all elements of it's list argument to be Sequences");
			size += ByteArray_size(BIVAR(string));
		}
		size += IoSeq_rawSize(self) * (List_size(strings)-1);
		
		ByteArray_sizeTo_(ba, size);
		
		for (i = 0; i < stringCount; i ++)
		{
			IoSeq *string = LIST_AT_(strings, i);
			ByteArray_append_(ba, BIVAR(string));
			
			if (i != List_size(strings) - 1)
			{ 
				ByteArray_append_(ba, BIVAR(self)); 
			}
		}
    }
    return IoSeq_newWithByteArray_copy_(IOSTATE, ba, 0);
}

IoObject *IoSeq_contains(IoObject *self, IoObject *locals, IoMessage *m)
{
    /*#io
    docSlot("contains(aNumber)", 
			"Returns true if the receiver contains an element equal in value to aNumber, false otherwise. ")
    */
    
    int v = IoMessage_locals_intArgAt_(m, locals, 0);
    
    return IOBOOL(self, ByteArray_containsByte_(BIVAR(self), (unsigned char)v));
}

IoObject *IoSeq_containsSeq(IoObject *self, IoObject *locals, IoMessage *m)
{
    /*#io
    docSlot("containsSeq(aSequence)", 
			"Returns true if the receiver contains the substring 
aSequence, false otherwise. ")
    */
    
    IoSeq *other = IoMessage_locals_seqArgAt_(m, locals, 0);
    
    return IOBOOL(self, ByteArray_contains_(BIVAR(self), BIVAR(other)));
}

IoObject *IoSeq_containsAnyCaseSeq(IoObject *self, IoObject *locals, IoMessage *m)
{
    /*#io
    docSlot("containsAnyCaseSeq(aSequence)", 
			"Returns true if the receiver contains the aSequence 
regardless of casing, false otherwise. ")
    */
    
    IoSeq *other = IoMessage_locals_seqArgAt_(m, locals, 0);
    return IOBOOL(self, ByteArray_containsAnyCase_(BIVAR(self), BIVAR(other)));
}

IoObject *IoSeq_isLowercase(IoObject *self, IoObject *locals, IoMessage *m)
{ 
    /*#io
    docSlot("isLowercase", 
			"Returns self if all the characters in the string are lower case.")
    */
    
    return IOBOOL(self, ByteArray_isLowercase(BIVAR(self))); 
}

IoObject *IoSeq_isUppercase(IoObject *self, IoObject *locals, IoMessage *m)
{ 
    /*#io
    docSlot("isUppercase", 
			"Returns self if all the characters in the string are upper case.")
    */
    
    return IOBOOL(self, ByteArray_isUppercase(BIVAR(self))); 
}

IoObject *IoSeq_isEqualAnyCase(IoObject *self, IoObject *locals, IoMessage *m)
{ 
    /*#io
    docSlot("isEqualAnyCase(aSequence)", 
			"Returns true if aSequence is equal to the receiver 
ignoring case differences, false otherwise.")
    */
    
    IoSeq *other = IoMessage_locals_seqArgAt_(m, locals, 0);
	
    return IOBOOL(self, ByteArray_equalsAnyCase_(BIVAR(self), BIVAR(other)));
}

IoObject *IoSeq_asNumber(IoObject *self, IoObject *locals, IoMessage *m)
{  
    /*#io
    docSlot("asNumber", 
			"Returns the receiver converted to a number. 
Initial whitespace is ignored.")
    */
    
    size_t size = ByteArray_size(BIVAR(self));
    char *s = (char *)ByteArray_bytes(BIVAR(self));
    char *endp;
    double d = strtod(s, &endp);
    
    if (size > 2 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X'))
    {
		return IONUMBER(IoSeq_rawAsDoubleFromHex(self));
    }
    
    if (errno == ERANGE || endp == s) 
    { 
		return IONUMBER(NAN);
    }
    
    return IONUMBER(d); 
}

IoList *IoSeq_whiteSpaceStrings(IoObject *self, IoObject *locals, IoMessage *m)
{
    /*
	 undocumented: yes
	 docSlot("whiteSpaceStrings", 
			 "Returns a List of strings. Each string contains a different 
	 whitespace character.")
	 */
    
    IoList *strings = IoList_new(IOSTATE);
    IoList_rawAppend_(strings, IOSYMBOL(" "));
    IoList_rawAppend_(strings, IOSYMBOL("\t"));
    IoList_rawAppend_(strings, IOSYMBOL("\n"));
    IoList_rawAppend_(strings, IOSYMBOL("\r"));
    return strings;
}

/* --- split --------------------------------------- */

IoList *IoSeq_stringListForArgs(IoObject *self, IoObject *locals, IoMessage *m)
{
    if (IoMessage_argCount(m) == 0) 
    {
		return IoSeq_whiteSpaceStrings(self, locals, m);
    }
    return IoMessage_evaluatedArgs(m, locals, m);
}

List *IoSeq_byteArrayListForArgs(IoObject *self, IoObject *locals, IoMessage *m)
{
    List *args = IoList_rawList(IoSeq_stringListForArgs(self, locals, m));
    List *list = List_new();
    
    LIST_FOREACH(args, i, s,
    
		if (!ISSEQ((IoSeq *)s))
		{
			List_free(list);
			IoState_error_(IOSTATE, m, 
						"requires Sequences as arguments, not %ss", 
						IoObject_name((IoSeq *)s));
		}
		
		List_append_(list, BIVAR(((IoSeq *)s)));
    );
    
    return list;
}

IoObject *IoSeq_splitToFunction(IoObject *self, 
								IoObject *locals, 
								IoMessage *m, 
								IoSplitFunction *func)
{
    IoList *output = IoList_new(IOSTATE);
    List *others = IoSeq_byteArrayListForArgs(self, locals, m);
    int i;

    for (i = 0; i < List_size(others); i ++)
    {
	    if (ByteArray_size(List_at_(others, i)) == 0)
	    {
		    IoState_error_(IOSTATE, m, "empty string argument");
	    }
    }
    
    {
    List *results = ByteArray_split_(BIVAR(self), others);
    
    for (i = 0; i < List_size(results); i ++)
    {
		ByteArray *ba = List_at_(results, i);
		/* 
		IoState_symbolWithByteArray_copy_  
		 or 
		 IoSeq_newWithByteArray_copy_ 
		 */
		IoObject *item = (*func)(IOSTATE, ba, 0);
		IoList_rawAppend_(output, item);
    }
    
    List_free(results);
    }
    
    List_free(others);
    return output;
}

IoObject *IoSeq_split(IoObject *self, IoObject *locals, IoMessage *m)
{
    /*#io
    docSlot("split(optionalArg1, optionalArg2, ...)", """
    Returns a list containing the sub-sequences of the receiver divided by the given arguments.
    If no arguments are given the sequence is split on white space.
    Examples:
    <pre>
    "a b c d" split == list("a", "b", "c", "d")
    "a*b*c*d" split("*") == list("a", "b", "c", "d")
    "a*b|c,d" split("*", "|", ",") == list("a", "b", "c", "d")
    "a   b  c d" split == list("a", "", "", "", "b", "", "", "c", "", "d")
    </pre>
    """)
    */
    
    return IoSeq_splitToFunction(self, locals, m, IoSeq_newWithByteArray_copy_);
}

IoObject *IoSeq_splitAt(IoObject *self, IoObject *locals, IoMessage *m)
{
    int index = IoMessage_locals_intArgAt_(m, locals, 0);
    IoList *splitSeqs = IoList_new(IOSTATE);
    index = ByteArray_wrapPos_(BIVAR(self), index);
    
    {
		char *s = ByteArray_asCString(BIVAR(self));
		IoSeq *s1 = IoState_symbolWithCString_length_(IOSTATE, s, index);
		IoSeq *s2 = IoState_symbolWithCString_(IOSTATE, s + index);
		IoList_rawAppend_(splitSeqs, (IoObject *)s1);
		IoList_rawAppend_(splitSeqs, (IoObject *)s2);
    }
    
    return splitSeqs;
}


/* --- data types ------------------------------------------------ */


IoObject *IoSeq_int32At(IoSeq *self, IoObject *locals, IoMessage *m)
{
    /*#io
    docSlot("signedInt32At(indexNumber)", 
			"Treats the buffer as an array of 32 bit signed integers 
and returns the Number at the specified index.")
    */
    
    int index = IoMessage_locals_intArgAt_(m, locals, 0);
    int length = ByteArray_size32(BIVAR(self));
    IOASSERT(index < length, "index out of bounds");
    return IONUMBER((double)ByteArray_int32At_(BIVAR(self), index));
}

IoObject *IoSeq_uint32At(IoSeq *self, IoObject *locals, IoMessage *m)
{
    /*#io
    docSlot("unsignedInt32At(indexNumber)", 
			"Treats the buffer as an array of 32 bit unsigned 
integers and returns the Number at the specified index.")
    */
    
    int index = IoMessage_locals_intArgAt_(m, locals, 0);
    int length = ByteArray_size32(BIVAR(self));
    IOASSERT(index < length, "index out of bounds");
    return IONUMBER((double)ByteArray_uint32At_(BIVAR(self), index));
}

IoObject *IoSeq_float32At(IoSeq *self, IoObject *locals, IoMessage *m)
{
    /*#io
    docSlot("float32At(indexNumber)", 
			"Treats the buffer as an array of 32 bit floats and 
returns the Number at the specified index.")
    */
    
    int index = IoMessage_locals_intArgAt_(m, locals, 0);
    int length = ByteArray_size32(BIVAR(self));
    IOASSERT(index < length, "index out of bounds");
    return IONUMBER((double)ByteArray_float32At_(BIVAR(self), index));
}

/* --- base -------------------------------------------------------------- */

IoObject *IoSeq_fromBase(IoObject *self, IoObject *locals, IoMessage *m)
{
    /*#io
    docSlot("fromBase(aNumber)", 
			"Returns a number with a base 10 representation of the receiver 
converted from the specified base. Only base 2 through 32 are currently supported.")
    */
    
    int base = IoMessage_locals_intArgAt_(m, locals, 0);
    char *s = CSTRING(self);
    unsigned long r;
    char *tail;
    errno = 0;
    r = strtoul(s, &tail, base);
    
    if (errno == EINVAL)
    {
		errno = 0;
		IoState_error_(IOSTATE, m, "conversion from base %i not supported", base);
    }
    else if (errno == ERANGE)
    {
		errno = 0;
		IoState_error_(IOSTATE, m, "resulting value \"%s\" was out of range", s);
    }
    else if (*s == 0 || *tail != 0 || errno != 0)
    {
		errno = 0;
		IoState_error_(IOSTATE, m, "conversion of \"%s\" to base %i failed", s, base);
    }
    
    return IONUMBER(r);
}

IoObject *IoSeq_toBase(IoObject *self, IoObject *locals, IoMessage *m)
{
    /*#io
    docSlot("toBase(aNumber)", 
			"Returns a Sequence containing the receiver(which is 
														assumed to be a base 10 number) converted to the specified base. 
Only base 8 and 16 are currently supported. ")
    */
    
    const char * const table = "0123456789abcdefghijklmnopqrstuvwxyz";
    int base = IoMessage_locals_intArgAt_(m, locals, 0);
    unsigned long n = (unsigned long) IoSeq_asDouble(self);
    char buf[64], *ptr = buf + 64;
    
    if (base < 2 || base > 36)
    {
		IoState_error_(IOSTATE, m, "conversion to base %i not supported", base);
    }
    
    /* Build the converted string backwards. */
    *(--ptr) = '\0';
    
    if (n == 0)
    {
		*(--ptr) = '0';
    }
    else
    {
		do 
		{
			*(--ptr) = table[n % base];
		}
		while ((n /= base) != 0);
    }
    
    return IoSeq_newWithCString_(IOSTATE, ptr);
}


IoObject *IoSeq_foreach(IoObject *self, IoObject *locals, IoMessage *m)
{
    /*#io
    docSlot("foreach(optionalIndex, value, message)", 
			"""For each element, set index to the index of the 
element and value the element value and execute message. Example:
<pre>
aSequence foreach(i, v, writeln("value at index ", i, " is ", v))
aSequence foreach(v, writeln("value ", v))
</pre>""")
    */
	
    IoObject *result = IONIL(self);
    IoMessage *doMessage;
    
    IoSymbol *indexSlotName;
    IoSymbol *characterSlotName;
    
    size_t i;
	
    IoMessage_foreachArgs(m, self, &indexSlotName, &characterSlotName, &doMessage);
    
    IoState_pushRetainPool(IOSTATE);
    
    for (i = 0; i < ByteArray_size(BIVAR(self)); i ++)
    {
        IoState_clearTopPool(IOSTATE);
	    
        if (indexSlotName)
        {
            IoObject_setSlot_to_(locals, indexSlotName, IONUMBER(i));
        }
	   
        IoObject_setSlot_to_(locals, characterSlotName, IONUMBER(ByteArray_at_(BIVAR(self), i)));
        result = IoMessage_locals_performOn_(doMessage, locals, locals);
        
        if (IoState_handleStatus(IOSTATE)) 
        {
            goto done;
        }
    }
done:
		IoState_popRetainPoolExceptFor_(IOSTATE, result);
    return result;
}

IoObject *IoSeq_asMessage(IoObject *self, IoObject *locals, IoMessage *m)
{
    /*#io
    docSlot("asMessage", 
			"Returns the compiled message object for the string.")
    */
    
    return IoMessage_newFromText_label_(IOSTATE, CSTRING(self), "[asMessage]");
}

/*#io
docSlot("..(aSequence)", "Returns a copy of the receiver with aSequence appended to it. ")
*/

IoObject *IoSeq_cloneAppendSeq(IoSeq *self, IoObject *locals, IoMessage *m)
{
    IoObject *other = IoMessage_locals_valueArgAt_(m, locals, 0);
    ByteArray *ba;
    
    if (ISNUMBER(other)) 
    { 
		other = IoNumber_justAsString((IoNumber *)other, (IoObject *)locals, m); 
    }
    
    if (!ISSEQ(other))
    {
		IoState_error_(IOSTATE, m, "argument 0 to method '%s' must be a number or string, not a '%s'",
								   CSTRING(IoMessage_name(m)), 
								   IoObject_name(other));
    }
    
    if (ByteArray_size(BIVAR(other)) == 0) 
    {
	    return self;
    }
    
    ba = ByteArray_clone(BIVAR(self));
    ByteArray_append_(ba, BIVAR(other));
    return IoState_symbolWithByteArray_copy_(IOSTATE, ba, 0);
}


IoObject *IoSeq_asMutable(IoSeq *self, IoObject *locals, IoMessage *m)
{
    /*#io
    docSlot("asMutable", 
			"Returns a mutable copy of the receiver. ")
    */
	
    return IoSeq_rawMutableCopy(self);
}

/* --- case ------------------------------------------------ */

IoObject *IoSeq_asUppercase(IoSeq *self, IoObject *locals, IoMessage *m)
{ 
    /*#io
    docSlot("asUppercase", 
			"Returns a symbol containing the reveiver made uppercase. ")
    */
    
    ByteArray *ba = ByteArray_clone(BIVAR(self));
    ByteArray_uppercase(ba);
    return IoState_symbolWithByteArray_copy_(IOSTATE, ba, 0);
}

IoObject *IoSeq_asLowercase(IoSeq *self, IoObject *locals, IoMessage *m)
{ 
    /*#io
    docSlot("asLowercase", 
			"Returns a symbol containing the reveiver made Lowercase. ")
    */
    
    ByteArray *ba = ByteArray_clone(BIVAR(self));
    ByteArray_Lowercase(ba);
    return IoState_symbolWithByteArray_copy_(IOSTATE, ba, 0);
}

/* --- path ------------------------------------------------ */


IoObject *IoSeq_lastPathComponent(IoObject *self, IoObject *locals, IoMessage *m)
{
    /*#io
    docSlot("lastPathComponent", 
			"Returns a string containing the receiver clipped up 
to the last path separator. ")
    */
    
    ByteArray *ba = ByteArray_lastPathComponent(BIVAR(self));
    return IoSeq_newWithByteArray_copy_(IOSTATE, ba, 0);
}

IoObject *IoSeq_pathExtension(IoObject *self, IoObject *locals, IoMessage *m)
{
    /*#io
    docSlot("pathExtension", 
			"Returns a string containing the receiver clipped up to the last period. ")
    */
    
    ByteArray *path = ByteArray_pathExtension(BIVAR(self));
    return IoState_symbolWithByteArray_copy_(IOSTATE, path, 0);
}

IoObject *IoSeq_fileName(IoObject *self, IoObject *locals, IoMessage *m)
{
    /*#io
    docSlot("fileName", 
			"Returns the last path component sans the path extension.")
    */
    
    ByteArray *path = ByteArray_fileName(BIVAR(self));
    return IoState_symbolWithByteArray_copy_(IOSTATE, path, 0);
}

IoObject *IoSeq_cloneAppendPath(IoObject *self, IoObject *locals, IoMessage *m)
{
    /*#io
    docSlot("cloneAppendPath(aSequence)", 
			"Appends argument to a copy the receiver such that there is one 
and only one path separator between the two and returns the result.")
    */
    
    IoSeq *component = IoMessage_locals_seqArgAt_(m, locals, 0);
    ByteArray *ba = ByteArray_clone(BIVAR(self));
    ByteArray_appendPathCString_(ba, (char *)BIVAR(component)->bytes);
    return IoState_symbolWithByteArray_copy_(IOSTATE, ba, 0);
}

IoObject *IoSeq_pathComponent(IoObject *self, IoObject *locals, IoMessage *m)
{
    /*#io
    docSlot("pathComponent", 
			"Returns a slice of the receiver before the last path separator as a symbol. ")
    */
    
    ByteArray *ba = ByteArray_clone(BIVAR(self));
    ByteArray_removeLastPathComponent(ba);
    return IoState_symbolWithByteArray_copy_(IOSTATE, ba, 0);
}

IoObject *IoSeq_beforeSeq(IoSeq *self, IoObject *locals, IoMessage *m)
{
    /*#io
    docSlot("beforeSeq(aSequence)", 
			"Returns the slice of the receiver (as a Symbol) before 
aSequence or self if aSequence is not found.")
    */
    
    IoSeq *other = IoMessage_locals_seqArgAt_(m, locals, 0);
    int pos = ByteArray_find_(BIVAR(self), BIVAR(other));
    
    if (pos != -1)
    {
		ByteArray *ba = ByteArray_newWithBytesFrom_to_(BIVAR(self), 0, pos);
		
		if (ISSYMBOL(self))
		{
			return IoState_symbolWithByteArray_copy_(IOSTATE, ba, 0);
		}
		else
		{
			return IoSeq_newWithByteArray_copy_(IOSTATE, ba, 0);
		}
    }
	
    if (ISSYMBOL(self)) 
    {
		return self;
    }
    
    return IOCLONE(self);
}

IoObject *IoSeq_afterSeq(IoSeq *self, IoObject *locals, IoMessage *m)
{
    /*#io
    docSlot("afterSeq(aSequence)", 
	"Returns the slice of the receiver (as a Symbol) 
	after aSequence or Nil if aSequence is not found. 
	If aSequence is empty, the receiver (or a copy of the 
	receiver if it is mutable) is returned.")
    */
    
    IoSeq *other = IoMessage_locals_seqArgAt_(m, locals, 0);
    int pos = ByteArray_find_(BIVAR(self), BIVAR(other));
    
    if (pos != -1)
    {
		ByteArray *ba = ByteArray_newWithBytesFrom_to_(BIVAR(self), 
													   pos + ByteArray_size(BIVAR(other)), 
													   ByteArray_size(BIVAR(self)));
		
		if (ISSYMBOL(self))
		{
			return IoState_symbolWithByteArray_copy_(IOSTATE, ba, 0);
		}
		else
		{
			return IoSeq_newWithByteArray_copy_(IOSTATE, ba, 0);
		}
    }
	
    return IONIL(self);
}

IoObject *IoSeq_asCapitalized(IoObject *self, IoObject *locals, IoMessage *m)
{ 
    /*#io
    docSlot("asCapitalized", 
			"Returns a copy of the receiver with the first charater made uppercase.")
    */
    
    /* need to fix for multi-byte characters */
    
    char firstchar = ByteArray_at_(BIVAR(self), 0);
    char upperchar = toupper(firstchar);
	
    if (ISSYMBOL(self) && (firstchar == upperchar)) 
    {
		return self;
    }
    else
    {
		ByteArray *ba = ByteArray_clone(BIVAR(self));
		ByteArray_at_put_(ba, 0, upperchar); 
		
		if (ISSYMBOL(self))
		{
			return IoState_symbolWithByteArray_copy_(IOSTATE, ba, 0);
		}
		
		return IoSeq_newWithByteArray_copy_(IOSTATE, ba, 0);
    }
}

IoObject *IoSeq_occurancesOfSeq(IoObject *self, IoObject *locals, IoMessage *m)
{ 
    /*#io
    docSlot("asCapitalized", 
			"Returns a copy of the receiver with the first charater made uppercase.")
    */
    
    /* need to fix for multi-byte characters */
    
    IoSeq *other = IoMessage_locals_seqArgAt_(m, locals, 0);
	
    size_t count = ByteArray_count_(BIVAR(self), BIVAR(other));
    return IONUMBER(count);
}

