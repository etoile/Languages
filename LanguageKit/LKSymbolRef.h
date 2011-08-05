#import "LKDeclRef.h"

/**
 * AST node representing a reference to a symbol (selector).
 */
@interface LKSymbolRef : LKAST
{
	/**
	 * The symbol.
	 */
	NSString *symbol;
}
/** Returns autoreleased reference for the specified symbol. */
+ (id) referenceWithSymbol:(NSString*)sym;
@end
