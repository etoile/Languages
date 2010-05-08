#import <objc/objc-api.h>
#include <sys/types.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>

// Dummy interfaces to make warnings go away
@interface BigInt {}
+ (id)bigIntWithLongLong:(long long)a;
- (id)plus:(id)a;
- (id)sub:(id)a;
- (id)div:(id)a;
- (id)mul:(id)a;
- (id)mod:(id)a;
- (id)to:(id)a by:(id)b do:(id)c;
- (id)to:(id)a do:(id)c;
- (id)and: (id)a;
- (id)or: (id)a;
- (id)bitwiseAnd: (id)a;
- (id)bitwiseOr: (id)a;
- (id)not;
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
void NSLog(NSString*, ...);

typedef struct
{
	void* isa;
	id(*value)(void*, SEL,...);
} Block;

/**
 * Preamble for a SmallInt message.  These are statically looked up and do not
 * have a selector argument.  Ideally, they are small enough to inline.
 * Replace : with _ in the selector name.
 */
#define MSG(retTy, name, ...) retTy SmallIntMsg ## name(void *obj, ## __VA_ARGS__)\
{\
	intptr_t val = (intptr_t)obj;\
	val >>= 1;
/**
 * Small int message with no arguments.
 */
#define MSG0(name) MSG(void*, name)
/**
 * Small int message with one argument.
 */
#define MSG1(name) MSG(void*, name, void *other)\
	intptr_t otherval = (intptr_t)other;\
	otherval >>= 1;

MSG0(log)
	NSLog(@"%lld", (long long) ((intptr_t)obj >>1));
	return obj;
}

MSG1(ifTrue_)
	if (val == 0)
	{
		return 0;
	}
	else
	{
		Block *block = other;
		return block->value(other, @selector(value));
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
		return block->value(other, @selector(value));
	}
}
id SmallIntMsgifTrue_ifFalse_(void* obj, void *t, void *f)
{
	uintptr_t val = (uintptr_t)obj;
	val >>= 1;
	if (val != 0)
	{
		Block *block = t;
		return block->value(t, @selector(value));
	}
	else
	{
		Block *block = f;
		return block->value(f, @selector(value));
	}
}
MSG1(timesRepeat_)
	Block *block = other;
	void *ret = NULL;
	for (intptr_t i=0 ; i<val ; i++)
	{
		ret = block->value(other, @selector(value));
	}
	return ret;
}
id SmallIntMsgto_by_do_(void* obj, void *to, void *by, void *tdo)
{
	intptr_t val = (intptr_t)obj;
	val >>= 1;
	intptr_t inc = (intptr_t) by;
	intptr_t max = (intptr_t) to;
	if (((inc & 1) == 0) || ((max & 1) == 0))
	{
		BigInt* increment = (BigInt*) by;
		BigInt* maximum = (BigInt*) to;
		if ((inc & 1) != 0)
		{
			inc >>= 1;
			increment = [BigInt bigIntWithLongLong: (long long)inc];	
		}
		if ((max & 1) != 0)
		{
			max >>= 1;
			maximum = [BigInt bigIntWithLongLong: (long long)max];	
		}
		BigInt* conv = [BigInt bigIntWithLongLong: (long long)val];
		return [conv to: maximum by: increment do: tdo];
	}
	inc >>= 1;
	max >>= 1;
	
	id result = nil;
	for (;val<=max;val+=inc)
	{
		Block *block = tdo;
		result = block->value(tdo, @selector(value:), [BigInt bigIntWithLongLong:(long long)val]);
	}
	return result;
}
id SmallIntMsgto_do_(void* obj, void *to, void *tdo)
{
	// increment by one -- ((1 << 1) & 1) == 3
	return SmallIntMsgto_by_do_(obj, to, (void*)3, tdo);
}


BOOL SmallIntMsgisEqual_(void *obj, void *other)
{
	if (obj == other)
	{
		return YES;
	}
	return NO;
}

#define BOX_AND_RETRY(op) [[BigInt bigIntWithLongLong:(long long)val] \
                    op:[BigInt bigIntWithLongLong:(long long)otherval]]

#define OTHER_OBJECT(op) \
	if ((((intptr_t)other) & 1) == 0)\
	{\
		intptr_t val = (intptr_t)obj >> 1;\
		return [[BigInt bigIntWithLongLong:(long long)val] op:other];\
	}
#define RETURN_INT(x)     if((x << 1 >> 1) != x)\
    {\
		return [BigInt bigIntWithLongLong:(long long)x];\
	}\
	return (void*)((x << 1) | 1);

