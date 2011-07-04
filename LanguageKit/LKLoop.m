#import "LKBlockExpr.h"
#import "LKLoop.h"
#import "LKMethod.h"

__thread void *unlabelledBreakBB;
__thread void *unlabelledContinueBB;

@implementation LKLoop
@synthesize initStatements;
+ (id) loopWithStatements:(NSMutableArray*)statementList
{
	return AUTORELEASE([[self alloc] initWithStatements:statementList]);
}
- (id) initWithStatements:(NSMutableArray*)statementList
{
	SELFINIT;
	DESTROY(label);
	DESTROY(initStatements);
	DESTROY(preCondition);
	ASSIGN(statements, statementList);
	DESTROY(postCondition);
	DESTROY(updateStatements);
	return self;
}
- (BOOL) check
{
	FOREACH(initStatements, initStatement, LKAST*)
	{
		[initStatement setParent:self];
		if (![initStatement check]) return NO;
	}
	if (preCondition)
	{
		[preCondition setParent:self];
		if (![preCondition check]) return NO;
	}
	FOREACH(statements, statement, LKAST*)
	{
		[statement setParent:self];
		if (![statement check]) return NO;
	}
	if (postCondition)
	{
		[postCondition setParent:self];
		if (![postCondition check]) return NO;
	}
	FOREACH(updateStatements, updateStatement, LKAST*)
	{
		[updateStatement setParent:self];
		if (![updateStatement check]) return NO;
	}
	return YES;
}
- (NSString*) description
{
	NSMutableString *str = [NSMutableString string];
	FOREACH(initStatements, initStatement, LKAST*)
	{
		[str appendString:[initStatement description]];
		[str appendString:@".\n"];
	}
	if (label)
	{
		[str appendFormat:@"%@: [\n", label];
	}
	else
	{
		[str appendString:@"[\n"];
	}
	if (preCondition)
	{
		[str appendFormat:@"(%@) ifFalse: [ break ].\n", preCondition];
	}
	FOREACH(statements, statement, LKAST*)
	{
		[str appendString:[statement description]];
		[str appendString:@".\n"];
	}
	if (postCondition)
	{
		[str appendFormat:@"(%@) ifFalse: [ break ].\n", postCondition];
	}
	FOREACH(updateStatements, updateStatement, LKAST*)
	{
		[str appendString:[updateStatement description]];
		[str appendString:@".\n"];
	}
	[str appendString:@"] loop"];
	return str;
}
- (void*) compileWithGenerator:(id<LKCodeGenerator>)aGenerator
{
	void *entryBB = [aGenerator currentBasicBlock];
	void *startBB = [aGenerator startBasicBlock: @"loop_start"];
	void *bodyBB = [aGenerator startBasicBlock: @"loop_body"];
	void *continueBB = [aGenerator startBasicBlock: @"loop_continue"];
	void *breakBB = [aGenerator startBasicBlock: @"loop_break"];
	void *oldBreakBB = NULL;
	void *oldContinueBB = NULL;
	void *oldUnlabelledBreakBB = unlabelledBreakBB;
	void *oldUnlabelledContinueBB = unlabelledContinueBB;
	unlabelledBreakBB = breakBB;
	unlabelledContinueBB = continueBB;
	NSString *breakLabel = nil;
	NSString *continueLabel = nil;
	if (label)
	{
		breakLabel = [@"break " stringByAppendingString: label];
		continueLabel = [@"continue " stringByAppendingString: label];
		oldBreakBB = [aGenerator basicBlockForLabel: breakLabel];
		oldContinueBB = [aGenerator basicBlockForLabel: continueLabel];
		[aGenerator setBasicBlock: breakBB forLabel: breakLabel];
		[aGenerator setBasicBlock: continueBB forLabel: continueLabel];
	}
	// Entry point
	[aGenerator moveInsertPointToBasicBlock: entryBB];
	FOREACH(initStatements, initStatement, LKAST*)
	{
		[initStatement compileWithGenerator: aGenerator];
	}
	[aGenerator goToBasicBlock: startBB];
	// Emit pre condition
	[aGenerator moveInsertPointToBasicBlock: startBB];
	if (preCondition)
	{
		void *preValue = [preCondition compileWithGenerator: aGenerator];
		[aGenerator branchOnCondition: preValue true: bodyBB
		                                       false: breakBB];
	}
	else
	{
		[aGenerator goToBasicBlock: bodyBB];
	}
	// Emit loop body
	[aGenerator moveInsertPointToBasicBlock: bodyBB];
	BOOL addTerminator = YES;
	FOREACH(statements, statement, LKAST*)
	{
		[statement compileWithGenerator: aGenerator];
		if ([statement isBranch])
		{
			addTerminator = NO;
			break;
		}
	}
	if (addTerminator)
	{
		if (postCondition)
		{
			void *postValue = [postCondition compileWithGenerator: aGenerator];
			[aGenerator branchOnCondition: postValue true: continueBB
			                                        false: breakBB];
		}
		else
		{
			[aGenerator goToBasicBlock: continueBB];
		}
	}
	// Emit continue block
	[aGenerator moveInsertPointToBasicBlock: continueBB];
	FOREACH(updateStatements, updateStatement, LKAST*)
	{
		[updateStatement compileWithGenerator: aGenerator];
	}
	[aGenerator goToBasicBlock: startBB];
	unlabelledBreakBB = oldUnlabelledBreakBB;
	unlabelledContinueBB = oldUnlabelledContinueBB;
	if (label)
	{
		[aGenerator setBasicBlock: oldBreakBB forLabel: breakLabel];
		[aGenerator setBasicBlock: oldContinueBB forLabel: continueLabel];
	}
	[aGenerator moveInsertPointToBasicBlock: breakBB];
	return NULL;
}
- (void) visitWithVisitor:(id<LKASTVisitor>)aVisitor
{
	[self visitArray: initStatements withVisitor: aVisitor];
	[preCondition visitWithVisitor: aVisitor];
	[self visitArray: statements withVisitor: aVisitor];
	[postCondition visitWithVisitor: aVisitor];
	[self visitArray: updateStatements withVisitor: aVisitor];
}
- (void) setLabel:(NSString*)aLabel
{
	ASSIGN(label, aLabel);
}
- (NSString*) label
{
	return label;
}
- (void) setStatements:(NSMutableArray*)anArray
{
	ASSIGN(statements, anArray);
}
- (NSMutableArray*) statements
{
	return statements;
}
- (void) setUpdateStatements:(NSMutableArray*)anArray
{
	ASSIGN(updateStatements, anArray);
}
- (NSMutableArray*) updateStatements
{
	return updateStatements;
}
- (void) setPreCondition:(LKAST*)condition
{
	ASSIGN(preCondition, condition);
}
- (LKAST*) preCondition
{
	return preCondition;
}
- (void) setPostCondition:(LKAST*)condition
{
	ASSIGN(postCondition, condition);
}
- (LKAST*) postCondition
{
	return postCondition;
}
- (void) dealloc
{
	DESTROY(label);
	DESTROY(initStatements);
	DESTROY(preCondition);
	DESTROY(statements);
	DESTROY(postCondition);
	DESTROY(updateStatements);
	[super dealloc];
}
@end

