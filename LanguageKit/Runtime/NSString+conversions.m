#import <Foundation/Foundation.h>

@implementation NSString (Conversions)
- (unsigned long long)unsignedLongLongValue
{
	return (unsigned long long)[self longLongValue];
}
@end
