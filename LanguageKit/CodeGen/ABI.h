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

