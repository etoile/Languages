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

typedef IoObject IoVector;

#define ISVECTOR(self) IoObject_hasCloneFunc_(self, (TagCloneFunc *)IoVector_rawClone)
#define IOVECTOR(vec) IoVector_newWithRawVector_((IoState*)IOSTATE, (Vector *)vec)
#define ISPOINT(self) (ISVECTOR(self) && (IoVector_rawSize(self) == 3))

void *IoMessage_locals_vectorArgAt_(IoMessage *self, void *locals, int n);
void *IoMessage_locals_pointArgAt_(IoMessage *self, void *locals, int n);

IoVector *IoVector_proto(void *state);
IoVector *IoVector_new(void *state);
IoVector *IoVector_newWithSize_(void *state, size_t size);
IoVector *IoVector_newX_y_z_(void *state, NUM_TYPE x, NUM_TYPE y, NUM_TYPE z);
IoVector *IoVector_rawClone(IoVector *self);

IoVector *IoVector_newWithRawVector_(void *state, Vector *vec);

void IoVector_free(IoVector *self);
/*void IoVector_mark(IoVector *self);*/

Vector *IoVector_rawVector(IoVector *self);
PointData *IoVector_rawPointData(IoVector *self);

void IoVector_rawGetXYZ(IoVector *self, NUM_TYPE *x, NUM_TYPE *y, NUM_TYPE *z);
void IoVector_rawSetXYZ(IoVector *self, NUM_TYPE x, NUM_TYPE y, NUM_TYPE z);

IoVector *IoVector_rawCopy(IoVector *self, IoVector *other);
IoObject *IoVector_copy_(IoVector *self, IoObject *locals, IoMessage *m);
IoSeq *IoVector_asBuffer(IoVector *self, IoObject *locals, IoMessage *m);
IoObject *IoVector_subarray(IoVector *self, IoObject *locals, IoMessage *m);

size_t IoVector_rawSize(IoVector *self);
IoObject *IoVector_setSize_(IoVector *self, IoObject *locals, IoMessage *m);
IoObject *IoVector_size(IoVector *self, IoObject *locals, IoMessage *m);
IoObject *IoVector_equals(IoVector *self, IoObject *locals, IoMessage *m);

IoObject *IoVector_at_(IoVector *self, IoObject *locals, IoMessage *m);
IoObject *IoVector_at_put_(IoVector *self, IoObject *locals, IoMessage *m);

/* --- math -------------------------------------------- */
IoObject *IoVector_plusEquals(IoVector *self, IoObject *locals, IoMessage *m);
IoObject *IoVector_plus(IoVector *self, IoObject *locals, IoMessage *m);
IoObject *IoVector_minusEquals(IoVector *self, IoObject *locals, IoMessage *m);
IoObject *IoVector_minus(IoVector *self, IoObject *locals, IoMessage *m);
IoObject *IoVector_multiplyEquals(IoVector *self, IoObject *locals, IoMessage *m);
IoObject *IoVector_multiply(IoVector *self, IoObject *locals, IoMessage *m);
IoObject *IoVector_divideEquals(IoVector *self, IoObject *locals, IoMessage *m);
IoObject *IoVector_divide(IoVector *self, IoObject *locals, IoMessage *m);

IoObject *IoVector_sum(IoVector *self, IoObject *locals, IoMessage *m);
IoObject *IoVector_min(IoVector *self, IoObject *locals, IoMessage *m);
IoObject *IoVector_max(IoVector *self, IoObject *locals, IoMessage *m);
IoObject *IoVector_mean(IoVector *self, IoObject *locals, IoMessage *m);
IoObject *IoVector_square(IoVector *self, IoObject *locals, IoMessage *m);
IoObject *IoVector_normalize(IoVector *self, IoObject *locals, IoMessage *m);
IoObject *IoVector_absolute(IoVector *self, IoObject *locals, IoMessage *m);

IoObject *IoVector_log(IoVector *self, IoObject *locals, IoMessage *m);
IoObject *IoVector_log10(IoVector *self, IoObject *locals, IoMessage *m);
IoObject *IoVector_pow(IoVector *self, IoObject *locals, IoMessage *m);
IoObject *IoVector_random(IoVector *self, IoObject *locals, IoMessage *m);
IoObject *IoVector_sort(IoVector *self, IoObject *locals, IoMessage *m);
IoObject *IoVector_reverseSort(IoVector *self, IoObject *locals, IoMessage *m);

IoObject *IoVector_meanSquare(IoVector *self, IoObject *locals, IoMessage *m);
IoObject *IoVector_rootMeanSquare(IoVector *self, IoObject *locals, IoMessage *m);
IoObject *IoVector_dotProduct(IoVector *self, IoObject *locals, IoMessage *m);
IoObject *IoVector_sin(IoVector *self, IoObject *locals, IoMessage *m);

IoObject *IoVector_setMin_(IoVector *self, IoObject *locals, IoMessage *m);
IoObject *IoVector_setMax_(IoVector *self, IoObject *locals, IoMessage *m);
IoObject *IoVector_set_(IoVector *self, IoObject *locals, IoMessage *m);
IoObject *IoVector_setAll_(IoVector *self, IoObject *locals, IoMessage *m);

IoObject *IoVector_zero(IoVector *self, IoObject *locals, IoMessage *m);
IoObject *IoVector_negate(IoVector *self, IoObject *locals, IoMessage *m);
IoObject *IoVector_lessThanOrEqualTo(IoVector *self, IoObject *locals, IoMessage *m);
IoObject *IoVector_greaterThanOrEqualTo(IoVector *self, IoObject *locals, IoMessage *m);

IoObject *IoVector_ceil(IoVector *self, IoObject *locals, IoMessage *m);
IoObject *IoVector_floor(IoVector *self, IoObject *locals, IoMessage *m);
IoObject *IoVector_distanceTo(IoVector *self, IoObject *locals, IoMessage *m);

IoObject *IoVector_cross(IoVector *self, IoObject *locals, IoMessage *m);
IoObject *IoVector_Min(IoVector *self, IoObject *locals, IoMessage *m);
IoObject *IoVector_Max(IoVector *self, IoObject *locals, IoMessage *m);
IoObject *IoVector_zero(IoVector *self, IoObject *locals, IoMessage *m);
IoObject *IoVector_isZero(IoVector *self, IoObject *locals, IoMessage *m);

/* --- Point --- */
IoObject *IoVector_x(IoVector *self, IoObject *locals, IoMessage *m); 
IoObject *IoVector_y(IoVector *self, IoObject *locals, IoMessage *m);
IoObject *IoVector_z(IoVector *self, IoObject *locals, IoMessage *m);
IoObject *IoVector_w(IoVector *self, IoObject *locals, IoMessage *m);

IoObject *IoVector_setX(IoVector *self, IoObject *locals, IoMessage *m);
IoObject *IoVector_setY(IoVector *self, IoObject *locals, IoMessage *m);
IoObject *IoVector_setZ(IoVector *self, IoObject *locals, IoMessage *m);
IoObject *IoVector_setW(IoVector *self, IoObject *locals, IoMessage *m);

IoObject *IoVector_print(IoVector *self, IoObject *locals, IoMessage *m);

IoObject *IoVector_rangeFill(IoVector *self, IoObject *locals, IoMessage *m);

#endif
