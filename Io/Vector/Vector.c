/* Copyright (c) 2004, Kentaro A. Kurahone
* All Rights Reserved.
*/

#include "Vector.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/*
#if defined(???)
#define HAS_SSE
#endif
 */
#if defined(__APPLE__) || defined(MACOSX)
#include <CoreServices/CoreServices.h>
#include <Accelerate/Accelerate.h>
#include <vecLib/vfp.h>
#include <vecLib/vectorOps.h>
//#include <vecLib/vBasicOps.h>

#define HAS_ALTIVEC

/* from http://home.san.rr.com/altivec/Pages/TipArchive.html */
#define vec_not(A) vec_nor(A,A)
#define vec_nand(A,B) vec_not(vec_and(A,B))
#define vec_abs(A) vec_sel(A,vec_sub( vec_sub(A,A),A),vec_cmpgt(vec_sub(A,A),A))
#define vec_2sComp(x) vec_sub(vec_sub(x,x), x)

/*
#include <vDsp.h>
#if defined(__VEC__)
#define HAS_ALTIVEC
#endif
 */
#endif

Vector *Vector_new(void)
{
	Vector *self = calloc(1, sizeof(Vector));
	Vector_setSize_(self, 1);
	return self;
}

void Vector_free(Vector *self)
{
	if (self->values) free(self->values);
	free(self);
}

Vector *Vector_clone(Vector *self)
{
	Vector *c = Vector_new();
	Vector_copy_(c, self);
	return c;
}

Vector *Vector_subarray_(Vector *self, size_t localStart, size_t length)
{
	if (self->size <= localStart + length)
	{
		Vector *c = Vector_new();
		Vector_setSize_(c, length);
		memcpy(c->values, self->values, length * sizeof(NUM_TYPE));
		return c;
	}
	
	return 0x0;
}

void Vector_copy_(Vector *self, Vector *other)
{
	if (self->size != other->size) 
	{
		Vector_setSize_(self, other->size);
	}
	
	/*
	 doesn't seem to work
#ifdef HAS_ALTIVEC
	 vScopy(self->size, self->values, other->values);
#else  
	 */
	memcpy(self->values, other->values, self->size * sizeof(NUM_TYPE));
	/*
#endif
	 */
}

void Vector_copyData_length_(Vector *self, uint8_t *data, size_t length)
{
	size_t size = ceil(((double)length) / ((double)sizeof(NUM_TYPE)));
	Vector_setSize_(self, size);
	
	/*
#ifdef HAS_ALTIVEC
	 vScopy(self->size, self->values, data);
#else 
	 */
	memcpy(self->values, data, size * sizeof(NUM_TYPE));
	/*
#endif
	 */
}

uint8_t *Vector_bytes(Vector *self)
{
	return (uint8_t *)self->values;
}

int Vector_equals_(Vector *self, Vector *other)
{
	return memcmp(self->values, other->values, self->size * sizeof(NUM_TYPE)) == 0;
}


size_t Vector_sizeInBytes(Vector *self)
{
	return self->size * sizeof(NUM_TYPE);
}

size_t Vector_size(Vector *self)
{
	return self->size;
}

void Vector_setSize_(Vector *self, size_t size)
{
	if (self->size == size) 
	{
		return;
	}
	/*
	 calloc already gives 16 byte allignment
	 
#ifdef HAS_ALTIVEC
	 size_t minSize = size > self->size ? self->size : size;
	 NUM_TYPE *v = vec_malloc(newMemSize);
	 memset(v, 0x0, newMemSize);
	 memcpy(v, self->values, minSize);
	 free(self->values);
	 self->values = v;
	 //self->values = vec_realloc(self->values, size * sizeof(NUM_TYPE));
#else
	 */
	if (size)
	{
		self->values = realloc(self->values, size * sizeof(NUM_TYPE));
	}
	
	/* When passing in a size of zero to malloc/realloc (e.g.,
	* deallocating), it's implementation defined whether the null
* pointer is return or a sentinel object is (C99 7.20.3/1).  Thus
* if size is zero, self->values may now be null.  Since this
* chunk of code simply zeroes out any differing space, that's now
* unnecessary, and would result in illegal pointer arithmetic on
* a null pointer anyway. emf*/
	
	if (self->values)
	{
		/* if new size if bigger, make the new elements zeros */
		if (self->size < size)
		{
			memset(self->values + self->size, 0x0, (size - self->size) * sizeof(NUM_TYPE));
		}
	}
	
	/*#endif*/
	self->size = size;
}

