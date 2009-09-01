#include <dlfcn.h>
#include <unistd.h>
#include <sys/resource.h>

#import <EtoileFoundation/EtoileFoundation.h>
#import "LKAST.h"
#import "LKCategory.h"
#import "LKCompiler.h"
#import "LKCompilerErrors.h"
#import "LKMethod.h"
#import "LKModule.h"

static NSMutableDictionary *compilersByExtension;
static NSMutableDictionary *compilersByLanguage;

static id<LKCompilerDelegate> DefaultDelegate;

@interface LKDefaultCompilerDelegate : NSObject<LKCompilerDelegate>
@end
@implementation LKDefaultCompilerDelegate
- (BOOL)compiler: (LKCompiler*)aCompiler
generatedWarning: (NSString*)aWarning
         details: (NSDictionary*)info
{
	NSLog(@"WARNING: %@", [info objectForKey: kLKHumanReadableDesciption]);
	return YES;
}
- (BOOL)compiler: (LKCompiler*)aCompiler
  generatedError: (NSString*)aWarning
         details: (NSDictionary*)info
{
	NSLog(@"ERROR: %@", [info objectForKey: kLKHumanReadableDesciption]);
	return NO;
}
@end

/**
 * Returns the modification date of a file.
 */
static NSDate *modificationDateForFile(NSString *file)
{
	NSFileManager *fm = [NSFileManager defaultManager];
	NSDictionary *attrib = [fm fileAttributesAtPath: file
	                                   traverseLink: YES];
	return [attrib objectForKey: NSFileModificationDate];
}
/**
 * Returns the most recent modification date of a set of files.
 */
static NSDate *mostRecentModificationDate(NSArray *files)
{
	NSDate *mostRecentDate = [NSDate distantPast];
	FOREACH(files, file, NSString*)
	{
		NSDate *date = modificationDateForFile(file);
		mostRecentDate = [mostRecentDate laterDate: date];
	}
	return mostRecentDate;
}

/**
 * Open the cached (statically-compiled) version of a file if it is not too old.
 */
static BOOL loadLibraryForBundle(NSString *so,
                                 NSBundle *bundle,
                                 NSDate *modified)
{
	NSLog(@"Trying cache %@", so);
	if (nil == so)
	{
		return NO;
	}
	NSDate *soDate = modificationDateForFile(so);
	// If the bundle has been modified after the cached version...
	if (nil == soDate || [modified compare: soDate] == NSOrderedDescending)
	{
		NSLog(@"Cache out of date");
		return NO;
	}
	NSLog(@"Attempting to load .so");
	// Return YES if dlopen succeeds.
	return NULL != dlopen([so UTF8String], RTLD_GLOBAL);
}
/**
 * Load any available cached version of the library.
 */
static BOOL loadAnyLibraryForBundle(NSBundle *bundle, NSDate *modified)
{
	NSString *so = [bundle pathForResource: @"languagekit-cache"
	                                ofType: @"so"];
	if (loadLibraryForBundle(so, bundle, modified))
	{
		return YES;
	}
	NSArray *dirs = NSSearchPathForDirectoriesInDomains( NSLibraryDirectory,
			NSUserDomainMask, YES);
	NSString *userCache = [dirs objectAtIndex: 0];
	userCache = [[userCache stringByAppendingPathComponent: @"LKCaches"]
			stringByAppendingPathComponent: [bundle bundlePath]];
	userCache = [userCache stringByAppendingPathComponent: @"languagekit-cache.so"];
	return loadLibraryForBundle(userCache, bundle, modified);
}

