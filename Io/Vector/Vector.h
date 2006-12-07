/*  Copyright (c) 2004, Steve Dekorte
 *  All rights reserved.
 */

#include "IoVectorApi.h"

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

IOVECTOR_API Vector *Vector_new(void);
IOVECTOR_API Vector *Vector_clone(Vector *self);
IOVECTOR_API Vector *Vector_subarray_(Vector *self, size_t localStart, size_t length);
IOVECTOR_API void Vector_free(Vector *self);

IOVECTOR_API void Vector_copy_(Vector *self, Vector *other);
IOVECTOR_API void Vector_copyData_length_(Vector *self, uint8_t *data, size_t length);

uint8_t *Vector_bytes(Vector *self);

IOVECTOR_API int Vector_equals_(Vector *self, Vector *other);

IOVECTOR_API size_t Vector_size(Vector *self);
IOVECTOR_API size_t Vector_sizeInBytes(Vector *self);
IOVECTOR_API void Vector_setSize_(Vector *self, size_t size);

IOVECTOR_API NUM_TYPE Vector_at_(Vector *self, size_t i);
IOVECTOR_API void Vector_at_put_(Vector *self, size_t i, NUM_TYPE v);
IOVECTOR_API void Vector_setXYZ(Vector *self, NUM_TYPE x, NUM_TYPE y, NUM_TYPE z);
IOVECTOR_API void Vector_getXYZ(Vector *self, NUM_TYPE *x, NUM_TYPE *y, NUM_TYPE *z);

/* ------------------------------------------------------------------------ */
IOVECTOR_API void Vector_addScalar_(Vector *self, NUM_TYPE v);
IOVECTOR_API void Vector_subtractScalar_(Vector *self, NUM_TYPE v);
IOVECTOR_API void Vector_multiplyScalar_(Vector *self, NUM_TYPE v);
IOVECTOR_API void Vector_divideScalar_(Vector *self, NUM_TYPE v);

/* ----------------------------------------------------------- */
IOVECTOR_API int Vector_addArray_(Vector *self, Vector *other);
IOVECTOR_API int Vector_addArray_at_(Vector *self, Vector *other, size_t index);
IOVECTOR_API int Vector_addArray_at_xscale_yscale_(Vector *self, Vector *other, long start, NUM_TYPE xscale, NUM_TYPE yscale);
IOVECTOR_API int Vector_subtractArray_(Vector *self, Vector *other);
IOVECTOR_API int Vector_multiplyArray_(Vector *self, Vector *other);
IOVECTOR_API int Vector_divideArray_(Vector *self, Vector *other);

IOVECTOR_API NUM_TYPE Vector_dotProduct_(Vector *self, Vector *other);
IOVECTOR_API NUM_TYPE Vector_product(Vector *self);

IOVECTOR_API void Vector_square(Vector *self);
IOVECTOR_API void Vector_normalize(Vector *self);
IOVECTOR_API void Vector_absolute(Vector *self);

IOVECTOR_API void Vector_log(Vector *self);
IOVECTOR_API void Vector_log10(Vector *self);
IOVECTOR_API void Vector_pow(Vector *self, double n);
IOVECTOR_API void Vector_random(Vector *self, float min, float max);

IOVECTOR_API void Vector_sort(Vector *self);
IOVECTOR_API void Vector_reverseSort(Vector *self);

IOVECTOR_API void Vector_negate(Vector *self);
/* ----------------------------------------------------------- */
IOVECTOR_API NUM_TYPE Vector_sum(Vector *self);
IOVECTOR_API NUM_TYPE Vector_min(Vector *self);
IOVECTOR_API NUM_TYPE Vector_max(Vector *self);
IOVECTOR_API NUM_TYPE Vector_mean(Vector *self);
IOVECTOR_API NUM_TYPE Vector_meanSquare(Vector *self);
IOVECTOR_API NUM_TYPE Vector_rootMeanSquare(Vector *self);

IOVECTOR_API void Vector_setMin_(Vector *self, NUM_TYPE v);
IOVECTOR_API void Vector_setMax_(Vector *self, NUM_TYPE v);
IOVECTOR_API void Vector_set_(Vector *self, NUM_TYPE v);

IOVECTOR_API void Vector_sin(Vector *self);
IOVECTOR_API void Vector_cos(Vector *self);
IOVECTOR_API void Vector_tan(Vector *self);
  
/* ----------------------------------------------------------- */

IOVECTOR_API PointData *Vector_pointData(Vector *self);

IOVECTOR_API int Vector_lessThanOrEqualTo_(Vector *self, Vector *other);
IOVECTOR_API int Vector_greaterThanOrEqualTo_(Vector *self, Vector *other);

IOVECTOR_API int Vector_lessThanOrEqualTo_minus_(Vector *self, Vector *v2, Vector *v3);
IOVECTOR_API int Vector_greaterThanOrEqualTo_minus_(Vector *self, Vector *v2, Vector *v3);

IOVECTOR_API int Vector_lessThanOrEqualToScalar_(Vector *self, NUM_TYPE v);
IOVECTOR_API int Vector_greaterThanOrEqualToScalar_(Vector *self, NUM_TYPE v);

IOVECTOR_API void Vector_crossProduct_(Vector *self, Vector *other);
IOVECTOR_API NUM_TYPE Vector_distanceTo_(Vector *self, Vector *other);
IOVECTOR_API int Vector_Min(Vector *self, Vector *other);
IOVECTOR_API int Vector_Max(Vector *self, Vector *other);

IOVECTOR_API void Vector_ceil(Vector *self);
IOVECTOR_API void Vector_floor(Vector *self);
IOVECTOR_API void Vector_print(Vector *self);

IOVECTOR_API void Vector_zero(Vector *self);
IOVECTOR_API int Vector_isZero(Vector *self);

IOVECTOR_API void Vector_sign(Vector *self);

IOVECTOR_API void Vector_rangeFill(Vector *self);
IOVECTOR_API void Vector_rangeFillWithShapeVectorDim(Vector *self, Vector *shape, size_t d);

#endif