NUM_TYPE Vector_at_(Vector *self, size_t i)
{
	if (i < self->size) return *(self->values + i);
	return 0;
}

void Vector_at_put_(Vector *self, size_t i, NUM_TYPE v)
{
	if (i >= self->size) 
	{
		Vector_setSize_(self, i + 1);
	}
	
	self->values[i] = v; 
}

void Vector_setXYZ(Vector *self, NUM_TYPE x, NUM_TYPE y, NUM_TYPE z)
{
	if (2 >= self->size) 
	{
		Vector_setSize_(self, 3);
	}
	
	self->values[0] = x; 
	self->values[1] = y; 
	self->values[2] = z;
}

void Vector_getXYZ(Vector *self, NUM_TYPE *x, NUM_TYPE *y, NUM_TYPE *z)
{
	if (2 >= self->size) 
	{
		Vector_setSize_(self, 3);
	}
	
	*x = self->values[0]; 
	*y = self->values[1]; 
	*z = self->values[2];
}

/* ------------------------------------------------------------------------ */

void Vector_addScalar_(Vector *self, NUM_TYPE v)
{
	size_t n = self->size;
	NUM_TYPE *d1 = self->values;
	
	while (n) 
	{ 
		*d1 += v; 
		d1 ++; 
		n--; 
	}
}

void Vector_subtractScalar_(Vector *self, NUM_TYPE v)
{
	NUM_TYPE *d1 = self->values;
	
	//#ifdef HAS_ALTIVEC
	//vSgesub(1, self->size, d1, 'n', d2, 'n', d1) 
	//#else
	{
		size_t n = self->size;
		
		while (n) 
		{ 
			*d1 -= v; 
			d1 ++; 
			n--; 
		}
	}
	//#endif
}

void Vector_multiplyScalar_andAddTo_(Vector *self, NUM_TYPE v, Vector *other)
{
	NUM_TYPE *d1 = self->values;
	NUM_TYPE *d2 = other->values;
	
	/*
#ifdef HAS_ALTIVEC
	#ifdef NUM_TYPE_IS_DOUBLE
	 vSaxpyD(self->size, v, d1, d2);
	#else
	 vSaxpy(self->size, v, d1, d2);
	#endif
	 //#elif HAS_SSE
#else
	 */
	{
		size_t n = self->size;
		
		while (n) 
		{ 
			*d2 += v * (*d1); 
			d1 ++; 
			d2 ++; 
			n--; 
		}
	}
	/*
#endif
	 */
}

void Vector_multiplyScalar_(Vector *self, NUM_TYPE v)
{
	NUM_TYPE *d1 = self->values;
	
#ifdef HAS_ALTIVEC
	//const int inc = 1;
	//SSCAL(&(self->size), &v, d1, &inc)  
	#ifdef NUM_TYPE_IS_DOUBLE
	vsmulD(d1, 1, &v, d1, 1, self->size);
	#else
	vsmul(d1, 1, &v, d1, 1, self->size);
	#endif
	//#elif HAS_SSE
#else
	{
		size_t n = self->size;
		while (n) { *d1 *= v; d1 ++; n--; }
	}
#endif
}

void Vector_divideScalar_(Vector *self, NUM_TYPE v)
{
	NUM_TYPE *d1 = self->values;
	
	{
		size_t n = self->size;
		while (n) { if(v) *d1 /= v; else *d1 = 0; d1 ++; n--; }
	}
}

