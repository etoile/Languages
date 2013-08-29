// Below line is a work around for a LLVM bug.
#define __NO_TLS

#import <objc/objc-api.h>
#include <sys/types.h>
#include <string.h>
#include <stdint.h>
#ifdef BROKEN_CTYPE
/*
 * If we have a libc that provides a (w)ctype implementation that is known not
 * to work with llvm, we link libicu instead and redefine the isw* stuff to
 * libicu functions. The TestIsAlphabetic test case for small case will reveal
 * if this is needed.
 */
#include <unicode/uchar.h>
#define iswalnum(val) (u_isUAlphabetic(val) || u_isdigit(val))
#define iswalpha(val) u_isUAlphabetic(val)
#define iswdigit(val) u_isdigit(val)
#define iswspace(val) u_isUWhiteSpace(val)
#define iswupper(val) u_isUUppercase(val)
#define iswlower(val) u_isULowercase(val)
#else
#include <wctype.h>
#endif
typedef intptr_t NSInteger;
// Redefine a few things so LKObject will work correctly.
#define NSCAssert(x, msg) if (!(x)) { NSLog(msg); abort(); }
@class NSString;
__attribute__((noreturn)) void abort(void);
void NSLog(NSString*, ...);
@interface NSNumber
- (NSInteger)integerValue;
- (double)doubleValue;
- (float)floatValue;
@end
#include "LKObject.h"

// Dummy interfaces to make warnings go away
@interface BigInt {}
+ (id)bigIntWithLongLong:(long long)a;
- (LKObject)plus:(id)a;
- (LKObject)sub:(id)a;
- (LKObject)div:(id)a;
- (LKObject)mul:(id)a;
- (LKObject)mod:(id)a;
- (LKObject)min:(id)a;
- (LKObject)max:(id)a;
- (id)to:(id)a by:(id)b do:(id)c;
- (id)to:(id)a do:(id)c;
- (id)and: (id)a;
- (id)or: (id)a;
- (id)not;
- (LKObject)bitwiseAnd: (id)a;
- (LKObject)bitwiseOr: (id)a;
- (BOOL)isLessThan: (id)a;
- (BOOL)isGreaterThan: (id)a;
- (BOOL)isLessThanOrEqualTo: (id)a;
- (BOOL)isGreaterThanOrEqualTo: (id)a;
- (BOOL)isAlphanumeric;
- (BOOL)isUppercase;
- (BOOL)isLowercase;
- (BOOL)isDigit;
- (BOOL)isAlphabetic;
- (id)value;
@end
@interface NSString
{
	id isa;
}
+ (id) stringWithFormat:(NSString*)a, ...;
@end
@interface NSConstantString : NSString
{
	char *str;
	int length;
}
@end

typedef struct
{
	void* isa;
	int flags;
	int reserved;
	id(*invoke)(void*,...);
} Block;

/**
 * Preamble for a SmallInt message.  These are statically looked up and do not
 * have a selector argument.  Ideally, they are small enough to inline.
 * Replace : with _ in the selector name.
 */
#define MSG(retTy, name, ...) retTy SmallIntMsg ## name(void *obj, ## __VA_ARGS__)\
{\
	intptr_t val = (intptr_t)obj;\
	val >>= OBJC_SMALL_OBJECT_SHIFT;
/**
 * Small int message with no arguments.
 */
#define MSG0(name) MSG(void*, name)
/**
 * Small int message with one argument.
 */
#define MSG1(name) MSG(void*, name, void *other)\
	intptr_t otherval = (intptr_t)other;\
	otherval >>= OBJC_SMALL_OBJECT_SHIFT;

#ifndef OBJC_SMALL_OBJECT_MASK
MSG0(log)
	NSLog(@"%lld", (long long) ((intptr_t)obj >>OBJC_SMALL_OBJECT_SHIFT));
	return obj;
}
MSG0(retain)
	return obj;
}
MSG0(autorelease)
	return obj;
}
MSG0(release) }
NSString *SmallIntMsgstringValue_(void *obj)
{
	return [NSString stringWithFormat:@"%lld", (long long)(((intptr_t)obj)>>OBJC_SMALL_OBJECT_SHIFT)];
}
#endif

