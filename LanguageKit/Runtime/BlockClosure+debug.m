#define __XSI_VISIBLE 600
#import "BlockClosure.h"
#import "BlockContext.h"
#include <signal.h>
#include <ucontext.h>


@interface LKStackTraceArray : NSArray {
@public
	int count;
	id *buffer;
}
@end
@implementation LKStackTraceArray
- (NSUInteger)count
{
	return count;
}
- (id)objectAtIndex: (NSUInteger)index
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

static __thread volatile char fellOffStack;
static __thread ucontext_t sigretcontext;

static void segv(int sig, siginfo_t *info, void *addr)
{
	fellOffStack = 1;
	setcontext(&sigretcontext);
}

static int getStackDirection(int *a)
{
	int b;
	return a - &b > 0 ? 1 : -1;
}

typedef struct 
{
	@defs(BlockClosure);
} *BlockClosureIvars;

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
	volatile int count = 0;
	volatile int buffersize = 8;

	char *foundContext = (char*)&a;
	// Set the SegV handler.
	struct sigaction new;
	new.sa_sigaction = segv;
	new.sa_flags = SA_SIGINFO;
	sigemptyset (&new.sa_mask);
	struct sigaction old;
	sigaction(SIGSEGV, &new, &old);
	fellOffStack = 0;
	// Save the context.  We'll return here after the segfault
	getcontext(&sigretcontext);
	// This flag is set to 1 if a SegV occurs
	while(!fellOffStack)
	{
		if (((Class)((id)foundContext)->class_pointer == StackContextClass
			|| (Class)((id)foundContext)->class_pointer == RetainedStackContextClass))
		{
			if ((Class)((id)(foundContext - offset))->class_pointer == StackBlockClosureClass)
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
				buffer[count++] = (id)foundContext;
			}
		}
		// Walk up the stack until we fall off
		foundContext += direction;
	}
	// Reset the SegV handler.
	sigaction(SIGSEGV, &old, NULL);
	LKStackTraceArray *array = [[[LKStackTraceArray alloc] init] autorelease];
	array->count = count;
	array->buffer = buffer;
	return array;
}
@end
