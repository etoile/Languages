#import "BigInt.h"

@interface BigInt (Private)
- (LKObject)plus: (BigInt*)other;
- (LKObject)mul: (BigInt*)other;
@end

static char *ops[] = {"add", "subtract", "multiply"};

long long smalltalk_overflow_handler(long long val, long long otherval, char op,
		        char width)
{
	switch(op>>1)
	{
		case 1:
		{
			LKObject bigInt = 
				[[BigInt bigIntWithLongLong:((long long)val) >> 1]
				plus: [BigInt bigIntWithLongLong:((long long)otherval) >> 1]];
			return (long long)*(intptr_t*)&bigInt;
		}
		case 3:
		{
			LKObject bigInt = [[BigInt bigIntWithLongLong:((long long)val) >> 1]
				mul:[BigInt bigIntWithLongLong:((long long)otherval) >> 1]];
			// We set the low bit here so that we can use XOR to set it in the 
			return (long long)((*(intptr_t*)&bigInt) | 1);
		}
	}
	// Should never be reached.
	char *opname = ops[(op >>1) - 1];
	char *sign = op & 1 ? "signed" : "unsigned";
	fprintf(stderr, "Unexpected integer overflow in Smalltalk code!\n"
		  	"Op %s %s on %d-bit values, Aborting!\n", 
			sign, opname, (int)width);
	abort();
}

long long (*__overflow_handler)(long long a, long long b, char op, char width)
	    = smalltalk_overflow_handler;

