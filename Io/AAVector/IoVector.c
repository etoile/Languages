/*#io
Vector ioDoc(
		   docCopyright("Steve Dekorte", 2002)
		   docLicense("BSD revised")
		   docCategory("Math")
		   docDescription("""
An object for fast operations on 1 dimensional arrays of single-precision floating point values. 
Vector processor acceleration is used for certain operations on some platforms.

<pre>
Io> v1 := vector(1, 2, 3)                    
==> vector(1, 2, 3)
Io> v1 *= 2
==> vector(2, 4, 6)
Io> v1 += 1
==> vector(3, 5, 7)
Io> v1 -= 1
==> vector(2, 4, 6)
Io> v1 /= 2
==> vector(1, 2, 3)
Io> v1 += v1
==> vector(2, 4, 6)
</pre>""")
		   */

#include "IoSeq.h"
#include "IoState.h"
#include "IoObject.h"
#include "IoVector.h"
#include "IoNumber.h"

#define VIVAR(self) ((Vector *)(IoObject_dataPointer(self)))

void *IoMessage_locals_vectorArgAt_(IoMessage *self, void *locals, int n)
{
	IoObject *v = IoMessage_locals_valueArgAt_(self, locals, n);
	
	if (!ISVECTOR(v)) 
	{
		IoMessage_locals_numberArgAt_errorForType_(self, locals, n, "Vector"); 
	}
	
	return v;
}

void *IoMessage_locals_pointArgAt_(IoMessage *m, void *locals, int n)
{
	IoVector *self = IoMessage_locals_vectorArgAt_(m, locals, n);
	IOASSERT(IoVector_rawSize(self) > 1, "Vector not long enough to be used as point argument");
	return self;
}


IoTag *IoVector_tag(void *state)
{
	IoTag *tag = IoTag_newWithName_("Vector");
	tag->state = state;
	tag->cloneFunc = (TagCloneFunc *)IoVector_rawClone;
	//tag->markFunc = (TagMarkFunc *)IoVector_mark;
	tag->freeFunc = (TagFreeFunc *)IoVector_free;
	return tag;
}

IoVector* IoVector_proto(void *state)
{
	IoMethodTable methodTable[] = {
	{"copy", IoVector_copy_},
	{"subarray", IoVector_subarray},
	{"size", IoVector_size},
	{"setSize", IoVector_setSize_},
	{"at", IoVector_at_},
	{"atPut", IoVector_at_put_},
		
	{"asBuffer", IoVector_asBuffer},
		
	{"==", IoVector_equals},
		
	{"+=", IoVector_plusEquals},
	{"+", IoVector_plus},
		
	{"-=", IoVector_minusEquals},
	{"-", IoVector_minus},
		
	{"*=", IoVector_multiplyEquals},
	{"*", IoVector_multiply},
		
	{"/=", IoVector_divideEquals},
	{"/", IoVector_divide},
		
	{"dot", IoVector_dotProduct},
		
	{"sum", IoVector_sum},
	{"min", IoVector_min},
	{"max", IoVector_max},
	{"mean", IoVector_mean},
	{"square", IoVector_square},
	{"meanSquare", IoVector_meanSquare},
	{"rootMeanSquare", 	IoVector_rootMeanSquare},
	{"normalize", IoVector_normalize},
	{"absolute", IoVector_absolute},
	{"abs", IoVector_absolute},
		
	{"log", IoVector_log},
	{"log10", IoVector_log10},
	{"pow", IoVector_pow},
	{"random", IoVector_random},
	{"sort", IoVector_sort},
	{"reverseSort", IoVector_reverseSort},
		
		
	{"setMin", IoVector_setMin_},
	{"setMax", IoVector_setMax_},
	{"set", IoVector_set_},
	{"setAll", IoVector_setAll_},
	{"sin", IoVector_sin},
		
		/* -------------------------- */
		
	{"x", IoVector_x},
	{"y", IoVector_y},
	{"z", IoVector_z},
	{"width", IoVector_x},
	{"height", IoVector_y},
	{"depth", IoVector_z},
		
	{"setX", IoVector_setX},
	{"setY", IoVector_setY},
	{"setZ", IoVector_setZ},
	{"setWidth", IoVector_setX},
	{"setHeight", IoVector_setY},
	{"setDepth", IoVector_setZ},
		
	{"zero", IoVector_zero},
		
	{"negate", IoVector_negate},
		
	{"<=", IoVector_lessThanOrEqualTo},
	{">=", IoVector_greaterThanOrEqualTo},
		
	{"cross", IoVector_cross},
	{"ceil", IoVector_ceil},
	{"floor", IoVector_floor},
	{"distanceTo", IoVector_distanceTo},
		//{"print", IoVector_print},
	{"Min", IoVector_Min},
	{"Max", IoVector_Max},
	{"zero", IoVector_zero},
	{"isZero", IoVector_isZero},
	{"rangeFill", IoVector_rangeFill},
		
	{NULL, NULL}
	};
	
	IoObject *self = IoObject_new(state);
	self->tag = IoVector_tag(state);
	IoObject_setDataPointer_(self, Vector_new());
	
	IoState_registerProtoWithFunc_(state, self, IoVector_proto);
	IoObject_addMethodTable_(self, methodTable);
	
	return self;
}

