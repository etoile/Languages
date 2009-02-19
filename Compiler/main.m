#import <EtoileFoundation/EtoileFoundation.h>
#import <AppKit/AppKit.h>
#include <time.h>
#include <sys/resource.h>
#import <LanguageKit/LanguageKit.h>

@implementation NSObject (DumpObject)
- (void) dumpObject
{
	int words = isa->instance_size / sizeof(int);
	unsigned *word = (unsigned*)self;
	for (unsigned i = 0 ; i<words ; i++)
	{
		printf("%x ", word[i]);
	}
	printf("\n");
}
@end
static NSBundle *mainBundle = nil;
@interface NSBundleHack : NSBundle {}
+ (void) enableHack;
@end
@implementation NSBundleHack
+ (void) enableHack
{
	[self poseAsClass:[NSBundle class]];
}
+ (NSBundle*) mainBundle
{
	return mainBundle;
}
@end

static NSString* stripScriptPreamble(NSString *script)
{
	if ([script length] > 2
		&&
		[[script substringToIndex:2] isEqualToString:@"#!"])
	{
		NSRange r = [script rangeOfString:@"\n"];
		if (r.location == NSNotFound)
		{
			script = nil;
		}
		else
		{
			script = [script substringFromIndex:r.location];
		}
	}
	return script;
}

static LKAST *parseScript(NSString *script, NSString *extension)
{
	[LKCompiler compilerForExtension:extension];
	script = stripScriptPreamble(script);
	id parser = [[[LKCompiler compilerForExtension:extension] parserClass] new];
	LKAST *module = [parser parseString:script];
	[parser release];
	return module;
}

static NSArray *Transforms = nil;

static void  applyTransforms(LKAST* anAST)
{
	FOREACH(Transforms, transform, id<LKASTVisitor>)
	{
		[anAST visitWithVisitor:transform];
	}
}

static BOOL jitScript(NSString *script, NSString *extension)
{
	NS_DURING
		LKAST *ast = parseScript(script, extension);
		[ast check];
		applyTransforms(ast);
		[ast compileWith:defaultJIT()];
	NS_HANDLER
		return NO;
	NS_ENDHANDLER
	return YES;
}

static BOOL staticCompileScript(NSString *script, NSString *outFile, 
		NSString *extension)
{
	NS_DURING
		LKAST *ast = parseScript(script, extension);
		[ast check];
		applyTransforms(ast);
		[ast compileWith:defaultStaticCompilterWithFile(outFile)];
	NS_HANDLER
		return NO;
	NS_ENDHANDLER
	return YES;
}

int main(int argc, char **argv)
{
	[NSAutoreleasePool new];
	// Forces the compiler to load plugins
	[LKCompiler supportedLanguages];

	NSDictionary *opts = ETGetOptionsDictionary("dtf:b:cC:l:L:v:", argc, argv);
	NSString *bundle = [opts objectForKey:@"b"];
	if (nil != bundle)
	{
		if (![bundle isAbsolutePath])
		{
			bundle = [[[NSFileManager defaultManager] currentDirectoryPath]
			   	stringByAppendingPathComponent:bundle];
		}
		mainBundle = [NSBundle bundleWithPath:bundle];
		[NSBundleHack enableHack];
		Class principalClass = 
			[LKCompiler loadLanguageKitBundle:mainBundle];
		if (principalClass == (Class)-1)
		{
			return 1;
		}
		[[[principalClass alloc] init] run];
		return 0;
	}
	// Load specified framework
	NSString *framework = [opts objectForKey:@"l"];
	if (nil != framework)
	{
		[LKCompiler loadFramework:framework];
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
				[LKCompiler loadFramework:f];
			}
		}
	}

	NSString *transformName = [opts objectForKey:@"v"];
	if (nil != transformName)
	{
		Class transform = NSClassFromString(transformName);
		if ([transform conformsToProtocol:@protocol(LKASTVisitor)])
		{
			Transforms = A([[transform new] autorelease]);
		}
		else
		{
			fprintf(stderr, "warning: Invalid transform '%s' specified \n",
				   	[transformName UTF8String]);
		}
	}
	// Debug mode.
	if ([[opts objectForKey:@"d"] boolValue])
	{
		[LKCompiler setDebugMode:YES];
	}
	NSString *ProgramFile = [opts objectForKey:@"f"];
	if (nil == ProgramFile)
	{
		fprintf(stderr, "Usage: %s -f {smalltalk file}\n", argv[0]);
		return 1;
	}
	NSString *Program = [NSString stringWithContentsOfFile:ProgramFile];
	NSString *extension = [ProgramFile pathExtension];
	// Static compile
	if ([[opts objectForKey:@"c"] boolValue])
	{
		NSString *bcFile = [[ProgramFile stringByDeletingPathExtension]
			stringByAppendingPathExtension:@"bc"];
		staticCompileScript(Program, bcFile, extension);
		return 0;
	}
	// JIT compile and run
	clock_t c1 = clock();
	if (!jitScript(Program, extension))
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

	NSString * className = [opts objectForKey:@"C"];
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
	id aTool = [tool new];
	[aTool run];
	if ([[opts objectForKey:@"d"] boolValue])
	{
		NSLog(@"Object at address %x has data: ", (unsigned)(uintptr_t)aTool);
		[aTool dumpObject];
	}

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
