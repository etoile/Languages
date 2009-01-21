#import <Foundation/Foundation.h>
#import "SmalltalkCompiler.h"

@class SmalltalkParser;

@implementation SmalltalkCompiler
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
