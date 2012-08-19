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
EMIT_STRING(kLKHumanReadableDescription)
/**
 * Key used to indicate the AST node responsible for the warning.
 */
EMIT_STRING(kLKASTNode)
/**
 * The name of the missing superclass.  Used for LKUndefinedSuperclass.
 */
EMIT_STRING(kLKMissingSuperclassName)
/**
 * Line number of a parser error.
 */
EMIT_STRING(kLKLineNumber)
/**
 * Text of the line containing an error.
 */
EMIT_STRING(kLKSourceLine)
/**
 * The character which generated the error.
 */
EMIT_STRING(kLKCharacterNumber)
/**
 * The type encoding associated with an error or warning.
 */
EMIT_STRING(kLKTypeEncoding)
/**
 * The name of a header that can't be loaded.
 */
EMIT_STRING(kLKHeaderName)

/**
 * A reference was made to an invalid symbol.
 */
EMIT_STRING(LKUndefinedSymbolError)
/**
 * The selector may not be used in LanguageKit code.
 */
EMIT_STRING(LKInvalidSelectorError)
/**
 * The compiler can't determine the type encoding for a referenced symbol.
 */
EMIT_STRING(LKUnknownTypeError)
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
/**
 * A selector is being used that is polymorphic.  The kLKTypeEncoding key will
 * provide the type encoding that was chosen.
 */
EMIT_STRING(LKPolymorphicSelectorWarning)
/**
 * A C header was referenced, but can't be loaded.  kLKHeaderName will provide
 * the name of the header.
 */
EMIT_STRING(LKMissingHeaderWarning)
/**
 * Parsing failed.  The info dictionary will contain kLKCharacterNumber,
 * kLKSourceLine, and kLKLineNumber to identify the location of the error.
 */
EMIT_STRING(LKParserError)

#undef EMIT_STRING
