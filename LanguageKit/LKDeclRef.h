#import "LKAST.h"

/**
 * AST node representing a reference to a variable.
 */
@interface LKDeclRef : LKAST {
@public
  /** The name of the variable being referenced. */
	NSString *symbol;
}
/** Returns autoreleased reference for the specified symbol. */
+ (id) referenceWithSymbol:(NSString*)sym;
/**
 * Returns the name of the variable being referenced.
 */
- (NSString*) symbol;
@end

/**
 * Class used to store the location of a bound variable in a block expression.
 */
@interface LKClosedDeclRef : NSObject {
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
