#import "LKReturn.h"

@implementation LKReturn
+ (id) returnWithExpr:(LKAST*)anExpression
{
	return [[[self alloc] initWithExpr: anExpression] autorelease];
}
- (id) initWithExpr:(LKAST*)anExpression
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
- (void*) compileWith:(id<LKCodeGenerator>)aGenerator
{
	void *retVal = [ret compileWith:aGenerator];
	[aGenerator setReturn:retVal];
	return retVal;
}
- (void) visitWithVisitor:(id<LKASTVisitor>)aVisitor
{
	id tmp = [aVisitor visitASTNode:ret];
	ASSIGN(ret, tmp);
	[ret visitWithVisitor:aVisitor];
}
- (LKAST*) expression
{
	return ret;
}
@end
