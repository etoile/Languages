#import "LKCompiler.h"
#import "LKCodeGen.h"

static Class defaultJitClass;
static Class defaultStaticClass;


@implementation LKCodeGenLoader
+ (void) initialize
{
	if (self == [LKCodeGenLoader class])
	{
		[LKCompiler loadFramework:@"LanguageKitCodeGen"];
		defaultJitClass = NSClassFromString(@"LLVMCodeGen");
		defaultStaticClass = NSClassFromString(@"LLVMStaticCodeGen");
	}
}
+ (id<LKCodeGenerator>) defaultJIT
{
	return [[defaultJitClass new] autorelease];
}
+ (id<LKStaticCodeGenerator>) defaultStaticCompilerWithFile:(NSString*)outFile
{
	return [[[defaultStaticClass alloc] initWithFile:outFile] autorelease];
}
@end
id <LKCodeGenerator> defaultJIT(void)
{
	return [LKCodeGenLoader defaultJIT];
}
id <LKCodeGenerator> defaultStaticCompilterWithFile(NSString* outFile)
{
	return [LKCodeGenLoader defaultStaticCompilerWithFile:outFile];
}
