#import "SmalltalkKit.h"
#import <Foundation/Foundation.h>

extern int DEBUG_DUMP_MODULES;
@implementation SmalltalkCompiler
+ (id) alloc
{
	return [super alloc];
}
+ (NSString*) languageName
{
	return @"Smalltalk";
}
+ (NSString*) fileExtension
{
	return @"st";
}
+ (Class) parser
{
	return [SmalltalkParser class];
}
@end
