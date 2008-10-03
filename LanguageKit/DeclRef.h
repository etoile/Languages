#import "AST.h"

/**
 * AST node representing a reference to a variable.
 */
@interface DeclRef : AST {
@public
  /** The name of the variable being referenced. */
	NSString *symbol;
}
/** Construct a reference for the specified symbol. */
+ (id) reference:(NSString*)sym;
/** Initialise a reference for the specified symbol. */
- (id) initWithSymbol:(NSString*)sym;
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
