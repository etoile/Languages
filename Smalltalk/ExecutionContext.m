#import <ExecutionContext.h>

@implementation ExecutionContext
- (id) initWithArgs:(id*)argv
{
	SELFINIT;
	args = argv;
	return SELF;
}
- (id) getArg:(int)argIndex
{
	return args[argIndex];
}
- (void) setArg:(id)arg atIndex:(int)argIndex
{
	args[argIndex] = arg;
}
@end
