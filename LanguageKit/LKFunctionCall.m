#import "LKFunctionCall.h"
#import "LKCompiler.h"
#import "LKCompilerErrors.h"

@interface LKCompiler (PrivateStuff)
+ (NSString*)typesForFunction: (NSString*)functionName;
@end

@implementation LKFunctionCall
@synthesize functionName, typeEncoding, arguments;
- (void)dealloc
{
	[functionName release];
	[typeEncoding release];
	[arguments release];
	[super dealloc];
}
- (BOOL)check
{
	ASSIGN(typeEncoding, [LKCompiler typesForFunction: functionName]);
	if (nil == typeEncoding)
	{
		NSDictionary *errorDetails = D(
			[NSString stringWithFormat: @"Can not determine type for %@", functionName],
				kLKHumanReadableDescription,
			self, kLKASTNode);
		if ([LKCompiler reportError: LKUnknownTypeError
		                    details: errorDetails])
		{
			return [self check];
		}
		return NO;
	}
	for (LKAST *node in arguments)
	{
		[node setParent: self];
		if (![node check])
		{
			return NO;
		}
	}
	return YES;
}
- (void*) compileWithGenerator: (id<LKCodeGenerator>)aGenerator
{
	int count = [arguments count];
	void *args[count];
	int i = 0;
	for (LKAST *arg in arguments)
	{
		args[i++] = [arg compileWithGenerator: aGenerator];
	}
	return [aGenerator callFunction: functionName
	                   typeEncoding: typeEncoding
	                      arguments: args
	                          count: count];
}
@end

