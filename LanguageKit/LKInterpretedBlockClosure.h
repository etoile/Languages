#import "Runtime/BlockClosure.h"
#import "LKInterpreter.h"

@interface LKInterpretedBlockClosure : BlockClosure
{
	LKBlockExpr *blockAST;
	LKInterpreterContext *interpreterContext;
}
- (id)initWithAST: (LKBlockExpr*)ast
    argumentNames: (NSArray*)argNames
    parentContext: (LKInterpreterContext*)parentContext;
- (LKBlockExpr*)blockAST;
- (LKInterpreterContext*)interpreterContext;
@end
