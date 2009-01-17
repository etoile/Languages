#import <LanguageKit/LanguageKit.h>

@interface LKCommentToLogTransform : LKASTVisitor
@end
@implementation LKCommentToLogTransform 
- (LKAST*) visitComment:(LKComment*)aNode
{
	LKMessageSend* msg = [LKMessageSend message: @"log"];
	NSString *string = [aNode stringValue];
	string = [@"Comment: " stringByAppendingString:string];
	[msg setTarget: [LKStringLiteral literalFromString:string]];
	[msg setParent: [aNode parent]];
	[msg check];
	return msg;
}
@end