MSG1(ifTrue_)
	if (val == 0)
	{
		return 0;
	}
	else
	{
		Block *block = other;
		return block->invoke(block);
	}
}
MSG1(ifFalse_)
	if (val != 0)
	{
		return 0;
	}
	else
	{
		Block *block = other;
		return block->invoke(block);
	}
}
id SmallIntMsgifTrue_ifFalse_(void* obj, void *t, void *f)
{
	uintptr_t val = (uintptr_t)obj;
	val >>= OBJC_SMALL_OBJECT_SHIFT;
	if (val != 0)
	{
		Block *block = t;
		return block->invoke(block);
	}
	else
	{
		Block *block = f;
		return block->invoke(block);
	}
}
MSG1(timesRepeat_)
	Block *block = other;
	void *ret = NULL;
	for (intptr_t i=0 ; i<val ; i++)
	{
		ret = block->invoke(block);
	}
	return ret;
}
id SmallIntMsgto_by_do_(void* obj, void *to, void *by, void *tdo)
{
	intptr_t val = (intptr_t)obj;
	val >>= OBJC_SMALL_OBJECT_SHIFT;
	intptr_t inc = (intptr_t) by;
	intptr_t max = (intptr_t) to;
	if (((inc & OBJC_SMALL_OBJECT_MASK) == 0) || ((max & OBJC_SMALL_OBJECT_MASK) == 0))
	{
		BigInt* increment = (BigInt*) by;
		BigInt* maximum = (BigInt*) to;
		if ((inc & OBJC_SMALL_OBJECT_MASK) != 0)
		{
			inc >>= OBJC_SMALL_OBJECT_SHIFT;
			increment = [BigInt bigIntWithLongLong: (long long)inc];
		}
		if ((max & OBJC_SMALL_OBJECT_MASK) != 0)
		{
			max >>= OBJC_SMALL_OBJECT_SHIFT;
			maximum = [BigInt bigIntWithLongLong: (long long)max];
		}
		BigInt* conv = [BigInt bigIntWithLongLong: (long long)val];
		return [conv to: maximum by: increment do: tdo];
	}
	inc >>= OBJC_SMALL_OBJECT_SHIFT;
	max >>= OBJC_SMALL_OBJECT_SHIFT;

	id result = nil;
	for (;val<=max;val+=inc)
	{
		Block *block = tdo;
		result = block->invoke(block, [BigInt bigIntWithLongLong:(long long)val]);
	}
	return result;
}
id SmallIntMsgto_do_(void* obj, void *to, void *tdo)
{
	return SmallIntMsgto_by_do_(obj, to, (void*)((1<<OBJC_SMALL_OBJECT_SHIFT) | 1), tdo);
}


BOOL SmallIntMsgisEqual_(void *obj, void *other)
{
	if (obj == other)
	{
		return YES;
	}
	return NO;
}

#define IS_OBJECT_POINTER(x) ((((intptr_t)x) & OBJC_SMALL_OBJECT_MASK) == 0)

#define BOX_AND_RETRY(op) [[BigInt bigIntWithLongLong:(long long)val] \
                    op:[BigInt bigIntWithLongLong:(long long)otherval]]

#define OTHER_OBJECT_CAST(op) \
	if (IS_OBJECT_POINTER(other))\
	{\
		intptr_t val = (intptr_t)obj >> OBJC_SMALL_OBJECT_SHIFT;\
		LKObject ret = \
			[[BigInt bigIntWithLongLong:(long long)val] op:other];\
		return *(void**)&ret;\
	}
#define OTHER_OBJECT(op) \
	if (IS_OBJECT_POINTER(other))\
	{\
		intptr_t val = (intptr_t)obj >> OBJC_SMALL_OBJECT_SHIFT;\
		return [[BigInt bigIntWithLongLong:(long long)val] op:other];\
	}
#define RETURN_INT(x)     if(((x) << OBJC_SMALL_OBJECT_SHIFT >> OBJC_SMALL_OBJECT_SHIFT) != (x)) \
    {\
	  return [BigInt bigIntWithLongLong:(long long)(x)];	\
	}\
  return (void*)(((x) << OBJC_SMALL_OBJECT_SHIFT) | (uintptr_t)1);

void *SmallIntMsgplus_(void *obj, void *other)
{
	OTHER_OBJECT_CAST(plus);
	// Clear the low bit on obj
	intptr_t val = ((intptr_t)other) & ~ OBJC_SMALL_OBJECT_MASK;
	// Add the two values together.  This will cause the overflow handler to be
	// invoked in case of overflow, otherwise it will contain the correct
	// result.
	return (void*)((intptr_t)obj + val);
}
void *SmallIntMsgsub_(void *obj, void *other)
{
	OTHER_OBJECT_CAST(sub);
	// Clear the low bit and invert the sign bit on other
	intptr_t val = -(((intptr_t)other) & ~ OBJC_SMALL_OBJECT_MASK);
	// Add the two values together.  This will cause the overflow handler to be
	// invoked in case of overflow, otherwise it will contain the correct
	// result.
	return (void*)((intptr_t)obj + val);
}
void *SmallIntMsgmul_(void *obj, void *other)
{
	OTHER_OBJECT_CAST(mul)
	// Clear the low bit on obj
	intptr_t val = ((intptr_t)obj) & ~ OBJC_SMALL_OBJECT_MASK;
	// Turn other into a C integer
	intptr_t otherval = ((intptr_t)other) >> OBJC_SMALL_OBJECT_SHIFT;
	// val * otherval will be the correct SmallInt value with the low bit
	// cleared.  This sets the low bit in this case.  To avoid an extra test in
	// case of overflow, we cheat a bit here and set the low bit spuriously
	// when returning a big integer.  This then clears that bit.
	return (void*)((val * otherval) ^ 1);
}
void *SmallIntMsgmin_(void *obj, void *other)
{
    OTHER_OBJECT_CAST(min)

	if (obj <= other)
	  return obj;
	else
	  return other;
}
void *SmallIntMsgmax_(void *obj, void *other)
{
    OTHER_OBJECT_CAST(max)

	if (obj <= other)
	  return other;
	else
	  return obj;
}

