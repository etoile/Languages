#import "LKReturn.h"

@implementation LKReturn
+ (id) returnWithExpr:(LKAST*)anExpression
{
	return [[[self alloc] initWithExpr: anExpression] autorelease];
}
- (id) initWithExpr:(LKAST*)anExpression
{
	SUPERINIT;
	ASSIGN(ret, anExpression);
	return self;
}
- (BOOL)check
{
	[ret setParent:self];
	return [ret check];
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
- (BOOL) isBranch
{
	return YES;
}
- (void)dealloc
{
	[ret release];
	[super dealloc];
}
@end

@implementation LKBlockReturn
- (void*) compileWithGenerator: (id<LKCodeGenerator>)aGenerator
{
	void *retVal = [ret compileWithGenerator: aGenerator];
	[aGenerator blockReturn:retVal];
	return retVal;
}
@end