void Vector_negate(Vector *self)
{
	NUM_TYPE *d1 = self->values;
	
	{
		size_t n = self->size;
		while (n) { *d1 = -(*d1); d1 ++; n--; }
	}
}

/* ------------------------------------------------------------------------ */

NUM_TYPE Vector_sum(Vector *self)
{
	NUM_TYPE total = 0;
	size_t n = self->size;
	NUM_TYPE *d1 = self->values;
	
	while (n)
	{
		total += *d1;
		d1 ++; 
		n --;
	}
	
	return total;
}

int Vector_addArray_(Vector *self, Vector *other)
{
	NUM_TYPE *d1 = self->values;
	NUM_TYPE *d2 = other->values;
	
	if (self->size < other->size)
	{
		Vector_setSize_(self, other->size);
	}
	
#ifdef HAS_ALTIVEC
	#ifdef NUM_TYPE_IS_DOUBLE
	vaddD(d1, 1, d2, 1, d1, 1, other->size);
	#else
	vadd(d1, 1, d2, 1, d1, 1, other->size);
	#endif
	//vaddfp(d1, 1, d2, 1, d1, 1, self->size);
	//#elif HAS_SSE
#else
	{
		size_t n = other->size;
		while (n)
		{
			*d1 += *d2;
			d1 ++; d2++;
			n --;
		}
	}
#endif
	return 0;
}

int Vector_subtractArray_(Vector *self, Vector *other)
{
	NUM_TYPE *d1 = self->values;
	NUM_TYPE *d2 = other->values;
	
	if (self->size < other->size)
	{
		Vector_setSize_(self, other->size);
	}
	
#ifdef HAS_ALTIVEC
	#ifdef NUM_TYPE_IS_DOUBLE
	vsubD(d2, 1, d1, 1, d1, 1, other->size);
	#else
	vsub(d2, 1, d1, 1, d1, 1, other->size);
	#endif
	//#elif HAS_SSE
#else
	{
		size_t n = other->size;
		while (n)
		{
			*d1 -= *d2;
			d1 ++; d2++;
			n --;
		}
	}
#endif
	return 0;
}

int Vector_multiplyArray_(Vector *self, Vector *other)
{
	NUM_TYPE *d1 = self->values;
	NUM_TYPE *d2 = other->values;
	
	if (self->size < other->size)
	{
		Vector_setSize_(self, other->size);
	}
	
#ifdef HAS_ALTIVEC
	#ifdef NUM_TYPE_IS_DOUBLE
	vmulD(d1, 1, d2, 1, d1, 1, other->size);
	#else
	vmul(d1, 1, d2, 1, d1, 1, other->size);
	#endif
	//#elif HAS_SSE
#else
	{
		size_t n = other->size;
		while (n)
		{
			*d1 *= *d2;
			d1 ++; d2++;
			n --;
		}
	}
#endif
	return 0;
}

int Vector_divideArray_(Vector *self, Vector *other)
{
	NUM_TYPE *d1 = self->values;
	NUM_TYPE *d2 = other->values;
	
	if (self->size < other->size)
	{
		Vector_setSize_(self, other->size);
	}
	
	{
		size_t n = other->size;
		while (n)
		{
			if (*d2) 
			{
				*d1 /= *d2;
			}
			else 
			{
				*d1 = 0;
			}
			d1 ++; d2++;
			n --;
		}
	}
	return 0;
}

NUM_TYPE Vector_dotProduct_(Vector *self, Vector *other)
{
	NUM_TYPE *d1 = self->values;
	NUM_TYPE *d2 = other->values;
	NUM_TYPE result = 0;
	
	if (self->size != other->size) 
	{
		printf("Error: Vector_dotProduct_() sizes not equal\n");
		return -1; 
	}
	
#ifdef HAS_ALTIVEC
	#ifdef NUM_TYPE_IS_DOUBLE
	dotprD(d1, 1, d2, 1, &result, self->size);
	#else
	dotpr(d1, 1, d2, 1, &result, self->size);
	#endif
	//#elif HAS_SSE
#else
	{
		size_t n = self->size;
		while (n)
		{
			result += (*d1) * (*d2);
			d1 ++; d2++;
			n --;
		}
	}
#endif
	return result;
}

