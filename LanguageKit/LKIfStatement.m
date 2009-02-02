#import "LKIfStatement.h"
#import <EtoileFoundation/Macros.h>
#import "LKCodeGen.h"

@implementation LKIfStatement
- (LKIfStatement*) initWithCondition:(LKAST*) aCondition
                                then:(NSArray*)thenClause
                                else:(NSArray*)elseClause
{
	SELFINIT;
	ASSIGN(condition, aCondition);
	ASSIGN(thenStatements, [thenClause mutableCopy]);
	ASSIGN(elseStatements, [elseClause mutableCopy]);
	return self;
}
- (void) dealloc
{
	[condition release];
	[thenStatements release];
	[elseStatements dealloc];
	[super dealloc];
}
+ (LKIfStatement*) ifStatementWithCondition:(LKAST*) aCondition
                                       then:(NSArray*)thenClause
                                       else:(NSArray*)elseClause
{
	return [[[self alloc] initWithCondition: aCondition
	                                   then: thenClause
	                                   else: elseClause] autorelease];
}

- (void*) compileWith:(id<LKCodeGenerator>)aGenerator
{
	void *compareValue = [condition compileWith: aGenerator];
	void *startBB = [aGenerator currentBasicBlock];
	void *continueBB = [aGenerator startBasicBlock: @"if_continue"];
	// Emit the 'then' clause
	void *thenBB = [aGenerator startBasicBlock: @"if_then"];
	FOREACH(thenStatements, thenStatement, LKAST*)
	{
		[thenStatement compileWith: aGenerator];
	}
	[aGenerator goTo:continueBB];
	// Emit 'else' clause
	void *elseBB = [aGenerator startBasicBlock:@"if_else"];
	FOREACH(elseStatements, elseStatement, LKAST*)
	{
		[elseStatement compileWith: aGenerator];
	}
	[aGenerator goTo:continueBB];
	// Emit branch
	[aGenerator moveInsertPointToBasicBlock: startBB];
	[aGenerator branchOnCondition: compareValue true: thenBB false: elseBB];
	[aGenerator moveInsertPointToBasicBlock: continueBB];
	return NULL;
}
- (void) visitWithVisitor:(id<LKASTVisitor>)aVisitor
{
	id tmp = [aVisitor visitASTNode:condition];
	ASSIGN(condition, tmp);
	[condition visitWithVisitor:aVisitor];
	[self visitArray:thenStatements withVisitor:aVisitor];
	[self visitArray:elseStatements withVisitor:aVisitor];
}
- (void) check
{
	[condition setParent:self];
	[condition check];
	FOREACH(thenStatements, then, LKAST*)
	{
	}
	FOREACH(thenStatements, thenStatement, LKAST*)
	{
		[thenStatement setParent:self];
		[thenStatement check];
	}
	FOREACH(elseStatements, elseStatement, LKAST*)
	{
		[elseStatement setParent:self];
		[elseStatement check];
	}
}
@end
