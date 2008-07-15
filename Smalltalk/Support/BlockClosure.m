#import "BlockClosure.h"

@implementation BlockClosure
- (id) value
{
	if (args > 0)
	{
		[NSException raise:@"InvalidBlockValueCall" format:@"Block expects %d arguments", args];
	}
	return function(self, _cmd);
}
- (id) value:(id)a1
{
	if (args != 1)
	{
		[NSException raise:@"InvalidBlockValueCall" format:@"Block expects %d arguments", args];
	}
	return function(self, _cmd, a1);
}
- (id) value:(id)a1 value:(id)a2;
{
	if (args != 2)
	{
		[NSException raise:@"InvalidBlockValueCall" format:@"Block expects %d arguments", args];
	}
	return function(self, _cmd, a1, a2);
}
- (id) value:(id)a1 value:(id)a2 value:(id)a3;
{
	if (args != 3)
	{
		[NSException raise:@"InvalidBlockValueCall" format:@"Block expects %d arguments", args];
	}
	return function(self, _cmd, a1, a2, a3);
}
- (id) value:(id)a1 value:(id)a2 value:(id)a3 value:(id)a4;
{
	if (args != 4)
	{
		[NSException raise:@"InvalidBlockValueCall" format:@"Block expects %d arguments", args];
	}
	return function(self, _cmd, a1, a2, a3, a4);
}
@end

typedef struct 
{
	@defs(BlockClosure);
}
* BlockClosure_t;

static __thread BlockClosure *pool = nil;
BlockClosure *NewBlock(void)
{
	if (pool == NULL)
	{
		return [BlockClosure new];
	}
	BlockClosure *next = pool;
	pool = ((BlockClosure_t)pool)->bound[0];
	return next;
}
void FreeBlock(BlockClosure* aBlock)
{
	//TODO: blocks are never freed, and this leeks badly on thread destruction.
	((BlockClosure_t)aBlock)->bound[0] = pool;
	pool = aBlock;
}
