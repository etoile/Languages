/*#io
docCopyright("Steve Dekorte, 2002. All rights reserved.")
docLicense("BSD revised") 
docDescription("A mutable array of unsigned chars supports basic operations, searching and reading/writing to a file")
*/
 

#ifndef BYTEARRAY_DEFINED
#define BYTEARRAY_DEFINED 1

#include "Common.h"
#include <stdarg.h>
#include <stdio.h>
#include <stddef.h>
#include "List.h"

// internally, Io always uses a forward slash "/" for path separators,
// but on Windows, back slashes are also tolerated as path separators.
#if defined(DOS) || defined(ON_WINDOWS)
#define OS_PATH_SEPARATOR "\\"
#define IO_PATH_SEPARATORS "\\/"
#else
#define OS_PATH_SEPARATOR "/"
#define IO_PATH_SEPARATORS "/"
#endif

#define IO_PATH_SEPARATOR   "/"
#define IO_PATH_SEPARATOR_DOT   IO_PATH_SEPARATOR "." IO_PATH_SEPARATOR

#ifdef __cplusplus
extern "C" {
#endif

#define BYTEARRAY_BYTES_PER_CHARACTER 1

typedef struct
{
    unsigned char *bytes;
    size_t size;
    size_t memSize;
    uint8_t encoding;
} ByteArray ;

ByteArray *ByteArray_new(void);
ByteArray *ByteArray_newWithData_size_copy_(unsigned char *buf, size_t size, char copy);
ByteArray *ByteArray_newWithCString_size_(const char *s, int size);
ByteArray *ByteArray_newWithCString_(const char *s);
ByteArray *ByteArray_newWithSize_(int size);
ByteArray *ByteArray_newWithBytesFrom_to_(ByteArray *self, int startpos, int endpos);
ByteArray *ByteArray_clone(ByteArray *self);

#include "Datum.h"
Datum ByteArray_asDatum(ByteArray *self);
Datum *ByteArray_asNewDatum(ByteArray *self);

void ByteArray_free(ByteArray *self);
size_t ByteArray_memorySize(ByteArray *self);
void ByteArray_compact(ByteArray *self);

void ByteArray_clear(ByteArray *self);
void ByteArray_setAllBytesTo_(ByteArray *self, unsigned char c);
void ByteArray_sizeTo_(ByteArray *self, size_t size);
void ByteArray_setSize_(ByteArray *self, size_t size);
void ByteArray_copy_(ByteArray *self, ByteArray *other);
void ByteArray_setData_size_(ByteArray *self, const unsigned char *data, size_t size);
void ByteArray_setCString_(ByteArray *self, const char *s);

/*
int ByteArray_compare_(ByteArray *self, ByteArray *other);
int ByteArray_compareData_size_(ByteArray *self, unsigned char *b2, unsigned int l2);
int ByteArray_compareDatum_(ByteArray *self, Datum d);
*/

// character ops 

int ByteArray_hasDigit(ByteArray *self);
unsigned long ByteArray_at_bytesCount_(ByteArray *self, int i, int l);
int ByteArray_at_put_(ByteArray *self, int pos, unsigned char c);
unsigned char ByteArray_dropLastByte(ByteArray *self);
void ByteArray_removeByteAt_(ByteArray *self, int pos);
void ByteArray_removeCharAt_(ByteArray *self, int pos);
void ByteArray_removeSlice(ByteArray *self, int from, int to);

// escape & quote 

void ByteArray_escape(ByteArray *self);
void ByteArray_unescape(ByteArray *self);

void ByteArray_quote(ByteArray *self);
void ByteArray_unquote(ByteArray *self);

// modification ops 

void ByteArray_appendChar_(ByteArray *self, char c);
void ByteArray_appendByte_(ByteArray *self, unsigned char c);
void ByteArray_append_(ByteArray *self, ByteArray *other);
void ByteArray_appendCString_(ByteArray *self, const char *s);
void ByteArray_appendAndEscapeCString_(ByteArray *self, const char *s);
void ByteArray_appendBytes_size_(ByteArray *self, const unsigned char *bytes, size_t length);

void ByteArray_prepend_(ByteArray *self, ByteArray *other);
void ByteArray_prependCString_(ByteArray *self, const char *s);
void ByteArray_prependBytes_size_(ByteArray *self, const unsigned char *bytes, size_t length);

void ByteArray_insert_at_(ByteArray *self, ByteArray *other, size_t pos);
void ByteArray_insertCString_at_(ByteArray *self, const char *s, size_t pos);
void ByteArray_insertBytes_size_at_(ByteArray *self, const unsigned char *bytes, size_t size, size_t pos);

// clipping 

char ByteArray_clipBefore_(ByteArray *self, ByteArray *other);
char ByteArray_clipBeforeEndOf_(ByteArray *self, ByteArray *other);
char ByteArray_clipAfter_(ByteArray *self, ByteArray *other);
char ByteArray_clipAfterStartOf_(ByteArray *self, ByteArray *other);

// strip 

int ByteArray_containsByte_(ByteArray *self, unsigned char b);
void ByteArray_strip_(ByteArray *self, ByteArray *other);
void ByteArray_lstrip_(ByteArray *self, ByteArray *other);
void ByteArray_rstrip_(ByteArray *self, ByteArray *other);

// enumeration 

// case

int ByteArray_isLowercase(ByteArray *self);
int ByteArray_isUppercase(ByteArray *self);
void ByteArray_Lowercase(ByteArray *self);
void ByteArray_uppercase(ByteArray *self);

// comparision 

int ByteArray_equals_(ByteArray *self, ByteArray *other);
int ByteArray_equalsAnyCase_(ByteArray *self, ByteArray *other);

// search 

int ByteArray_contains_(ByteArray *self, ByteArray *other);
int ByteArray_containsAnyCase_(ByteArray *self, ByteArray *other);
int ByteArray_find_(ByteArray *self, ByteArray *other);
int ByteArray_beginsWith_(ByteArray *self, ByteArray *other);
int ByteArray_endsWith_(ByteArray *self, ByteArray *other);

int ByteArray_findAnyCase_(ByteArray *self, ByteArray *other);
int ByteArray_find_from_(ByteArray *self, ByteArray *other, int from);
int ByteArray_findCString_from_(ByteArray *self, const char *other, int from);
int ByteArray_rFind_from_(ByteArray *self, ByteArray *other, int from);
int ByteArray_rFindCharacters_from_(ByteArray *self, const char *chars, int from);
int ByteArray_findAnyCase_from_(ByteArray *self, ByteArray *other, int from);
size_t ByteArray_count_(ByteArray *self, ByteArray *other);

int ByteArray_findByteWithValue_from_(ByteArray *self, unsigned char b, int from);
int ByteArray_findByteWithoutValue_from_(ByteArray *self, unsigned char b, int from);

void ByteArray_setByteWithValue_from_to_(ByteArray *self, unsigned char b, size_t from, size_t to);

// replace

void ByteArray_replaceCString_withCString_(ByteArray *self, const char *s1, const char *s2);
void ByteArray_replaceFrom_size_with_(ByteArray *self, 
    size_t index, 
    size_t substringSize, 
    ByteArray *other);
size_t ByteArray_replace_with_(ByteArray *self, ByteArray *substring, ByteArray *other);
size_t ByteArray_replaceFirst_from_with_(ByteArray *self, ByteArray *substring, size_t start, ByteArray *other);
/* for effiency, replacement is done without allocating a seperate byte array */
void ByteArray_replace_with_output_(ByteArray *self, ByteArray *substring, ByteArray *other, ByteArray *output);

// input/output

void ByteArray_print(ByteArray *self);
size_t ByteArray_writeToCStream_(ByteArray *self, FILE *stream);
int ByteArray_writeToFilePath_(ByteArray *self, const char *path); /* returns 0 on success */

int ByteArray_readFromFilePath_(ByteArray *self, const char *path); /* returns 0 on success */
unsigned char ByteArray_readLineFromCStream_(ByteArray *self, FILE *stream);
int ByteArray_readFromCStream_(ByteArray *self, FILE *stream);
size_t ByteArray_readNumberOfBytes_fromCStream_(ByteArray *self, long length, FILE *stream);

// private utility functions 

int ByteArray_wrapPos_(ByteArray *self, int pos);

ByteArray *ByteArray_newWithFormat_(const char *format, ...);
ByteArray *ByteArray_newWithVargs_(const char *format, va_list ap);
ByteArray *ByteArray_fromFormat_(ByteArray *self, const char *format, ...);
void ByteArray_fromVargs_(ByteArray *self, const char *format, va_list ap);
 
// hex conversion

ByteArray *ByteArray_asNewHexStringByteArray(ByteArray *self);

// file paths

void ByteArray_appendPathCString_(ByteArray *self, const char *path);
void ByteArray_removeLastPathComponent(ByteArray *self);
void ByteArray_clipBeforeLastPathComponent(ByteArray *self);
ByteArray *ByteArray_lastPathComponent(ByteArray *self);
char *ByteArray_lastPathComponentAsCString(ByteArray *self);
void ByteArray_removePathExtension(ByteArray *self);
ByteArray *ByteArray_pathExtension(ByteArray *self);
ByteArray *ByteArray_fileName(ByteArray *self);

// bitwise ops

void ByteArray_and_(ByteArray *self, ByteArray *other);
void ByteArray_or_(ByteArray *self, ByteArray *other);
void ByteArray_xor_(ByteArray *self, ByteArray *other);
void ByteArray_compliment(ByteArray *self);
void ByteArray_byteShiftLeft_(ByteArray *self, int s);
void ByteArray_byteShiftRight_(ByteArray *self, int s);
void ByteArray_bitShiftLeft_(ByteArray *self, int s);

unsigned char ByteArray_byteAt_(ByteArray *self, size_t i);
int ByteArray_bitAt_(ByteArray *self, size_t i);
void ByteArray_setBit_at_(ByteArray *self, int b, size_t i);
void ByteArray_printBits(ByteArray *self);
unsigned int ByteArray_bitCount(ByteArray *self);

// typed array ops

float ByteArray_aveAbsFloat32From_to_(ByteArray *self, size_t from, size_t to);
int ByteArray_aveAbsSignedInt32From_to_(ByteArray *self, size_t from, size_t to);
void ByteArray_convertFloat32ArrayToInt32(ByteArray *self);
void ByteArray_convertFloat32ArrayToInt16(ByteArray *self);
void ByteArray_convertInt16ArrayToFloat32(ByteArray *self);

// split

int ByteArray_splitCount_(ByteArray *self, List *others);
/*int ByteArray_nextChunkUsingStopList_from_(ByteArray *self, List *others, size_t startIndex);*/
List *ByteArray_split_(ByteArray *self, List *others);

/*
unsigned int ByteArray_HashData_size_(const unsigned char *bytes, unsigned int length);
uint32_t ByteArray_orderedHash32(ByteArray *self);
*/

// indexwise ops

void ByteArray_removeOddIndexesOfSize_(ByteArray *self, size_t typeSize);
void ByteArray_removeEvenIndexesOfSize_(ByteArray *self, size_t typeSize);
void ByteArray_duplicateIndexesOfSize_(ByteArray *self, size_t typeSize);

int ByteArray_endsWithCString_(ByteArray *self, const char *suffix);

size_t ByteArray_matchingPrefixSizeWith_(ByteArray *self, ByteArray *other);

#include "ByteArray_inline.h"

#ifdef __cplusplus
}
#endif
#endif

