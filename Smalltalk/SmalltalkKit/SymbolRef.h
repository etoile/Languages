#import "AST.h"

/**
 * AST node representing a reference to a symbol.
 */
@interface SymbolRef : AST {
@public
  /** The name of the variable being referenced. */
	NSString *symbol;
}
@end
