#import "AST.h"

@class LKDeclRef;

/**
 * AST node representing an assignment.
 */
@interface LKAssignExpr :LKAST {
/** The target of the assignment. */
  LKDeclRef *target;
/** The expression representing the assigned value. */
  LKAST *expr;
}
/**
 * Return new assignment with target and expression.
 */
+ (id) assignWithTarget:(LKDeclRef*)aTarget expr:(LKAST*)expression;
/**
 * Initialise new assignment with target and expression.
 */
- (id) initWithTarget:(LKDeclRef*)aTarget expr:(LKAST*)expression;
@end