int DEBUG_DUMP_MODULES = 0;
@implementation LKCompiler
+ (void) loadBundles
{
	NSFileManager *fm = [NSFileManager defaultManager];
	NSArray *dirs =
		NSSearchPathForDirectoriesInDomains(
			NSLibraryDirectory,
			NSAllDomainsMask,
			YES);
	FOREACH(dirs, dir, NSString*)
	{
		NSString *f =
			[[dir stringByAppendingPathComponent:@"Bundles"]
				stringByAppendingPathComponent:@"LanguageKit"];
		// Check that the framework exists and is a directory.
		NSArray *bundles = [fm directoryContentsAtPath:f];
		FOREACH(bundles, bundle, NSString*)
		{
			bundle = [f stringByAppendingPathComponent:bundle];
			BOOL isDir = NO;
			if ([fm fileExistsAtPath:bundle isDirectory:&isDir]
				&& isDir)
			{
				NSBundle *b = [NSBundle bundleWithPath:bundle];
				if ([b load])
				{
					// Message is ignored, just ensure that +initialize is
					// called and the class is connected to the hierarchy.
					[[b principalClass] description];
				}
			}
		}
	}
}
+ (void) initialize 
{
	if (self != [LKCompiler class]) { return; }

	DefaultDelegate = [LKDefaultCompilerDelegate new];

	[self loadBundles];

	compilersByExtension = [NSMutableDictionary new];
	compilersByLanguage = [NSMutableDictionary new];
	NSArray *classes = [self directSubclasses];
	FOREACH(classes, nextClass, Class)
	{
		[compilersByLanguage setObject:nextClass
		                        forKey:[nextClass languageName]];
		[compilersByExtension setObject:nextClass
		                         forKey:[nextClass fileExtension]];
	}
}
+ (id) alloc
{
	if (self == [LKCompiler class])
	{
		[NSException raise:@"LKInstantiationError"
		            format:@"LKCompiler instances are invalid"];
		return nil;
	}
	return [super alloc];
}
- (id)init
{
	SUPERINIT;
	delegate = DefaultDelegate;
	return self;
}
+ (LKCompiler*) compiler
{
	return AUTORELEASE([[self alloc] init]);
}
+ (void) setDebugMode:(LKDebuggingMode)aFlag
{
	DEBUG_DUMP_MODULES = (int) aFlag;
}
static void emitParseError(NSException *localException)
{
	NSDictionary *e = [localException userInfo];
	id lineNumber = [e objectForKey:@"lineNumber"];
	id character = [e objectForKey:@"character"];
	id line = [e objectForKey:@"line"];
	NSDictionary *parseErrorInfo = D(
			lineNumber, kLKLineNumber,
			character, kLKCharacterNumber,
			line, kLKSourceLine,
			[NSString stringWithFormat: @"Parse error on line %@.  "
				"Unexpected token at character %@ while parsing:\n%@", 
				lineNumber, character, line], kLKHumanReadableDesciption);
	[LKCompiler reportError: LKParserError
	                details: parseErrorInfo];
}
- (BOOL) compileString:(NSString*)source withGenerator:(id<LKCodeGenerator>)cg
{
	id p = AUTORELEASE([[[[self class] parserClass] alloc] init]);
	LKModule *ast;
	NS_DURING
		ast = [p parseString: source];
	NS_HANDLER
		emitParseError(localException);
		return NO;
	NS_ENDHANDLER
	NSMutableDictionary *dict = [[NSThread currentThread] threadDictionary];
	[dict setObject: self forKey: @"LKCompilerContext"];
	BOOL success = [ast check];
	[dict removeObjectForKey: @"LKCompilerContext"];
	if (!success)
	{
		return NO;
	}
	[ast compileWithGenerator: cg];
	return YES;
}
- (BOOL) compileString:(NSString*)source output:(NSString*)bitcode;
{
	id<LKCodeGenerator> cg = defaultStaticCompilterWithFile(bitcode);
	return [self compileString:source withGenerator:cg];
}
- (BOOL) compileString:(NSString*)source
{
	return [self compileString:source withGenerator:defaultJIT()];
}

- (BOOL) compileMethod: (NSString*)source
          onClassNamed: (NSString*)name
         withGenerator: (id<LKCodeGenerator>)cg
{
	id p = AUTORELEASE([[[[self class] parserClass] alloc] init]);
	LKAST *ast;
	LKModule *module;
	NS_DURING
		ast = [p parseMethod: source];
		ast = [LKCategoryDef categoryOnClassNamed: name
		                                  methods: [NSArray arrayWithObject:ast]];
		module = [LKModule module];
		[module addCategory: (LKCategory*)ast];
	NS_HANDLER
		emitParseError(localException);
		return NO;
	NS_ENDHANDLER
	NSMutableDictionary *dict = [[NSThread currentThread] threadDictionary];
	[dict setObject: self forKey: @"LKCompilerContext"];
	BOOL success = [module check];
	[dict removeObjectForKey: @"LKCompilerContext"];
	if (!success) 
	{
		return NO;
	}
	[module compileWithGenerator: cg];
	return YES;
}
- (BOOL) compileMethod:(NSString*)source
          onClassNamed:(NSString*)name
                output:(NSString*)bitcode
{
	id<LKCodeGenerator> cg = defaultStaticCompilterWithFile(bitcode);
	return [self compileMethod:source onClassNamed:name withGenerator:cg];
}
- (BOOL) compileMethod:(NSString*)source onClassNamed:(NSString*)name
{
	return [self compileMethod: source
	              onClassNamed: name 
	             withGenerator: defaultJIT()];
}

