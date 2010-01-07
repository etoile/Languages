#import "LKLiteral.h"

@implementation LKLiteral
- (id) initWithString:(NSString*)aString
{
	SUPERINIT;
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
- (BOOL) check { return YES; }
- (void)dealloc
{
	[value release];
	[super dealloc];
}
@end

static NSDictionary *ObjCConstants;
@implementation LKNumberLiteral
+ (void) initialize
{
	if (self != [LKNumberLiteral class]) { return; }

	NSString *plist = [[NSBundle bundleForClass:self]
		pathForResource:@"ObjCConstants" ofType:@"plist"];
	ObjCConstants = [[NSDictionary dictionaryWithContentsOfFile:plist] retain];
}
+ (id) literalFromSymbol:(NSString*)aString
{
	NSString *val = [ObjCConstants objectForKey:aString];
	if (nil == val)
	{
		[NSException raise:@"InvalidLiteral" 
		            format:@"Invalid symbolic constant %@", aString];
	}
	return [LKNumberLiteral literalFromString:val];
}
- (void*) compileWithGenerator: (id<LKCodeGenerator>)aGenerator
{
	return [aGenerator intConstant:value];
}
@end

@implementation LKFloatLiteral
- (void*) compileWithGenerator: (id<LKCodeGenerator>)aGenerator
{
	return [aGenerator floatConstant: value];
}
@end

@implementation LKStringLiteral
- (NSString*) description
{
	return [NSString stringWithFormat:@"'%@'", value];
}
- (void*) compileWithGenerator: (id<LKCodeGenerator>)aGenerator
{
	NSMutableString *escaped = [value mutableCopy];
	[escaped replaceOccurrencesOfString:@"\\n"
	                         withString:@"\n" 
	                            options:0
	                              range:NSMakeRange(0, [escaped length])];
	[escaped replaceOccurrencesOfString:@"\\t"
	                         withString:@"\t" 
	                            options:0
	                              range:NSMakeRange(0, [escaped length])];

	void *ret = [aGenerator stringConstant:escaped];
	[escaped release];
	return ret;
}
@end
