
#include <time.h>
#include "PortableGettimeofday.h"

#if defined(__WIN32__) || defined(WIN32) || defined(_WIN32) || defined(_MSC_VER)

#ifndef IO_ADDON_Sockets 
void gettimeofday(struct timeval *tv, struct timezone *tz)
{
  time_t rawtime;
  time(&rawtime);
  tv->tv_sec = (long)rawtime;
  tv->tv_usec = 0;
}
#endif

#else

/* just to make compiler happy */
void PorableGettimeOfday(void) 
{ 
}

#endif

double secondsSince1970(void)
{
  double result;
  struct timeval tv;
  struct timezone tz;
  gettimeofday(&tv, &tz);
  result = tv.tv_sec;
  result += tv.tv_usec / 1000000.0;
  return result;
}




