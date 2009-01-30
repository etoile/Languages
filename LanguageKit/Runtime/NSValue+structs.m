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
@end
