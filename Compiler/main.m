#import <EtoileFoundation/EtoileFoundation.h>
#import <EtoileFoundation/runtime.h>
#import <AppKit/AppKit.h>
#include <time.h>
#include <sys/resource.h>
#import <LanguageKit/LanguageKit.h>
#import <LanguageKit/LKInterpreter.h>

@implementation NSObject (DumpObject)
- (void) dumpObject
{
	int words = class_getInstanceSize(isa) / sizeof(int);
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
	Method mine = class_getClassMethod(self, @selector(mainBundle));
	class_replaceMethod(objc_getMetaClass("NSBundle"), @selector(mainBundle), 
		method_getImplementation(mine), method_getTypeEncoding(mine));
}
+ (NSBundle*) mainBundle
{
	return mainBundle;
}
@end

@interface LKCompilerWarningIgnoringDelegate : NSObject<LKCompilerDelegate>
@end
@implementation LKCompilerWarningIgnoringDelegate
- (BOOL)compiler: (LKCompiler*)aCompiler
generatedWarning: (NSString*)aWarning
         details: (NSDictionary*)info { return YES; }
- (BOOL)compiler: (LKCompiler*)aCompiler
  generatedError: (NSString*)aWarning
         details: (NSDictionary*)info
{
	NSLog(@"ERROR: %@", [info objectForKey: kLKHumanReadableDescription]);
	return NO;
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
	[LKCompiler compilerClassForFileExtension:extension];
	script = stripScriptPreamble(script);
	id parser = 
		[[[LKCompiler compilerClassForFileExtension:extension]
	   			parserClass] new];
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

static BOOL jitScript(NSString *script, NSString *extension, BOOL interpret)
{
	NS_DURING
		LKAST *ast = parseScript(script, extension);
		if (![ast check])
		{
			NS_VALUERETURN(NO, BOOL);
		}
		applyTransforms(ast);
		if (NO == interpret)
		{
			id codeGenerator = defaultJIT();
			if (nil != codeGenerator)
			{
				[ast compileWithGenerator: codeGenerator];
			}
			else
			{
				[ast interpretInContext: nil];
			}
		}
		else
		{
			[ast interpretInContext: nil];
		}
	NS_HANDLER
		NSLog(@"%@", localException);
		return NO;
	NS_ENDHANDLER
	return YES;
}

static BOOL staticCompileScript(NSString *script, NSString *outFile, 
		NSString *extension)
{
	NS_DURING
		LKAST *ast = parseScript(script, extension);
		if (![ast check])
		{
			NS_VALUERETURN(NO, BOOL);
		}
		applyTransforms(ast);
		[ast compileWithGenerator: defaultStaticCompilterWithFile(outFile)];
	NS_HANDLER
		NSLog(@"%@", localException);
		return NO;
	NS_ENDHANDLER
	return YES;
}

static BOOL enableTiming;

static void logTimeSinceWithMessage(clock_t c1, NSString *message)
{
	if (!enableTiming) { return; }
	clock_t c2 = clock();
	struct rusage r;
	getrusage(RUSAGE_SELF, &r);
	NSLog(@"%@ took %f seconds.  Peak used %ldKB.", message, 
		((double)c2 - (double)c1) / (double)CLOCKS_PER_SEC, r.ru_maxrss);
}

/*
 * Functions for listing the type encodings used by a specific method name.
 * Useful for tracking down where polymorphic selector warnings are coming
 * from...
static void checkMethodInClass(const char *name, Class c)
{
	unsigned int methodCount;
	Method *methods = class_copyMethodList(c, &methodCount);
	if (NULL != methods)
	{
		for (unsigned int i=0 ; i<methodCount ; i++)
		{
			Method m = methods[i];
			const char *n = sel_getName(method_getName(m));
			if (strcmp(n, name) == 0)
			{
				fprintf(stderr, "%s declares %c%s, %s\n", class_getName(c),
				        class_isMetaClass(c) ? '+' : '-', name,
				        method_getTypeEncoding(m));
			}
		}
		free(methods);
	}
}

static void check_method(const char *name)
{
	int numClasses = objc_getClassList(NULL, 0);
	Class *classes = malloc(sizeof(Class) * numClasses);

	numClasses = objc_getClassList(classes, numClasses);
	for (int i=0 ; i<numClasses ; i++)
	{
		Class c = classes[i];
		checkMethodInClass(name, c);
		checkMethodInClass(name, object_getClass(c));
	}
	free(classes);
}
*/

int main(int argc, char **argv)
{
	[NSAutoreleasePool new];
	//check_method("value");
	// Forces the compiler to load plugins
	[LKCompiler supportedLanguageNames];

	NSDictionary *opts = ETGetOptionsDictionary("dtf:b:cC:l:L:v:iq", argc, argv);

	// Debug mode.
	if ([[opts objectForKey:@"d"] boolValue])
	{
		[LKCompiler setDebugMode:YES];
	}
	// Suppress warnings
	if ([[opts objectForKey:@"q"] boolValue])
	{
		[LKCompiler setDefaultDelegate: [LKCompilerWarningIgnoringDelegate new]];
	}
	enableTiming = [[opts objectForKey:@"t"] boolValue];
	clock_t c1;

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
		c1 = clock();
		Class principalClass = 
			[LKCompiler loadLanguageKitBundle:mainBundle];
		logTimeSinceWithMessage(c1, @"Loading bundle");
		if ([principalClass isKindOfClass: [NSNull class]])
		{
			return 1;
		}
		c1 = clock();
		[[[principalClass alloc] init] run];
		logTimeSinceWithMessage(c1, @"Smalltalk execution");
		return 0;
	}
	// Load specified framework
	NSString *framework = [opts objectForKey:@"l"];
	if (nil != framework)
	{
		[LKCompiler loadFrameworkNamed: framework];
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
				[LKCompiler loadFrameworkNamed: f];
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

	NSString *ProgramFile = [opts objectForKey:@"f"];
	if (nil == ProgramFile)
	{
		fprintf(stderr, "Usage: %s -f {smalltalk file}\n", argv[0]);
		return 1;
	}
	NSString *Program = [NSString stringWithContentsOfFile:ProgramFile];
	if (nil == Program)
	{
		NSLog(@"Failed to open file %@", ProgramFile);
		return 1;
	}
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
	c1 = clock();
	BOOL onlyInterpret = [[opts objectForKey:@"i"] boolValue];
	if (!jitScript(Program, extension, onlyInterpret))
	{
		NSLog(@"Failed to compile input.");
		return 2;
	}
	logTimeSinceWithMessage(c1, @"Smalltalk compilation");

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
	@autoreleasepool {
	[aTool run];
	}
	if ([[opts objectForKey:@"d"] boolValue])
	{
		NSLog(@"Object at address %x has data: ", (unsigned)(uintptr_t)aTool);
		[aTool dumpObject];
	}
	logTimeSinceWithMessage(c1, @"Smalltalk execution");

	return 0;
}
