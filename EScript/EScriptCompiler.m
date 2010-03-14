#import <Foundation/Foundation.h>
#import "EScriptCompiler.h"
#import "EScriptParser.h"

@implementation EScriptCompiler
+ (NSString*) languageName
{
	return @"EScript";
}
+ (NSString*) fileExtension
{
	return @"escript";
}
+ (Class) parserClass
{
	return [EScriptParser class];
}
@end
