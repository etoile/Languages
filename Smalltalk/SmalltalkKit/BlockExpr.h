#import "AST.h"

/**
 * AST node representing a block closure expression.
 */
@interface BlockExpr : AST {
/** Counter used when binding variables. */
  int nextClosed;
/** Lost of statements in this node. */
	NSMutableArray * statements;
}
/**
 * Set the statements in this node.
 */
- (void) setStatements: (NSMutableArray*)statements;
@end
