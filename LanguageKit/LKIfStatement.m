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
	thenStatements = [thenClause mutableCopy];
	elseStatements = [elseClause mutableCopy];
	return self;
}
- (void) dealloc
{
	[condition release];
	[thenStatements release];
	[elseStatements release];
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

static void *emitBlock(id<LKCodeGenerator> aGenerator, 
                       NSArray *statements,
                       void *continueBB,
                       NSString *bbname)
{
	void *thenBB = [aGenerator startBasicBlock: bbname];
	BOOL addBranch = YES;
	FOREACH(statements, statement, LKAST*)
	{
		[statement compileWithGenerator:  aGenerator];
		if ([statement isBranch])
		{
			addBranch = NO;
			break;
		}
	}
	if (addBranch)
	{
		[aGenerator goToBasicBlock: continueBB];
	}
	return thenBB;
}

- (void*) compileWithGenerator: (id<LKCodeGenerator>)aGenerator
{
	void *compareValue = [condition compileWithGenerator:  aGenerator];
	void *startBB = [aGenerator currentBasicBlock];
	void *continueBB = [aGenerator startBasicBlock: @"if_continue"];
	// Emit the 'then' clause
	void *thenBB = 
		emitBlock(aGenerator, thenStatements, continueBB, @"if_then");
	void *elseBB = 
		emitBlock(aGenerator, elseStatements, continueBB, @"if_else");
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
