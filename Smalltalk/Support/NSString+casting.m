#import <Foundation/Foundation.h>

@implementation NSString (Casting)
- (unsigned) unsignedIntValue
{
	return (unsigned)[self intValue];
}
@end
