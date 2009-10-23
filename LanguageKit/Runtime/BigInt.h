#import <EtoileFoundation/EtoileFoundation.h>
#include <gmp.h>

@interface BigInt : NSObject {
@public
  /**
   * Value for this object.  Public so it can be accessed from others to
   * slightly lower the cost of operations on BiInts.
   */
	mpz_t v;
}
+ (BigInt*) bigIntWithCString:(const char*) aString;
+ (BigInt*) bigIntWithLongLong:(long long)aVal;
+ (BigInt*) bigIntWithLong:(long)aVal;
+ (BigInt*) bigIntWithUnsignedLong:(unsigned long)aVal;
+ (BigInt*) bigIntWithMP:(mpz_t)aVal;
@end
