#import "BlockClosure.h"

@interface LKStackTraceArray : NSArray {
@public
	int count;
	id *buffer;
}
@end
@implementation LKStackTraceArray
- (NSInteger)count
{
	return count;
}
- (id)objectAtIndex: (NSInteger)index
{
	if (index > count) { return nil; }
	return buffer[index];
}
- (void)dealloc
{
	free(buffer);
	[super dealloc];
}
@end

static int getStackDirection(int *a)
{
	int b;
	printf("%d\n", a - &b);
	return a - &b > 0 ? 1 : -1;
}

typedef struct 
{
	@defs(BlockClosure);
} *BlockClosureIvars;

@class StackContext;
@class RetainedStackContext;
@class StackBlockClosure;

char *LanguageKitStackTopAddress;

static Class StackContextClass;
static Class RetainedStackContextClass;
static Class StackBlockClosureClass;
@implementation BlockClosure (Debug)
+ (void)load
{
	StackContextClass = [StackContext class];
	RetainedStackContextClass = [RetainedStackContext class];
	StackBlockClosureClass = [StackBlockClosure class];
}
+ (NSArray*)stackContexts;
{
	int a;
	ptrdiff_t offset = (ptrdiff_t)&(((BlockClosureIvars)0)->context);
	offset -= (ptrdiff_t)&(((BlockClosureIvars)0)->isa);

	int direction = getStackDirection(&a);

	id *buffer = calloc(8, sizeof(id));
	int count = 0;
	int buffersize = 8;

	char *foundcontext = (char*)&a;
	while(foundcontext < LanguageKitStackTopAddress)
	{
		if (((id)foundcontext)->class_pointer == StackContextClass
			|| ((id)foundcontext)->class_pointer == RetainedStackContextClass)
		{
			if (((id)(foundcontext - offset))->class_pointer == StackBlockClosureClass)
			{
				// Ignore the BlockClosure object for now. 
			}
			else
			{
				if (count >= buffersize)
				{
					buffersize *= 2;
					buffer = realloc(buffer, buffersize);
				}
				buffer[count++] = (id)foundcontext;
			}
		}
		// Stack grows down.
		foundcontext += direction;
	}
	LKStackTraceArray *array = [[[LKStackTraceArray alloc] init] autorelease];
	array->count = count;
	array->buffer = buffer;
	return array;
}
@end