static NSString *loadFramework(NSString *framework)
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
				return [bundle bundlePath];
			}
		}
	}
	return nil;
}

+ (BOOL) loadFrameworkNamed:(NSString*)framework
{
	return nil != loadFramework(framework);
}

+ (Class) loadLanguageKitBundle:(NSBundle*)bundle
{
	//TODO: Static compile and cache the result in a .so, and load this on
	// subsequent runs
	NSString *plistPath = [bundle pathForResource:@"LKInfo" ofType:@"plist"];
	NSDictionary *plist = [NSDictionary dictionaryWithContentsOfFile:plistPath];
	NSArray *frameworks = [plist objectForKey:@"Frameworks"];
	NSMutableArray *pathsToCheckDateOf = [NSMutableArray array];
	[pathsToCheckDateOf addObject: plistPath];
	BOOL success = YES;
	FOREACH(frameworks, framework, NSString*)
	{
		NSString *path = loadFramework(framework);
		BOOL isLoaded = (nil != path);
		success &= isLoaded;
		if (isLoaded)
		{
			[pathsToCheckDateOf addObject: path];
		}
	}
	NSArray *sourceFiles = [plist objectForKey:@"Sources"];
	FOREACH(sourceFiles, source, NSString*)
	{
		NSString *path = [bundle pathForResource: [source stringByDeletingPathExtension]
		                                  ofType: [source pathExtension]];
		BOOL isSourceFound = (nil != path);
		success &= isSourceFound;
		if (isSourceFound)
		{
			[pathsToCheckDateOf addObject: path];
		}
	}
	NSDate *recentModificationDate = mostRecentModificationDate(pathsToCheckDateOf);
	// TODO: Specify a set of AST transforms to apply.
	if (!(success = loadAnyLibraryForBundle(bundle, recentModificationDate)))
	{
		success = YES;
		FOREACH(sourceFiles, source, NSString*)
		{
			success &= [self loadScriptNamed: source fromBundle: bundle];
		}
		// Spawn a new process to do the background JTL compile.
		if (fork() == 0)
		{
			// Make the child process really, really, low priority.
			setpriority(PRIO_PROCESS, 0, 20);
			[self justTooLateCompileBundle: bundle];
			exit(0);
		}
	}
	if (!success)
	{
		return [NSNull class];
	}
	NSString *className = [plist objectForKey:@"PrincipalClass"];
	if (nil != className)
	{
		return NSClassFromString(className);
	}
	return Nil;
}
+ (BOOL) loadAllPlugInsForApplication
{
	NSArray *dirs = NSSearchPathForDirectoriesInDomains(NSLibraryDirectory,
		NSUserDomainMask, YES);
	NSString *processName = [[NSProcessInfo processInfo] processName];
	NSFileManager *fm = [NSFileManager defaultManager];
	BOOL success = YES;
	FOREACH(dirs, dir, NSString*)
	{
		NSString *pluginDir = 
			[[dir stringByAppendingPathComponent:@"LKPlugins"]
				stringByAppendingPathComponent:processName];
		NSArray *plugins = [fm directoryContentsAtPath:pluginDir];
		BOOL isDir = NO;
		FOREACH(plugins, plugin, NSString*)
		{
			plugin = [pluginDir stringByAppendingPathComponent:plugin];
			if ([fm fileExistsAtPath:plugin isDirectory:&isDir] && isDir &&
			    [@"lkplugin" isEqualToString:[plugin pathExtension]])
			{
				Class newclass = [self loadLanguageKitBundle:
					[NSBundle bundleWithPath:plugin]];
				if (newclass == (Class)-1)
				{
					success = NO;
				}
				else
				{
					// Create an instance of the new class that can perform any
					// loading code it needs in +initialize
					[[[newclass alloc] init] release];
				}
			}
		}
	}
	return success;
}

