#include "runtime.h"
#define objc_object objc_object_gnu
#define id object_ptr_gnu
#define IMP objc_imp_gnu
#define Method objc_method_gnu
#include <objc/objc.h>
#include <objc/objc-api.h>
#undef Method 
#undef IMP
#undef id
#undef objc_object
#include <string.h>
#include <stdlib.h>
#include <assert.h>

/**
 * Private runtime function for updating a dtable.
 */
void __objc_update_dispatch_table_for_class(Class);
/**
 * Private runtime function for determining whether a class responds to a
 * selector.
 */
BOOL __objc_responds_to(Class, SEL);
/**
 *  Runtime library constant for uninitialized dispatch table.
 */
extern struct sarray *__objc_uninstalled_dtable;
/**
 * Mutex used to protect the ObjC runtime library data structures.
 */
extern objc_mutex_t __objc_runtime_mutex;

static Method class_getInstanceMethodNonrecursive(Class aClass, SEL aSelector)
{
	const char *name = sel_get_name(aSelector);
	const char *types = sel_get_type(aSelector);

	for (struct objc_method_list *methods = aClass->methods;
		methods != NULL ; methods = methods->method_next)
	{
		for (int i=0 ; i<methods->method_count ; i++)
		{
			Method_t method = &methods->method_list[i];
			if (strcmp(sel_get_name(method->method_name), name) == 0)
			{
				if (NULL == types || 
					strcmp(types, sel_get_type(method->method_name)) == 0)
				{
					return method;
				}
				// Return NULL if the method exists with this name but has the 
				// wrong types
				return NULL;
			}
		}
	}
	return NULL;
}


static void objc_updateDtableForClassContainingMethod(Method m)
{
	Class nextClass = Nil;
	void *state;
	SEL sel = method_getName(m);
	while (Nil != (nextClass = objc_next_class(&state)))
	{
		if (class_getInstanceMethodNonrecursive(nextClass, sel) == m)
		{
			__objc_update_dispatch_table_for_class(nextClass);
			return;
		}
	}
}


BOOL class_addIvar(Class cls,
                   const char *name,
                   size_t size,
                   uint8_t alignment,
                   const char *types)
{
	if (CLS_ISRESOLV(cls) || CLS_ISMETA(cls))
	{
		return NO;
	}

	struct objc_ivar_list *ivarlist = cls->ivars;

	if (class_getInstanceVariable(cls, name) != NULL) { return NO; }

	ivarlist->ivar_count++;
	// objc_ivar_list contains one ivar.  Others follow it.
	cls->ivars = objc_realloc(ivarlist, sizeof(struct objc_ivar_list) 
			+ (ivarlist->ivar_count - 1) * sizeof(struct objc_ivar));

	Ivar ivar = &cls->ivars->ivar_list[cls->ivars->ivar_count - 1];
	ivar->ivar_name = strdup(name);
	ivar->ivar_type = strdup(types);
	// Round up the offset of the ivar so it is correctly aligned.
	ivar->ivar_offset = cls->instance_size + (cls->instance_size % alignment);
	// Increase the instance size to make space for this.
	cls->instance_size = ivar->ivar_offset + size;
	return YES;
}

BOOL class_addMethod(Class cls, SEL name, IMP imp, const char *types)
{
	const char *methodName = sel_get_name(name);
	struct objc_method_list *methods;
	for (methods=cls->methods; methods!=NULL ; methods=methods->method_next)
	{
		for (int i=0 ; i<methods->method_count ; i++)
		{
			Method_t method = &methods->method_list[i];
			if (strcmp(sel_get_name(method->method_name), methodName) == 0)
			{
				return NO;
			}
		}
	}

	methods = objc_malloc(sizeof(struct objc_method_list));
	methods->method_next = cls->methods;
	cls->methods = methods;

	methods->method_list[1].method_name = sel_get_typed_uid(methodName, types);
	methods->method_list[1].method_types = strdup(types);
	methods->method_list[1].method_imp = (objc_imp_gnu)imp;

	__objc_update_dispatch_table_for_class(cls);

	return YES;
}

BOOL class_addProtocol(Class cls, Protocol *protocol)
{
	if (class_conformsToProtocol(cls, protocol)) { return NO; }
	struct objc_protocol_list *protocols = cls->protocols;
	protocols = objc_malloc(sizeof(struct objc_protocol_list));
	if (protocols == NULL) { return NO; }
	protocols->next = cls->protocols;
	protocols->count = 1;
	protocols->list[0] = protocol;
	cls->protocols = protocols;

	return YES;
}

