#import "LKBlockExpr.h"
#import "LKInterpretedBlockClosure.h"
#import "LKInterpreter.h"
#include <stdarg.h>

static id LKBlockFunction(id receiver, SEL cmd, ...)
{
	LKBlockExpr *blockAST = [receiver blockAST];
	LKInterpreterContext *context = [receiver interpreterContext];
	int count = [receiver argumentCount];
	id params[count];
	
	va_list arglist;
	va_start(arglist, cmd);
	for (int i = 0; i < count; i++)
	{
		params[i] = (id) va_arg(arglist, id);
	}
	va_end(arglist);
	
	return [blockAST executeWithArguments: params
	                                count: count
	                            inContext: context];
}

@implementation LKInterpretedBlockClosure : BlockClosure
- (id)initWithAST: (LKBlockExpr*)ast
    argumentNames: (NSArray*)argNames
    parentContext: (LKInterpreterContext*)parentContext
{
	SUPERINIT;
	blockAST = [ast retain];
	
	interpreterContext = [[LKInterpreterContext alloc]
				initWithSymbolTable: [ast symbols]
				             parent: parentContext];
	[interpreterContext setValue: self forSymbol: @"self"];
	args = [argNames count];
	function = LKBlockFunction;
	return self;
}

- (void)dealloc
{
	[interpreterContext release];
	[blockAST release];
	[super dealloc];
}
- (LKBlockExpr*)blockAST
{
	return blockAST;
}
- (LKInterpreterContext*)interpreterContext
{
	return interpreterContext;
}
@end