+ (BOOL) loadScriptNamed: (NSString*)fileName fromBundle: (NSBundle*)bundle
{
	NSString *name = [fileName stringByDeletingPathExtension];
	NSString *extension = [fileName pathExtension];
	id compiler = [[compilersByExtension objectForKey: extension] compiler];
	return [compiler loadScriptNamed: name fromBundle: bundle];
}
- (BOOL) loadScriptNamed: (NSString*)name fromBundle:(NSBundle*)bundle
{
	NSString *extension = [[self class] fileExtension];
	NSString *path = [bundle pathForResource:name ofType:extension];
	if (nil == path)
	{
		NSLog(@"Unable to find %@.%@ in bundle %@.", name, extension, bundle);
		return NO;
	}
	return [self compileString:[NSString stringWithContentsOfFile:path]];
}

+ (BOOL) loadApplicationScriptNamed:(NSString*)fileName
{
	return [self loadScriptNamed: fileName fromBundle: [NSBundle mainBundle]];
}
- (BOOL) loadApplicationScriptNamed:(NSString*)name
{
	return [self loadScriptNamed: name fromBundle: [NSBundle mainBundle]];
}
+ (void)setDefaultDelegate: (id<LKCompilerDelegate>)aDelegate
{
	DefaultDelegate = [aDelegate retain];
}
- (void)setDelegate: (id<LKCompilerDelegate>)aDelegate
{
	delegate = aDelegate;
}
- (id<LKCompilerDelegate>)delegate
{
	return delegate;
}

+ (BOOL) loadScriptsFromBundle:(NSBundle*) aBundle
{
	BOOL success = YES;
	FOREACH(compilersByLanguage, class, Class)
	{
		id compiler = [[class alloc] init];
		success &= [compiler loadScriptsFromBundle: aBundle];
		RELEASE(compiler);
	}
	return success;
}
- (BOOL) loadScriptsFromBundle:(NSBundle*) aBundle
{
	NSAutoreleasePool *pool = [NSAutoreleasePool new];
	NSString *extension = [[self class] fileExtension];
	NSArray *scripts = [aBundle pathsForResourcesOfType:extension
	                                        inDirectory:nil];
	BOOL success = YES;
	FOREACH(scripts, scriptFile, NSString*)
	{
		NSString *script = [NSString stringWithContentsOfFile: scriptFile];
		success &= [self compileString:script];
	}
	[pool release];
	return success;
}

+ (BOOL) loadAllScriptsForApplication
{
	return [self loadScriptsFromBundle:[NSBundle mainBundle]];
}
- (BOOL) loadAllScriptsForApplication
{
	return [self loadScriptsFromBundle:[NSBundle mainBundle]];
}

+ (NSString*) fileExtension
{
	[self subclassResponsibility:_cmd];
	return nil;
}
+ (NSString*) languageName
{
	[self subclassResponsibility:_cmd];
	return nil;
}
+ (NSArray*) supportedLanguageNames
{
	return [compilersByLanguage allKeys];
}
+ (Class) compilerForLanguage:(NSString*) aLanguage
{
	return [compilersByLanguage objectForKey:aLanguage];
}
+ (Class) compilerClassForFileExtension:(NSString*) anExtension
{
	return [compilersByExtension objectForKey:anExtension];
}
+ (BOOL)reportWarning: (NSString*)aWarning
              details: (NSDictionary*)info
{
	LKCompiler *compiler =
		[[[NSThread currentThread] threadDictionary] 
			objectForKey: @"LKCompilerContext"];
	id<LKCompilerDelegate> errorDelegate = [compiler delegate];
	if (nil == errorDelegate)
	{
		errorDelegate = DefaultDelegate;
	}
	return [errorDelegate compiler: compiler
	              generatedWarning: aWarning
	                       details: info];
}
+ (BOOL)reportError: (NSString*)aWarning
            details: (NSDictionary*)info
{
	LKCompiler *compiler =
		[[[NSThread currentThread] threadDictionary] 
			objectForKey: @"LKCompilerContext"];
	id<LKCompilerDelegate> errorDelegate = [compiler delegate];
	if (nil == errorDelegate)
	{
		errorDelegate = DefaultDelegate;
	}
	return [errorDelegate compiler: compiler
	                generatedError: aWarning
	                       details: info];
}
+ (Class) parserClass
{
	[self subclassResponsibility:_cmd];
	return Nil;
}
// Replaced in category if a bundle supports JTL compilation, otherwise does nothing
+ (void) justTooLateCompileBundle: (NSBundle*)aBundle {}
@end
