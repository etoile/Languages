#import "Comparison.h"

@implementation Compare
+ (Compare*) compare:(AST*)expr1 to:(AST*)expr2
{
	return [[[Compare alloc] initComparing:expr1 to:expr2] autorelease];
}
- (id) initComparing:(AST*)expr1 to:(AST*)expr2
{
	SELFINIT;
	ASSIGN(lhs, expr1);
	ASSIGN(rhs, expr2);
	return self;
}
- (NSString*) description
{
	return [NSString stringWithFormat:@"%@ = %@", lhs, rhs];
}
- (void) check
{
	[lhs setParent:self];
	[rhs setParent:self];
	[lhs check];
	[rhs check];
}
- (void*) compileWith:(id<CodeGenerator>)aGenerator
{
	void * l = [lhs compileWith:aGenerator];
	void * r = [rhs compileWith:aGenerator];
	return [aGenerator comparePointer:l to:r];
}
@end
