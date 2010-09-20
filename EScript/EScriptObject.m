#import "EScriptObject.h"
#import <LanguageKitRuntime/BlockClosure.h>
#import <LanguageKitRuntime/BlockContext.h>

@implementation EScriptObject
- (id) copyWithZone: (NSZone*)zone
{
	return [[[self class] allocWithZone: zone] init];
}

- (id) construct
{
	return self;
}
@end
@implementation BlockClosure (EScript)
- (id)construct
{
	id prototype = [context valueForSymbol: @"prototype"];
	if (nil == prototype)
	{
		prototype = [EScriptObject new];
	}
	id clone = [prototype clone];
	id oldSelf = [context valueForSymbol: @"self"];
	[context setValue: clone forSymbol: @"self"];
	[self value];
	[context setValue: oldSelf forSymbol: @"self"];
	return clone;
}
@end
