#import <EtoileFoundation/EtoileFoundation.h>
#import "LKObject.h"
#include <gmp.h>

@interface BigInt : NSObject {
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

static inline LKObject LKObjectFromNSInteger(NSInteger integer)
{
	LKObject obj;
	if((integer << 1 >> 1) != integer)
	{
		obj.object = [BigInt bigIntWithLongLong: (long long)integer];
	}
	else
	{
		obj.smallInt = (integer << 1) | 1;
	}
	return obj;
}
