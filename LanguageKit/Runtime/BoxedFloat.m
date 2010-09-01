#define class_pointer isa
#import "BoxedFloat.h"
#import "LKObject.h"

@implementation BoxedFloat
+ (BoxedFloat*) boxedFloatWithCString:(const char*) aString
{
	BoxedFloat *b = [[[BoxedFloat alloc] init] autorelease];
	b->value = strtold(aString, NULL);
	return b;
}
+ (BoxedFloat*) boxedFloatWithDouble:(double)aVal
{
	BoxedFloat *b = [[[BoxedFloat alloc] init] autorelease];
	b->value = aVal;
	return b;
}
+ (BoxedFloat*) boxedFloatWithFloat:(float)aVal
{
	BoxedFloat *b = [[[BoxedFloat alloc] init] autorelease];
	b->value = aVal;
	return b;
}

#define op(name, op) \
- (LKObjectPtr) name: (id)other\
{\
	BoxedFloat *b = [[[BoxedFloat alloc] init] autorelease];\
	if (other->isa == isa)\
	{\
		b->value = value op ((BoxedFloat*)other)->value;\
	}\
	else\
	{\
		b->value = value op [other doubleValue];\
	}\
	return LKOBJECTPTR(b);\
}

#define cmp(name, op) \
- (BOOL) name:(id)other\
{\
	if (other->isa == isa || [other isKindOfClass: isa])\
	{\
		return value op ((BoxedFloat*)other)->value;\
	}\
	return value op [other doubleValue];\
}

op(plus, +)
op(sub, -)
op(mul, *)
op(div, /)
cmp(isLessThan, <)
cmp(isGreaterThan, >)
cmp(isEqual, ==)
- (id) ifTrue:(id)t
{
	if (value != 0)
	{
		return [t value];
	}
	return nil;
}
- (id) ifFalse:(id)f
{
	if (value == 0)
	{
		return [f value];
	}
	return nil;
}
- (id) ifTrue:(id)t ifFalse:(id)f
{
	if (value == 0)
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
	int max = value;
	if (value > INT_MAX)
	{
		max = INT_MAX;
	}
	for (int i=0 ; i<max ; i++)
	{
		result = [aBlock value];
	}
	return result;
}
- (id) to: (id) other by: (id) incr do: (id) aBlock
{
	id result = nil;
	double to = [other doubleValue];
	double by = [incr doubleValue];
	for (double i=value ; i<to ; i+=by)
	{
		result = [aBlock value];
	}
	return result;
}
- (id) to: (id) other do: (id) aBlock
{
	return [self to: other by: [BoxedFloat boxedFloatWithDouble: 1] do: aBlock];
}

- (NSString*) description
{
	NSString *str = [NSString stringWithFormat: @"%f", value];
	return str;
}
- (NSString*) stringValue
{
	return [self description];
}


#define CASTMETHOD(returnType, name)\
- (returnType) name {\
	return (returnType) value;\
}

CASTMETHOD(char, charValue)
CASTMETHOD(unsigned char, unsignedCharValue)
CASTMETHOD(short int, shortValue)
CASTMETHOD(unsigned short int, unsignedShortValue)
CASTMETHOD(int, intValue)
CASTMETHOD(unsigned int, unsignedIntValue)
CASTMETHOD(long int, longValue)
CASTMETHOD(unsigned long int, unsignedLongValue)
CASTMETHOD(long long int, longLongValue)
CASTMETHOD(unsigned long long int, unsignedLongLongValue)
CASTMETHOD(float, floatValue)
CASTMETHOD(double, doubleValue)
CASTMETHOD(BOOL, boolValue)
@end
