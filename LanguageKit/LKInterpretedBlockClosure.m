#import "LKBlockExpr.h"
#import "LKInterpretedBlockClosure.h"
#import "LKInterpreter.h"
#include <stdarg.h>

static id LKBlockFunction(id receiver, SEL cmd, ...)
{
	LKBlockExpr *blockAST = [receiver blockAST];
	LKInterpreterContext *context = [[LKInterpreterContext alloc]
	            initWithSymbolTable: [blockAST symbols]
	                         parent: [receiver blockContext]];
	int count = [receiver argumentCount];
	id params[count];
	
	va_list arglist;
	va_start(arglist, cmd);
	for (int i = 0; i < count; i++)
	{
		params[i] = (id) va_arg(arglist, id);
	}
	va_end(arglist);
	
	@try
	{
		return [blockAST executeWithArguments: params
		                                count: count
		                            inContext: context];
	}
	@finally
	{
		[context release];
	}
}



@implementation LKInterpretedBlockClosure
- (id)initWithAST: (LKBlockExpr*)ast
    argumentNames: (NSArray*)argNames
    parentContext: (LKInterpreterContext*)parentContext
{
	SUPERINIT;
	blockAST = [ast retain];
	
	ASSIGN(context, (BlockContext*)parentContext);
	args = [argNames count];
	function = LKBlockFunction;
	return self;
}

- (void)dealloc
{
	[blockAST release];
	[super dealloc];
}
- (LKBlockExpr*)blockAST
{
	return blockAST;
}
@end
