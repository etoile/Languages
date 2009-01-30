#import <Foundation/NSValue.h>

@implementation NSValue (structures)
+ (id) rangeWithLocation:(int)loc length:(int)len
{
	NSRange r = [[NSValue valueWithRange:NSMakeRange(loc, len)] rangeValue];
	return [NSValue valueWithRange:NSMakeRange(loc, len)];
}
@end
