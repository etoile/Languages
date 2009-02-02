#import <Foundation/NSValue.h>

@implementation NSValue (structures)
+ (id) rangeWithLocation:(int)loc length:(int)len
{
	return [NSValue valueWithRange:NSMakeRange(loc, len)];
}

+ (id) pointWithX:(float)x Y:(float)y
{
	return [NSValue valueWithPoint:NSMakePoint(x, y)];
}

+ (id) rectWithX:(float)x Y:(float)y width:(float)width height:(float)height
{
	return [NSValue valueWithRect:NSMakeRect(x, y, width, height)];
}

+ (id) sizeWithWidth:(float)width height:(float)height
{
	return [NSValue valueWithSize:NSMakeSize(width, height)];
}
@end