void *SmallIntMsgplus_(void *obj, void *other)
{
	OTHER_OBJECT(plus);
	// Clear the low bit on obj
	intptr_t val = ((intptr_t)other) & ~ 1;
	// Add the two values together.  This will cause the overflow handler to be
	// invoked in case of overflow, otherwise it will contain the correct
	// result.
	return (void*)((intptr_t)obj + val);
}
void *SmallIntMsgsub_(void *obj, void *other)
{
	OTHER_OBJECT(sub);
	// Clear the low bit and invert the sign bit on other 
	intptr_t val = -(((intptr_t)other) & ~1);
	// Add the two values together.  This will cause the overflow handler to be
	// invoked in case of overflow, otherwise it will contain the correct
	// result.
	return (void*)((intptr_t)obj + val);
}
void *SmallIntMsgmul_(void *obj, void *other)
{
	OTHER_OBJECT(mul)
	// Clear the low bit on obj
	intptr_t val = ((intptr_t)obj) & ~ 1;
	// Turn other into a C integer
	intptr_t otherval = ((intptr_t)other) >> 1;
	// val * otherval will be the correct SmallInt value with the low bit
	// cleared.  This sets the low bit in this case.  To avoid an extra test in
	// case of overflow, we cheat a bit here and set the low bit spuriously
	// when returning a big integer.  This then clears that bit.  
	return (void*)((val * otherval) ^ 1);
}
MSG1(div_)
	OTHER_OBJECT(div)
	RETURN_INT((val / otherval));
}
MSG1(mod_)
	OTHER_OBJECT(mod)
	RETURN_INT((val % otherval));
}
MSG1(bitwiseAnd_)
	OTHER_OBJECT(bitwiseAnd)
	RETURN_INT((val & otherval));
}
MSG1(bitwiseOr_)
	OTHER_OBJECT(bitwiseOr)
	RETURN_INT((val | otherval));
}

#define BOOLMSG0(name) MSG(BOOL, name)
#define BOOLMSG1(name) MSG(BOOL, name, void *other)\
	intptr_t otherval = (intptr_t)other;\
	otherval >>= 1;

MSG1(and_)
	OTHER_OBJECT(and)
	RETURN_INT(val && otherval);
}
MSG1(or)
	OTHER_OBJECT(or)
	RETURN_INT(val || otherval);
}
MSG0(not)
	RETURN_INT(!val);
}
BOOLMSG0(isAlphanumeric)
	return isalpha(val) || isdigit(val);
}
BOOLMSG0(isUppercase)
	return isupper(val);
}
BOOLMSG0(isLowercase)
	return islower(val);
}
BOOLMSG0(isDigit)
	return isdigit(val);
}
BOOLMSG0(isAlphabetic)
	return isalpha(val);
}
BOOLMSG0(isWhitespace)
	return isspace(val);
}
MSG0(value)
	return obj;
}

#define COMPARE(msg, op) \
BOOL SmallIntMsg ## msg ## _(void *obj, void *other) \
{ \
	OTHER_OBJECT(msg) \
	intptr_t val = (intptr_t)obj; \
	val >>= 1; \
	intptr_t otherval = (intptr_t)other; \
	otherval >>= 1; \
	return val op otherval; \
}
COMPARE(isLessThan, <)
COMPARE(isGreaterThan, >)
COMPARE(isLessThanOrEqualTo, <=)
COMPARE(isGreaterThanOrEqualTo, >=)

void *MakeSmallInt(long long val) {
	//fprintf(stderr, "Trying to make %lld into a small int\n", val);
	intptr_t ptr = val << 1;
	//fprintf(stderr, "Failing if it is not %lld \n", (long long)(ptr >> 1));
	if (((ptr >> 1)) != val) {
		return [BigInt bigIntWithLongLong:val];
	}
	return (void*)(ptr | 1);
}

void *BoxSmallInt(void *obj) {
	if (obj == NULL) return NULL;
	intptr_t val = (intptr_t)obj;
	val >>= 1;
	//fprintf(stderr, "Boxing %d\n", (int) val);
	return [BigInt bigIntWithLongLong:(long long)val];
}
void *BoxObject(void *obj) {
	intptr_t val = (intptr_t)obj;
	if (val == 0 || (val & 1) == 0) {
		return obj;
	}
	val >>= 1;
	return [BigInt bigIntWithLongLong:(long long)val];
}

#define CAST(x) NCAST(x,x)
#define CASTMSG(type, name) type SmallIntMsg##name##Value(void *obj) { return (type) ((intptr_t)obj>>1); }

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
CASTMSG(float, float)
CASTMSG(double, double)
NSString *SmallIntMsgstringValue_(void *obj)
{
	return [NSString stringWithFormat:@"%lld", (long long)(((intptr_t)obj)>>1)];
}
