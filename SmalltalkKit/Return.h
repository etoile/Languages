#import "AST.h"

/**
 * AST node representing a return statement.
 */
@interface Return : AST {
  /** Value to be returned. */
  AST *ret;
}
/**
 * Autoreleased instance with an expression containing the value to return.
 */
+ (id) returnWithExpr:(AST*)anExpression;
/**
 * Initialise with an expression containing the value to return.
 */
- (id) initWithExpr:(AST*)anExpression;
@end
