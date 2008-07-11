#import "StringMap.h"

/**
 * A trivial hash from a string, made by casting the first few characters of a
 * string to an integer.
 */
unsigned simpleStringHash(NSMapTable *table, const void *anObject)
{
	int len = strlen(anObject) + 1;
	if(len >= sizeof(unsigned))
	{
		return *(unsigned *)anObject;
	}
	if(len >= sizeof(unsigned short))
	{
		return (unsigned)*(unsigned short*)anObject;
	}
	if(len >= sizeof(unsigned char))
	{
		return (unsigned)*(unsigned char*)anObject;
	}
	//Should never be reached
	return 0;
}
/**
 * String comparison function.  Simple wrapper around strcmp.
 */
BOOL isCStringEqual(NSMapTable *table, const void * str1, const void * str2)
{
	return strcmp(str1, str2) == 0;
}

