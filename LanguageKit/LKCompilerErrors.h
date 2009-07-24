#import <Foundation/NSString.h>
/**
 * If this file is being included as normal, then declare extern versions of
 * the string.  If it is being included in the file used for generating the
 * real versions of the strings then create the unique string instances.
 */
#ifndef EMIT_STRING
#define EMIT_STRING(x) extern NSString *x;
#endif
/**
 * Key used for the human readable error or warning description.
 */
EMIT_STRING(kLKHumanReadableDesciption)
/**
 * Key used to indicate the AST node responsible for the warning.
 */
EMIT_STRING(kLKASTNode)
/**
 * The name of the missing superclass.  Used for LKUndefinedSuperclass.
 */
EMIT_STRING(kLKMissingSuperclassName)


/**
 * A reference was made to an invalid symbol.
 */
EMIT_STRING(LKUndefinedSymbolError)
/**
 * An attempt was made to subclass a nonexistent class.  This error is usually
 * fatal, but can be recovered by adding the superclass to the runtime.  The
 * name of the missing superclass will be stored in the dictionary with the
 * kLKMissingSuperclassName key.
 */
EMIT_STRING(LKUndefinedSuperclassError)
/**
 * An attempt was made to compile a class which already exists.  This is only a
 * problem when JIT compiling; emitting a copy of a currently-loaded class is
 * perfectly acceptable.
 */
EMIT_STRING(LKRedefinedClassWarning)

#undef EMIT_STRING