BOOL class_conformsToProtocol(Class cls, Protocol *protocol)
{
	for (struct objc_protocol_list *protocols = cls->protocols;
		protocols != NULL ; protocols = protocols->next)
	{
		for (int i=0 ; i<protocols->count ; i++)
		{
			if (strcmp(protocols->list[i]->protocol_name, 
						protocol->protocol_name) == 0)
			{
				return YES;
			}
		}
	}
	return NO;
}

Ivar * class_copyIvarList(Class cls, unsigned int *outCount)
{
	struct objc_ivar_list *ivarlist = cls->ivars;
	if (outCount != NULL)
	{
		*outCount = ivarlist->ivar_count;
	}
	size_t size = sizeof(struct objc_ivar) * ivarlist->ivar_count;
	if (size == 0) { return NULL; }
	Ivar *list = malloc(size);
	memcpy(list, &ivarlist->ivar_list, size);
	return list;
}

Method * class_copyMethodList(Class cls, unsigned int *outCount)
{
	size_t size = 0;
	for (struct objc_method_list *methods = cls->methods;
		methods != NULL ; methods = methods->method_next)
	{
		size += methods->method_count;
	}
	
	Method *list = malloc(size * sizeof(struct objc_method));
	Method *copyDest = list;

	for (struct objc_method_list *methods = cls->methods;
		methods != NULL ; methods = methods->method_next)
	{
		memcpy(copyDest, &methods->method_list, 
				methods->method_count * sizeof(struct objc_method));
		copyDest += methods->method_count;
	}

	return list;
}

id class_createInstance(Class cls, size_t extraBytes)
{
	id obj = objc_malloc(cls->instance_size + extraBytes);
	obj->isa = cls;
	return obj;
}

Method class_getInstanceMethod(Class aClass, SEL aSelector)
{
	Method method = class_getInstanceMethodNonrecursive(aClass, aSelector);
	if (method == NULL)
	{
		// TODO: Check if this should be NULL or aClass
		Class superclass = class_getSuperclass(aClass);
		if (superclass == NULL)
		{
			return NULL;
		}
		return class_getInstanceMethod(superclass, aSelector);
	}
	return method;
}

Method class_getClassMethod(Class aClass, SEL aSelector)
{
	return class_getInstanceMethod(aClass->class_pointer, aSelector);
}

Ivar class_getClassVariable(Class cls, const char* name)
{
	assert(0 && "Class variables not implemented");
	return NULL;
}

size_t class_getInstanceSize(Class cls)
{
	return class_get_instance_size(cls);
}

Ivar class_getInstanceVariable(Class cls, const char* name)
{
	struct objc_ivar_list *ivarlist = cls->ivars;

	for (int i=0 ; i<ivarlist->ivar_count ; i++)
	{
		Ivar ivar = &ivarlist->ivar_list[i];
		if (strcmp(ivar->ivar_name, name) == 0)
		{
			return ivar;
		}
	}
	return NULL;
}

const char *class_getIvarLayout(Class cls)
{
	return NULL;
}

IMP class_getMethodImplementation(Class cls, SEL name)
{
	struct objc_object_gnu obj = { cls };
	return (IMP)objc_msg_lookup(&obj, name);
}

IMP class_getMethodImplementation_stret(Class cls, SEL name)
{
	struct objc_object_gnu obj = { cls };
	return (IMP)objc_msg_lookup(&obj, name);
}

const char * class_getName(Class cls)
{
	return class_get_class_name(cls);
}

Class class_getSuperclass(Class cls)
{
	return class_get_super_class(cls);
}

int class_getVersion(Class theClass)
{
	return class_get_version(theClass);
}

const char *class_getWeakIvarLayout(Class cls)
{
	assert(0 && "Weak ivars not supported");
	return NULL;
}

BOOL class_isMetaClass(Class cls)
{
	return CLS_ISMETA(cls);
}

IMP class_replaceMethod(Class cls, SEL name, IMP imp, const char *types)
{
	Method method = class_getInstanceMethodNonrecursive(cls, name);
	if (method == NULL)
	{
		class_addMethod(cls, name, imp, types);
		return NULL;
	}
	IMP old = (IMP)method->method_imp;
	method->method_imp = (objc_imp_gnu)imp;
	return old;
}


