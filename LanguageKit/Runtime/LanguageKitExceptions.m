#import <Foundation/NSObject.h>
#import <Foundation/NSString.h>
#import <Foundation/NSException.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "dwarf_eh.h"

#define D(chr, byte) ((((uint64_t)chr)<<(64 - (8*byte))))
static const uint64_t LKEXCEPTION_TYPE = D('E',1) + D('T',2) + D('O',3) +
D('I',4) + D('L',5) + D('E',6) + D('L',7) + D('K',8);
#undef D

char __LanguageKitNonLocalReturn;

/**
 * Thread-local LanguageKit exception object.  Could be allocated with
 * malloc/free, but not really worth it.
 */
typedef struct 
{
	/** The unwinding lib's exception object. */
	struct _Unwind_Exception exception;
	/** The context object for the method frame for the block. */
	void *context;
	/** The value to be returned.  */
	void *retval;
} LKException;

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
 * Returns YES if this is an LK catch handler, NO if it is a cleanup.
 */
static BOOL check_action_record(struct _Unwind_Context *context,
                                struct dwarf_eh_lsda *lsda,
                                dw_eh_ptr_t action_record,
                                unsigned long *selector)
{
	//if (!action_record) { return handler_cleanup; }
	while (action_record)
	{
		int filter = read_sleb128(&action_record);
		dw_eh_ptr_t action_record_offset_base = action_record;
		int displacement = read_sleb128(&action_record);
		*selector = filter;
		if (filter > 0)
		{
			*selector = 1;
			return YES;
		}
		else if (filter == 0)
		{
			// Cleanup?  I think the GNU ABI doesn't actually use this, but it
			// would be a good way of indicating a non-id catchall...
			return NO;
		}
		else
		{
			fprintf(stderr, "Filter value: %d\n"
					"Your compiler and I disagree on the correct layout of EH data.\n", 
					filter);
			abort();
		}
		*selector = 0;
		action_record = displacement ? 
			action_record_offset_base + displacement : 0;
	}
	return NO;
}

_Unwind_Reason_Code __LanguageKitEHPersonalityRoutine(
			int version,
			_Unwind_Action actions,
			uint64 exceptionClass,
			struct _Unwind_Exception *exceptionObject,
			struct _Unwind_Context *context)
{
	NSLog(@"Called Lk personality routine!");
	// This personality function is for version 1 of the ABI.  If you use it
	// with a future version of the ABI, it won't know what to do, so it
	// reports a fatal error and give up before it breaks anything.
	if (1 != version)
	{
		return _URC_FATAL_PHASE1_ERROR;
	}
	// Check if this is a foreign exception.  We only catch LK exceptions, but
	// we run cleanups anyway.
	BOOL foreignException = exceptionClass != LKEXCEPTION_TYPE;


	unsigned char *lsda_addr = (void*)_Unwind_GetLanguageSpecificData(context);

	// No LSDA implies no landing pads - try the next frame
	if (0 == lsda_addr) { return _URC_CONTINUE_UNWIND; }

	// These two variables define how the exception will be handled.
	struct dwarf_eh_action action = {0};
	unsigned long selector = 0;
	
	struct dwarf_eh_lsda lsda = parse_lsda(context, lsda_addr);
	action = dwarf_eh_find_callsite(context, &lsda);
	BOOL handler = check_action_record(context, &lsda, action.action_record,
			&selector);
	if (actions & _UA_SEARCH_PHASE)
	{
		// If there's no action record, we've only found a cleanup, so keep
		// searching for something real
		if (handler && !foreignException)
		{
			return _URC_HANDLER_FOUND;
		}
		return _URC_CONTINUE_UNWIND;
	}

	if (!(actions & _UA_HANDLER_FRAME))
	{
		// If there's no cleanup here, continue unwinding.
		if (0 == action.landing_pad)
		{
			return _URC_CONTINUE_UNWIND;
		}
		selector = 0;
	}
	else
	{
		selector = !foreignException;
	}

	_Unwind_SetIP(context, (unsigned long)action.landing_pad);
	_Unwind_SetGR(context, __builtin_eh_return_data_regno(0), exceptionObject);
	_Unwind_SetGR(context, __builtin_eh_return_data_regno(1), selector);

	return _URC_INSTALL_CONTEXT;
}

static void LKCleanupException(_Unwind_Reason_Code reason, void *exc)
{
	LKException *exception = exc;
	// Autorelease the object in the current pool to make sure it doesn't leak
	if ((((uintptr_t)exception->retval) & 1) == 0)
	{
		[(id)exception->retval autorelease];
	}
	free(exc);
}

typedef struct
{
	void* isa;
	int flags;
	int reserved;
	id(*invoke)(void*,...);
	void* descriptor;
	void* context;
} Block;
/**
 * Create an exception object that will be unwound to the frame containing
 * context, return retval.
 */
void __LanguageKitThrowNonLocalReturn(Block *context, void *retval)
{
	if (context->context != context)
	{
		[NSException raise: @"LKInvalidReturnException"
		            format: @"Can not return from a block after the enclosing scope returned"];
	}
	LKException *exception = calloc(1, sizeof(LKException));
	exception->exception.exception_class = LKEXCEPTION_TYPE;
	exception->context = context;
	exception->exception.exception_cleanup = LKCleanupException;
	// TODO: We could probably have the return value space allocated in the
	// top-level method frame, and then it could be written there directly from
	// the block.

	// If it's an object, retain it.
	if ((((uintptr_t)retval) & 1) == 0)
	{
		retval = [(id)retval retain];
	}
	exception->retval = retval;
	_Unwind_RaiseException(&exception->exception);
}

/**
 * Called from a non-local return handler.  Tests whether the non-local return
 * was meant to be caught by this frame.  If it was not, then it rethrows the
 * exception.  If the caller is the correct handler, then this function
 * destroys the exception object and returns..
 */
void __LanguageKitTestNonLocalReturn(void *context,
                                    struct _Unwind_Exception *exception,
                                    void **retval)
{
	NSLog(@"Testing exeption for %@", context);
	// This must only be called with a LanguageKit exception
	assert(NULL != exception && exception->exception_class == LKEXCEPTION_TYPE);

	LKException *LanguageKitException = (LKException*)exception;
	// Test if this frame is the correct one.
	if (LanguageKitException->context == context)
	{
		*retval = LanguageKitException->retval;
		if ((((uintptr_t)*retval) & 1) == 0)
		{
			*retval = [*(id*)retval autorelease];
		}
		free(exception);
		return;
	}
	_Unwind_Resume_or_Rethrow(exception);
	// Should not be reached:
	abort();
}

void __LanguageKitInvalidTypeEncoding(id self, SEL cmd)
{
	[NSException raise: @"LKTypeInfoException"
	            format: @"Unable to determine type info for [%@ %@]", self, NSStringFromSelector(cmd)];
}
