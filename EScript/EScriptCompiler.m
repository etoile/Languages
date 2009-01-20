#import <Foundation/Foundation.h>
#import "EScriptCompiler.h"

@class EScriptParser;

@implementation EScriptCompiler
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