BOOL class_respondsToSelector(Class cls, SEL sel)
{
	return __objc_responds_to(cls, sel);
}

void class_setIvarLayout(Class cls, const char *layout)
{
	assert(0 && "Not implemented");
}

__attribute__((deprecated))
Class class_setSuperclass(Class cls, Class newSuper)
{
	Class oldSuper = cls->super_class;
	cls->super_class = newSuper;
	return oldSuper;
}

void class_setVersion(Class theClass, int version)
{
	class_set_version(theClass, version);
}

void class_setWeakIvarLayout(Class cls, const char *layout)
{
	assert(0 && "Not implemented");
}

const char * ivar_getName(Ivar ivar)
{
	return ivar->ivar_name;
}

ptrdiff_t ivar_getOffset(Ivar ivar)
{
	return ivar->ivar_offset;
}

const char * ivar_getTypeEncoding(Ivar ivar)
{
	return ivar->ivar_type;
}

void method_exchangeImplementations(Method m1, Method m2)
{
	IMP tmp = (IMP)m1->method_imp;
	m1->method_imp = m2->method_imp;
	m2->method_imp = (objc_imp_gnu)tmp;
	objc_updateDtableForClassContainingMethod(m1);
	objc_updateDtableForClassContainingMethod(m2);
}

IMP method_getImplementation(Method method)
{
	return (IMP)method->method_imp;
}

SEL method_getName(Method method)
{
	return method->method_name;
}

const char * method_getTypeEncoding(Method method)
{
	return method->method_types;
}

IMP method_setImplementation(Method method, IMP imp)
{
	IMP old = (IMP)method->method_imp;
	method->method_imp = (objc_imp_gnu)old;
	objc_updateDtableForClassContainingMethod(method);
	return old;
}

id objc_getClass(const char *name)
{
	return (id)objc_lookup_class(name);
}

int objc_getClassList(Class *buffer, int bufferLen)
{
	int count = 0;
	if (buffer == NULL)
	{
		void *state = NULL;
		while(Nil != objc_next_class(&state))
		{
			count++;
		}
	}
	else
	{
		Class nextClass;
		void *state = NULL;
		while (Nil != (nextClass = objc_next_class(&state)) && bufferLen > 0)
		{
			count++;
			bufferLen--;
			*(buffer++) = nextClass;
		}
	}
	return count;
}

id objc_getMetaClass(const char *name)
{
	Class cls = (Class)objc_getClass(name);
	return cls == Nil ? nil : (id)cls->class_pointer;
}

id objc_getRequiredClass(const char *name)
{
	id cls = objc_getClass(name);
	if (nil == cls)
	{
		abort();
	}
	return cls;
}

id objc_lookUpClass(const char *name)
{
	// TODO: Check these are the right way around.
	return (id)objc_get_class(name);
}


Class objc_allocateClassPair(Class superclass, const char *name, size_t extraBytes)
{
	// Check the class doesn't already exist.
	if (nil != objc_lookUpClass(name)) { return Nil; }

	Class newClass = calloc(1, sizeof(struct objc_class) + extraBytes);

	if (Nil == newClass) { return Nil; }

	// Create the metaclass
	Class metaClass = calloc(1, sizeof(struct objc_class));

	// Initialize the metaclass
	metaClass->class_pointer = newClass->class_pointer->class_pointer;
	metaClass->super_class = newClass->class_pointer;
	metaClass->info = _CLS_META;
	metaClass->dtable = __objc_uninstalled_dtable;

	// Set up the new class
	newClass->class_pointer = metaClass;
	newClass->super_class = superclass;
	newClass->name = name;
	newClass->info = _CLS_CLASS;
	newClass->dtable = __objc_uninstalled_dtable;

	return newClass;
}

void objc_registerClassPair(Class cls)
{
	Class metaClass = cls->class_pointer;
	// Initialize the dispatch table for the class and metaclass.
	__objc_update_dispatch_table_for_class(metaClass);
	__objc_update_dispatch_table_for_class(cls);
	CLS_SETINITIALIZED(metaClass);
	CLS_SETINITIALIZED(cls);
	// Add pointer from super class
	objc_mutex_lock(__objc_runtime_mutex);
	cls->sibling_class = cls->super_class->subclass_list;
	cls->super_class->subclass_list = cls;
	metaClass->sibling_class = metaClass->super_class->subclass_list;
	metaClass->super_class->subclass_list = metaClass;
	objc_mutex_unlock(__objc_runtime_mutex);


}