IoVector* IoVector_rawClone(IoVector *proto)
{
	IoObject *self = IoObject_rawClonePrimitive(proto);
	IoObject_setDataPointer_(self, Vector_clone(VIVAR(proto)));
	return self;
}

IoVector *IoVector_new(void *state)
{
	IoObject *proto = IoState_protoWithInitFunction_(state, IoVector_proto);
	return IOCLONE(proto);
}

IoVector *IoVector_newWithSize_(void *state, size_t size)
{
	IoVector *self = IoVector_new(state);
	Vector_setSize_(VIVAR(self), size);
	return self;
}

IoVector *IoVector_newX_y_z_(void *state, NUM_TYPE x, NUM_TYPE y, NUM_TYPE z)
{
	IoVector *self = IoVector_new(state);
	Vector_setXYZ(VIVAR(self), x, y, z);
	return self;
}

IoVector *IoVector_newWithRawVector_(void *state, Vector *vec)
{
	IoVector *proto = IoState_protoWithInitFunction_(state, IoVector_proto);
	IoVector *self = IOCLONE(proto);
	IoObject_setDataPointer_(self, (Vector *)vec);
	return self;
}

void IoVector_setVector_(IoVector *self, Vector *na)
{
	if (VIVAR(self) != na) Vector_free(VIVAR(self));
	IoObject_setDataPointer_(self, (Vector *)na);
}

Vector *IoVector_rawVector(IoVector *self) 
{ 
	return VIVAR(self); 
}

PointData *IoVector_rawPointData(IoVector *self)
{
	return Vector_pointData(VIVAR(self)); 
}


void IoVector_free(IoVector *self)
{
	Vector_free(VIVAR(self));
}

IoObject *IoVector_size(IoVector *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("size", 
		   "Returns the number of elements in the receiver.")
	*/
	
	return IONUMBER(Vector_size(VIVAR(self)));
}

size_t IoVector_rawSize(IoVector *self)
{
	return Vector_size(VIVAR(self));
}

IoObject *IoVector_setSize_(IoVector *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("setSize(aNumber)", 
		   "Sets the number of elements in the receiver. Returns self.")
	*/
	
	int n = IoMessage_locals_intArgAt_(m, locals, 0);
	Vector_setSize_(VIVAR(self), n);
	return self;
}

IoVector *IoVector_rawCopy(IoVector *self, IoVector *other)
{
	Vector_copy_(VIVAR(self), VIVAR(other));
	return self;
}

void IoVector_rawSetXYZ(IoVector *self, NUM_TYPE x, NUM_TYPE y, NUM_TYPE z)
{
	Vector_setXYZ(VIVAR(self), x, y, z);
}

void IoVector_rawGetXYZ(IoVector *self, NUM_TYPE *x, NUM_TYPE *y, NUM_TYPE *z)
{
	Vector_getXYZ(VIVAR(self), x, y, z);
}