NUM_TYPE Vector_product(Vector *self)
{
	NUM_TYPE *d1 = self->values;
	NUM_TYPE result = 1;
	size_t n;
	
	for (n = 0; n < self->size; n ++)
	{
		result *= d1[n];
	}
	
	return result;
}


void Vector_square(Vector *self)
{
	NUM_TYPE *d1 = self->values;
	
#ifdef HAS_ALTIVEC
	#ifdef NUM_TYPE_IS_DOUBLE
	vsqD(d1, 1, d1, 1, self->size);
	#else
	vsq(d1, 1, d1, 1, self->size);
	#endif
	//#elif HAS_SSE
#else
	{
		size_t n = self->size;
		while (n)
		{
			*d1 *= (*d1);
			d1 ++; 
			n --;
		}
	}
#endif
}

void Vector_normalize(Vector *self)
{
	NUM_TYPE v1 = Vector_max(self);
	NUM_TYPE v2 = (NUM_TYPE)fabs((double)Vector_min(self));
	NUM_TYPE v = v1 > v2 ? v1 : v2;
	
	if (!v) return;
	Vector_multiplyScalar_(self, 1.0/v);
}

void Vector_absolute(Vector *self)
{
	NUM_TYPE *d1 = self->values;
	
	/*
#ifdef HAS_ALTIVEC
	 vfabf(d1);
#else
	 */
	{	
		size_t n = self->size;
		while (n)
		{
			if (*d1 < 0) *d1 = -(*d1);
			d1 ++; 
			n --;
		}
	}
}

void Vector_log(Vector *self)
{
	NUM_TYPE *d1 = self->values;
	
	/*
#ifdef HAS_ALTIVEC
	 vlogf(d1);
#else	
	 */
	{	
		size_t n = self->size;
		while (n)
		{
			*d1 = (NUM_TYPE)log((double)(*d1));
			d1 ++; 
			n --;
		}
	}
}

void Vector_log10(Vector *self)
{
	size_t n = self->size;
	NUM_TYPE *d1 = self->values;
	
	
	while (n)
	{
		*d1 = (NUM_TYPE)log10((double)(*d1));
		d1 ++; 
		n --;
	}
}

void Vector_pow(Vector *self, double p)
{
	size_t n = self->size;
	NUM_TYPE *d1 = self->values;
	
	/*
#ifdef HAS_ALTIVEC
	 vipowf(d1, p);
#else
	 */
	while (n)
	{
		*d1 = (NUM_TYPE)pow((double)(*d1), p);
		d1 ++; 
		n --;
	}
}

void Vector_random(Vector *self, float min, float max)
{
	size_t n = self->size;
	NUM_TYPE *d1 = self->values;
	NUM_TYPE range = max - min;
	
	while (n)
	{
		*d1 = min + range * ((NUM_TYPE)rand())/((NUM_TYPE)RAND_MAX);
		d1 ++; 
		n --;
	}
}

int Vector_ElementCmp(const void *a, const void *b)
{ 
	return *((NUM_TYPE *)a) < *((NUM_TYPE *)b); 
}

void Vector_sort(Vector *self)
{
	if (!self->size) 
	{
		return;
	}
	
	qsort(self->values, self->size, sizeof(NUM_TYPE), Vector_ElementCmp);
}

int Vector_ElementReverseCmp(const void *a, const void *b)
{ 
	return *((NUM_TYPE *)a) > *((NUM_TYPE *)b); 
}

void Vector_reverseSort(Vector *self)
{
	if (!self->size) 
	{
		return;
	}
	qsort(self->values, self->size, sizeof(NUM_TYPE), Vector_ElementReverseCmp);
}