@implementation LKLoopFlowControl
- (id) initWithLabel:(NSString*)aLabel
{
	SELFINIT;
	ASSIGN(label, aLabel);
	return self;
}
- (BOOL) check
{
	for (id ast = [self parent]; ast; ast = [ast parent])
	{
		if ([ast isKindOfClass: [LKBlockExpr class]] ||
		    [ast isKindOfClass: [LKMethod class]])
		{
			break;
		}
		if ([ast isKindOfClass: [LKLoop class]])
		{
			if (label == nil || [[ast label] isEqualToString: label])
			{
				return YES;
			}
		}
	}
/*  MUST REPLACE THIS WITH THE NEW ERROR REPORTING STUFF:

	[NSException raise: @"SemanticError"
	            format: @"%@ statement outside of loop construct, or matching label not found.",
	                    [[self flowControlFlavor] capitalizedString]];
*/
	return NO;
}
- (NSString*) description
{
	NSString *str = [self flowControlFlavor];
	if (label)
	{
		return [NSString stringWithFormat: @"%@ %@", str, label];
	}
	return str;
}
- (void*) compileWithGenerator:(id<LKCodeGenerator>)aGenerator
{
	[aGenerator goToLabelledBasicBlock:
		[NSString stringWithFormat:@"%@ %@", [self flowControlFlavor], label]];
	return NULL;
}
- (BOOL) isBranch
{
	return YES;
}
- (NSString*) flowControlFlavor
{
	[self doesNotRecognizeSelector:_cmd];
	return nil;
}
- (void) dealloc
{
	DESTROY(label);
	[super dealloc];
}
@end

@implementation LKBreak
+ (id) breakWithLabel:(NSString*)aLabel
{
	return AUTORELEASE([[self alloc] initWithLabel: aLabel]);
}
- (void*) compileWithGenerator:(id<LKCodeGenerator>)aGenerator
{
	if (label)
	{
		return [super compileWithGenerator: aGenerator];
	}
	[aGenerator goToBasicBlock: unlabelledBreakBB];
	return NULL;
}
- (NSString*) flowControlFlavor
{
	return @"break";
}
@end

@implementation LKContinue
+ (id) continueWithLabel:(NSString*)aLabel
{
	return AUTORELEASE([[self alloc] initWithLabel: aLabel]);
}
- (void*) compileWithGenerator:(id<LKCodeGenerator>)aGenerator
{
	if (label)
	{
		return [super compileWithGenerator: aGenerator];
	}
	[aGenerator goToBasicBlock: unlabelledContinueBB];
	return NULL;
}
- (NSString*) flowControlFlavor
{
	return @"continue";
}
@end
