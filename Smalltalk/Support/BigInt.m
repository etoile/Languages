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

op(add)
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

@end