/* ---------------------------------------------------------- */

NUM_TYPE Vector_min(Vector *self)
{
	NUM_TYPE m;
	size_t n = self->size;
	NUM_TYPE *d1 = self->values;
	if (!d1) return 0;
	
	m = *d1;
	while (n)
	{
		if (*d1 < m) m = *d1;
		d1 ++; 
		n --;
	}
	return m;
}

NUM_TYPE Vector_max(Vector *self)
{
	NUM_TYPE m;
	size_t n = self->size;
	NUM_TYPE *d1 = self->values;
	if (!d1) return 0;
	
	m = *d1;
	while (n)
	{
		if (*d1 > m) m = *d1;
		d1 ++; 
		n --;
	}
	return m;
}

NUM_TYPE Vector_mean(Vector *self)
{
	NUM_TYPE total = 0;
	size_t n = self->size;
	NUM_TYPE *d1 = self->values;
	if (!d1) return 0;
	
	while (n)
	{
		total += (*d1);
		d1 ++; 
		n --;
	}
	return total / ((NUM_TYPE)self->size);
}

NUM_TYPE Vector_meanSquare(Vector *self)
{
	NUM_TYPE total = 0;
	size_t n = self->size;
	NUM_TYPE *d1 = self->values;
	if (!d1) return 0;
	
	while (n)
	{
		total += (*d1)*(*d1);
		d1 ++; 
		n --;
	}
	return total / ((NUM_TYPE)self->size);
}

void Vector_setMin_(Vector *self, NUM_TYPE v)
{
	size_t n = self->size;
	NUM_TYPE *d1 = self->values;
	
	
	while (n)
	{
		if (*d1 < v) *d1 = v;
		d1 ++; 
		n --;
	}
}

void Vector_setMax_(Vector *self, NUM_TYPE v)
{
	size_t n = self->size;
	NUM_TYPE *d1 = self->values;
	
	
	while (n)
	{
		if (*d1 > v) *d1 = v;
		d1 ++; 
		n --;
	}
}

void Vector_set_(Vector *self, NUM_TYPE v)
{
	size_t n = self->size;
	NUM_TYPE *d1 = self->values;
	
	
	while (n)
	{
		*d1 = v;
		d1 ++; 
		n --;
	}
}

NUM_TYPE Vector_rootMeanSquare(Vector *self)
{
	return sqrt(Vector_meanSquare(self)); 
}

#define PI 3.14159265

void Vector_sin(Vector *self)
{
	NUM_TYPE *d1 = self->values;
	
	/*
#ifdef HAS_ALTIVEC
	 vsinf(d1);
#else
	 */
	{
		size_t n = self->size;
		while (n)
		{
			*d1 = (NUM_TYPE)sin((double)*d1);
			d1 ++; 
			n --;
		}
	}
}

void Vector_cos(Vector *self)
{
	NUM_TYPE *d1 = self->values;
	
	/*
#ifdef HAS_ALTIVEC
	 vcosf(d1);
#else
	 */
	{
		size_t n = self->size;
		while (n)
		{
			*d1 = (NUM_TYPE)cos((double)*d1);
			d1 ++; 
			n --;
		}
	}
}

void Vector_tan(Vector *self)
{
	NUM_TYPE *d1 = self->values;
	
	/*
#ifdef HAS_ALTIVEC
	 vtanf(d1);
#else
	 */
	{
		size_t n = self->size;
		while (n)
		{
			*d1 = (NUM_TYPE)tan((double)*d1);
			d1 ++; 
			n --;
		}
	}
}

/* ----------------------------------------------------------- */

PointData *Vector_pointData(Vector *self)
{
	return (PointData *)self->values;
}

int Vector_lessThanOrEqualTo_(Vector *self, Vector *other)
{
	NUM_TYPE *d1 = self->values;
	NUM_TYPE *d2 = other->values;
	size_t n = self->size < other->size ? self->size : other->size;
	
	//if (n != other->size) return 0;
	
	while (n)
	{
		if (*d1 > *d2) return 0;
		d1 ++;
		d2 ++;
		n --;
	}
	
	return 1;
}

