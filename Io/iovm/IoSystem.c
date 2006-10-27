/*#io
System ioDoc(
		   docCopyright("Steve Dekorte", 2002)
		   docLicense("BSD revised")
		   docObject("System")
		   docDescription("Contains methods related to the IoVM.")
		   docCategory("Core")
		   */

#include "IoSystem.h"
#include "IoNumber.h"
#include "IoMessage_parser.h"

#if defined(linux)
#include <unistd.h>
#endif

#if defined(unix) || defined(__APPLE__) || defined(__NetBSD__)
#include <sys/utsname.h>
#ifdef __NetBSD__
# include <sys/param.h>
#endif
#include <sys/sysctl.h>
#endif

#ifdef WIN32
#include <windows.h>

static void setenv(const char *varName, const char* value, int force)
{
	const char *safeValue;
	char *buf;

	if (!varName)
	{
		return;
	}
	
	if (!value)
	{
		safeValue = "";
	}
	else
	{
		safeValue = value;
	}
	
	// buffer for var and value plus '=' and the \0
	buf = (char*)malloc(strlen(varName) + strlen(safeValue) + 2);
	
	if (!buf)
	{
		return;
	}
	
	strcpy(buf, varName);
	strcat(buf, "=");
	strcat(buf, safeValue);

	_putenv(buf);
	free(buf);
}

//#define setenv(k, v, o) SetEnvironmentVariable((k), (v))
#endif

#if defined(__CYGWIN__) || defined(_WIN32)
#include <windows.h>
#endif

IoObject *IoSystem_proto(void *state)
{
	IoMethodTable methodTable[] = { 
	{"errno", IoObject_errnoDescription},
	{"exit", IoObject_exit},
	{"getenv", IoObject_getenv},
	{"setenv", IoObject_setenv},
	{"system", IoObject_system},
	//{"memorySizeOfState", IoObject_memorySizeOfState},
	//{"compactState", IoObject_compactState},
	{"platform", IoObject_platform},
	{"platformVersion", IoObject_platformVersion},
	{"sleep", IoObject_sleep},
	{"activeCpus", IoObject_activeCpus},
	{"createThread", IoObject_createThread},
	{"threadCount", IoObject_threadCount},
	{0x0, 0x0},
	};
	
	IoObject *self = IoObject_new(state);
	IoObject_addMethodTable_(self, methodTable);
	
	IoObject_setSlot_to_(self, IOSYMBOL("version"), IONUMBER(IO_VERSION_NUMBER));
	IoObject_setSlot_to_(self, IOSYMBOL("distribution"), IOSYMBOL("Io"));
	
	return self;
}

/*
IoObject *IoObject_errno(IoObject *self, IoObject *locals, IoMessage *m)
{
	return IONUMBER(errno);
}
*/

#include <stdio.h>
#include <errno.h>

IoObject *IoObject_errnoDescription(IoObject *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("errno", "Returns the C errno string.")
	*/
	return errno ? IOSYMBOL(strerror(errno)) : IONIL(self);
}

IoObject *IoObject_exit(IoObject *self, IoObject *locals, IoMessage *m)
{ 
	/*#io
	docSlot("exit(optionalReturnCodeNumber)", 
		   "Shutdown the IoState (free all objects) and return 
control to the calling program (if any). ")
	*/
	
	int returnCode = 0;
	
	if (IoMessage_argCount(m))
	{
		returnCode = IoMessage_locals_intArgAt_(m, locals, 0);
	}
	
	IoState_exit(IOSTATE, returnCode); 
	return self; 
}

IoObject *IoObject_getenv(IoObject *self, IoObject *locals, IoMessage *m)
{ 
	/*#io
	docSlot("getenv(nameString)", 
		   "Returns a string with the value of the environment 
variable whose name is specified by nameString.") 
	*/
	
	IoSymbol *key = IoMessage_locals_symbolArgAt_(m, locals, 0);
	char *s = getenv(CSTRING(key));
	
	if (!s) 
	{
		return ((IoState *)IOSTATE)->ioNil;
	}
	
	return IoState_symbolWithCString_(IOSTATE, s);
}

IoObject *IoObject_system(IoObject *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("system(aString)", 
		   "Makes a system call and returns a Number for the return value.")
	*/
	
	IoSymbol *s = IoMessage_locals_symbolArgAt_(m, locals, 0);
	int result = system(CSTRING(s));
	//printf("system result = %i", result);
	return IONUMBER((double)result);
}

IoObject *IoObject_memorySizeOfState(IoObject *self, IoObject *locals, IoMessage *m)
{ 
	/*
	 docSlot("memorySizeOfState", 
		    "Returns the number of bytes in the IoState 
	 (this may not include memory allocated by C libraries).")
	 */
	
	return IONUMBER(0); 
	//return IONUMBER(IoState_memorySize(IOSTATE)); 
}

IoObject *IoObject_compactState(IoObject *self, IoObject *locals, IoMessage *m)
{ 
	/*
	 docSlot("compactState", 
		    "Attempt to compact the memory of the IoState if possible.")
	 */
	
	//IoState_compact(IOSTATE); 
	return self; 
}
 
