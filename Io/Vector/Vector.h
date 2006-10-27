/*  Copyright (c) 2004, Steve Dekorte
 *  All rights reserved.
 */

#include "Common.h"

#ifndef NUMBERARRAY
#define NUMBERARRAY

#define NUM_TYPE float
//#define NUM_TYPE double
//#define NUM_TYPE_IS_DOUBLE 1

typedef struct
{
    size_t size;
    NUM_TYPE *values;
} Vector;

typedef struct
{
    NUM_TYPE x;
    NUM_TYPE y;
    NUM_TYPE z;
    NUM_TYPE w;
} PointData;

Vector *Vector_new(void);
Vector *Vector_clone(Vector *self);
Vector *Vector_subarray_(Vector *self, size_t localStart, size_t length);
void Vector_free(Vector *self);

void Vector_copy_(Vector *self, Vector *other);
void Vector_copyData_length_(Vector *self, uint8_t *data, size_t length);

uint8_t *Vector_bytes(Vector *self);

int Vector_equals_(Vector *self, Vector *other);

size_t Vector_size(Vector *self);
size_t Vector_sizeInBytes(Vector *self);
void Vector_setSize_(Vector *self, size_t size);

NUM_TYPE Vector_at_(Vector *self, size_t i);
void Vector_at_put_(Vector *self, size_t i, NUM_TYPE v);
void Vector_setXYZ(Vector *self, NUM_TYPE x, NUM_TYPE y, NUM_TYPE z);
void Vector_getXYZ(Vector *self, NUM_TYPE *x, NUM_TYPE *y, NUM_TYPE *z);

/* ------------------------------------------------------------------------ */
void Vector_addScalar_(Vector *self, NUM_TYPE v);
void Vector_subtractScalar_(Vector *self, NUM_TYPE v);
void Vector_multiplyScalar_(Vector *self, NUM_TYPE v);
void Vector_divideScalar_(Vector *self, NUM_TYPE v);

/* ----------------------------------------------------------- */
int Vector_addArray_(Vector *self, Vector *other);
int Vector_addArray_at_(Vector *self, Vector *other, size_t index);
int Vector_addArray_at_xscale_yscale_(Vector *self, Vector *other, long start, NUM_TYPE xscale, NUM_TYPE yscale);
int Vector_subtractArray_(Vector *self, Vector *other);
int Vector_multiplyArray_(Vector *self, Vector *other);
int Vector_divideArray_(Vector *self, Vector *other);

NUM_TYPE Vector_dotProduct_(Vector *self, Vector *other);
NUM_TYPE Vector_product(Vector *self);

void Vector_square(Vector *self);
void Vector_normalize(Vector *self);
void Vector_absolute(Vector *self);

void Vector_log(Vector *self);
void Vector_log10(Vector *self);
void Vector_pow(Vector *self, double n);
void Vector_random(Vector *self, float min, float max);

void Vector_sort(Vector *self);
void Vector_reverseSort(Vector *self);

void Vector_negate(Vector *self);
/* ----------------------------------------------------------- */
NUM_TYPE Vector_sum(Vector *self);
NUM_TYPE Vector_min(Vector *self);
NUM_TYPE Vector_max(Vector *self);
NUM_TYPE Vector_mean(Vector *self);
NUM_TYPE Vector_meanSquare(Vector *self);
NUM_TYPE Vector_rootMeanSquare(Vector *self);

void Vector_setMin_(Vector *self, NUM_TYPE v);
void Vector_setMax_(Vector *self, NUM_TYPE v);
void Vector_set_(Vector *self, NUM_TYPE v);

void Vector_sin(Vector *self);
void Vector_cos(Vector *self);
void Vector_tan(Vector *self);
  
/* ----------------------------------------------------------- */

PointData *Vector_pointData(Vector *self);

int Vector_lessThanOrEqualTo_(Vector *self, Vector *other);
int Vector_greaterThanOrEqualTo_(Vector *self, Vector *other);

int Vector_lessThanOrEqualTo_minus_(Vector *self, Vector *v2, Vector *v3);
int Vector_greaterThanOrEqualTo_minus_(Vector *self, Vector *v2, Vector *v3);

int Vector_lessThanOrEqualToScalar_(Vector *self, NUM_TYPE v);
int Vector_greaterThanOrEqualToScalar_(Vector *self, NUM_TYPE v);

void Vector_crossProduct_(Vector *self, Vector *other);
NUM_TYPE Vector_distanceTo_(Vector *self, Vector *other);
int Vector_Min(Vector *self, Vector *other);
int Vector_Max(Vector *self, Vector *other);

void Vector_ceil(Vector *self);
void Vector_floor(Vector *self);
void Vector_print(Vector *self);

void Vector_zero(Vector *self);
int Vector_isZero(Vector *self);

void Vector_sign(Vector *self);

void Vector_rangeFill(Vector *self);
void Vector_rangeFillWithShapeVectorDim(Vector *self, Vector *shape, size_t d);

#endif
