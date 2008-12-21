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
	ASSIGN(thenStatements, thenClause);
	ASSIGN(elseStatements, elseClause);
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
	void *startBB = [aGenerator currentBasicBlock];
	void *compareValue = [condition compileWith: aGenerator];
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
@end
