#import "AST.h"

@class DeclRef;

/**
 * AST node representing an assignment.
 */
@interface AssignExpr :AST {
@public
/** The target of the assignment. */
  DeclRef *target;
/** The expression representing the assigned value. */
  AST *expr;
}
@end
