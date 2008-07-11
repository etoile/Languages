#import "Literal.h"

@implementation Literal
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

@implementation NumberLiteral
- (void*) compileWith:(id<CodeGenerator>)aGenerator
{
	return [aGenerator intConstant:value];
}
@end

@implementation StringLiteral
- (NSString*) description
{
	return [NSString stringWithFormat:@"'%@'", value];
}
- (void*) compileWith:(id<CodeGenerator>)aGenerator
{
	return [aGenerator stringConstant:value];
}
@end
