/*#io
System ioDoc(
		   docCopyright("Steve Dekorte", 2002)
		   docLicense("BSD revised")
		   docObject("System")
		   docDescription("Contains methods related to the IoVM.")
		   docCategory("Core")
		   */

#if defined(unix) || defined(__APPLE__) || defined(__NetBSD__)
#include <sys/utsname.h>
#endif

#include "IoSystem.h"
#include "IoNumber.h"
#include "IoMessage_parser.h"

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

