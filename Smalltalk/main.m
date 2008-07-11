#include <EtoileFoundation/EtoileFoundation.h>
#import "SymbolTable.h"
#import "Parser.h"
#import "LLVMCodeGen.h"

@interface TestBlock : NSObject{
}
- (id) run:(id)aBlock;
@end
@implementation TestBlock
- (id) run:(id)aBlock
{
  NSLog(@"Block %@", aBlock);
  return [aBlock value];
}
@end

int main(int argc, char **argv)
{
	[NSAutoreleasePool new];
	NSDictionary *opts = ETGetOptionsDictionary("f:b:c:", argc, argv);
	NSString *bundle = [opts objectForKey:@"b"];
	NSCAssert(nil == bundle, @"Smalltalk bundles are not yet supported.  Sorry.");
	NSString *ProgramFile = [opts objectForKey:@"f"];

	if (nil == ProgramFile)
	{
#ifdef DEBUG
		ProgramFile = @"test.st";
#else
		fprintf(stderr, "Usage: %s -f {smalltalk file}\n", argv[0]);
		return 1;
#endif
	}
	id Program = [NSString stringWithContentsOfFile:ProgramFile];
	Parser * p = [[Parser alloc] init];
	AST *ast = [p parseString: Program];
	id cg = [LLVMCodeGen new];
#ifdef DEBUG
	DEBUG_DUMP_MODULES = 1;
#endif
	[ast compileWith:cg];
	NSString * className = [opts objectForKey:@"c"];
	if (nil == className)
	{
		className = @"SmalltalkTool";
	}
	Class tool = NSClassFromString(className);
	if (![tool instancesRespondToSelector:@selector(run)])
	{
		fprintf(stderr, "%s instances must respond to run message\n",
				[className UTF8String]);
		return 2;
	}
	[[tool new] run];
	return 0;
}
