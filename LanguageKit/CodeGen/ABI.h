/**
 * ABI.h defines three constants for each target architecture which define the
 * calling convention for returning structures from functions.  These are:
 *
 * MAX_INTS_IN_REGISTERS - This defines the maximum number of integers in a
 * structure that can be returned in registers.
 *
 * MAX_FLOATS_IN_REGISTERS - This defines the maximum number of floating point
 * values in a structure that can be returned in registers.  
 *
 * Any structure with more elements than this will be returned on the stack.
 * The final value, SMALL_FLOAT_STRUCTS_ON_STACK, is a flag which indicates
 * whether small structures (e.g. NSPoint) should be returned in integer
 * registers.  This is the case on FreeBSD/i386, for example.
 */
// These are different for x86-64, but I don't know what they should be.
#ifdef __i386__
#	ifdef __FreeBSD__
static const unsigned MAX_INTS_IN_REGISTERS = 2;
static const unsigned MAX_FLOATS_IN_REGISTERS = 0;
static const bool SMALL_FLOAT_STRUCTS_ON_STACK = true;
#	elif defined(__linux__)
static const unsigned MAX_INTS_IN_REGISTERS = 0;
static const unsigned MAX_FLOATS_IN_REGISTERS = 0;
static const bool SMALL_FLOAT_STRUCTS_ON_STACK = false;
#	endif
#else
// Some placeholder values
static const unsigned MAX_INTS_IN_REGISTERS = 0;
static const unsigned MAX_FLOATS_IN_REGISTERS = 0;
static const bool SMALL_FLOAT_STRUCTS_ON_STACK = false;
#endif

