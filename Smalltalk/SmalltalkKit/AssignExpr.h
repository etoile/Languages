#import "AST.h"

@class DeclRef;

/**
 * AST node representing an assignment.
 */
@interface AssignExpr :AST {
/** The target of the assignment. */
  DeclRef *target;
/** The expression representing the assigned value. */
  AST *expr;
}
/**
 * Return new assignment with target and expression.
 */
+ (id) assignWithTarget:(DeclRef*)aTarget expr:(AST*)expression;
/**
 * Initialise new assignment with target and expression.
 */
- (id) initWithTarget:(DeclRef*)aTarget expr:(AST*)expression;
@end
