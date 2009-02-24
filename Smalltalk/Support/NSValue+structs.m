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

- (unsigned int) location
{
	return [self rangeValue].location;
}

- (unsigned int) length
{
	return [self rangeValue].length;
}

- (float) x
{
	return [self pointValue].x;
}

- (float) y
{
	return [self pointValue].y;
}

- (NSValue *) origin
{
	return [NSValue valueWithPoint: [self rectValue].origin];
}

- (NSValue *) size
{
	return [NSValue valueWithSize: [self rectValue].size];
}

- (float) width
{
	return [self sizeValue].width;
}

- (float) height
{
	return [self sizeValue].height;
}
@end
