#include <stdint.h>
#include <stddef.h>
#include <sys/types.h>

#ifdef ERROR_UNSUPPORTED_RUNTIME_FUNCTIONS
#	define OBJC_GNU_RUNTIME_UNSUPPORTED(x) \
		__attribute__((error(x " not supported by the GNU runtime")))
#else
#	define OBJC_GNU_RUNTIME_UNSUPPORTED(x)
#endif

typedef struct objc_ivar* Ivar;

typedef const struct objc_selector *SEL;

typedef struct objc_class *Class;

typedef struct objc_object
{
	Class isa;
} *id;

typedef id (*IMP)(id, SEL, ...);

#ifdef STRICT_APPLE_COMPATIBILITY
typedef signed char BOOL;
#else
#	ifdef __vxwords
typedef  int BOOL
#	else
typedef unsigned char BOOL;
#	endif
#endif


typedef struct objc_method *Method;

typedef void *objc_property_t;
#ifdef __OBJC__
@class Protocol;
#else
typedef struct objc_protocol Protocol;
#endif

#ifndef YES
#define YES ((BOOL)1)
#endif
#ifndef NO 
#define NO ((BOOL)0)
#endif

#ifdef __GNUC
#define _OBJC_NULL_PTR __null
#elif defined(__cplusplus)
#define _OBJC_NULL_PTR 0
#else
#define _OBJC_NULL_PTR ((void*)0)
#endif

#ifndef nil
#define nil ((id)_OBJC_NULL_PTR)
#endif

#ifndef Nil
#define Nil ((Class)_OBJC_NULL_PTR)
#endif

BOOL class_addIvar(Class cls,
                   const char *name,
                   size_t size,
                   uint8_t alignment,
                   const char *types);

BOOL class_addMethod(Class cls, SEL name, IMP imp, const char *types);

BOOL class_addProtocol(Class cls, Protocol *protocol);

BOOL class_conformsToProtocol(Class cls, Protocol *protocol);

Ivar * class_copyIvarList(Class cls, unsigned int *outCount);

Method * class_copyMethodList(Class cls, unsigned int *outCount);

OBJC_GNU_RUNTIME_UNSUPPORTED("Property introspection")
objc_property_t * class_copyPropertyList(Class cls, unsigned int *outCount);

Protocol ** class_copyProtocolList(Class cls, unsigned int *outCount);

id class_createInstance(Class cls, size_t extraBytes);

Method class_getClassMethod(Class aClass, SEL aSelector);

Ivar class_getClassVariable(Class cls, const char* name);

Method class_getInstanceMethod(Class aClass, SEL aSelector);

size_t class_getInstanceSize(Class cls);

Ivar class_getInstanceVariable(Class cls, const char* name);

const char *class_getIvarLayout(Class cls);

IMP class_getMethodImplementation(Class cls, SEL name);

IMP class_getMethodImplementation_stret(Class cls, SEL name);

const char * class_getName(Class cls);

OBJC_GNU_RUNTIME_UNSUPPORTED("Property introspection")
objc_property_t class_getProperty(Class cls, const char *name);

Class class_getSuperclass(Class cls);

int class_getVersion(Class theClass);

OBJC_GNU_RUNTIME_UNSUPPORTED("Weak instance variables")
const char *class_getWeakIvarLayout(Class cls);

BOOL class_isMetaClass(Class cls);

IMP class_replaceMethod(Class cls, SEL name, IMP imp, const char *types);

BOOL class_respondsToSelector(Class cls, SEL sel);

void class_setIvarLayout(Class cls, const char *layout);

__attribute__((deprecated))
Class class_setSuperclass(Class cls, Class newSuper);

void class_setVersion(Class theClass, int version);

OBJC_GNU_RUNTIME_UNSUPPORTED("Weak instance variables")
void class_setWeakIvarLayout(Class cls, const char *layout);

const char * ivar_getName(Ivar ivar);

ptrdiff_t ivar_getOffset(Ivar ivar);

const char * ivar_getTypeEncoding(Ivar ivar);

char * method_copyArgumentType(Method method, unsigned int index);

char * method_copyReturnType(Method method);

void method_exchangeImplementations(Method m1, Method m2);

void method_getArgumentType(Method method, unsigned int index, char *dst, size_t dst_len);

IMP method_getImplementation(Method method);

SEL method_getName(Method method);

unsigned method_getNumberOfArguments(Method method);

void method_getReturnType(Method method, char *dst, size_t dst_len);

const char * method_getTypeEncoding(Method method);

IMP method_setImplementation(Method method, IMP imp);

Class objc_allocateClassPair(Class superclass, const char *name, size_t extraBytes);

id objc_getClass(const char *name);

int objc_getClassList(Class *buffer, int bufferLen);

id objc_getMetaClass(const char *name);

id objc_getRequiredClass(const char *name);

id objc_lookUpClass(const char *name);

Class objc_allocateClassPair(Class superclass, const char *name, size_t extraBytes);

id object_copy(id obj, size_t size);

id object_dispose(id obj);

#define objc_msgSend(theReceiver, theSelector, ...) objc_msg_lookup(theReceiver, theSelector)(theReceiver, theSelector, ## __VA_LIST__);

