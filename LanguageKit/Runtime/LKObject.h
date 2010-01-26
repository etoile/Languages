/**
 * LKObject.h defines the type used for LanguageKit object and some functions
 * for mapping these to and from Objective-C values.  Objects in LanguageKit
 * follow the Smalltalk model.  They are either object pointers or small
 * integer values stored in the most significant sizeof(void*)-1 bits of the
 * pointer.  
 *
 * Note: In future versions of LanguageKit, on 64-bit platforms, LK objects may
 * be modified to contain 62-bit integers, 32-bit floats, or pointers.
 */

#import <Foundation/Foundation.h>

typedef NSInteger SmallInt;

typedef union 
{
	id object;
	SmallInt smallInt;
} LKObject;

#import "BigInt.h"

static inline BOOL LKObjectIsSmallInt(LKObject obj)
{
	return (obj.smallInt & 1);
}

static inline BOOL LKObjectIsObject(LKObject obj)
{
	return (obj.smallInt & 1) == 0;
}

static inline NSInteger NSIntegerFromSmallInt(SmallInt smallInt)
{
	NSCAssert(smallInt & 1, @"Not a SmallInt!");
	return smallInt >> 1;
}

static inline LKObject LKObjectFromNSInteger(NSInteger integer)
{
	LKObject obj;
	if((integer << 1 >> 1) != integer)
	{
		obj.object = [BigInt bigIntWithLongLong: (long long)integer];
	}
	else
	{
		obj.smallInt = (integer << 1) | 1;
	}
	return obj;
}
