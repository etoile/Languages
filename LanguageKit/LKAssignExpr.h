#import <LanguageKit/LKAST.h>

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
/**
 * Returns the target of the assignment
 */
- (LKDeclRef*) target;
/**
 * Returns the expression representing the assigned value.
 */
- (LKAST*) expression;
@end
