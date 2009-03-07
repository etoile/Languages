#import "EScriptObject.h"

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
