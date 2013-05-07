#import <EtoileFoundation/EtoileCompatibility.h>
#import <objc/runtime.h>

/**
 * If we have small object support in the runtime, then we just use it and get
 * rid of all of the hackery.
 */
#pragma clang diagnostic ignored "-Wdeprecated-objc-pointer-introspection"
#ifdef OBJC_SMALL_OBJECT_SHIFT

@class NSSmallInt;

typedef NSSmallInt* SmallInt;

typedef id LKObject;

#define LKOBJECT(x) x

__attribute__((unused))
static inline BOOL LKObjectIsSmallInt(LKObject obj)
{
	return ((NSInteger)obj & OBJC_SMALL_OBJECT_MASK) != 0;
}

__attribute__((unused))
static id LKObjectToId(LKObject obj)
{
	return obj;
}

__attribute__((unused))
static inline BOOL LKObjectIsObject(LKObject obj)
{
	return ((NSInteger)obj & OBJC_SMALL_OBJECT_MASK) == 0;
}

__attribute__((unused))
static inline NSInteger NSIntegerFromSmallInt(LKObject smallInt)
{
	return [smallInt integerValue];
}
__attribute__((unused))
static inline LKObject LKObjectFromObject(id obj)
{
	return obj;
}


#else

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
static inline NSInteger NSIntegerFromSmallInt(LKObject smallInt)
{
	NSCAssert(smallInt.smallInt & 1, @"Not a SmallInt!");
	return smallInt.smallInt >> 1;
}

__attribute__((unused))
static inline LKObject LKObjectFromObject(id obj)
{
	LKObject o;
	o.object = obj;
	return o;
}

#endif
