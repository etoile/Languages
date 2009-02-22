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
- (void*) compileWithGenerator: (id<LKCodeGenerator>)aGenerator
{
	void *retVal = [ret compileWithGenerator: aGenerator];
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
- (void)dealloc
{
	[ret release];
	[super dealloc];
}
@end
