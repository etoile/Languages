#import "SmalltalkKit.h"
#import <Foundation/Foundation.h>

extern int DEBUG_DUMP_MODULES;
@implementation SmalltalkCompiler
+ (NSString*) fileExtension
{
	return @"st";
}
+ (Class) parser
{
	return [SmalltalkParser class];
}
@end
