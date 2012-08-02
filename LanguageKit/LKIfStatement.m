#import "LKIfStatement.h"
#import <EtoileFoundation/Macros.h>
#import "LKCodeGen.h"

@implementation LKIfStatement
- (LKIfStatement*) initWithCondition:(LKAST*) aCondition
                                then:(NSArray*)thenClause
                                else:(NSArray*)elseClause
{
	SUPERINIT;
	ASSIGN(condition, aCondition);
	thenStatements = [thenClause mutableCopy];
	elseStatements = [elseClause mutableCopy];
	return self;
}
+ (LKIfStatement*) ifStatementWithCondition:(LKAST*) aCondition
                                       then:(NSArray*)thenClause
                                       else:(NSArray*)elseClause
{
	return [[self alloc] initWithCondition: aCondition
	                                   then: thenClause
	                                   else: elseClause];
}

static void *emitBlock(id<LKCodeGenerator> aGenerator, 
                       NSArray *statements,
                       void *continueBB,
                       NSString *bbname)
{
	void *bb = [aGenerator startBasicBlock: bbname];
	FOREACH(statements, statement, LKAST*)
	{
		[statement compileWithGenerator: aGenerator];
		if ([statement isBranch])
		{
			return bb;
		}
	}
	[aGenerator goToBasicBlock: continueBB];
	return bb;
}

- (void*) compileWithGenerator: (id<LKCodeGenerator>)aGenerator
{
	void *compareValue = [condition compileWithGenerator: aGenerator];
	void *startBB = [aGenerator currentBasicBlock];
	void *continueBB = [aGenerator startBasicBlock: @"if_continue"];
	// Emit 'then' and 'else' clauses
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
- (NSString*) description
{
	NSMutableString *str = [NSMutableString string];
	[str appendFormat:@"(%@)", condition];
	if (thenStatements)
	{
		[str appendString:@" ifTrue: [\n"];
		FOREACH(thenStatements, thenStatement, LKAST*)
		{
			[str appendString:[thenStatement description]];
			[str appendString:@".\n"];
		}
		[str appendString:@"]"];
	}
	if (elseStatements)
	{
		[str appendString:@" ifFalse: [\n"];
		FOREACH(elseStatements, elseStatement, LKAST*)
		{
			[str appendString:[elseStatement description]];
			[str appendString:@".\n"];
		}
		[str appendString:@"]"];
	}
	return str;
}
- (BOOL)check
{
	[condition setParent:self];
	BOOL success = [condition check];
	FOREACH(thenStatements, thenStatement, LKAST*)
	{
		[thenStatement setParent:self];
		success &= [thenStatement check];
	}
	FOREACH(elseStatements, elseStatement, LKAST*)
	{
		[elseStatement setParent:self];
		success &= [elseStatement check];
	}
	return success;
}
@end
