#include <dlfcn.h>
#include <EtoileFoundation/glibc_hack_unistd.h>
#include <sys/resource.h>
#include <objc/runtime.h>
#include <objc/hooks.h>

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

// Don't include the header yet - we don't want to add a SCK dependency, just
// use it if we have it.
@interface SCKSourceCollection : NSObject
@property (nonatomic, readonly) NSMutableDictionary *functions;
@property (nonatomic, readonly) NSMutableDictionary *globals;
@property (nonatomic, readonly) NSMutableDictionary *enumerationValues;
@property (nonatomic, readonly) NSMutableDictionary *enumerations;
- (id)sourceFileForPath: (NSString*)aPath;
@end


static SCKSourceCollection *collection;
static NSMutableArray *loaders;
static BOOL inDevMode;

static Class lookup_class(const char *name)
{
	static BOOL recursing;
	if (recursing) { return nil; }

	recursing = YES;
	NSString *className = [NSString stringWithUTF8String: name];
	for (LKClassLoader loader in loaders)
	{
		loader(className);
	}
	Class cls = objc_getClass(name);
	recursing = NO;
	return cls;
}

@interface LKDefaultCompilerDelegate : NSObject<LKCompilerDelegate>
{
	NSMutableSet *polymorphicSelectors;
}
@end
@implementation LKDefaultCompilerDelegate
- (id)init
{
	SUPERINIT;
	polymorphicSelectors = [NSMutableSet new];
	return self;
}
- (BOOL)compiler: (LKCompiler*)aCompiler
generatedWarning: (NSString*)aWarning
         details: (NSDictionary*)info
{
	if (aWarning == LKPolymorphicSelectorWarning)
	{
		NSString *selector = [(LKMessageSend*)[info objectForKey: kLKASTNode] selector];
		// Only log polymorphic selector warnings once per selector.
		if ([polymorphicSelectors containsObject: selector])
		{
			return YES;
		}
		[polymorphicSelectors addObject: selector];
	}
	NSLog(@"WARNING: %@", [info objectForKey: kLKHumanReadableDescription]);
	return YES;
}
- (BOOL)compiler: (LKCompiler*)aCompiler
  generatedError: (NSString*)aWarning
         details: (NSDictionary*)info
{
	NSLog(@"ERROR: %@", [info objectForKey: kLKHumanReadableDescription]);
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
static BOOL loadLibraryForBundle(NSString *soFile,
                                 NSBundle *bundle,
                                 NSDate *modified)
{
	//NSLog(@"Trying cache %@", soFile);
	if (nil == soFile)
	{
		return NO;
	}
	NSDate *soDate = modificationDateForFile(soFile);
	// If the bundle has been modified after the cached version...
	if (nil == soDate || [modified compare: soDate] == NSOrderedDescending)
	{
		return NO;
	}
	// Return YES if dlopen succeeds.
	//return NULL != dlopen([soFile UTF8String], RTLD_GLOBAL);
	void *so = dlopen([soFile UTF8String], RTLD_GLOBAL);
	if (so == NULL)
	{
		NSLog(@"Failed to load cache.  dlopen() error: %s", dlerror());
	}
	return NULL != so;
}
/**
 * Load any available cached version of the library.
 */
static BOOL loadAnyLibraryForBundle(NSBundle *bundle, NSDate *modified)
{
	NSString *soFile = [bundle pathForResource: @"languagekit-cache"
	                                    ofType: @"so"];
	if (loadLibraryForBundle(soFile, bundle, modified))
	{
		return YES;
	}
	NSArray *dirs = NSSearchPathForDirectoriesInDomains( NSLibraryDirectory,
			NSUserDomainMask, YES);
	NSString *userCache = [dirs objectAtIndex: 0];
	userCache = [[userCache stringByAppendingPathComponent: @"LKCaches"]
			stringByAppendingPathComponent: [bundle bundlePath]];
	userCache = 
		[userCache stringByAppendingPathComponent: @"languagekit-cache.so"];
	return loadLibraryForBundle(userCache, bundle, modified);
}

int DEBUG_DUMP_MODULES = 0;
@implementation LKCompiler
@synthesize transforms;
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
		FOREACH(bundles, b, NSString*)
		{
			NSString *bundle = [f stringByAppendingPathComponent: b];
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
	// If SourceCodeKit is installed, let's use it!
	[self loadFrameworkNamed: @"SourceCodeKit"];
	collection = [NSClassFromString(@"SCKSourceCollection") new];
	if (collection)
	{
		[self loadFrameworkNamed: @"EtoileFoundation"];
	}
}

+ (void)addClassLoader: (LKClassLoader)aBlock;
{
	if (nil == loaders)
	{
		loaders = [NSMutableArray new];
		_objc_lookup_class = lookup_class;
	}
	[loaders addObject: aBlock];
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
	transforms = [NSMutableArray new];
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
	NSDictionary *info = [localException userInfo];
	id lineNumber = [info objectForKey: @"lineNumber"];
	id character = [info objectForKey: @"character"];
	id line = [info objectForKey: @"line"];
	NSDictionary *parseErrorInfo = D(
			lineNumber, kLKLineNumber,
			character, kLKCharacterNumber,
			line, kLKSourceLine,
			[NSString stringWithFormat: @"Parse error on line %@.  "
				"Unexpected token at character %@ while parsing:\n%@", 
				lineNumber, character, line], kLKHumanReadableDescription);
	[LKCompiler reportError: LKParserError
	                details: parseErrorInfo];
}
- (LKAST*) compileString:(NSString*)source withGenerator:(id<LKCodeGenerator>)cg
{
	id parser = AUTORELEASE([[[[self class] parserClass] alloc] init]);
	LKAST *ast;
	NS_DURING
		ast = [parser parseString: source];
	NS_HANDLER
		emitParseError(localException);
		return nil;
	NS_ENDHANDLER

	NSMutableDictionary *dict = [[NSThread currentThread] threadDictionary];
	[dict setObject: self forKey: @"LKCompilerContext"];
	BOOL success = [ast check];
	if (!success)
	{
		return nil;
	}
	for (id<LKASTVisitor> transform in transforms)
	{
		[ast visitWithVisitor: transform];
	}
	success = [ast check];
	[dict removeObjectForKey: @"LKCompilerContext"];
	if (success)
	{
		[ast compileWithGenerator: cg];
	}
	return success ? ast : nil;
}
- (LKAST*) compileString:(NSString*)source output:(NSString*)bitcode;
{
	id<LKCodeGenerator> cg = defaultStaticCompilterWithFile(bitcode);
	return [self compileString:source withGenerator:cg];
}
- (LKAST*) compileString:(NSString*)source
{
	return [self compileString:source withGenerator:defaultJIT()];
}

- (BOOL) compileMethod: (NSString*)source
          onClassNamed: (NSString*)name
         withGenerator: (id<LKCodeGenerator>)cg
{
	id parser = AUTORELEASE([[[[self class] parserClass] alloc] init]);
	LKAST *ast;
	LKModule *module;
	NS_DURING
		ast = [parser parseMethod: source];
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
	if (success) 
	{
		[module compileWithGenerator: cg];
	}
	return success;
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
	NSString *f = [NSString stringWithFormat:@"./%@.framework", framework];
	NSBundle *bundle = nil;
	BOOL isDir = NO;
 
	if ([fm fileExistsAtPath:f isDirectory:&isDir] && isDir)
	{
		  bundle = [NSBundle bundleWithPath:f];
	}
	else
	{
		FOREACH(dirs, dir, NSString*)
		{
			f = [NSString stringWithFormat:@"%@/Frameworks/%@.framework",
									dir, framework];
			// Check that the framework exists and is a directory.
			isDir = NO;
			if ([fm fileExistsAtPath:f isDirectory:&isDir] && isDir)
			{
				bundle = [NSBundle bundleWithPath:f];
				break;
			}
		}
	}
	if (nil != bundle && [bundle load])
	{
		// If a framework header exists, then load it
		if (nil != collection)
		{
			f = [f stringByAppendingPathComponent: @"Headers"];
			f = [f stringByAppendingPathComponent: [framework stringByAppendingPathExtension: @"h"]];
			if ([fm fileExistsAtPath:f isDirectory:&isDir] && !isDir)
			{
				[collection sourceFileForPath: f];
			}
		}
		return [bundle bundlePath];
	}
	return nil;
}

static BOOL loadLibraryInPath(NSFileManager *fm, NSString *aLibrary, NSString *basePath)
{
	NSString *lib = [basePath stringByAppendingPathComponent: aLibrary];
	BOOL isDir = NO;
	if ([fm fileExistsAtPath: lib isDirectory:&isDir] && !isDir)
	{
		return dlopen([lib UTF8String], RTLD_GLOBAL) != NULL;
	}
	// Add .so to the end if it isn't there
	if (![@"so" isEqualToString: [aLibrary pathExtension]])
	{
		return loadLibraryInPath(fm, [aLibrary stringByAppendingPathExtension: @"so"], basePath);
	}
	if (([aLibrary length] < 4) || ![@"lib" isEqualToString: [aLibrary substringToIndex: 3]])
	{
		return loadLibraryInPath(fm, [@"lib" stringByAppendingString: aLibrary], basePath);
	}
	return NO;

}

+ (BOOL) loadLibrary: (NSString*)aLibrary
{
	// If SourceCodeKit is not loaded, give up
	if (nil == collection) { return NO; }

	NSFileManager *fm = [NSFileManager defaultManager];
	NSArray *dirs = NSSearchPathForDirectoriesInDomains(NSLibraryDirectory,
			NSAllDomainsMask, YES);
	NSBundle *bundle = nil;
	BOOL isDir = NO;
 
	// Check local paths
	if ([fm fileExistsAtPath: aLibrary isDirectory:&isDir] && !isDir)
	{
		return dlopen([aLibrary UTF8String], RTLD_GLOBAL) != NULL;
	}
	else
	{
		// See if it's installed in the GNUstep system
		for (NSString *dir in dirs)
		{
			NSString *path = [dir stringByAppendingPathComponent: @"Libraries"];
			if (loadLibraryInPath(fm, aLibrary, path))
			{
				return YES;
			}
		}
		// Check system include paths
		dirs = A(@"/usr/local/lib", @"/usr/lib");
		for (NSString *dir in dirs)
		{
			if (loadLibraryInPath(fm, aLibrary, dir))
			{
				return YES;
			}
		}
		// FIXME: Should respect LD_LIBRARY_PATH
	}
	return NO;
}

+ (BOOL) loadHeader: (NSString*)aHeader
{
	// If SourceCodeKit is not loaded, give up
	if (nil == collection) { return NO; }

	NSFileManager *fm = [NSFileManager defaultManager];
	NSArray *dirs = NSSearchPathForDirectoriesInDomains(NSLibraryDirectory,
			NSAllDomainsMask, YES);
	NSBundle *bundle = nil;
	BOOL isDir = NO;
 
	// Check local paths
	if ([fm fileExistsAtPath: aHeader isDirectory:&isDir] && !isDir)
	{
		[collection sourceFileForPath: aHeader];
		return YES;
	}
	else
	{
		// See if it's installed in the GNUstep system
		for (NSString *dir in dirs)
		{
			NSString *f = [dir stringByAppendingPathComponent: @"Headers"];
			f = [f stringByAppendingPathComponent: aHeader];
			// Check that the framework exists and is a directory.
			isDir = NO;
			if ([fm fileExistsAtPath:f isDirectory:&isDir] && !isDir)
			{
				[collection sourceFileForPath: f];
				return YES;
			}
		}
		// Check system include paths
		dirs = A(@"/usr/local/include", @"/usr/include");
		for (NSString *dir in dirs)
		{
			NSString *f = [dir stringByAppendingPathComponent: aHeader];
			// Check that the framework exists and is a directory.
			isDir = NO;
			if ([fm fileExistsAtPath:f isDirectory:&isDir] && !isDir)
			{
				[collection sourceFileForPath: f];
				return YES;
			}
		}
	}
	return NO;
}

+ (BOOL) loadFrameworkNamed:(NSString*)framework
{
	return nil != loadFramework(framework);
}

+ (BOOL)inDevMode
{
	return inDevMode;
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
	inDevMode = [[plist objectForKey: @"Development Mode"] boolValue];
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
	if (!(success &= loadAnyLibraryForBundle(bundle, recentModificationDate)))
	{
		success = YES;
		FOREACH(sourceFiles, source, NSString*)
		{
			success &= [self loadScriptNamed: source fromBundle: bundle];
		}
		// Spawn a new process to do the background JTL compile.
		if (success && fork() == 0)
		{
			[LKCompiler setDefaultDelegate: nil];
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
		FOREACH(plugins, p, NSString*)
		{
			NSString *plugin = [pluginDir stringByAppendingPathComponent: p];
			if ([fm fileExistsAtPath:plugin isDirectory:&isDir] && isDir &&
			    [@"lkplugin" isEqualToString:[plugin pathExtension]])
			{
				Class newclass = [self loadLanguageKitBundle:
					[NSBundle bundleWithPath:plugin]];
				if (newclass == [NSNull class])
				{
					success = NO;
				}
				else
				{
					// Create an instance of the new class that can perform any
					// loading code it needs in +initialize
					[[newclass alloc] init];
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
	return [self compileString:[NSString stringWithContentsOfFile:path]] != nil;
}

+ (BOOL) loadApplicationScriptNamed:(NSString*)fileName
{
	return [self loadScriptNamed: fileName fromBundle: [NSBundle mainBundle]];
}
+ (NSString*)typesForFunction: (NSString*)functionName
{
	return [[[collection functions] objectForKey: functionName] typeEncoding];
}
+ (NSString*)typesForGlobal: (NSString*)globalName
{
	return [[[collection globals] objectForKey: globalName] typeEncoding];
}
+ (id)valueOf: (NSString*)enumName inEnumeration: (NSString*)anEnumeration
{
	if (nil == anEnumeration)
	{
		return [[collection enumerationValues] objectForKey: enumName];
	}
	return [[[[collection enumerations] objectForKey: anEnumeration] values] objectForKey: enumName];
}

- (BOOL) loadApplicationScriptNamed:(NSString*)name
{
	return [self loadScriptNamed: name fromBundle: [NSBundle mainBundle]];
}
+ (void)setDefaultDelegate: (id<LKCompilerDelegate>)aDelegate
{
	ASSIGN(DefaultDelegate, aDelegate);
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
	@autoreleasepool {
		NSString *extension = [[self class] fileExtension];
		NSArray *scripts = [aBundle pathsForResourcesOfType:extension
		                                        inDirectory:nil];
		BOOL success = YES;
		FOREACH(scripts, scriptFile, NSString*)
		{
			NSString *script = [NSString stringWithContentsOfFile: scriptFile];
			success &= ([self compileString:script] != nil);
		}
		return success;
	}
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
