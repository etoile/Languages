/*#io
docCopyright("Steve Dekorte", 2002)
docLicense("BSD revised")
*/

#include "Base.h"

#ifndef DATE_DEFINED
#define DATE_DEFINED 1

#include "Common.h"
#include "Duration.h"
#include "PortableGettimeofday.h"
#include <time.h>
#include "UTinstant.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct 
{
    UTinstant *ut;
    struct timezone tz;
} Date;

Date *Date_new(void);
void Date_copy_(Date *self, Date *other);
void Date_free(Date *self);
int Date_compare(Date *self, Date *other);

void Date_now(Date *self);
void Date_setTimevalue_(Date *self, struct timeval tv);
void Date_setToLocalTimeZone(Date *self);
double Date_Clock(void);

void Date_fromLocalTime_(Date *self, struct tm *t);
void Date_fromTime_(Date *self, time_t t);
time_t Date_asTime(Date *self);

/* --- Zone ----------------------------------------------------------- */
void Date_setToLocalTimeZone(Date *self);
struct timezone Date_timeZone(Date *self);
void Date_setTimeZone_(Date *self, struct timezone tz);
void Date_convertToTimeZone_(Date *self, struct timezone tz);

/* --- Components ----------------------------------------------------- */
void Date_setYear_(Date *self, long y);
long Date_year(Date *self);

void Date_setMonth_(Date *self, int m);
int Date_month(Date *self);

void Date_setDay_(Date *self, int d);
int Date_day(Date *self);

void Date_setHour_(Date *self, int h);
int Date_hour(Date *self);

void Date_setMinute_(Date *self, int m);
int Date_minute(Date *self);

void Date_setSecond_(Date *self, double s);
double Date_second(Date *self);

unsigned char Date_isDaylightSavingsTime(Date *self);
int Date_isLeapYear(Date *self);

/* --- Seconds -------------------------------------------------------- */
double Date_asSeconds(Date *self);
void Date_fromSeconds_(Date *self, double s);

void Date_addSeconds_(Date *self, double s);
double Date_secondsSince_(Date *self, Date *other);

/* --- Format --------------------------------------------------------- */
void Date_fromString_format_(Date *self, const char *s, const char *format);

/* --- Durations ------------------------------------------------------ */
Duration *Date_newDurationBySubtractingDate_(Date *self, Date *other);
void Date_addDuration_(Date *self, Duration *d);
void Date_subtractDuration_(Date *self, Duration *d);

/* -------------------------------------------------------------------- */
double Date_secondsSinceNow(Date *self);

ByteArray *Date_asString(Date *self, const char *format);

#ifdef __cplusplus
}
#endif
#endif
