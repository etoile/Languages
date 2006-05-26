
#ifndef PORTABLEGETTIMEOFDAY_DEFINED
#define PORTABLEGETTIMEOFDAY_DEFINED 1

#ifdef __cplusplus
extern "C" {
#endif

    #if defined(__WIN32__) || defined(WIN32) || defined(_WIN32) || defined(_MSC_VER)
    #if defined(_MSC_VER)
	#include <winsock2.h>
/*	struct timeval 
	{
	  long tv_sec;   
	  long tv_usec; 
	};*/
    #else
    /* for MingW */
    #include <sys/time.h>
    #endif

	struct timezone 
	{
	   int tz_minuteswest; /* of Greenwich */
	   int tz_dsttime;     /* type of dst correction to apply */
	};

	extern void gettimeofday(struct timeval *tv, struct timezone *tz);

    #else
	#include <sys/time.h>
    #endif

#ifdef __cplusplus
}
#endif

#endif

double secondsSince1970(void);
