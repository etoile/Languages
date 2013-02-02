#import <Foundation/Foundation.h>

@implementation NSObject (log)
- (void) log
{
	NSLog(@"%@", [self description]);
}
- (void) printOn:(NSFileHandle*)handle
{
	[handle writeData:[[self description] dataUsingEncoding:NSUTF8StringEncoding]];
}
- (void) ifNotNil: (id(^)(void))aBlock
{
	aBlock();
}
@end
