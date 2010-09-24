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
void _Unwind_SetIP(struct _Unwind_Context *context, uintptr_t);
uintptr_t _Unwind_GetLanguageSpecificData(struct _Unwind_Context *context);
uintptr_t _Unwind_GetRegionStart(struct _Unwind_Context *context);

typedef int _Unwind_Action;
static const _Unwind_Action _UA_SEARCH_PHASE = 1;
static const _Unwind_Action _UA_CLEANUP_PHASE = 2;
static const _Unwind_Action _UA_HANDLER_FRAME = 4;
static const _Unwind_Action _UA_FORCE_UNWIND = 8;

