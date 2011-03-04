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

#ifdef __linux__
//#define BRAINDEAD_ABI
#endif

typedef NSInteger SmallInt;

typedef __attribute__ ((__transparent_union__)) union
{
	id object;
	SmallInt smallInt;
} LKObject;

/**
 * Some ABIs (*cough* Linux *cough*) are really stupid.  Rather than passing a
 * union of two register-sized values in a register, they pass it on the stack
 * via a pointer.  
 *
 * To hack around this, we define two types: LKObject and LKObjectPtr.  On sane
 * platforms, these are the same.  On braindead platforms, LKObjectPtr is
 * something different.  You should always declare parameters as LKObjectPtr
 * and then cast them to LKObject using the LKOBJECT() macro if you want to
 * interoperate with code compiled with LanguageKit.
 */
#ifdef BRAINDEAD_ABI
struct LKObject_hack { LKObject hack; };
typedef struct LKObject_hack* LKObjectPtr;
#else
typedef LKObject LKObjectPtr;
#endif
#define LKOBJECT(x) (*(LKObject*)&x)
#define LKOBJECTPTR(x) (*(LKObjectPtr*)&x)

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