IoObject *IoObject_setenv(IoObject *self, IoObject *locals, IoMessage *m)
{ 
	// setenv() takes different args in different implementations
	IoSymbol *key = IoMessage_locals_symbolArgAt_(m, locals, 0);
	IoSymbol *value = IoMessage_locals_symbolArgAt_(m, locals, 1);
	setenv(CSTRING(key), CSTRING(value), 1); 
	return self;
}

IoObject *IoObject_platform(IoObject *self, IoObject *locals, IoMessage *m)
{
	/*#io
	 docSlot("platform", "Returns a string description of the platform.")
	 */
	
	char *platform = "Unknown";

#if defined(__CYGWIN__)

	platform = "cygwin";

#elif defined(_WIN32)

	OSVERSIONINFO os;
	
	os.dwOSVersionInfoSize = sizeof( OSVERSIONINFO );
	GetVersionEx(&os);
	
	switch(os.dwPlatformId) 
	{
		case VER_PLATFORM_WIN32_WINDOWS:
			switch(os.dwMinorVersion)
			{
				case 0:
					platform = "Windows 95";
					break;
				case 10:
					platform = "Windows 98";
					break;
				case 90:
					platform = "Windows ME";
					break;
				default:
					platform = "Windows 9X";
					break;
			}
			break;
			
		case VER_PLATFORM_WIN32_NT:
			if (os.dwMajorVersion == 3 || os.dwMajorVersion == 4)
			{
				platform = "Windows NT"; 
			}
			else if (os.dwMajorVersion == 5)
			{
				switch(os.dwMinorVersion)
				{
					case 0:
						platform = "Windows 2000";
						break;
					case 1:
						platform = "Windows XP";
						break;
					default:
						platform = "Windows";
						break;
				}
			}
			break;
	}
	
#elif defined(unix) || defined(__APPLE__) || defined(__NetBSD__)
	/* Why Apple and NetBSD don't define 'unix' I'll never know. */
	struct utsname os;
	int ret = uname(&os);
	
	if (ret == 0) 
	{
		platform = os.sysname;
	}
#endif
	
	return IoState_symbolWithCString_(self->state, platform);
}

IoObject *IoObject_platformVersion(IoObject *self, IoObject *locals, IoMessage *m)
{
    char platformVersion[256];

#if defined(_WIN32)

	OSVERSIONINFO os;
	
	os.dwOSVersionInfoSize = sizeof( OSVERSIONINFO );
	GetVersionEx(&os);
	
	snprintf(platformVersion, sizeof(platformVersion) - 1, "%d.%d",
		os.dwMajorVersion, os.dwMinorVersion);

#elif defined(unix) || defined(__APPLE__) || defined(__NetBSD__)
	/* Why Apple and NetBSD don't define 'unix' I'll never know. */
	struct utsname os;
	int ret = uname(&os);
	
	if (ret == 0) 
	{
		snprintf(platformVersion, sizeof(platformVersion) - 1, os.release);
	}
#endif
	
	return IoState_symbolWithCString_(self->state, platformVersion);
}

IoObject *IoObject_activeCpus(IoObject *self, IoObject *locals, IoMessage *m)
{
	/*#io
	 docSlot("activeCpus", "Returns the number of active CPUs.")
	 */
	int cpus = 1;
#if defined(CTL_HW)
	int mib[2];
	size_t len = sizeof(cpus);
	mib[0] = CTL_HW;
#if defined(HW_AVAILCPU)
	mib[1] = HW_AVAILCPU;
#elif defined(HW_NCPU)
	mib[1] = HW_NCPU;
#else
#error
#endif
	sysctl(mib, 2, &cpus, &len, NULL, 0);
#elif defined(_SC_NPROCESSORS_ONLN)
	cpus = sysconf(_SC_NPROCESSORS_ONLN);
#elif defined(_SC_NPROC_ONLN)
	cpus = sysconf(_SC_NPROC_ONLN);
#elif defined(WIN32)
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	cpus = si.dwNumberOfProcessors;
#else
#error
#endif
	return IONUMBER(cpus);
}

#include "PortableUsleep.h"

IoObject *IoObject_sleep(IoObject *self, IoObject *locals, IoMessage *m)
{
	double seconds = IoMessage_locals_doubleArgAt_(m, locals, 0);
	unsigned int microseconds = (seconds * 1000000);
	usleep(microseconds);
	return self;
}

/*#io
 docSlot("version", "Returns a version number for Io.")
 */

/*#io
 docSlot("distribution", "Returns the Io distribution name as a string.")
 */

#include "IoState_threads.h"

IoObject *IoObject_createThread(IoObject *self, IoObject *locals, IoMessage *m)
{
	IoSeq *s = IoMessage_locals_seqArgAt_(m, locals, 0);
	IoState_createThreadAndEval(IOSTATE, CSTRING(s));
	return self;
}

IoObject *IoObject_threadCount(IoObject *self, IoObject *locals, IoMessage *m)
{
	Thread_Init(); // I don't like this call, but if it hasn't been called
	               // before we get a crash.  I think when Io starts
	               // Thread_Init should be called.
	return IONUMBER(IoState_threadCount(IOSTATE));
}

