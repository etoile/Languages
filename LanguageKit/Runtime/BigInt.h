#import <EtoileFoundation/EtoileFoundation.h>
#import "LKObject.h"
#include <gmp.h>

@interface BigInt : NSNumber {
@public
  /**
   * Value for this object.  Public so it can be accessed from others to
   * slightly lower the cost of operations on BigInts.
   */
	mpz_t v;
}
+ (BigInt*) bigIntWithCString:(const char*) aString;
+ (BigInt*) bigIntWithLongLong:(long long)aVal;
+ (BigInt*) bigIntWithLong:(long)aVal;
+ (BigInt*) bigIntWithUnsignedLong:(unsigned long)aVal;
+ (BigInt*) bigIntWithMP:(mpz_t)aVal;
@end
#ifndef OBJC_SMALL_OBJECT_SHIFT
#define OBJC_SMALL_OBJECT_SHIFT ((sizeof(id) == 4) ? 1 : 3)
#endif

static inline LKObject LKObjectFromNSInteger(NSInteger integer)
{
	if((integer << 1 >> 1) != integer)
	{
		return LKObjectFromObject([BigInt bigIntWithLongLong: (long long)integer]);
	}
	else
	{
		return LKObjectFromObject((__bridge id)((integer << OBJC_SMALL_OBJECT_SHIFT) | 1));
	}
}
