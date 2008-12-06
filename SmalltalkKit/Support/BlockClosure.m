#import <Foundation/Foundation.h>
#import "BlockClosure.h"

@interface BlockContext : NSObject {
@public
	BlockContext *parent;
	int count;
	char **symbolTable;
	id objects[0];
}
@end

@interface StackContext : BlockContext {}
@end

@interface RetainedStackContext : StackContext {}
@end

@implementation BlockContext
- (void) retainWithPointer:(BlockContext**)pointer
{
	[self retain];
}
- (void) dealloc
{
	for (unsigned i=0 ; i<count ; i++)
	{
		uintptr_t val = (uintptr_t)objects[i];
		if ((val & 1) == 0)
		{
			[objects[i] release];
		}
	}
	[super dealloc];
}
@end

@implementation StackContext
- (void) retainWithPointer:(BlockContext**)pointer
{
	isa = [RetainedStackContext class];
	id ***pointers = (id***)*(char **)self;
	pointers -= 1;
	if (NULL == *pointers)
	{
		*pointers = calloc(sizeof(id), 8);
		NSLog(@"Allocated array %x at %x", *pointers, pointers);
		(*pointers)[7] = (id*)-1;
	}
	unsigned i = 0;
	while (NULL != (*pointers)[i])
	{
		// If we come to the end of the array, resize it
		if (((id*)-1) == (*pointers)[i])
		{
			*pointers = realloc(*pointers, sizeof(id) * i * 2);
			for (unsigned j=i ; i<i*2 ; j++)
			{
				(*pointers)[j] = NULL;
			}
			(*pointers)[i*2 - 1] = (id*)-1;
			break;
		}
		i++;
	}
	(*pointers)[i] = pointer;
}
@end
@implementation RetainedStackContext
/**
 * Create a copy of this object on the heap.
 */
- (void) promote
{
	// Allocate the space
	BlockContext *block = (BlockContext*)NSAllocateObject([BlockContext class],
			count * sizeof(id), NSDefaultMallocZone());
	// Copy the data
	block->parent = parent;
	[parent retainWithPointer:&(block->parent)];
	block->count = count;
	block->symbolTable = symbolTable;
	memcpy(block->objects, objects, count * sizeof(id));
	// Update all of the pointers
	id ***pointers = (id***)*(char **)self;
	pointers -= 1;
	NSLog(@"Accessing array %x at %x", *pointers, pointers);
	for (unsigned i=0 ; (*pointers)[i] != NULL && (*pointers)[i] != ((id*)-1) ; i++)
	{
		*(*pointers)[i] = block;
	}
	// Retain all of the bound objects in this scope
	for (unsigned i=0 ; i<count ; i++)
	{
		uintptr_t val = (uintptr_t)block->objects[i];
		if ((val & 1) == 0)
		{
			block->objects[i] = [block->objects[i] retain];
		}
	}	
	if (NULL != *pointers)
	{
		free(*pointers);
	}
}
@end

@interface StackBlockClosure : BlockClosure {}
@end
@implementation StackBlockClosure
- (id) retain
{
	BlockClosure *block = [[BlockClosure alloc] init];
	block->function = function;
	block->args = args;
	block->context = context;
	[context retainWithPointer:&(block->context)];
	return block;
}
@end

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
- (id) whileTrue:(id)anotherBlock
{
	if (args > 0)
	{
		[NSException raise:@"InvalidBlockValueCall" format:@"Block expects %d arguments", args];
	}
	id last = nil;
	while(nil != function(self, _cmd))
	{
		last = [anotherBlock value];
	}
	return last;
}

- (id) on: (NSString*) exceptionName do: (BlockClosure*) handler
{
  NS_DURING
    NS_VALUERETURN([self value], id);
  NS_HANDLER
    if ([[localException name] isEqualToString: exceptionName])
      {
	return [handler value: localException];
      }
    else
      {
	[localException raise];
	return nil; // won't happen.
      }
  NS_ENDHANDLER
}
- (void) dealloc
{
	[context release];
	[super dealloc];
}
@end
