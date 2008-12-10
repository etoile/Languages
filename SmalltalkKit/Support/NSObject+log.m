#import <Foundation/Foundation.h>

@implementation NSObject (log)
+ (Class) autorelease 
{
	return self;
}
+ (Class) retain
{
	return self;
}
- (void) log
{
	NSLog(@"%@", [self description]);
}
- (void) printOn:(NSFileHandle*)handle
{
	[handle writeData:[[self description] dataUsingEncoding:NSUTF8StringEncoding]];
}
@end
