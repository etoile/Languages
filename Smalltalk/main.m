#import <EtoileFoundation/EtoileFoundation.h>
#import <AppKit/AppKit.h>
#import "SymbolTable.h"
#import "Parser.h"
#import "LLVMCodeGen.h"

static BOOL compileString(NSString *s)
{
	Parser * p = [[Parser alloc] init];
	AST *ast;
	NS_DURING
		ast = [p parseString: s];
	NS_HANDLER
		NSDictionary *e = [localException userInfo];
		NSLog(@"Parse error on line %@.  Unexpected token at character %@ while parsing:\n%@",
										   [e objectForKey:@"lineNumber"],
										   [e objectForKey:@"character"],
										   [e objectForKey:@"line"]);
		NS_VALUERETURN(NO, BOOL);
	NS_ENDHANDLER	
	id cg = [LLVMCodeGen new];
#ifdef DEBUG
	DEBUG_DUMP_MODULES = 1;
#endif
	[ast compileWith:cg];
	return YES;
}

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
	NSString *Program = [NSString stringWithContentsOfFile:ProgramFile];
	if (!compileString(Program))
	{
		NSLog(@"Failed to compile input.");
		return 2;
	}
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
		return 3;
	}
	[[tool new] run];
	return 0;
}
