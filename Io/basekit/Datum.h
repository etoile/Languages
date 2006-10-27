/*#io
docCopyright("Steve Dekorte", 2004)
docLicense("BSD revised")
docObject("Datum")    
*/

#ifndef Datum_DEFINED
#define Datum_DEFINED 1

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

//#define PID_TYPE uint32_t
#define PID_TYPE size_t
#define PID_FORMAT "zi"

typedef struct
{
    PID_TYPE size;
    unsigned char *data;
} Datum;

PID_TYPE Datum_size(Datum *self);
unsigned char *Datum_data(Datum *self);

// return stack allocated datums 

Datum Datum_FromData_length_(unsigned char *data, PID_TYPE size);
Datum Datum_FromCString_(const char *s);
//Datum Datum_FromPid_(PID_TYPE pid);
Datum Datum_Empty(void);

void *Datum_asByteArray(Datum *self);

Datum Datum_datumAt_(Datum *self, size_t i);
Datum *Datum_newFrom_to_(Datum *self, size_t start, size_t end);

// comparison

int Datum_compare_length_(Datum *self, Datum *other, size_t limit);
int Datum_compare_(Datum *self, Datum *other);
int Datum_compareCString_(Datum *self, const char *s);
int Datum_beginsWith_(Datum *self, Datum *other);
int Datum_endsWith_(Datum *self, Datum *other);
size_t Datum_matchingPrefixSizeWith_(Datum *self, Datum *other);

void *Datum_split_(Datum *self, void *delims); /* returns a List */

//int Datum_next(Datum *self);

unsigned int Datum_hash(Datum *self);

typedef int (DatumDetectWithFunc)(void *, Datum *); /* 1 = match, -1 = break */
int Datum_detect_with_(Datum *self, DatumDetectWithFunc *func, void *target);


#include "ByteArray.h"

Datum Datum_FromByteArray_(ByteArray *ba);

#ifdef __cplusplus
}
#endif
#endif
