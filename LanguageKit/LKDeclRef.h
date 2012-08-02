#import "LKAST.h"

/**
 * AST node representing a reference to a variable.
 */
@interface LKDeclRef : LKAST 
/** The name of the variable being referenced.  This is initially set to a
 * string and later resolved to a symbol. */
@property (strong, nonatomic) id symbol;
/** Returns autoreleased reference for the specified symbol. */
+ (id) referenceWithSymbol:(NSString*)sym;
@end

/**
 * Abstract class for all built-in symbols.  These are subclasses of LKDeclRef,
 * so it's possible to set their name.  For example, a Java-like language may
 * choose to call the self builtin 'this', or a Go-like frontend may give it a
 * different name for every method.
 */
@interface LKBuiltinSymbol : LKDeclRef
/**
 * Returns a new autoreleased instance of he receiver.
 */
+ (LKBuiltinSymbol*)builtin;
@end

/**
 * A nil (object) value.
 */
@interface LKNilRef : LKBuiltinSymbol @end
/**
 * Reference to the receiver, in methods and blocks inside methods.
 */
@interface LKSelfRef : LKBuiltinSymbol @end
/**
 * Reference to the superclass of he receiver.
 */
@interface LKSuperRef : LKBuiltinSymbol @end
/**
 * Reference to the block object, from within its scope.
 */
@interface LKBlockSelfRef : LKBuiltinSymbol @end
