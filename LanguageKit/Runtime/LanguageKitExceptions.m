#import <Foundation/NSObject.h>
#import <Foundation/NSString.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

/**
 * Typedef to allow prototypes from EH documentation to work unmodified
 */
typedef uint64_t uint64;

/**
 * Standard exception handling definitions which don't seem to be in a header.
 */
typedef enum {
	_URC_NO_REASON = 0,
	_URC_FOREIGN_EXCEPTION_CAUGHT = 1,
	_URC_FATAL_PHASE2_ERROR = 2,
	_URC_FATAL_PHASE1_ERROR = 3,
	_URC_NORMAL_STOP = 4,
	_URC_END_OF_STACK = 5,
	_URC_HANDLER_FOUND = 6,
	_URC_INSTALL_CONTEXT = 7,
	_URC_CONTINUE_UNWIND = 8
} _Unwind_Reason_Code;

typedef void (*_Unwind_Exception_Cleanup_Fn)
	(_Unwind_Reason_Code reason,
			  void *exc);

struct _Unwind_Exception {
	uint64			 exception_class;
	_Unwind_Exception_Cleanup_Fn exception_cleanup;
	uint64			 private_1;
	uint64			 private_2;
};

_Unwind_Reason_Code _Unwind_RaiseException
	  ( struct _Unwind_Exception *exception_object );

void _Unwind_Resume (struct _Unwind_Exception *exception_object);

struct _Unwind_Context;
uintptr_t _Unwind_GetIP(struct _Unwind_Context *context);
uintptr_t _Unwind_GetLanguageSpecificData(struct _Unwind_Context *context);
uintptr_t _Unwind_GetRegionStart(struct _Unwind_Context *context);

typedef int _Unwind_Action;
static const _Unwind_Action _UA_SEARCH_PHASE = 1;
static const _Unwind_Action _UA_CLEANUP_PHASE = 2;
static const _Unwind_Action _UA_HANDLER_FRAME = 4;
static const _Unwind_Action _UA_FORCE_UNWIND = 8;


/**
 * Thread-local LanguageKit exception object.  Could be allocated with
 * malloc/free, but not really worth it.
 */
__thread struct 
{
	/** The unwinding lib's exception object. */
	struct _Unwind_Exception exception;
	/** The context object for the method frame for the block. */
	void *context;
	/** The value to be returned.  */
	void *retval;
} LanguageKitException;

/** 
 * Read an unsigned, little-endian, base-128, DWARF value.  Updates *data to
 * point to the end of the value.
 */
static size_t read_uleb128(unsigned char** data)
{
	size_t uleb = 0;
	int bit = 0;
	unsigned char digit = 0;
	// We have to read at least one octet, and keep reading until we get to one
	// with the high bit unset
	do
	{
		// This check is a bit too strict - we should also check the highest
		// bit of the digit.
		assert(bit < sizeof(size_t) * 8);
		// Get the base 128 digit 
		digit = (**data) & 0x7f;
		// Add it to the current value
		uleb += digit << bit;
		// Proceed to the next octet
		(*data)++;
	} while ((**data) == digit);

	return uleb;
}

/**
 * Read a long DWARF value.
 */
static size_t read_long(unsigned char **data)
{
	size_t value = *(size_t*)(*data);
	*data += sizeof(size_t);
	return value;
}

/**
 * Try to read the expected value and fail loudly if you can't.
 */
static inline void expect(unsigned char **data, unsigned char value)
{
	if (**data != value)
	{
		fprintf(stderr, "Expected byte %x, got %x\n", (int)value, (int)**data);
		abort();
	}
	(*data)++;
}

/**
 * Look up the landing pad that corresponds to the current invoke.
 */
static size_t landingPadForInvoke(struct _Unwind_Context *context)
{
	size_t invokesite = _Unwind_GetIP(context) - _Unwind_GetRegionStart(context);
	unsigned char *tables = 
		(unsigned char*)_Unwind_GetLanguageSpecificData(context);
	expect(&tables, 0xff);
	expect(&tables, 0x00);
	read_uleb128(&tables);

	if (*tables != 0x03) return -1;
	expect(&tables, 0x03);
	// End of the callsites table.
	unsigned char *tableStart = tables;
	unsigned char *actions = tables + read_uleb128(&tables);
	while(tables <= actions)
	{
		size_t callsite = read_long(&tables);
		size_t callsiteSize= read_long(&tables);
		size_t action = read_long(&tables);
		if (invokesite >= callsite && invokesite <= (callsite + callsiteSize))
		{
			return action;
		}
		read_uleb128(&tables);
	}
	return 0;
}

_Unwind_Reason_Code __LanguageKitEHPersonalityRoutine(
			int version,
			_Unwind_Action actions,
			uint64 exceptionClass,
			struct _Unwind_Exception *exceptionObject,
			struct _Unwind_Context *context)
{
	if ((actions & _UA_SEARCH_PHASE))
	{
		// Try to find a landing pad in this function
		if (landingPadForInvoke(context) != 0)
		{
			return _URC_HANDLER_FOUND;
		}
		return _URC_CONTINUE_UNWIND;
	}
	// Always install the handler - we check if it's a valid LanguageKit
	// exception elsewhere.
	if (actions & _UA_HANDLER_FRAME) 
	{
		size_t offset = landingPadForInvoke(context);
		_Unwind_SetIP(context, _Unwind_GetRegionStart(context) + offset);
		return _URC_INSTALL_CONTEXT;
	}
	return _URC_CONTINUE_UNWIND;
}

/**
 * Create an exception object that will be unwound to the frame containing
 * context, return retval.
 */
void __LanguageKitThrowNonLocalReturn(void *context, void *retval)
{
	LanguageKitException.exception.exception_class = *(uint64*)"ETOILEST";
	LanguageKitException.context = context;
	// TODO: We could probably have the return value space allocated in the
	// top-level method frame, and then it could be written there directly from
	// the block.

	// If it's an object, retain it.
	if ((((uintptr_t)retval) & 1) == 0)
	{
		retval = [(id)retval retain];
	}
	LanguageKitException.retval = retval;
	_Unwind_Reason_Code fail = 
		_Unwind_RaiseException(&LanguageKitException.exception);
}

/**
 * Rest whether a given exception object is a valid non-local return.  Returns
 * if it is, otherwise instructs the unwinding runtime to continue propagating
 * the exception up the stack.  Copies the return value from the LanguageKit
 * exception to the address pointed to by retval.
 */
char __LanguageKitTestNonLocalReturn(void *context,
                                   struct _Unwind_Exception *exception,
								   void **retval)
{
	// Test if this is a smalltalk exception at all
	if (exception == &LanguageKitException.exception)
	{
		// Test if this frame is the correct one.
		if (LanguageKitException.context == context)
		{
			*retval = LanguageKitException.retval;
			return 1;
		}
		// Rethrow it if it isn't.
		_Unwind_RaiseException(&LanguageKitException.exception);
	}
	// This does not return, it jumps back to the unwind library, which jumps
	// to the LanguageKit personality function, and proceeds to unwind the stack.
	_Unwind_Resume(exception);
	return 0;
}
