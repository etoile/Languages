#import "ObjCXXRuntime.h"
#import "LLVMCodeGen.h"
extern "C" {
#import <EtoileFoundation/EtoileFoundation.h>
#import <LanguageKit/LKCompiler.h>
#import <LanguageKit/LKAST.h>
#import <LanguageKit/LKModule.h>
}

//Don't use @class because clang will complain if we send messages to a
//forward-declared class.
@interface LLVMStaticCodeGen : LLVMCodeGen {}
- (id) initWithFile:(NSString*)file;
@end
/**
 * Look up the path of a binary in the user's path.
 */
static NSString *pathForExecutable(NSString *exe)
{
	NSString *pathString = 
		[[[NSProcessInfo processInfo] environment] objectForKey: @"PATH"];
	NSArray *paths = [pathString componentsSeparatedByString: @":"];
	NSFileManager *fm = [NSFileManager defaultManager];
	FOREACH(paths, p, NSString*)
	{
		NSString *path = [p stringByAppendingPathComponent: exe];
		if ([fm fileExistsAtPath: path])
		{
			return path;
		}
	}
	return nil;
}

/**
 * Runs cmd, an executable in the user's path, with args as arguments and waits
 * for it to exit.
 */
static BOOL run(NSString *cmd, NSArray *args)
{
	NSTask *task = [NSTask launchedTaskWithLaunchPath: pathForExecutable(cmd)
	                                        arguments: args];
	[task waitUntilExit];
	return [task terminationStatus] == 0;
}

@implementation LKCompiler (LLVM_JTL)

/**
 * Runs a sequence of LLVM commands that generate a native .so from the bitcode
 * files.  We could generate this in-process, but it seems safer and more
 * flexible to invoke external commands.
 */
+ (NSString*) linkBitcodeFiles: (NSMutableArray*) files outputDir: (NSString*)dir outputFile: (NSString*)fileName
{

#define ABS_PATH(x) [dir stringByAppendingPathComponent: x]
	// Add the small int message file.
	[files addObject: [LLVMCodeGen smallIntBitcodeFile]];
	[files addObject: @"-o"];
	[files addObject: ABS_PATH(@"bundle-bitcode.bc")];
	NSString *outFile = ABS_PATH(fileName);
	NSString *asmFile = [outFile stringByAppendingPathExtension: @"s"];
	// Link the bitcode files together
	if (run(@"llvm-link", files) &&
	    // Optimise the bitcode
	    run(@"opt", A(@"-O3", 
	                  @"-o", ABS_PATH(@"bundle-bitcode.optimised.bc"),
	                  @"-f", ABS_PATH(@"bundle-bitcode.bc"))) &&
	    // TODO: Change this to use @"-filetype=dynlib" when LLVM supports
	    // emitting .so files
	    run(@"llc", A(ABS_PATH(@"bundle-bitcode.optimised.bc"), @"-O3",
	                  @"-relocation-model=pic", @"-o", asmFile)) &&
	    // Until then, use GCC to generate the .so
	    run(@"clang", A(asmFile, @"-shared",
	                  @"-o", outFile)))
	{
		return outFile; 
	}
	return nil;
#undef ABS_PATH
}

+ (void) justTooLateCompileBundle: (NSBundle*)aBundle
{
	@autoreleasepool
	{
		[NSThread setThreadPriority: 0];
		NSFileManager *fm = [NSFileManager defaultManager];
		NSString *tempDirectory = [fm tempDirectory];

		NSString *path = [aBundle pathForResource:@"LKInfo" ofType:@"plist"];
		NSDictionary *plist = [NSDictionary dictionaryWithContentsOfFile: path];
		// TODO: Specify a set of AST transforms to apply.
		NSArray *sourceFiles = [plist objectForKey:@"Sources"];
		NSMutableArray *bitcodeFiles = [NSMutableArray array];
		FOREACH(sourceFiles, s, NSString*)
		{
			NSString *source = [aBundle pathForResource: s ofType: nil];
			NSString *outFile = 
				[tempDirectory stringByAppendingPathComponent: 
					[source lastPathComponent]];
			outFile = [outFile stringByAppendingPathExtension: @"bc"];
			NSString *code = [NSString stringWithContentsOfFile: source];
			LKAST *ast = [LKCompiler parseScript: code forFileExtension: [source pathExtension]];
			[ast check];
			//applyTransforms(ast);
			[ast compileWithGenerator: 
				[[LLVMStaticCodeGen alloc] initWithFile:outFile]];
			[bitcodeFiles addObject: outFile];
		}
		NSString * so = [LKCompiler linkBitcodeFiles: bitcodeFiles outputDir: tempDirectory outputFile: @"jtl.so"];
		if (nil != so)
		{
			// Try to move the generated library to somewhere where it can be found
			// the next time.
			NSString *path =
				[[[aBundle bundlePath] stringByAppendingPathComponent: @"Resources"]
					stringByAppendingPathComponent: @"languagekit-cache.so"];
			// Delete an old cache
			[fm removeFileAtPath: path handler: nil];
			// Install the new one in the bundle
			if ([fm movePath: so toPath: path handler: nil])
			{
				[fm removeFileAtPath: tempDirectory handler: nil];
				return;
			}
			NSArray *dirs = NSSearchPathForDirectoriesInDomains( NSLibraryDirectory,
				NSUserDomainMask, YES);
			NSString *userCache = [dirs objectAtIndex: 0];
			userCache = [[userCache stringByAppendingPathComponent: @"LKCaches"]
			stringByAppendingPathComponent: [aBundle bundlePath]];
			[fm   createDirectoryAtPath: userCache
			withIntermediateDirectories: YES
			                 attributes: nil
			                      error: NULL];
			// Delete an old cache
			[fm removeFileAtPath: userCache handler: nil];
			// This shouldn't fail, but if it does then we just give up and JIT
			// every time.
			[fm movePath: so toPath: userCache handler: nil];
			[fm removeFileAtPath: tempDirectory handler: nil];
		}
	}
}
@end
