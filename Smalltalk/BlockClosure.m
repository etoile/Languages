#import "BlockClosure.h"

@implementation BlockClosure
- (id) value
{
	NSLog(@"Evaluating block.  Function is %x", function);
	//NSLog(@"Bound values are: %@, %@, %@, %@, %@", bound[0], bound[1],bound[2],bound[3],bound[4]);
	if (args > 0)
	{
		[NSException raise:@"InvalidBlockValueCall" format:@"Block expects %d arguments", args];
	}
	return function(self, _cmd);
}
@end

static __thread BlockClosure *pool = nil;
BlockClosure *NewBlock(void)
{
	if (pool == NULL)
	{
		return [BlockClosure new];
	}
	BlockClosure *next = pool;
	pool = pool->bound[0];
	return next;
}
void FreeBlock(BlockClosure* aBlock)
{
	//TODO: blocks are never freed, and this leeks badly on thread destruction.
	aBlock->bound[0] = pool;
	pool = aBlock;
}
