#import <Foundation/Foundation.h>

@protocol Tool
- (void) run;
@end

int main(void)
{
	[NSAutoreleasePool new];
	[[[NSClassFromString(@"SmalltalkTool") alloc] init] run];
	return 0;
}