MSG1(div_)
	OTHER_OBJECT_CAST(div)
	RETURN_INT((val / otherval));
}
MSG1(mod_)
	OTHER_OBJECT_CAST(mod)
	RETURN_INT((val % otherval));
}
MSG1(bitwiseAnd_)
	OTHER_OBJECT_CAST(bitwiseAnd)
	RETURN_INT((val & otherval));
}
MSG1(bitwiseOr_)
	OTHER_OBJECT_CAST(bitwiseOr)
	RETURN_INT((val | otherval));
}

#define BOOLMSG0(name) MSG(BOOL, name)
#define BOOLMSG1(name) MSG(BOOL, name, void *other)\
	intptr_t otherval = (intptr_t)other;\
	otherval >>= OBJC_SMALL_OBJECT_SHIFT;

MSG1(and_)
	OTHER_OBJECT(and)
	RETURN_INT(val && otherval);
}
MSG1(or_)
	OTHER_OBJECT(or)
	RETURN_INT(val || otherval);
}
MSG0(not)
	RETURN_INT(!val);
}
BOOLMSG0(isAlphanumeric)
	return iswalnum(val) != 0;
}
BOOLMSG0(isUppercase)
	return iswupper(val) != 0;
}
BOOLMSG0(isLowercase)
	return iswlower(val) != 0;
}
BOOLMSG0(isDigit)
	return iswdigit(val) != 0;
}
BOOLMSG0(isAlphabetic)
	return iswalpha(val) != 0;
}
BOOLMSG0(isWhitespace)
	return iswspace(val) != 0;
}
MSG0(value)
	return obj;
}

#define COMPARE(msg, op) \
BOOL SmallIntMsg ## msg ## _(void *obj, void *other) \
{ \
	OTHER_OBJECT(msg) \
	intptr_t val = (intptr_t)obj; \
	val >>= OBJC_SMALL_OBJECT_SHIFT; \
	intptr_t otherval = (intptr_t)other; \
	otherval >>= OBJC_SMALL_OBJECT_SHIFT; \
	return val op otherval; \
}
COMPARE(isLessThan, <)
COMPARE(isGreaterThan, >)
COMPARE(isLessThanOrEqualTo, <=)
COMPARE(isGreaterThanOrEqualTo, >=)

// If we're building the version that's linked into the run-time support
// library, then also compile these functions as real methods.
#ifdef STATIC_COMPILE
@interface NSSmallInt @end
@implementation NSSmallInt (LanguageKit)
#define BOOLMETHOD0(method) \
	- (BOOL)method\
	{\
		return SmallIntMsg ## method(self);\
	}
#define METHOD0(method) \
	- (id)method\
	{\
		return SmallIntMsg ## method(self);\
	}
#define BOOLMETHOD1(method) \
	- (BOOL)method: (id)other\
	{\
		return SmallIntMsg ## method ## _(self, other);\
	}
#define METHOD1(method) \
	- (id)method: (id)other\
	{\
		return SmallIntMsg ## method ## _(self, other);\
	}
METHOD1(plus)
METHOD1(sub)
METHOD1(mul)
METHOD1(div)
METHOD1(min)
METHOD1(max)
METHOD1(timesRepeat)
BOOLMETHOD1(isLessThan)
BOOLMETHOD1(isGreaterThan)
BOOLMETHOD1(isGreaterThanOrEqualTo)
BOOLMETHOD1(isLessThanOrEqualTo)
METHOD1(mod)
METHOD1(bitwiseAnd)
METHOD1(bitwiseOr)
METHOD1(and)
METHOD1(or)
METHOD0(not)
BOOLMETHOD0(isAlphanumeric)
BOOLMETHOD0(isUppercase)
BOOLMETHOD0(isLowercase)
BOOLMETHOD0(isDigit)
BOOLMETHOD0(isAlphabetic)
BOOLMETHOD0(isWhitespace)
METHOD0(value)

@end
#endif

