#import <EtoileFoundation/EtoileFoundation.h>
#import "LKCompiler.h"
#import "LKAST.h"

// Semi-formal protocol for parsers.
@protocol LKParser
- (LKAST*) parseString:(NSString*)aProgram;
@end

static NSMutableDictionary *compilersByExtension;
static NSMutableDictionary *compilersByLanguage;

extern int DEBUG_DUMP_MODULES;
@implementation LKCompiler
+ (void) initialize 
{
	if (self != [LKCompiler class])
	{
		return;
	}
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
	[NSException raise:@"InstantiationError"
				format:@"LKCompiler instances are invalid"];
	return nil;
}
+ (void) setDebugMode:(BOOL)aFlag
{
	DEBUG_DUMP_MODULES = (int) aFlag;
}
+ (BOOL) compileString:(NSString*)s withGenerator:(id<LKCodeGenerator>)cg
{
	id p = [[[[self parser] alloc] init] autorelease];
	LKAST *ast;
	NS_DURING
		ast = [p parseString: s];
	NS_HANDLER
		NSDictionary *e = [localException userInfo];
		if ([[localException name] isEqualToString:@"ParseError"])
		{
			NSLog(@"Parse error on line %@.  Unexpected token at character %@ while parsing:\n%@",
										   [e objectForKey:@"lineNumber"],
										   [e objectForKey:@"character"],
										   [e objectForKey:@"line"]);
		}
		else
		{
			NSLog(@"Semantic error: %@", [localException reason]);
		}
		NS_VALUERETURN(NO, BOOL);
	NS_ENDHANDLER	
	[ast compileWith:cg];
	return YES;
}
+ (BOOL) compileString:(NSString*) source output:(NSString*)bitcode;
{
	id<LKCodeGenerator> cg = defaultStaticCompilterWithFile(bitcode);
	return [self compileString:source withGenerator:cg];
}
+ (BOOL) compileString:(NSString*)s
{
	return [self compileString:s withGenerator:defaultJIT()];
}

+ (BOOL) loadFramework:(NSString*)framework
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

+ (BOOL) loadScriptInBundle:(NSBundle*)bundle named:(NSString*)name
{
	NSString *path = [bundle pathForResource:name ofType:[self fileExtension]];
	if (nil == path)
	{
		NSLog(@"Unable to find %@.%@ in bundle %@.", name,
				[self fileExtension], bundle);
		return NO;
	}
	return [self compileString:[NSString stringWithContentsOfFile:path]];
}
+ (BOOL) loadApplicationScriptNamed:(NSString*)name
{
        return [self loadScriptInBundle: [NSBundle mainBundle] named: name];
}
+ (BOOL) loadScriptsInBundle:(NSBundle*) aBundle
{
	NSAutoreleasePool *pool = [NSAutoreleasePool new];
	NSArray *scripts = [aBundle pathsForResourcesOfType:[self fileExtension]
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
	NSAutoreleasePool *pool = [NSAutoreleasePool new];
	BOOL result = [self loadScriptsInBundle:[NSBundle mainBundle]];
	[pool release];
	return result;
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
+ (NSArray*) supportedLanguages
{
	return [compilersByLanguage allKeys];
}
+ (Class) compilerForLanguage:(NSString*) aLanguage
{
	return [compilersByLanguage objectForKey:aLanguage];
}
+ (Class) compilerForExtension:(NSString*) anExtension
{
	return [compilersByExtension objectForKey:anExtension];
}
+ (Class) parser
{
	return [self subclassResponsibility:_cmd];
	return Nil;
}
@end
