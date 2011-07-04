#import <EtoileFoundation/EtoileCompatibility.h>

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

typedef NSInteger SmallInt;

typedef __attribute__ ((__transparent_union__)) union
{
	/**
	 * The object value.  
	 */
	__unsafe_unretained id object;
	SmallInt smallInt;
} LKObject;


#define LKOBJECT(x) (*(__bridge LKObject*)&x)

__attribute__((unused))
static inline BOOL LKObjectIsSmallInt(LKObject obj)
{
	return (obj.smallInt & 1);
}

__attribute__((unused))
static id LKObjectToId(LKObject obj)
{
	if (1 == (obj.smallInt & 1))
	{
		return nil;
	}
	return obj.object;
}

__attribute__((unused))
static inline BOOL LKObjectIsObject(LKObject obj)
{
	return (obj.smallInt & 1) == 0;
}

__attribute__((unused))
static inline NSInteger NSIntegerFromSmallInt(SmallInt smallInt)
{
	NSCAssert(smallInt & 1, @"Not a SmallInt!");
	return smallInt >> 1;
}

