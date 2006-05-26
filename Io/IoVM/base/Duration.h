/*#io
docCopyright("Steve Dekorte", 2002)
docLicense("BSD revised")
*/

#ifndef DURATION_DEFINED
#define DURATION_DEFINED 1

#include "Common.h"
#include "ByteArray.h"
#include "PortableGettimeofday.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct 
{
    double seconds;
} Duration;

Duration *Duration_new(void);
Duration *Duration_newWithSeconds_(double s);
Duration *Duration_clone(Duration *self);
void Duration_copy_(Duration *self, Duration *other);

void Duration_free(Duration *self);
int Duration_compare(Duration *self, Duration *other);

// components 

int Duration_years(Duration *self);
void Duration_setYears_(Duration *self, double y);

int Duration_days(Duration *self);
void Duration_setDays_(Duration *self, double d);

int Duration_hours(Duration *self);
void Duration_setHours_(Duration *self, double m);

int Duration_minutes(Duration *self);
void Duration_setMinutes_(Duration *self, double m);

double Duration_seconds(Duration *self);
void Duration_setSeconds_(Duration *self, double s);

// total seconds 

double Duration_asSeconds(Duration *self);
void Duration_fromSeconds_(Duration *self, double s);

// strings

ByteArray *Duration_asByteArrayWithFormat_(Duration *self, const char *format);
void Duration_print(Duration *self);

// math

void Duration_add_(Duration *self, Duration *other);
void Duration_subtract_(Duration *self, Duration *other);

#ifdef __cplusplus
}
#endif
#endif
