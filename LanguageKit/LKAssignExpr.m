#import "LKAssignExpr.h"
#import "LKDeclRef.h"

@implementation LKAssignExpr
+ (void) initialize
{
	if (self != [LKAssignExpr class]) { return; }
}
+ (id) assignWithTarget:(LKDeclRef*)aTarget expr:(LKAST*)expression
{
	return [[[self alloc] initWithTarget:aTarget expr:expression] autorelease];
}
- (id) initWithTarget:(LKDeclRef*)aTarget expr:(LKAST*)expression
{
	SUPERINIT;
	ASSIGN(target, aTarget);
	ASSIGN(expr, expression);
	return self;
}
- (BOOL)check
{
	[expr setParent:self];
	[target setParent:self];
	BOOL check = [target check] && [expr check];
	/*
	if (check && [[target->symbol typeEncoding] characterAtIndex: 0] != '@')
	{
		[NSException raise: @"InvalidAssignmentException"
					format: @"Can not yet generate code for assignment"];
	}
	*/
	return check;
}
- (NSString*) description
{
	return [NSString stringWithFormat:@"%@ := %@", target->symbol, expr];
}
- (void*) compileWithGenerator: (id<LKCodeGenerator>)aGenerator
{
	void * rval = [expr compileWithGenerator: aGenerator];
	LKSymbol *symbol = target->symbol;
	switch([symbol scope])
	{
		case LKSymbolScopeLocal:
		case LKSymbolScopeExternal:
		case LKSymbolScopeArgument:
		case LKSymbolScopeObject:
		case LKSymbolScopeClass:
			[aGenerator storeValue: rval inVariable: symbol];
			break;
		case LKSymbolScopeBuiltin:
		case LKSymbolScopeGlobal:
		default:
			return [super compileWithGenerator: aGenerator];
	}
	// Assignments aren't expressions in Smalltalk, but they might be in some
	// other language that wants to use this code and it doesn't cost more than
	// returning NULL.
	return rval;
}
- (void) visitWithVisitor:(id<LKASTVisitor>)aVisitor
{
	id tmp = [aVisitor visitASTNode:target];
	ASSIGN(target, tmp);
	[target visitWithVisitor:aVisitor];

	tmp = [aVisitor visitASTNode:expr];
	ASSIGN(expr, tmp);
	[expr visitWithVisitor:aVisitor];
}
- (LKDeclRef*) target
{
	return target;
}
- (LKAST*) expression
{
	return expr;
}
- (void)dealloc
{
	[target release];
	[expr release];
	[super dealloc];
}
@end
