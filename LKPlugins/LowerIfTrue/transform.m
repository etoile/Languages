#import <LanguageKit/LanguageKit.h>

NSArray *statementsFromBlock(LKAST*aBlock)
{
	if ([aBlock isKindOfClass: [LKBlockExpr class]])
	{
		LKBlockExpr *blockExpr = (LKBlockExpr*)aBlock;
		return [[blockExpr statements] copy];
	}
	LKMessageSend *valueMsg = [LKMessageSend message: @"value"];
	[valueMsg setTarget: aBlock];
	return A(valueMsg);
}	

@interface LKLowerIfTrueTransform : LKASTVisitor
@end
@implementation LKLowerIfTrueTransform 
- (LKAST*) visitMessageSend:(LKMessageSend*)aNode
{
	NSArray *thenClause = nil;
	NSArray *elseClause = nil;
	if ([@"ifTrue:ifFalse:" isEqualToString:[aNode selector]])
	{
		NSArray *args = [aNode arguments];
		thenClause = statementsFromBlock([args objectAtIndex:0]);
		elseClause = statementsFromBlock([args objectAtIndex:1]);
	}
	else if ([@"ifTrue:" isEqualToString:[aNode selector]])
	{
		NSArray *args = [aNode arguments];
		thenClause = statementsFromBlock([args objectAtIndex:0]);
	}
	else if ([@"ifFalse:" isEqualToString:[aNode selector]])
	{
		NSArray *args = [aNode arguments];
		elseClause = statementsFromBlock([args objectAtIndex:0]);
	}
	else 
	{
		return aNode;
	}
	LKAST *receiver = [aNode target];
	LKMessageSend *condition = [LKMessageSend message: @"boolValue"];
	[condition setTarget: receiver];

	LKIfStatement *ifStatement = 
		[LKIfStatement ifStatementWithCondition: receiver //condition
										   then: thenClause
										   else: elseClause];

	[ifStatement setParent: [aNode parent]];
	[ifStatement check];
	return ifStatement;
}
@end