int Vector_greaterThanOrEqualTo_(Vector *self, Vector *other)
{
	NUM_TYPE *d1 = self->values;
	NUM_TYPE *d2 = other->values;
	size_t n = self->size < other->size ? self->size : other->size;
	
	//if (n != other->size) return 0;
	
	while (n)
	{
		if (*d1 < *d2) return 0;
		d1 ++;
		d2 ++;
		n --;
	}
	
	return 1;
}

int Vector_lessThanOrEqualTo_minus_(Vector *self, Vector *v2, Vector *v3)
{
	NUM_TYPE *d1 = self->values;
	NUM_TYPE *d2 = v2->values;
	NUM_TYPE *d3 = v3->values;
	size_t n = self->size;
	n = n < v2->size ? n : v2->size;
	n = n < v3->size ? n : v3->size;
	
	while (n)
	{
		if (*d1 > *d2 - *d3) return 0;
		d1 ++;
		d2 ++;
		n --;
	}
	
	return 1;
}

int Vector_greaterThanOrEqualTo_minus_(Vector *self, Vector *v2, Vector *v3)
{
	NUM_TYPE *d1 = self->values;
	NUM_TYPE *d2 = v2->values;
	NUM_TYPE *d3 = v3->values;
	size_t n = self->size;
	n = n < v2->size ? n : v2->size;
	n = n < v3->size ? n : v3->size;
	
	while (n)
	{
		if (*d1 < *d2 - *d3) return 0;
		d1 ++;
		d2 ++;
		n --;
	}
	
	return 1;
}


int Vector_lessThanOrEqualToScalar_(Vector *self, NUM_TYPE v)
{
	NUM_TYPE *d1 = self->values;
	size_t n = self->size;
	
	while (n)
	{
		if (*d1 > v) return 0;
		d1 ++;
		n --;
	}
	
	return 1;
}

int Vector_greaterThanOrEqualToScalar_(Vector *self, NUM_TYPE v)
{
	NUM_TYPE *d1 = self->values;
	size_t n = self->size;
	
	while (n)
	{
		if (*d1 < v) return 0;
		d1 ++;
		n --;
	}
	
	return 1;
}

void Vector_crossProduct_(Vector *self, Vector *other)
{
	PointData *p = (PointData *)self->values;
	PointData *p2 = (PointData *)other->values;
	NUM_TYPE a[3] = { p->x, p->y, p->z };
	NUM_TYPE b[3] = { p2->x, p2->y, p2->z };
	
	NUM_TYPE i = (a[1]*b[2]) - (a[2]*b[1]);
	NUM_TYPE j = (a[2]*b[0]) - (a[0]*b[2]);
	NUM_TYPE k = (a[0]*b[1]) - (a[1]*b[0]);
	
	(p->x) = i;
	(p->y) = j;
	(p->z) = k;
}

NUM_TYPE Vector_distanceTo_(Vector *self, Vector *other)
{
	NUM_TYPE *d1 = self->values;
	NUM_TYPE *d2 = other->values;
	size_t n = self->size;
	NUM_TYPE sum = 0;
	
	if (n != other->size) return 0;
	
	while (n)
	{
		NUM_TYPE d = (*d2 - *d1);
		sum += d * d;
		d1 ++;
		d2 ++;
		n --;
	}
	
	return (NUM_TYPE)sqrt((double)sum);
}

void Vector_ceil(Vector *self)
{
	NUM_TYPE *d1 = self->values;
	size_t n = self->size;
	
	while (n)
	{
		*d1 = (NUM_TYPE)ceil((double)*d1);
		d1 ++; 
		n --;
	}
}

void Vector_floor(Vector *self)
{
	NUM_TYPE *d1 = self->values;
	size_t n = self->size;
	
	while (n)
	{
		*d1 = (NUM_TYPE)floor((double)*d1);
		d1 ++; 
		n --;
	}
}

