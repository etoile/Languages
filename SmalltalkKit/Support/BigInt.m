#include "BigInt.h"

static mpz_t ZERO;

@implementation BigInt
+ (void) initialize
{
	mpz_init_set_si(ZERO, 0);
}
+ (BigInt*) bigIntWithLongLong:(long long)aVal
{
	BigInt *b = [[[BigInt alloc] init] autorelease];
	//NSLog(@"Big int created for %lld", aVal);
	if (aVal < LONG_MAX && aVal > -LONG_MAX)
	{
		mpz_init_set_si(b->v, (long) aVal);
	}
	else
	{
		// FIXME: GMP must get code for initialising with 64-bit values soon.
		// When it does, replace this with something less ugly.
		uint32_t low = (uint32_t)aVal;
		int32_t high = (int32_t)(aVal >> 32);
		mpz_init_set_si(b->v, (long) high);
		mpz_mul_2exp(b->v, b->v, 32);
		mpz_add_ui(b->v, b->v, (unsigned long)low);
	}
	return b;
}
#define op2(name, func) \
- (BigInt*) name:(BigInt*)other\
{\
	BigInt *b = [[[BigInt alloc] init] autorelease];\
	mpz_init(b->v);\
	mpz_## func (b->v, v, other->v);\
	return b;\
}

#define op(name) op2(name, name)

op2(plus, add)
op(sub)
op(mul)
op(mod)
op2(div, tdiv_q)
- (BOOL) isLessThan:(id)other
{
	if ([other isKindOfClass:isa])
	{
		BigInt *o = other;
		return mpz_cmp(v, o->v) < 0;
	}
	if ([other respondsToSelector:@selector(intValue)])
	{
		return mpz_cmp_si(v, [other intValue]) < 0;
	}
	return NO;
}
- (BOOL) isGreaterThan:(id)other
{
	if ([other isKindOfClass:isa])
	{
		BigInt *o = other;
		return mpz_cmp(v, o->v) > 0;
	}
	if ([other respondsToSelector:@selector(intValue)])
	{
		return mpz_cmp_si(v, [other intValue]) > 0;
	}
	return NO;
}
- (BOOL) isEqual:(id)other
{
	if ([other isKindOfClass:isa])
	{
		BigInt *o = other;
		return mpz_cmp(v, o->v) == 0;
	}
	else if ([other respondsToSelector:@selector(longValue)])
	{
		long o = [other longValue];
		if (mpz_fits_slong_p(v))
		{
			return mpz_get_si(v) == o;
		}
	}
	return NO;
}
- (id) ifTrue:(id)t
{
	if (mpz_cmp(v, ZERO) != 0)
	{
		return [t value];
	}
	return nil;
}
- (id) ifFalse:(id)f
{
	if (mpz_cmp(v, ZERO) == 0)
	{
		return [f value];
	}
	return nil;
}
- (id) ifTrue:(id)t ifFalse:(id)f
{
	if (mpz_cmp(v, ZERO) == 0)
	{
		return [f value];
	}
	else
	{
		return [t value];
	}
	return nil;
}
- (id) timesRepeat:(id) aBlock
{
	id result = nil;
	if (mpz_fits_sint_p(v))
	{
		int  max = mpz_get_si(v);
		for (int i=0 ; i<max ; i++)
		{
			result = [aBlock value];
		}
	}
	else
	{
		//TODO: This is very slow, and can be optimised a lot
		mpz_t i;
		mpz_init_set(i, v);
		while(mpz_sgn(i) > 0)
		{
			result = [aBlock value];
			mpz_sub_ui(i, i, 1);
		}
	}
	return result;
}

- (NSString*) description
{
	char * cstr = mpz_get_str(NULL, 10, v);
	NSString *str = [NSString stringWithUTF8String:cstr];
	free(cstr);
	return str;
}
- (void) dealloc
{
	mpz_clear(v);
	[super dealloc];
}

#define CASTMETHOD(returnType, name, gmpFunction)\
- (returnType) name {\
	return (returnType) gmpFunction(v);\
}

CASTMETHOD(char, charValue, mpz_get_si)
CASTMETHOD(unsigned char, unsignedCharValue, mpz_get_ui)
CASTMETHOD(short int, shortValue, mpz_get_si)
CASTMETHOD(unsigned short int, unsignedShortValue, mpz_get_ui)
CASTMETHOD(int, intValue, mpz_get_si)
CASTMETHOD(unsigned int, unsignedIntValue, mpz_get_ui)
CASTMETHOD(long int, longValue, mpz_get_si)
CASTMETHOD(unsigned long int, unsignedLongValue, mpz_get_ui)
//FIXME: GMP doesn't have a function to get a long long int, so these methods aren't too useful.
CASTMETHOD(long long int, longLongValue, mpz_get_si)
CASTMETHOD(unsigned long long int, unsignedLongLongValue, mpz_get_ui)
CASTMETHOD(float, floatValue, mpz_get_d)
CASTMETHOD(double, doubleValue, mpz_get_d)
CASTMETHOD(BOOL, boolValue, mpz_get_ui)

@end
