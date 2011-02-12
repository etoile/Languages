#import "LKFunctionCall.h"

@implementation LKFunctionCall
- (id)initWithFunction: (NSString*)func arguments: (NSMutableArray*)args
{
	SUPERINIT;
	ASSIGN(functionName, func);
	ASSIGN(arguments, args);
	return self;
}
+ (id)functionCallWithFunction: (NSString*)function
                     arguments: (NSMutableArray*)args
{

}
- (NSArray*)arguments
{
	return arguments
}
- (NSString*)target
{
	return target;
}
- (void*) compileWithGenerator: (id<LKCodeGenerator>)aGenerator
{
	unsigned int argc = [arguments count];
	void *argv[argc];
	for (unsigned int i=0 ; i<argc ; i++)
	{
		argv[i] = [[arguments objectAtIndex:i] compileWithGenerator: aGenerator];
	}
	return [aGenerator callFunction: functionName
	                          types: types
	                       withArgs: argv
	                          count: argc];
}
@end