void *MakeSmallInt(long long val) {
	//fprintf(stderr, "Trying to make %lld into a small int\n", val);
	intptr_t ptr = val << OBJC_SMALL_OBJECT_SHIFT;
	//fprintf(stderr, "Failing if it is not %lld \n", (long long)(ptr >> 1));
	if (((ptr >> OBJC_SMALL_OBJECT_SHIFT)) != val) {
		return [BigInt bigIntWithLongLong:val];
	}
	return (void*)(ptr | 1);
}

void *BoxSmallInt(void *obj) {
	if (obj == NULL) return NULL;
	intptr_t val = (intptr_t)obj;
	val >>= OBJC_SMALL_OBJECT_SHIFT;
	//fprintf(stderr, "Boxing %d\n", (int) val);
	return [BigInt bigIntWithLongLong:(long long)val];
}
void *BoxObject(void *obj) {
	intptr_t val = (intptr_t)obj;
	if (val == 0 || (val & OBJC_SMALL_OBJECT_MASK) == 0) {
		return obj;
	}
	val >>= OBJC_SMALL_OBJECT_SHIFT;
	return [BigInt bigIntWithLongLong:(long long)val];
}

#define CAST(x) NCAST(x,x)
#define CASTMSG(type, name) type SmallIntMsg##name##Value(void *obj) { return (type) ((intptr_t)obj>>OBJC_SMALL_OBJECT_SHIFT); }

CASTMSG(char, char)
CASTMSG(unsigned char, unsignedChar)
CASTMSG(short, short)
CASTMSG(unsigned short, unsignedShort)
CASTMSG(int, int)
CASTMSG(unsigned int, unsignedInt)
CASTMSG(long, long)
CASTMSG(unsigned long, unsignedLong)
CASTMSG(long long, longLong)
CASTMSG(unsigned long long, unsignedLongLong)
CASTMSG(BOOL, bool)

/* We currently only use the fast path for small ints, not doubles, so skip this.
 */
#if 3 == OBJC_SMALL_OBJECT_SHIFT
/*
 * On 64bit platforms, gnustep-base provides a special case for small doubles or
 * floats, so we redefine CASTMSG to unbox them correctly.
 */

union BoxedDouble
{
	uintptr_t bits;
	double d;
};

static inline double unboxExtendedDouble(uintptr_t boxed)
{
	uintptr_t mask = boxed & 8;
	union BoxedDouble ret;
	boxed &= ~7;
	ret.bits = boxed | (mask >> 1) | (mask >> 2) | (mask >> 3);
	return ret.d;
}

static inline double unboxRepeatingDouble(uintptr_t boxed)
{
	uintptr_t mask = boxed & 56;
	union BoxedDouble ret;
	boxed &= ~7;
	ret.bits = boxed |  (mask >> 3);
	return ret.d;
}
#undef CASTMSG
#define CASTMSG(type, name) type SmallintMsg##name##Value(void *obj) {\
	uintptr_t smallClass = (((uintptr_t)obj) & OBJC_SMALL_OBJECT_MASK);\
	if (2 == smallClass)\
	{\
		return (type) unboxExtendedDouble((uintptr_t)obj);\
	}\
	else if ((3 == smallClass) || (5 == smallClass))\
	{\
		return (type) unboxRepeatingDouble((uintptr_t)obj);\
	}\
	return [(NSNumber*)obj name##Value];\
}
#endif
CASTMSG(float, float)
CASTMSG(double, double)

enum
{
	BLOCK_FIELD_IS_OBJECT   =  3,
	BLOCK_FIELD_IS_BLOCK    =  7,
	BLOCK_FIELD_IS_BYREF    =  8,
	BLOCK_FIELD_IS_WEAK  = 16,
	BLOCK_BYREF_CALLER    = 128,
};


void _Block_object_assign(void *destAddr, const void *object, const int flags);
void _Block_object_dispose(const void *object, const int flags);

struct _block_byref_object
{
	void *isa;
	struct _block_byref_voidBlock *forwarding;
	int flags;
	int size;
	void (*byref_keep)(struct _block_byref_object*, struct _block_byref_object*);
	void (*byref_dispose)(struct _block_byref_object *);
	void *captured;
};


// Helper functions called by the block byref copy /destroy functions.

void LKByRefKeep(struct _block_byref_object *dst, struct _block_byref_object*src)
{
	dst->captured = src->captured;
	if ((((intptr_t)dst->captured) & OBJC_SMALL_OBJECT_MASK) == 0)
	{
		objc_retain(dst->captured);
	}
}

void objc_release(id);

void LKByRefDispose(struct _block_byref_object*src)
{
	if ((((intptr_t)src) & OBJC_SMALL_OBJECT_MASK) != 0)
	{
		objc_release(src->captured);
	}
}
