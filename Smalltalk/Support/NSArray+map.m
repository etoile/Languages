#import "NSArray+map.h"
#import "BlockClosure.h"
#import <EtoileFoundation/EtoileFoundation.h>

@implementation NSArray (map)
- (NSArray*) map:(id)aClosure
{
	id new[[self count]];
	int i = 0;
	FOREACHI(self, obj)
	{
		new[i] = [aClosure value:obj];
		i++;
	}
	return [NSArray arrayWithObjects:new count:i];
}
- (void) foreach:(id)aClosure
{
	FOREACHI(self, obj)
	{
		[aClosure value:obj];
	}
}
- (void) do:(id)aClosure
{
	FOREACHI(self, obj)
	{
		[aClosure value:obj];
	}
}
- (NSArray*) select:(id)aClosure
{
	id new[[self count]];
	int i = 0;
	FOREACHI(self, obj)
	{
		if ([[aClosure value:obj] boolValue])
		{
			new[i++] = obj;
		}
	}
	return [NSArray arrayWithObjects:new count:i];
}
- (id) detect:(id)aClosure
{
	id new[[self count]];
	int i = 0;
	FOREACHI(self, obj)
	{
		if ([[aClosure value:obj] boolValue])
		{
			return obj;
		}
	}
	return nil;
}
- (id) inject:(id)aValue into:aClosure
{
	id collect = aValue;
	FOREACHI(self, obj)
	{
		collect = [aClosure value:obj value:collect];
	}
	return collect;
}
- (id) fold:(id)aClosure
{
	return [self inject:nil into:aClosure];
}
@end
