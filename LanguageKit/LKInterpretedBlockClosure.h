#import "Runtime/BlockClosure.h"
#import "LKInterpreter.h"

@interface LKInterpretedBlockClosure : BlockClosure
{
	LKBlockExpr *blockAST;
}
- (id)initWithAST: (LKBlockExpr*)ast
    argumentNames: (NSArray*)argNames
    parentContext: (LKInterpreterContext*)parentContext;
- (LKBlockExpr*)blockAST;
@end
