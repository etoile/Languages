#import "LKASTVisitor.h"

#define DEFCLASS(x) \
@class x;\
Class x ## Class;
DEFCLASS(LKArrayExpr);
DEFCLASS(LKAssignExpr);
DEFCLASS(LKBlockExpr);
DEFCLASS(LKCategoryDef);
DEFCLASS(LKComment);
DEFCLASS(LKCompare);
DEFCLASS(LKDeclRef);
DEFCLASS(LKIfStatement);
DEFCLASS(LKLiteral);
DEFCLASS(LKMessageSend);
DEFCLASS(LKMessageCascade);
DEFCLASS(LKMethod);
DEFCLASS(LKCompilationUnit);
DEFCLASS(LKReturn);
DEFCLASS(LKSubclass);
DEFCLASS(LKVariableDecl);

@implementation LKASTVisitor
#define SETCLASS(x) do {x ## Class = [x class];} while(0)
+ (void) initialize
{
	if (self == [LKASTVisitor class])
	{
		SETCLASS(LKArrayExpr);
		SETCLASS(LKAssignExpr);
		SETCLASS(LKBlockExpr);
		SETCLASS(LKCategoryDef);
		SETCLASS(LKComment);
		SETCLASS(LKCompare);
		SETCLASS(LKDeclRef);
		SETCLASS(LKIfStatement);
		SETCLASS(LKLiteral);
		SETCLASS(LKMessageSend);
		SETCLASS(LKMessageCascade);
		SETCLASS(LKMethod);
		SETCLASS(LKCompilationUnit);
		SETCLASS(LKReturn);
		SETCLASS(LKSubclass);
		SETCLASS(LKVariableDecl);
	}
}

#define TRYCLASS(x)\
	if ([aNode isKindOfClass: LK ## x ## Class]) \
	{\
		SEL sel = @selector(visit ## x:);\
		if ([self respondsToSelector:sel])\
		{\
			return [self performSelector:sel withObject:aNode];\
		}\
		return aNode;\
	}

- (LKAST*) visitASTNode:(LKAST*)aNode
{
		TRYCLASS(ArrayExpr);
		TRYCLASS(AssignExpr);
		TRYCLASS(BlockExpr);
		TRYCLASS(CategoryDef);
		TRYCLASS(Comment);
		TRYCLASS(Compare);
		TRYCLASS(DeclRef);
		TRYCLASS(IfStatement);
		TRYCLASS(Literal);
		TRYCLASS(MessageSend);
		TRYCLASS(MessageCascade);
		TRYCLASS(Method);
		TRYCLASS(CompilationUnit);
		TRYCLASS(Return);
		TRYCLASS(Subclass);
		TRYCLASS(VariableDecl);
		NSLog(@"Unrecognised AST node type: %@", [aNode class]);
		return aNode;
}
@end
