#import "SmalltalkKit.h"
#import <Foundation/Foundation.h>

extern int DEBUG_DUMP_MODULES;
@implementation SmalltalkCompiler
+ (id) alloc
{
	[NSException raise:@"InstantiationError"
				format:@"SmalltalkCompiler instances are invalid"];
	return nil;
}
+ (void) setDebugMode:(BOOL)aFlag
{
	DEBUG_DUMP_MODULES = (int) aFlag;
}
+ (BOOL) compileString:(NSString*)s
{
	Parser * p = [[Parser alloc] init];
	AST *ast;
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
	id cg = defaultCodeGenerator();
	[ast compileWith:cg];
	return YES;
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

+ (BOOL) loadApplicationScriptNamed:(NSString*)name
{
	NSString *path = [[NSBundle mainBundle] pathForResource:name
                                                         ofType:@"st"];
        if (nil == path)
        {
                NSLog(@"Unable to find %@.st in the application bundle.", name);
                return NO;
        }
        return [SmalltalkCompiler compileString: [NSString stringWithContentsOfFile: path]];
}
+ (BOOL) loadScriptsInBundle:(NSBundle*) aBundle
{
	NSAutoreleasePool *pool = [NSAutoreleasePool new];
	NSArray *scripts = [aBundle pathsForResourcesOfType:@"st" inDirectory:nil];
	BOOL success = YES;
	FOREACH(scripts, scriptFile, NSString*)
	{
		NSString *script = [NSString stringWithContentsOfFile: scriptFile];
		success &= [SmalltalkCompiler compileString:script];
	}
	[pool release];
	return success;
}
+ (BOOL) loadAllScriptsForApplication
{
	NSAutoreleasePool *pool = [NSAutoreleasePool new];
	BOOL result = [SmalltalkCompiler loadScriptsInBundle:[NSBundle mainBundle]];
	[pool release];
	return result;
}
@end
