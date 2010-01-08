#define class_pointer isa
#import "BigInt.h"
#import "BlockClosure.h"

static mpz_t ZERO;
Class BigIntClass;

static BigInt *BigIntYES;
static BigInt *BigIntNO;

@implementation BigInt
+ (void) initialize
{
	if ([BigInt class] == self)
	{
		BigIntClass = self;
		mpz_init_set_si(ZERO, 0);
		BigIntNO = [[BigInt bigIntWithLongLong: 0] retain];
		BigIntYES = [[BigInt bigIntWithLongLong: 1] retain];
	}
}
+ (BigInt*) bigIntWithCString:(const char*) aString
{
	BigInt *b = [[[BigInt alloc] init] autorelease];
	mpz_init_set_str(b->v, aString, 10);
	return b;
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
+ (BigInt*) bigIntWithLong:(long)aVal
{
	BigInt *b = [[[BigInt alloc] init] autorelease];
	mpz_init_set_si(b->v, aVal);
	return b;
}
+ (BigInt*) bigIntWithUnsignedLong:(unsigned long)aVal
{
	BigInt *b = [[[BigInt alloc] init] autorelease];
	mpz_init_set_ui(b->v, aVal);
	return b;
}

+ (BigInt*) bigIntWithMP:(mpz_t)aVal
{
	BigInt *b = [[[BigInt alloc] init] autorelease];
	mpz_init_set(b->v, aVal);
	return b;
}
#define op2(name, func) \
- (BigInt*) name:(id)other\
{\
	BigInt *b = [[[BigInt alloc] init] autorelease];\
	mpz_init(b->v);\
	if (other->isa == BigIntClass || [other isKindOfClass: BigIntClass])\
	{\
		mpz_## func (b->v, v, ((BigInt*)other)->v);\
	}\
	else\
	{\
		mpz_t number;\
		mpz_init_set_si(number, [other intValue]);\
		mpz_## func (b->v, number, v);\
		mpz_clear(number);\
	}\
	return b;\
}

#define op(name) op2(name, name)

op2(plus, add)
op(sub)
op(mul)
op(mod)
op2(div, tdiv_q)
- (id)and: (id)a
{
	return mpz_get_si(v) && [a intValue] ? BigIntYES : BigIntNO;
}
- (id)or: (id)a;
{
	return mpz_get_si(v) || [a intValue] ? BigIntYES : BigIntNO;
}
op2(bitwiseAnd, and);
op2(bitwiseOr, ior);
- (id)not
{
	if (mpz_cmp(v, ZERO) != 0)
	{
		return BigIntNO;
	}
	return BigIntYES;
}
#define CTYPE(name, op) \
- (BOOL)name\
{\
	if (mpz_fits_sint_p(v))\
	{\
		int  max = mpz_get_si(v);\
		return op(max);\
	}\
	return NO;\
}
CTYPE(isAlphanumeric, isalnum)
CTYPE(isUppercase, isupper)
CTYPE(isLowercase, islower)
CTYPE(isDigit, isdigit)
CTYPE(isAlphabetic, isalpha)
- (id)value
{
	return self;
}
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
- (id) to: (id) other by: (id) incr do: (id) aBlock
{
	id result = nil;
	if ([other isKindOfClass: isa] && [incr isKindOfClass: isa])
	{
		if (mpz_fits_sint_p(v) 
			&& mpz_fits_sint_p(((BigInt*)other)->v)
			&& mpz_fits_sint_p(((BigInt*)incr)->v))
		{
			int i = mpz_get_si(v);
			int max = mpz_get_si(((BigInt*)other)->v);
			int inc = mpz_get_si(((BigInt*)incr)->v);
			for (;i<=max;i+=inc)
			{
				result = [(BlockClosure*)aBlock value: 
					   [BigInt bigIntWithLongLong: (long long) i]];
			}
		}
		else
		{
			mpz_t i, max, inc;
			mpz_init_set(i, v);
			mpz_init_set(max, ((BigInt*)other)->v);
			mpz_init_set(inc, ((BigInt*)incr)->v);
			while (mpz_cmp(i, max)<=0)
			{
				result = [(BlockClosure*)aBlock value: [BigInt bigIntWithMP: i]];
				mpz_add(i, i, inc);
			}
		}
	}
	else
	{
		mpz_t i, max, inc;
		mpz_init_set(i, v);

		if ([other isKindOfClass: isa])
		{
			mpz_init_set(max, ((BigInt*)other)->v);
		}
		else if ([other respondsToSelector: @selector(intValue)])
		{
			mpz_init_set_si(max, [other intValue]);
		}
		else if ([other respondsToSelector: @selector(longValue)])
		{
			mpz_init_set_si(max, [other longValue]);
		}
		else
		{
			return NO;
		}

		if ([incr isKindOfClass: isa])
		{
			mpz_init_set(inc, ((BigInt*)incr)->v);
		}
		else if ([incr respondsToSelector: @selector(intValue)])
		{
			mpz_init_set_si(max, [incr intValue]);
		}
		else if ([incr respondsToSelector: @selector(longValue)])
		{
			mpz_init_set_si(max, [incr longValue]);
		}
		else
		{
			return NO;
		}
		while (mpz_cmp(i, max)<=0)
		{
			result = [(BlockClosure*)aBlock value: [BigInt bigIntWithMP: i]];
			mpz_add(i, i, inc);
		}
		mpz_clear(i);
		mpz_clear(inc);
		mpz_clear(max);
	}
	return result;
}
- (id) to: (id) other do: (id) aBlock
{
	return [self to: other by: [BigInt bigIntWithLongLong: 1] do: aBlock];
}

- (NSString*) description
{
	char * cstr = mpz_get_str(NULL, 10, v);
	NSString *str = [NSString stringWithUTF8String:cstr];
	free(cstr);
	return str;
}
- (NSString*) stringValue
{
	return [self description];
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
