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
		new[i++] = [aClosure value:obj];
	}
	return [NSArray arrayWithObjects:new count:i];
}
@end
