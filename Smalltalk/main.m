#import <EtoileFoundation/EtoileFoundation.h>
#import <AppKit/AppKit.h>
#include <time.h>
#include <sys/resource.h>
#import <SmalltalkKit/SmalltalkKit.h>

extern int DEBUG_DUMP_MODULES;
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
	id cg = defaultCodeGenerator();
	DEBUG_DUMP_MODULES = 0;
#ifdef DEBUG
	DEBUG_DUMP_MODULES = 1;
#endif
	[ast compileWith:cg];
	return YES;
}

BOOL loadFramework(NSString *framework)
{
	NSFileManager *fm = [NSFileManager defaultManager];
	NSArray *dirs = NSSearchPathForDirectoriesInDomains(NSLibraryDirectory,
			NSAllDomainsMask, YES);
	FOREACH(dirs, dir, NSString*)
	{
		NSString *f = [NSString stringWithFormat:@"%@/Frameworks/%@.framework",
				 dir, framework];
		// Check that the framework exists and is a directory.
		BOOL isDir = NO;
		if ([fm fileExistsAtPath:f isDirectory:&isDir] && isDir)
		{
			NSBundle *bundle = [NSBundle bundleWithPath:f];
			if ([bundle load]) 
			{
				NSLog(@"Loaded bundle %@", f);
				return YES;
			}
		}
	}
	return NO;
}

int main(int argc, char **argv)
{
	[NSAutoreleasePool new];
	NSDictionary *opts = ETGetOptionsDictionary("tf:b:c:l:L:", argc, argv);
	NSString *bundle = [opts objectForKey:@"b"];
	NSCAssert(nil == bundle, @"Smalltalk bundles are not yet supported.  Sorry.");
	// Load specified framework
	NSString *framework = [opts objectForKey:@"l"];
	if (nil != framework)
	{
		loadFramework(framework);
	}
	// Load frameworks specified in plist.
	NSString *frameworks = [opts objectForKey:@"L"];
	if (nil != frameworks)
	{
		NSArray *frameworkarray = [NSArray arrayWithContentsOfFile:frameworks];
		if (nil != frameworkarray)
		{
			FOREACH(frameworkarray, f, NSString*)
			{
				loadFramework(f);
			}
		}
	}

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
	clock_t c1;
	c1 = clock();
	[[tool new] run];
	if ([[opts objectForKey:@"t"] boolValue])
	{
		clock_t c2 = clock();
		struct rusage r;
		getrusage(RUSAGE_SELF, &r);
		NSLog(@"Smalltalk execution took %f seconds.  Peak used %dKB.",
				((double)c2 - (double)c1) / (double)CLOCKS_PER_SEC, r.ru_maxrss);
	}
	return 0;
}
