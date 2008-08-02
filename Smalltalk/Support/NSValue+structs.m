#import <Foundation/NSValue.h>

@implementation NSValue (structures)
+ (id) rangeWithLocation:(int)loc length:(int)len
{
	NSLog(@"Creating range from %d to %d", loc, len);
	NSRange r = [[NSValue valueWithRange:NSMakeRange(loc, len)] rangeValue];
	NSLog(@"Created Range: %d, %d", r.location, r.length);
	return [NSValue valueWithRange:NSMakeRange(loc, len)];
}
@end
