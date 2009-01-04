// These are different for x86-64, but I don't know what they should be.
#ifdef __i386__
#	ifdef __FreeBSD__
static const unsigned MAX_INTS_IN_REGISTERS = 2;
static const unsigned MAX_FLOATS_IN_REGISTERS = 2;
#	elif defined(__LINUX__)
static const unsigned MAX_INTS_IN_REGISTERS = 0;
static const unsigned MAX_FLOATS_IN_REGISTERS = 0;
#	endif
#else
// Some placeholder values
static const unsigned MAX_INTS_IN_REGISTERS = 0;
static const unsigned MAX_FLOATS_IN_REGISTERS = 0;
#endif