IoSeq *IoVector_asBuffer(IoVector *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("asBuffer", 
		   "Returns the elements of the vector as a buffer of floating 
point values in the current architecture's byte order and float format.")
	*/
	
	return IoSeq_newWithData_length_(IOSTATE, 
							   Vector_bytes(VIVAR(self)), 
							   Vector_sizeInBytes(VIVAR(self)));
}

IoSeq *IoVector_equals(IoVector *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("== aVector", 
		   "Returns true if all elements of aVector are equal to 
those of the receiver. Returns false otherwise.")
	*/
	
	IoObject *other = IoMessage_locals_vectorArgAt_(m, locals, 0);
	Vector *v1 = VIVAR(self);
	Vector *v2 = VIVAR(other);
	int e = Vector_equals_(v1, v2);
	return IOBOOL(self, e);
}


IoObject *IoVector_copy_(IoVector *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("copy(aVectorOrBuffer)", 
		   "Sets the size of the receiver to match that of aVector and copies 
all elements of aVector to the receiver. If the argument is a Buffer, 
it will copy it's data, leaving any remaining bytes set to zero.")
	*/
	
	IoObject *other = IoMessage_locals_valueArgAt_(m, locals, 0);
	
	if (ISVECTOR(other))
	{
		Vector_copy_(VIVAR(self), VIVAR(other));
	}
	else if (ISSEQ(other))
	{
		ByteArray *ba = IoSeq_rawByteArray(other);
		Vector_copyData_length_(VIVAR(self), ByteArray_bytes(ba), ByteArray_size(ba));
	}
	else
	{
		IoState_error_(IOSTATE, m, "Vector += requires Vector or Number argument");     
	}
	
	return self;
}

IoObject *IoVector_subarray(IoVector *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("subarray(startIndex, length)", 
		   "Returns a new Vector containing the elements from startIndex to startIndex + length.")
	*/
	
	int start = IoMessage_locals_intArgAt_(m, locals, 0);
	int length = IoMessage_locals_intArgAt_(m, locals, 1);
	Vector *a = Vector_subarray_(VIVAR(self), start, length);
	
	if (a)
	{
		IoVector *ioa = IoVector_new(IOSTATE);
		IoVector_setVector_(ioa, a);
		return ioa;
	}
	
	return IONIL(self);
}

IoObject *IoVector_at_(IoVector *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("at(indexNumber)", 
		   "Returns a Number containing the value of the receivers element at indexNumber.")
	*/
	
	int n = IoMessage_locals_intArgAt_(m, locals, 0);
	double d = (double)Vector_at_(VIVAR(self), n);
	return IONUMBER(d);
}

IoObject *IoVector_at_put_(IoVector *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("atPut(indexNumber, numberValue)", 
		   "Puts numberValue into the element at indexNumber. 
Expands the size of the vector if needed.")
	*/
	
	size_t n = IoMessage_locals_longArgAt_(m, locals, 0);
	double d = IoMessage_locals_doubleArgAt_(m, locals, 1);
	Vector_at_put_(VIVAR(self), n, (NUM_TYPE)d);
	return self;
}

/* --- math -------------------------------------------- */

IoObject *IoVector_plusEquals(IoVector *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("+=(aValue)", 
		   "If aValue is a Number, an in-place scalar addition is performed on the receiver. 
If aValue is a vector, an in-place vector addition is performed on the receiver. Otherwise, a 'Vector +=' exception is raised. Returns self.")
	*/
	
	IoObject *other = IoMessage_locals_valueArgAt_(m, locals, 0);
	
	if (ISVECTOR(other))
	{ 
		Vector_addArray_(VIVAR(self), VIVAR(other)); 
	}
	else if (ISNUMBER(other))
	{ 
		Vector_addScalar_(VIVAR(self), CNUMBER(other)); 
	}
	else
	{
		IoState_error_(IOSTATE, m, "Vector += requires Vector or Number argument"); 
	}
	
	return self;
}

IoObject *IoVector_plus(IoVector *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("+(aValue)", "Same as: receiver clone +=(aValue)")
	*/
	
	IoObject *other = IoMessage_locals_valueArgAt_(m, locals, 0);
	Vector *vec = Vector_new();
	
	Vector_copy_(vec, VIVAR(self));
	
	/*
	 IOASSERT(IoVector_rawSize(self) == IoVector_rawSize(other), "Vectors not of equal size");
	 */
	
	if (ISVECTOR(other))
	{ 
		Vector_addArray_(vec, VIVAR(other)); 
	}
	else if (ISNUMBER(other))
	{ 
		Vector_addScalar_(vec, CNUMBER(other)); 
	}
	else
	{
		IoState_error_(IOSTATE, m, "Vector + requires Vector or Number argument"); 
	}
	return IOVECTOR(vec);
}

IoObject *IoVector_minusEquals(IoVector *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("-=(aValue)", 
		   "If aValue is a Number, an in-place scalar subtraction is performed on the receiver. 
If aValue is a vector, an in-place vector subtraction is performed on the receiver. Otherwise, a Vector -= exception is raised. Returns self.")
	*/
	
	IoObject *other = IoMessage_locals_valueArgAt_(m, locals, 0);
	
	if (ISVECTOR(other))
	{ 
		Vector_subtractArray_(VIVAR(self), VIVAR(other)); 
	}
	else if (ISNUMBER(other))
	{ 
		Vector_subtractScalar_(VIVAR(self), CNUMBER(other)); 
	}
	else
	{
		IoState_error_(IOSTATE, m, "Vector -= requires Vector or Number argument"); 
	}
	
	return self;
}

IoObject *IoVector_minus(IoVector *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("-(aValue)", "Same as: receiver clone -=(aValue)")
	*/
	
	IoObject *other = IoMessage_locals_valueArgAt_(m, locals, 0);
	
	Vector *vec = Vector_new();
	Vector_copy_(vec, VIVAR(self));
	
	if (ISVECTOR(other))
	{ 
		Vector_subtractArray_(vec, VIVAR(other)); 
	}
	else if (ISNUMBER(other))
	{ 
		Vector_subtractScalar_(vec, CNUMBER(other)); 
	}
	else
	{
		IoState_error_(IOSTATE, m, "Vector vectorSubtracting requires Vector or Number argument"); 
	}
	return IOVECTOR(vec);
}

IoObject *IoVector_multiplyEquals(IoVector *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("*=(aValue)", 
		   "If aValue is a Number, each element of the receiver is replaced by the product of it's value with aValue. If aValue is a vector, each element of the receiver is replaced by the product it's value with the element at the same index in the aValue vector. Returns self.")
	*/
	
	IoObject *other = IoMessage_locals_valueArgAt_(m, locals, 0);
	
	if (ISVECTOR(other))
	{ 
		Vector_multiplyArray_(VIVAR(self), VIVAR(other)); 
	}
	else if (ISNUMBER(other))
	{ 
		Vector_multiplyScalar_(VIVAR(self), CNUMBER(other)); 
	}
	else
	{
		IoState_error_(IOSTATE, m, "Vector multiply requires Vector or Number argument"); 
	}
	
	return self;
}

IoObject *IoVector_multiply(IoVector *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("*(aValue)", "Same as: receiver clone *=(aValue)")
	*/
	
	IoObject *other = IoMessage_locals_valueArgAt_(m, locals, 0);
	Vector *vec = Vector_new();
	Vector_copy_(vec, VIVAR(self));
	
	if (ISVECTOR(other))
	{ 
		Vector_multiplyArray_(vec, VIVAR(other)); 
	}
	else if (ISNUMBER(other))
	{ 
		Vector_multiplyScalar_(vec, CNUMBER(other)); 
	}
	else
	{
		IoState_error_(IOSTATE, m, "Vector vectorMultiplying requires Vector or Number argument"); 
	}
	
	return IOVECTOR(vec);
}

IoObject *IoVector_divideEquals(IoVector *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("*=(aValue)", 
		   "If aValue is a Number, each element of the receiver is replaced by the it's value divided by aValue. If aValue is a vector, each element of the receiver is replaced by the product it's value divided by the element at the same index in the aValue vector. Returns self.")
	*/
	
	IoObject *other = IoMessage_locals_valueArgAt_(m, locals, 0);
	
	if (ISVECTOR(other))
	{ 
		Vector_divideArray_(VIVAR(self), VIVAR(other)); 
	}
	else if (ISNUMBER(other))
	{ 
		Vector_divideScalar_(VIVAR(self), CNUMBER(other)); 
	}
	else
	{
		IoState_error_(IOSTATE, m, "Vector divide requires Vector or Number argument"); 
	}
	
	return self;
}

IoObject *IoVector_divide(IoVector *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("/(aValue)", "Same as: receiver clone /=(aValue)")
	*/
	
	IoObject *other = IoMessage_locals_valueArgAt_(m, locals, 0);
	Vector *vec = Vector_new();
	Vector_copy_(vec, VIVAR(self));
	
	if (ISVECTOR(other))
	{ 
		Vector_divideArray_(vec, VIVAR(other)); 
	}
	else if (ISNUMBER(other))
	{ 
		Vector_divideScalar_(vec, CNUMBER(other)); 
	}
	else
	{
		IoState_error_(IOSTATE, m, "Vector vectorDividing requires Vector or Number argument"); 
	}
	return IOVECTOR(vec);
}

IoObject *IoVector_dotProduct(IoVector *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("dotProduct(aVector)", 
		   "Same as '(receiver + aVector) sum', although the implementation is much faster.")
	*/
	
	IoObject *other = IoMessage_locals_vectorArgAt_(m, locals, 0);
	double r = Vector_dotProduct_(VIVAR(self), VIVAR(other));
	return IONUMBER(r);
}

IoObject *IoVector_sum(IoVector *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("sum", "Returns the sum of all elements of the receiver.")
	*/
	
	return IONUMBER(Vector_sum(VIVAR(self))); 
}

IoObject *IoVector_min(IoVector *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("min", 
		   "Returns the element of the vector with the lowest value.")
	*/
	
	return IONUMBER(Vector_min(VIVAR(self))); 
}

IoObject *IoVector_max(IoVector *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("max", 
		   "Returns the element of the vector with the greatest value.")
	*/
	
	return IONUMBER(Vector_max(VIVAR(self))); 
}

IoObject *IoVector_mean(IoVector *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("mean", 
		   "Same as 'receiver sum / receiver size'.")
	*/
	
	return IONUMBER(Vector_mean(VIVAR(self))); 
}


IoObject *IoVector_square(IoVector *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("squares", 
		   "Each element of the vector is squared. Returns self.")
	*/
	
	Vector_square(VIVAR(self)); 
	return self; 
}

IoObject *IoVector_normalize(IoVector *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("normalize", 
		   "Same as 'receiver /= receiver sum', but with a faster implementation. Returns self.")
	*/
	
	Vector_normalize(VIVAR(self)); 
	return self; 
}

IoObject *IoVector_absolute(IoVector *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("absolute", 
		   "Each element of the receiver is replaced with it's absolute value. Returns self.")
	*/
	
	Vector_absolute(VIVAR(self)); 
	return self; 
}


IoObject *IoVector_log(IoVector *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("log", 
		   "Each element of the receiver is replaced with it's natural log value. Returns self.")
	*/
	
	Vector_log(VIVAR(self)); 
	return self; 
}

IoObject *IoVector_log10(IoVector *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("log10", 
		   "Each element of the receiver is replaced with it's log base 10 value. Returns self.")
	*/
	
	Vector_log10(VIVAR(self)); 
	return self; 
}

IoObject *IoVector_pow(IoVector *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("pow(aNumber)", 
		   "Each element of the receiver is replaced with it's value taken to the power of aNumber. Returns self.")
	*/
	
	int p = IoMessage_locals_intArgAt_(m, locals, 0);
	Vector_pow(VIVAR(self), p); 
	return self; 
}

IoObject *IoVector_random(IoVector *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("random(minNumber, maxNumber)", 
		   "Each element of the receiver is replaced with random value between minNumber and maxNumber. Returns self.")
	*/
	
	int min = IoMessage_locals_intArgAt_(m, locals, 0);
	int max = IoMessage_locals_intArgAt_(m, locals, 1);
	Vector_random(VIVAR(self), min, max); 
	return self; 
}

IoObject *IoVector_sort(IoVector *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("sort", 
		   "The elements of the receiver are sorted from smallest to greatest. Returns self.")
	*/
	
	Vector_sort(VIVAR(self)); 
	return self; 
}

IoObject *IoVector_reverseSort(IoVector *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("reverseSort", 
		   "The elements of the receiver are sorted from greatest to smallest. Returns self.")
	*/
	
	Vector_reverseSort(VIVAR(self)); 
	return self; 
}




IoObject *IoVector_meanSquare(IoVector *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("meanSquare", 
		   "Same as 'receiver clone square sum / receiver size'.")
	*/
	
	return IONUMBER(Vector_meanSquare(VIVAR(self))); 
}

IoObject *IoVector_rootMeanSquare(IoVector *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("rootMeanSquare", 
		   "Same as 'receiver meanSquare squareRoot' ")
	*/
	
	return IONUMBER(Vector_rootMeanSquare(VIVAR(self))); 
}

IoObject *IoVector_setMin_(IoVector *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("setMin(aNumber)", 
		   "Sets all elements of the receiver that are less than aNumber to aNumber. Returns self.")
	*/
	
	double v = IoMessage_locals_doubleArgAt_(m, locals, 0);
	Vector_setMin_(VIVAR(self), v);
	return self; 
}

IoObject *IoVector_setMax_(IoVector *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("setMax(aValue)", 
		   "Sets all elements of the receiver that are greater than aNumber to aNumber. Returns self.")
	*/
	
	double v = IoMessage_locals_doubleArgAt_(m, locals, 0);
	Vector_setMax_(VIVAR(self), v);
	return self; 
}

IoObject *IoVector_set_(IoVector *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("set(value1, value2, ...)", 
		   "Sets the elements of the array to the value of the arguments. Returns self.")
	*/
	int i, max = IoMessage_argCount(m);
	Vector *vec = VIVAR(self);
	
	for (i = 0; i < max; i ++)
	{
		float v = IoMessage_locals_doubleArgAt_(m, locals, i);
		Vector_at_put_(vec, i, v);
	}
	
	return self; 
}

IoObject *IoVector_setAll_(IoVector *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("setAll(aNumber)", 
		   "Sets all elements of the receiver to aNumber.")
	*/
	
	double v = IoMessage_locals_doubleArgAt_(m, locals, 0);
	Vector_set_(VIVAR(self), v);
	return self; 
}

IoObject *IoVector_sin(IoVector *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("sin", 
		   "Sets all elements to the sin of their value. Returns self.")
	*/
	
	Vector_sin(VIVAR(self));
	return self;
}


IoObject *IoVector_x(IoVector *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("x", "Same as 'receiver at(0)' ")
	*/
	
     /*#io
	docSlot("width", "Same as 'receiver at(0)'")
	*/
	
	Vector *vec = VIVAR(self);
	IOASSERT(Vector_size(vec) > 0, "Vector missing element at index 0");
	return IONUMBER(Vector_at_(vec, 0));
}

IoObject *IoVector_y(IoVector *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("y", "Same as 'receiver at(1)'")
	*/
	
	/*#io
	docSlot("height", "Same as 'receiver at(1)'")
	*/
	
	Vector *vec = VIVAR(self);
	IOASSERT(Vector_size(vec) > 1, "Vector missing element at index 1");
	return IONUMBER(Vector_at_(vec, 1));
}

IoObject *IoVector_z(IoVector *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("z", "Same as 'receiver at(2)'")
	*/
	
	/*#io
	docSlot("depth", "Same as 'receiver at(2)'")
	*/
	
	Vector *vec = VIVAR(self);
	IOASSERT(Vector_size(vec) > 2, "Vector missing element at index 2");
	return IONUMBER(Vector_at_(vec, 2));
}

IoObject *IoVector_w(IoVector *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("w", "Same as 'receiver at(3)'")
	*/
	
	Vector *vec = VIVAR(self);
	IOASSERT(Vector_size(vec) > 3, "Vector missing element at index 3");
	return IONUMBER(Vector_at_(vec, 3));
}

IoObject *IoVector_setX(IoVector *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("setX(aValue)", 
		   "Same as 'receiver atPut(0, aValue)'")
	*/
	
	/*#io
	docSlot("setWidth(aValue)", "Same as 'receiver atPut(0, aValue)'")
	*/
	
	Vector *vec = VIVAR(self);
	double v = IoMessage_locals_doubleArgAt_(m, locals, 0);
	Vector_at_put_(vec, 0, (NUM_TYPE)v);
	return self;
}

IoObject *IoVector_setY(IoVector *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("setY(aValue)", "Same as 'receiver atPut(0, aValue)'")
	*/
	
	/*#io
	docSlot("setHeight(aValue)", "Same as 'receiver atPut(0, aValue)'")
	*/
	
	Vector *vec = VIVAR(self);
	double v = IoMessage_locals_doubleArgAt_(m, locals, 0);
	Vector_at_put_(vec, 1, (NUM_TYPE)v);
	return self;
}

IoObject *IoVector_setZ(IoVector *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("setZ(aValue)", "Same as: receiver atPut(0, aValue)")
	*/
	
	/*#io
	docSlot("setDepth(aValue)", "Same as: receiver atPut(0, aValue)")
	*/
	
	Vector *vec = VIVAR(self);
	double v = IoMessage_locals_doubleArgAt_(m, locals, 0);
	Vector_at_put_(vec, 2, (NUM_TYPE)v);
	return self;
}

IoObject *IoVector_setW(IoVector *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("setW(aValue)", "Same as: receiver atPut(3, aValue)")
	*/
	
	Vector *vec = VIVAR(self);
	double v = IoMessage_locals_doubleArgAt_(m, locals, 3);
	Vector_at_put_(vec, 3, (NUM_TYPE)v);
	return self;
}


IoObject *IoVector_negate(IoVector *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("negate", "Multiplies each element by -1. Returns self.")
	*/
	
	Vector *vec = VIVAR(self);
	Vector_negate(vec);
	return self;
}

IoObject *IoVector_lessThanOrEqualTo(IoVector *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("<=(aValue)", 
		   "If aValue is a Vector, true is returned if each element of the receiver 
is less than or equal to the corresponding element in aValue or Nil is returned otherwise. 
If aValue is a Number, then self is returned if all elements of the receiver are less 
than aValue or false otherwise. If aValue is niether a Number or a Vector, an exception is raised.")
	*/
	
	IoObject *other = IoMessage_locals_valueArgAt_(m, locals, 0);
	Vector *vec  = VIVAR(self);
	
	if (ISVECTOR(other))
	{
		Vector *vec2 = VIVAR(other);
		//IOASSERT(Vector_size(vec) == Vector_size(vec2), "Vectors not of equal size");
		return IOBOOL(self, Vector_lessThanOrEqualTo_(vec, vec2));
	}
	
	if (ISNUMBER(other))
	{
		return IOBOOL(self, Vector_lessThanOrEqualToScalar_(vec, CNUMBER(other)));
	}
	else
	{
		IoState_error_(IOSTATE, m, "Vector <= requires Vector or Number argument");     
	}
	
	return self;
}


IoObject *IoVector_greaterThanOrEqualTo(IoVector *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot(">=(aValue)", 
		   "If aValue is a Vector, self is returned if each element of the receiver is greater than or equal to the corresponding element in aValue or Nil is returned otherwise. If aValue is a Number, then self is returned if all elements of the receiver are greater than aValue or false otherwise. If aValue is niether a Number or a Vector, an exception is raised.")
	*/
	
	IoObject *other = IoMessage_locals_valueArgAt_(m, locals, 0);
	Vector *vec  = VIVAR(self);
	
	if (ISVECTOR(other))
	{
		Vector *vec2 = VIVAR(other);
		//IOASSERT(Vector_size(vec) == Vector_size(vec2), "Vectors not of equal size");
		return IOBOOL(self, Vector_greaterThanOrEqualTo_(vec, vec2));
	}
	
	if (ISNUMBER(other))
	{
		return IOBOOL(self, Vector_greaterThanOrEqualToScalar_(vec, CNUMBER(other)));
	}
	else
	{
		IoState_error_(IOSTATE, m, "Vector <= requires Vector or Number argument");     
	}
	
	return self;
}

IoObject *IoVector_cross(IoVector *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("cross(aVector)", 
		   "Sets the receiver to the cross product of itself with aVector. Returns self.")
	*/
	
	IoVector *other = IoMessage_locals_vectorArgAt_(m, locals, 0);
	Vector *vec  = VIVAR(self);
	Vector *vec2 = VIVAR(other);
	IOASSERT(Vector_size(vec) < 4, "Vectors must be of size < 4");
	IOASSERT(Vector_size(vec) == Vector_size(vec2), "Vectors not of equal size");
	Vector_crossProduct_(vec, vec2);
	return self;
}

IoObject *IoVector_ceil(IoVector *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("ceil", 
		   "Applies the ceil operator to each element of the receiver. Return self.")
	*/
	
	Vector *vec  = VIVAR(self);
	Vector_ceil(vec);
	return self;
}

IoObject *IoVector_floor(IoVector *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("floor", 
		   "Applies the floor operator to each element of the receiver. Return self.")
	*/
	
	Vector *vec  = VIVAR(self);
	Vector_floor(vec);
	return self;
}

IoObject *IoVector_distanceTo(IoVector *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("distanceTo(aVector)", 
		   "Returns the distance from the receiver to aVector.")
	*/
	
	IoVector *other = IoMessage_locals_vectorArgAt_(m, locals, 0);
	Vector *vec  = VIVAR(self);
	Vector *vec2 = VIVAR(other);
	IOASSERT(Vector_size(vec) == Vector_size(vec2), "Vectors not of equal size");
	return IONUMBER(Vector_distanceTo_(vec, vec2));
}


IoObject *IoVector_Min(IoVector *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("Min(aVector)", 
		   "Sets each element of the receiver to be the lesser of 
itself and the corresponding element of aVector.")
	*/
	
	IoVector *other = IoMessage_locals_vectorArgAt_(m, locals, 0);
	Vector *vec  = VIVAR(self);
	Vector *vec2 = VIVAR(other);
	IOASSERT(Vector_size(vec) == Vector_size(vec2), "Vectors not of equal size");
	Vector_Min(vec, vec2);
	return self;
}

IoObject *IoVector_Max(IoVector *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("Max(aVector)", 
		   "Sets each element of the receiver to be the greater of 
itself and the corresponding element of aVector.")
	*/
	
	IoVector *other = IoMessage_locals_vectorArgAt_(m, locals, 0);
	Vector *vec  = VIVAR(self);
	Vector *vec2 = VIVAR(other);
	IOASSERT(Vector_size(vec) == Vector_size(vec2), "Vectors not of equal size");
	Vector_Max(vec, vec2);
	return self;
}

IoObject *IoVector_zero(IoVector *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("zero", "Sets each element of the receiver to zero.")
	*/
	
	Vector *vec  = VIVAR(self);
	Vector_zero(vec);
	return self;
}

IoObject *IoVector_isZero(IoVector *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("isZero", 
		   "Returns true if all elements of the receiver are zero, false otherwise.")
	*/
	
	Vector *vec  = VIVAR(self);
	return IOBOOL(self, Vector_isZero(vec));
}

IoObject *IoVector_print(IoVector *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("print", 
		   "Prints a human readable representation of the receiver to standard output.")
	*/
	
	Vector_print(VIVAR(self));
	return self;
}


IoObject *IoVector_rangeFill(IoVector *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("rangeFill(shapeVector, indexDimension)", "Set the values of the array to the their index values for the indexDimension given the specified shapeVector. The array will be resized to a size that is the product of the shapeVector. Examples:
	<pre>
 vector rangeFill(vector(2, 2), 0) == vector(0, 1, 0, 1)
 vector rangeFill(vector(2, 2), 1) == vector(0, 0, 1, 1)
 </pre>
	")
	*/
	
	IoVector *other = IoMessage_locals_vectorArgAt_(m, locals, 0);
	size_t d = IoMessage_locals_intArgAt_(m, locals, 1);
	
	Vector_rangeFillWithShapeVectorDim(VIVAR(self), VIVAR(other), d);
	return self;
}


