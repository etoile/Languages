#include <sys/types.h>
#include <string.h>

typedef struct
{
	void* isa;
	id(*value)(void*, SEL);
} Block;
/**
 * Function for sending messages to a small integer.  Selector is the constant
 * string representation of the selector.
 */
void * BinaryMessageSmallInt(void * obj, const char * sel, void * other)
{
	intptr_t val = (intptr_t)(unsigned long)obj;
	val >>= 1;
	long otherval = (intptr_t)(unsigned long)other;
	otherval >>= 1;
	if (strcmp(sel, "ifTrue:") == 0)
	{
		if (val == 0)
		{
			return 0;
		}
		else
		{
			Block *block = other;
			return block->value(other, @selector(value));
		}
	}
	switch(*sel)
	{
		case '+':
		{
			val += otherval;
			if(val>0xEFFFFFFF)
			{
				//TODO: Overflow
			}
			return (void*)((val << 1) | 1);
		}
		default:
    printf("Operation %s not yet supported on SmallInts\n", sel);
		//TODO: Promote to BigInt and try sending message?
		return 0;
	}
}

#define SCAST(x, y) if(strcmp(sel, "c" #x "Value") == 0) return (void*)(y)(long)obj;
#define UCAST(x, y) if(strcmp(sel, "cu" #x "Value") == 0) return (void*)(unsigned y)(unsigned long)obj;
#define NCAST(x, y) SCAST(x,y) UCAST(x,y)
#define CAST(x) NCAST(x,x)

@class NSString;
void * UnaryMessageSmallInt(void * obj, const char * sel)
{
	if(strcmp(sel, "stringValue") == 0) return (void*)[NSString stringWithFormat:@"%lld", (long long)(((intptr_t)obj)>>1)];
	if(strcmp(sel, "log") == 0) 
	{
		NSLog(@"<SmallInt:%d>", (long long)(((intptr_t)obj)>>1));
		return obj;
	}
	CAST(char);
	CAST(short);
	CAST(int);
	CAST(long);
	NCAST(longlong, long long);
  printf("Operation %s not yet supported on SmallInts\n", sel);
	//TODO: Promote to BigInt and try sending message?
	return obj;
}
