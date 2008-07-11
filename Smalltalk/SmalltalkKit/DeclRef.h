#import "AST.h"

/**
 * AST node representing a reference to a variable.
 */
@interface DeclRef : AST {
@public
  /** The name of the variable being referenced. */
	NSString *symbol;
}
/** Construct a DeclRef for the specified symbol. */
+ (DeclRef*) refForDecl:(NSString*)sym;
@end

/**
 * Class used to store the location of a bound variable in a block expression.
 */
@interface ClosedDeclRef : NSObject {
@public
  /** The variable name */
	NSString * symbol;
  /** The index in the block object's bound variable array. */
	int index;
  /** The offset from the start of the pointed object (used for instance
   * variables) */
	int offset;
}
@end
