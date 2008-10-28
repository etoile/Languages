#import "LKAST.h"

/**
 * AST node representing a return statement.
 */
@interface LKReturn : LKAST {
  /** Value to be returned. */
  LKAST *ret;
}
/**
 * Autoreleased instance with an expression containing the value to return.
 */
+ (id) returnWithExpr:(LKAST*)anExpression;
/**
 * Initialise with an expression containing the value to return.
 */
- (id) initWithExpr:(LKAST*)anExpression;
@end