void Vector_print(Vector *self)
{
	NUM_TYPE *d1 = self->values;
	size_t n = self->size;
	
	printf("vector(");
	while (n)
	{
		
		printf("%f", *d1);
		
		if (n) 
		{ 
			printf(", "); 
		}
		
		d1 ++;
		n --;
	}
	printf(")");
}

int Vector_Min(Vector *self, Vector *other)
{
	NUM_TYPE *d1 = self->values;
	NUM_TYPE *d2 = other->values;
	size_t n = self->size;
	
	if (n != other->size) return -1;
	
	while (n)
	{
		if (*d1 > *d2)
		{
			*d1 = *d2;
		}
		d1 ++;
		d2 ++;
		n --;
	}
	return 0;
}

int Vector_Max(Vector *self, Vector *other)
{
	NUM_TYPE *d1 = self->values;
	NUM_TYPE *d2 = other->values;
	size_t n = self->size;
	
	if (n != other->size) return -1;
	
	while (n)
	{
		if (*d1 < *d2)
		{
			*d1 = *d2;
		}
		d1 ++;
		d2 ++;
		n --;
	}
	return 0;
}

void Vector_zero(Vector *self)
{
	NUM_TYPE *d1 = self->values;
	size_t n = self->size;
	
	while (n)
	{
		*d1 = 0;
		d1 ++; 
		n --;
	}
}

int Vector_isZero(Vector *self)
{
	NUM_TYPE *d1 = self->values;
	size_t n = self->size;
	
	while (n)
	{
		if (*d1) 
		{
			return 0;
		}
		
		d1 ++; 
		n --;
	}
	return 1;
}

void Vector_sign(Vector *self)
{
	NUM_TYPE *d1 = self->values;
	size_t n = self->size;
	
	while (n)
	{
		if (*d1 < 0)
		{
			*d1 = -1;
		} 
		else if(*d1 > 0)
		{
			*d1 = 1;
		}; 
		d1 ++; 
		n --;
	}
}

void Vector_rangeFill(Vector *self)
{
	NUM_TYPE *d1 = self->values;
	size_t i, max = self->size;
	
	for (i = 0; i < max; i ++)
	{
		*d1 = i;
		d1 ++;
	}
}

// vector rangeFill(vector(2, 2), 0) == vector(0, 1, 0, 1)
// vector rangeFill(vector(2, 2), 1) == vector(0, 0, 1, 1)


void Vector_rangeFillWithShapeVectorDim(Vector *self, Vector *shape, size_t d)
{
	Vector_setSize_(self, Vector_product(shape));

	{
		NUM_TYPE *d1 = self->values;
		size_t i, max = self->size;
		NUM_TYPE *s = shape->values;
		size_t *c = calloc(1, sizeof(size_t) * (shape->size + 1)); 
		//size_t dim = 0;
		size_t j;

		if (d > shape->size - 1) return;
		
		for (i = 0; i < max; i ++)
		{
			d1[i] = c[d];
			c[0] ++;
			//printf("c[0] = %i\n", (int)c[0]);
			//printf("s[0] = %i\n", (int)s[0]);
			
			for (j = 0; (j < shape->size) && (c[j] > s[j] - 1); j ++)
			{
				c[j] = 0;
				c[j + 1] ++;
			}
		}
		
		free(c);
	}
}

/*
 void Vector_bitCount(Vector *self)
 {
	 typedef vector unsigned char vUInt8;
	 
	 vector unsigned char CountBits( vUInt8 v )
	 {
		 
		 vUInt8 tab1 = (vUInt8) (0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4);
		 vUInt8 tab2 = vec_add(tab1, vec_splat_u8(1));
		 vUInt8 highBits = vec_sr(v, vec_splat_u8(5) );
		 
		 return vec_add( vec_perm( tab1, tab2, v ), vec_perm( tab1, tab2, highBits ) );
		 
	 }
 }
 */
