#import "LKAST.h"

/**
 * AST node representing a reference to a variable.
 */
@interface LKDeclRef : LKAST {
@public
	/** The name of the variable being referenced.  This is initially set to a
	 * string and later resolved to a symbol. */
	id symbol;
}
/** Returns autoreleased reference for the specified symbol. */
+ (id) referenceWithSymbol:(NSString*)sym;
@end
