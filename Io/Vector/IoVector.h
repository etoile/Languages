/*#io
docCopyright("Steve Dekorte", 2002)
docLicense("BSD revised")
*/

#include "Common.h"
#include "IoObject.h"
#include "IoSeq.h"
#include "Vector.h"

#ifndef IOVECTOR_DEFINED
#define IOVECTOR_DEFINED 1

#include "IoVectorApi.h"

typedef IoObject IoVector;

#define ISVECTOR(self) IoObject_hasCloneFunc_(self, (TagCloneFunc *)IoVector_rawClone)
#define IOVECTOR(vec) IoVector_newWithRawVector_((IoState*)IOSTATE, (Vector *)vec)
#define ISPOINT(self) (ISVECTOR(self) && (IoVector_rawSize(self) == 3))

IOVECTOR_API void *IoMessage_locals_vectorArgAt_(IoMessage *self, void *locals, int n);
IOVECTOR_API void *IoMessage_locals_pointArgAt_(IoMessage *self, void *locals, int n);

IOVECTOR_API IoVector *IoVector_proto(void *state);
IOVECTOR_API IoVector *IoVector_new(void *state);
IOVECTOR_API IoVector *IoVector_newWithSize_(void *state, size_t size);
IOVECTOR_API IoVector *IoVector_newX_y_z_(void *state, NUM_TYPE x, NUM_TYPE y, NUM_TYPE z);
IOVECTOR_API IoVector *IoVector_rawClone(IoVector *self);

IOVECTOR_API IoVector *IoVector_newWithRawVector_(void *state, Vector *vec);

IOVECTOR_API void IoVector_free(IoVector *self);
/*void IoVector_mark(IoVector *self);*/

IOVECTOR_API Vector *IoVector_rawVector(IoVector *self);
IOVECTOR_API PointData *IoVector_rawPointData(IoVector *self);

IOVECTOR_API void IoVector_rawGetXYZ(IoVector *self, NUM_TYPE *x, NUM_TYPE *y, NUM_TYPE *z);
IOVECTOR_API void IoVector_rawSetXYZ(IoVector *self, NUM_TYPE x, NUM_TYPE y, NUM_TYPE z);

IOVECTOR_API IoVector *IoVector_rawCopy(IoVector *self, IoVector *other);
IOVECTOR_API IoObject *IoVector_copy_(IoVector *self, IoObject *locals, IoMessage *m);
IOVECTOR_API IoSeq *IoVector_asBuffer(IoVector *self, IoObject *locals, IoMessage *m);
IOVECTOR_API IoObject *IoVector_subarray(IoVector *self, IoObject *locals, IoMessage *m);

IOVECTOR_API size_t IoVector_rawSize(IoVector *self);
IOVECTOR_API IoObject *IoVector_setSize_(IoVector *self, IoObject *locals, IoMessage *m);
IOVECTOR_API IoObject *IoVector_size(IoVector *self, IoObject *locals, IoMessage *m);
IOVECTOR_API IoObject *IoVector_equals(IoVector *self, IoObject *locals, IoMessage *m);

IOVECTOR_API IoObject *IoVector_at_(IoVector *self, IoObject *locals, IoMessage *m);
IOVECTOR_API IoObject *IoVector_at_put_(IoVector *self, IoObject *locals, IoMessage *m);

/* --- math -------------------------------------------- */
IOVECTOR_API IoObject *IoVector_plusEquals(IoVector *self, IoObject *locals, IoMessage *m);
IOVECTOR_API IoObject *IoVector_plus(IoVector *self, IoObject *locals, IoMessage *m);
IOVECTOR_API IoObject *IoVector_minusEquals(IoVector *self, IoObject *locals, IoMessage *m);
IOVECTOR_API IoObject *IoVector_minus(IoVector *self, IoObject *locals, IoMessage *m);
IOVECTOR_API IoObject *IoVector_multiplyEquals(IoVector *self, IoObject *locals, IoMessage *m);
IOVECTOR_API IoObject *IoVector_multiply(IoVector *self, IoObject *locals, IoMessage *m);
IOVECTOR_API IoObject *IoVector_divideEquals(IoVector *self, IoObject *locals, IoMessage *m);
IOVECTOR_API IoObject *IoVector_divide(IoVector *self, IoObject *locals, IoMessage *m);

IOVECTOR_API IoObject *IoVector_sum(IoVector *self, IoObject *locals, IoMessage *m);
IOVECTOR_API IoObject *IoVector_min(IoVector *self, IoObject *locals, IoMessage *m);
IOVECTOR_API IoObject *IoVector_max(IoVector *self, IoObject *locals, IoMessage *m);
IOVECTOR_API IoObject *IoVector_mean(IoVector *self, IoObject *locals, IoMessage *m);
IOVECTOR_API IoObject *IoVector_square(IoVector *self, IoObject *locals, IoMessage *m);
IOVECTOR_API IoObject *IoVector_normalize(IoVector *self, IoObject *locals, IoMessage *m);
IOVECTOR_API IoObject *IoVector_absolute(IoVector *self, IoObject *locals, IoMessage *m);

