#import <Foundation/NSValue.h>

@implementation NSValue (structures)
+ (id) valueWithBytesOrNil:(char*)bytes objCType:(char*)type
{
	if (NULL == bytes)
	{
		return nil;
	}
	return [NSValue valueWithBytes:bytes objCType:type];
}
+ (id) rangeWithLocation:(int)loc length:(int)len
{
	NSRange r = [[NSValue valueWithRange:NSMakeRange(loc, len)] rangeValue];
	return [NSValue valueWithRange:NSMakeRange(loc, len)];
}
@end
