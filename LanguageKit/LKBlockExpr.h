#import <LanguageKit/LKAST.h>

/**
 * AST node representing a block closure expression.
 */
@interface LKBlockExpr : LKAST {
/** List of statements in this node. */
	NSMutableArray * statements;
}
/**
 * Return a new Block with the specified arguments, locals and statements.
 */
+ (id) blockWithArguments:(NSMutableArray*)arguments locals:(NSMutableArray*)locals statements:(NSMutableArray*)statementList;
/**
 * Initialise a new Block with the specified arguments, locals and statements.
 */
- (id) initWithArguments:(NSMutableArray*)arguments locals:(NSMutableArray*)locals statements:(NSMutableArray*)statementList;
/**
 * Set the statements in this node.
 */
- (void) setStatements: (NSMutableArray*)statements;
/**
 * Returns the list of statements in the block
 */
- (NSMutableArray*) statements;
@end
