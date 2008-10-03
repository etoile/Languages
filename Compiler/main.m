#import <EtoileFoundation/EtoileFoundation.h>
#import <AppKit/AppKit.h>
#include <time.h>
#include <sys/resource.h>
#import <SmalltalkKit/SmalltalkKit.h>


int main(int argc, char **argv)
{
	[NSAutoreleasePool new];
	NSDictionary *opts = ETGetOptionsDictionary("dtf:b:c:l:L:", argc, argv);
	NSString *bundle = [opts objectForKey:@"b"];
	NSCAssert(nil == bundle, @"Smalltalk bundles are not yet supported.  Sorry.");
	// Load specified framework
	NSString *framework = [opts objectForKey:@"l"];
	if (nil != framework)
	{
		[SmalltalkCompiler loadFramework:framework];
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
				[SmalltalkCompiler loadFramework:f];
			}
		}
	}

	// Debug mode.
	if ([[opts objectForKey:@"d"] boolValue])
	{
		[SmalltalkCompiler setDebugMode:YES];
	}
	NSString *ProgramFile = [opts objectForKey:@"f"];
	if (nil == ProgramFile)
	{
		fprintf(stderr, "Usage: %s -f {smalltalk file}\n", argv[0]);
		return 1;
	}
	NSString *Program = [NSString stringWithContentsOfFile:ProgramFile];
	clock_t c1 = clock();
	[SmalltalkCompiler compileString:[NSString stringWithContentsOfFile:@"t1.st"]];
	if (![SmalltalkCompiler compileString:Program])
	{
		NSLog(@"Failed to compile input.");
		return 2;
	}
	if ([[opts objectForKey:@"t"] boolValue])
	{
		clock_t c2 = clock();
		struct rusage r;
		getrusage(RUSAGE_SELF, &r);
		NSLog(@"Smalltalk compilation took %f seconds.  Peak used %dKB.",
			((double)c2 - (double)c1) / (double)CLOCKS_PER_SEC, r.ru_maxrss);
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
