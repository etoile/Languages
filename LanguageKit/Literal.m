#import "Literal.h"

@implementation LKLiteral
- (id) initWithString:(NSString*)aString
{
	SELFINIT;
	ASSIGN(value, aString);
	return self;
}
+ (id) literalFromString:(NSString*)aString
{
	return [[[self alloc] initWithString:aString] autorelease];
}
- (NSString*) description
{
	return value;
}
// No checking needed for literals - they are always semantically valid
- (void) check {}
@end

static NSDictionary *ObjCConstants;
@implementation LKNumberLiteral
+ (void) initialize
{
	NSString *plist = [[NSBundle bundleForClass:self]
		pathForResource:@"ObjCConstants" ofType:@"plist"];
	ObjCConstants = [[NSDictionary dictionaryWithContentsOfFile:plist] retain];
}
+ (id) literalFromSymbol:(NSString*)aString
{
	NSString *val = [ObjCConstants objectForKey:aString];
	if (nil != val)
	{
		return [self literalFromString:val];
	}
	[NSException raise:@"InvalidLiteral" format:@"Invalid symbolic constant %@",
		aString];
	// Not reached:
	return nil;
}
- (void*) compileWith:(id<LKCodeGenerator>)aGenerator
{
	return [aGenerator intConstant:value];
}
@end

@implementation LKStringLiteral
- (NSString*) description
{
	return [NSString stringWithFormat:@"'%@'", value];
}
- (void*) compileWith:(id<LKCodeGenerator>)aGenerator
{
	NSMutableString *escaped = [value mutableCopy];
	[escaped replaceOccurrencesOfString:@"\\n"
	                         withString:@"\n" 
	                            options:0
	                              range:NSMakeRange(0, [escaped length])];

	return [aGenerator stringConstant:escaped]; }
@end