IOVECTOR_API IoObject *IoVector_log(IoVector *self, IoObject *locals, IoMessage *m);
IOVECTOR_API IoObject *IoVector_log10(IoVector *self, IoObject *locals, IoMessage *m);
IOVECTOR_API IoObject *IoVector_pow(IoVector *self, IoObject *locals, IoMessage *m);
IOVECTOR_API IoObject *IoVector_random(IoVector *self, IoObject *locals, IoMessage *m);
IOVECTOR_API IoObject *IoVector_sort(IoVector *self, IoObject *locals, IoMessage *m);
IOVECTOR_API IoObject *IoVector_reverseSort(IoVector *self, IoObject *locals, IoMessage *m);

IOVECTOR_API IoObject *IoVector_meanSquare(IoVector *self, IoObject *locals, IoMessage *m);
IOVECTOR_API IoObject *IoVector_rootMeanSquare(IoVector *self, IoObject *locals, IoMessage *m);
IOVECTOR_API IoObject *IoVector_dotProduct(IoVector *self, IoObject *locals, IoMessage *m);
IOVECTOR_API IoObject *IoVector_sin(IoVector *self, IoObject *locals, IoMessage *m);

IOVECTOR_API IoObject *IoVector_setMin_(IoVector *self, IoObject *locals, IoMessage *m);
IOVECTOR_API IoObject *IoVector_setMax_(IoVector *self, IoObject *locals, IoMessage *m);
IOVECTOR_API IoObject *IoVector_set_(IoVector *self, IoObject *locals, IoMessage *m);
IOVECTOR_API IoObject *IoVector_setAll_(IoVector *self, IoObject *locals, IoMessage *m);

IOVECTOR_API IoObject *IoVector_zero(IoVector *self, IoObject *locals, IoMessage *m);
IOVECTOR_API IoObject *IoVector_negate(IoVector *self, IoObject *locals, IoMessage *m);
IOVECTOR_API IoObject *IoVector_lessThanOrEqualTo(IoVector *self, IoObject *locals, IoMessage *m);
IOVECTOR_API IoObject *IoVector_greaterThanOrEqualTo(IoVector *self, IoObject *locals, IoMessage *m);

IOVECTOR_API IoObject *IoVector_ceil(IoVector *self, IoObject *locals, IoMessage *m);
IOVECTOR_API IoObject *IoVector_floor(IoVector *self, IoObject *locals, IoMessage *m);
IOVECTOR_API IoObject *IoVector_distanceTo(IoVector *self, IoObject *locals, IoMessage *m);

IOVECTOR_API IoObject *IoVector_cross(IoVector *self, IoObject *locals, IoMessage *m);
IOVECTOR_API IoObject *IoVector_Min(IoVector *self, IoObject *locals, IoMessage *m);
IOVECTOR_API IoObject *IoVector_Max(IoVector *self, IoObject *locals, IoMessage *m);
IOVECTOR_API IoObject *IoVector_zero(IoVector *self, IoObject *locals, IoMessage *m);
IOVECTOR_API IoObject *IoVector_isZero(IoVector *self, IoObject *locals, IoMessage *m);
IOVECTOR_API IoObject *IoVector_sign(IoVector *self, IoObject *locals, IoMessage *m);

/* --- Point --- */
IOVECTOR_API IoObject *IoVector_x(IoVector *self, IoObject *locals, IoMessage *m); 
IOVECTOR_API IoObject *IoVector_y(IoVector *self, IoObject *locals, IoMessage *m);
IOVECTOR_API IoObject *IoVector_z(IoVector *self, IoObject *locals, IoMessage *m);
IOVECTOR_API IoObject *IoVector_w(IoVector *self, IoObject *locals, IoMessage *m);

IOVECTOR_API IoObject *IoVector_setX(IoVector *self, IoObject *locals, IoMessage *m);
IOVECTOR_API IoObject *IoVector_setY(IoVector *self, IoObject *locals, IoMessage *m);
IOVECTOR_API IoObject *IoVector_setZ(IoVector *self, IoObject *locals, IoMessage *m);
IOVECTOR_API IoObject *IoVector_setW(IoVector *self, IoObject *locals, IoMessage *m);

IOVECTOR_API IoObject *IoVector_print(IoVector *self, IoObject *locals, IoMessage *m);

IOVECTOR_API IoObject *IoVector_rangeFill(IoVector *self, IoObject *locals, IoMessage *m);

#endif
