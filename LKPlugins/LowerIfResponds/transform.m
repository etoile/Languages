#import <LanguageKit/LanguageKit.h>
/**
 * Transforms:
 *    [[foo ifResponds] bar];
 * To:
 *    if ([foo respondsToSelector: bar]) [foo bar];
 */
@interface LKLowerIfRespondsTransform : LKASTVisitor
@end
@implementation LKLowerIfRespondsTransform 
- (LKAST*)visitMessageSend: (LKMessageSend*)aNode
{
	LKAST *receiver = [aNode target];
	if (![receiver isKindOfClass: [LKMessageSend class]])
	{
		return aNode;
	}
	
	LKMessageSend *msgSend = (LKMessageSend*)receiver;
	if (![@"ifResponds" isEqualToString: [msgSend selector]])
	{
		return aNode;
	}

	receiver = [msgSend target];

	LKMessageSend *condition = 
		[LKMessageSend messageWithSelectorName: @"respondsToSelector:"];
	[condition setTarget: receiver];
	[condition addArgument: [LKSymbolRef referenceWithSymbol: [aNode selector]]];

	[aNode setTarget: receiver];

	LKIfStatement *ifStatement = 
		[LKIfStatement ifStatementWithCondition: condition
		                                   then: A(aNode)
		                                   else: [NSArray array]];

	[ifStatement setParent: [aNode parent]];
	[ifStatement check];
	return ifStatement;
}
@end
