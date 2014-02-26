#import <LanguageKit/LKAST.h>

/**
 * AST node representing a method return.
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
/**
 * Returns the return statement's expression AST node.
 */
- (LKAST*) expression;
@end

/**
 * AST node representing a block return.
 */
@interface LKBlockReturn : LKReturn {}
@end
