#import "Return.h"

@implementation Return
+ (id) returnWithExpr:(AST*)anExpression
{
	return [[[self alloc] initWithExpr: anExpression] autorelease];
}
- (id) initWithExpr:(AST*)anExpression
{
	SELFINIT;
	ASSIGN(ret, anExpression);
	return self;
}
- (void) check
{
	[ret setParent:self];
	[ret check];
}
- (NSString*) description
{
	return [NSString stringWithFormat:@"^%@.", ret];
}
- (void*) compileWith:(id<CodeGenerator>)aGenerator
{
	void *retVal = [ret compileWith:aGenerator];
	[aGenerator setReturn:retVal];
	return retVal;
}
@end
