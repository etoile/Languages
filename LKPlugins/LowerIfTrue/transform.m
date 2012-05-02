#import <LanguageKit/LanguageKit.h>
#import <EtoileFoundation/EtoileFoundation.h>

NSArray *statementsFromBlock(LKAST*aBlock)
{
	if ([aBlock isKindOfClass: [LKBlockExpr class]])
	{
		LKBlockExpr *blockExpr = (LKBlockExpr*)aBlock;
		return [[[blockExpr statements] copy] autorelease];
	}
	LKMessageSend *valueMsg = [LKMessageSend messageWithSelectorName: @"value"];
	[valueMsg setTarget: aBlock];
	return A(valueMsg);
}
@interface LKCollectExternalRefs : LKASTVisitor
{
	@public
	NSMutableSet *refs;
}
@end
@implementation LKCollectExternalRefs
- (LKAST*)visitDeclRef: (LKDeclRef*)aDeclRef
{
	[refs addObject: [[aDeclRef symbol] stringValue]];
	return aDeclRef;
}
@end

@interface LKLowerIfTrueTransform : LKASTVisitor
@end
@implementation LKLowerIfTrueTransform 
- (LKAST*) visitMessageSend:(LKMessageSend*)aNode
{
	LKBlockExpr *thenBlock = nil;
	LKBlockExpr *elseBlock = nil;
	if ([@"ifTrue:ifFalse:" isEqualToString:[aNode selector]])
	{
		NSArray *args = [aNode arguments];
		thenBlock = [args objectAtIndex:0];
		elseBlock = [args objectAtIndex:1];
	}
	else if ([@"ifTrue:" isEqualToString:[aNode selector]])
	{
		NSArray *args = [aNode arguments];
		thenBlock = [args objectAtIndex:0];
	}
	else if ([@"ifFalse:" isEqualToString:[aNode selector]])
	{
		NSArray *args = [aNode arguments];
		elseBlock = [args objectAtIndex:0];
	}
	else 
	{
		return aNode;
	}

	// Remove references to symbols from the blocks we're deleting
	LKCollectExternalRefs *exts = [LKCollectExternalRefs new];
	LKSymbolTable *symTab = [aNode symbols];
	NSMutableSet *refs = [NSMutableSet new];
	exts->refs = refs;
	[thenBlock visitWithVisitor: exts];
	for (NSString *sym in refs)
	{
		LKSymbol *s = [symTab symbolForName: sym];
		s.referencingScopes--;
	}
	[refs removeAllObjects];
	[elseBlock visitWithVisitor: exts];
	for (NSString *sym in refs)
	{
		LKSymbol *s = [symTab symbolForName: sym];
		s.referencingScopes--;
	}
	[refs release];
	[exts release];

	NSArray *thenClause = statementsFromBlock(thenBlock);
	NSArray *elseClause = statementsFromBlock(elseBlock);
	LKAST *receiver = [aNode target];
	LKMessageSend *condition = 
		[LKMessageSend messageWithSelectorName: @"boolValue"];
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
