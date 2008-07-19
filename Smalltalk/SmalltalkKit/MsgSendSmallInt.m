#import <objc/objc-api.h>
#include <sys/types.h>
#include <string.h>

// Dummy interfaces to make warnings go away
@interface BigInt {}
+ (id) bigIntWithLongLong:(long long)a;
- (id) add:(id)a;
- (id) sub:(id)a;
- (id) div:(id)a;
- (id) mul:(id)a;
@end
@interface NSString {}
+ (id) stringWithFormat:(NSString*)a, ...;
@end
NSLog(NSString*, ...);

typedef struct
{
	void* isa;
	id(*value)(void*, SEL);
} Block;

/**
 * Preamble for a SmallInt message.  These are statically looked up and do not
 * have a selector argument.  Ideally, they are small enough to inline.
 * Replace : with _ in the selector name.
 */
#define MSG(name, ...) void *SmallIntMsg ## name(void *obj, ## __VA_ARGS__)\
{\
	intptr_t val = (intptr_t)obj;\
	val >>= 1;
/**
 * Small int message with no arguments.
 */
#define MSG0(name) MSG(name)
/**
 * Small int message with one argument.
 */
#define MSG1(name) MSG(name, void *other)\
	long otherval = (intptr_t)other;\
	otherval >>= 1;

MSG0(log)
	NSLog(@"%lld", (long long) ((intptr_t)obj >>1));
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
MSG1(timesRepeat_)
	Block *block = other;
	for (intptr_t i=0 ; i<val ; i++)
	{
		block->value(other, @selector(value));
	}
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

#define RETURN_INT(x)     if((x << 1 >> 1) != x)\
    {\
		return [BigInt bigIntWithLongLong:(long long)x];\
	}\
	return (void*)((x << 1) | 1);
				
// FIXME: These only work currently on SmallInt args
MSG1(add_)
	intptr_t res = val + otherval;
	//fprintf(stderr, "Adding %d to %d\n", (int)val, (int)otherval);
	if((val<=res)==((((uintptr_t)otherval)>>((sizeof(uintptr_t)*8) - 1))))
	{
		//fprintf(stderr, "Add overflowed - promoting.\n");
		BOX_AND_RETRY(add);
	}
	RETURN_INT(res);
}

MSG1(sub_)
	intptr_t res = val - otherval;
	// Check for overflow
	if ((val & ~otherval & ~res) | (~val & otherval & res) < 0)
	{
		BOX_AND_RETRY(sub);
	}
	RETURN_INT(res);
}
MSG1(mul_)
	// FIXME: Far too conservative - replace with something more efficient.
	if (val != (short)val || otherval != (short) otherval)
	{
		return BOX_AND_RETRY(mul);
	}
	val *= otherval;
	RETURN_INT(val);
}
MSG1(div_)
	return ((val / otherval) << 1) | 1;
}
MSG1(mod_)
	return ((val % otherval) << 1) | 1;
}

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
