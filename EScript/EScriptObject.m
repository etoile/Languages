#import "EScriptObject.h"
#import <LanguageKitRuntime/BlockClosure.h>
#import <LanguageKitRuntime/BlockContext.h>

@implementation EScriptObject
- (id) construct
{
	return self;
}
@end
@interface _NSBlock : NSObject
- (id)value;
@end

@implementation _NSBlock (EScript)
- (id)construct
{
	id prototype = [[self slotValueForKey: @"prototype"] clone];
	if (nil == prototype)
	{
		prototype = [EScriptObject new];
	}
	[self setValue: prototype forKey: @"this"];
	[self value];
	[self setValue: nil forKey: @"this"];
	return [prototype autorelease];
}
@end

@implementation NSObject (EScript)
+ (id)construct
{
	return [[[self alloc] init] autorelease];
}
- (id)value
{
	return self;
}
@end
