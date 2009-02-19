#import <Foundation/Foundation.h>

@implementation NSString (Casting)
- (unsigned int) unsignedIntValue
{
	return (unsigned int)[self intValue];
}
- (SEL) selValue
{
	return NSSelectorFromString(self);
}
@end
