#import <Foundation/Foundation.h>
#import "BlockClosure.h"
#import "BlockContext.h"

// Small int 0 is ((0 << 1) & 1).  
#define IS_NIL_OR_ZERO(x) (((uintptr_t)(x)) < 2)

NSString *LKSmalltalkBlockNonLocalReturnException =
    @"LKSmalltalkBlockNonLocalReturnException";

@implementation BlockContext
- (BlockContext*)parent
{
	return parent;
}
- (BOOL)setValue: (id)aValue forSymbol: (NSString*)aSymbol
{
	// Symbol table doesn't contain the first bound var, which is always the
	// self symbol.
	if ([aSymbol isEqualToString: @"self"])
	{
		objects[0] = aValue;
		return YES;
	}

	if (NULL == symbolTable) { return NO; }

	const char *sym = [aSymbol UTF8String];
	for (unsigned int i=0 ; i<=count-1 ; i++)
	{
		if (strcmp(sym, symbolTable[i]) == 0)
		{
			objects[i+1] = aValue;
			return YES;
		}
	}
	return NO;
}
- (id)valueForSymbol: (NSString*)aSymbol
{
	if ([aSymbol isEqualToString: @"self"])
	{
		return objects[0];
	}

	if (NULL == symbolTable) { return nil; }

	const char *sym = [aSymbol UTF8String];
	for (unsigned int i=0 ; i<count-1 ; i++)
	{
		if (strcmp(sym, symbolTable[i]) == 0)
		{
			return objects[i+1];
		}
	}
	return nil;
}
- (NSArray*)symbols
{
	NSMutableArray *array = [NSMutableArray array];
	[array addObject: @"self"];

	if (NULL == symbolTable) { return array; }

	for (unsigned int i=0 ; i<count-1 ; i++)
	{
		[array addObject: [NSString stringWithUTF8String: symbolTable[i]]];
	}
	return array;
}
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
	id ***pointers = (id***)self;
	pointers -= 1;
	if (NULL == *pointers)
	{
		*pointers = calloc(sizeof(id), 8);
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
	id ***pointers = (id***)self;
	pointers -= 1;
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
void __LanguageKitThrowNonLocalReturn(void *context, void *retval);
- (void) nonLocalReturn:(id) anObject
{
	//TODO: We can do this without the loop if we do it in the compiler.  Is
	//that worth bloating the code and churning the icache?  Probably not -
	//test it later.
	BlockContext *aContext = context;
	while (aContext != nil && nil != aContext->parent)
	{
		aContext = aContext->parent;
	}
	__LanguageKitThrowNonLocalReturn(aContext, anObject);
}
@end


@implementation BlockClosure
- (void) nonLocalReturn:(id) anObject
{
	[NSException raise:@"NoSkillException!"
	            format:@"You fail."];
}
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
	for (id ret = function(self, _cmd) ;
		(uintptr_t)ret != 1 && [ret boolValue] ;
		ret = function(self, _cmd)) 
	{
		last = [anotherBlock value];
	}
	return last;
}
- (int32_t) argumentCount
{
	return args;
}
- (int)intValue
{
	return [[self value] intValue];
}
- (void) dealloc
{
	[context release];
	[super dealloc];
}
@end
