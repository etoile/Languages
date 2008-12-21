#import "EScriptKit.h"
#import <Foundation/Foundation.h>

extern int DEBUG_DUMP_MODULES;
@implementation EScriptCompiler
+ (id) alloc
{
	return [super alloc];
}
+ (NSString*) languageName
{
	return @"EScript";
}
+ (NSString*) fileExtension
{
	return @"escript";
}
+ (Class) parser
{
	return [EScriptParser class];
}
@end
