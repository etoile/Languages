#import "LKAST.h"

/**
 * AST node representing a loop construct.
 *
 * The loop may have pre- and post-conditions that will terminate the loop when
 * evaluated to false, or one might rely on LKReturn or LKBreak to end the loop.
 *
 * It is possible to set a list of statements that will be executed after each
 * iteration, typically to increment a counter or similar. Initialization may
 * be done from a statement list in this node as well.
 *
 * Each LKLoop may be labelled to make it possible to break or continue outer
 * loops when there are nested flows of control.
 */
@interface LKLoop : LKAST {
	/** Label used for break or continue in nested loops. */
	NSString *label;
	/** List of statements in the loop body. */
	NSMutableArray *statements;
	/** List of statements to initialize the loop. */
	NSMutableArray *initStatements;
	/** List of statements executed after each iteration of the loop. */
	NSMutableArray *updateStatements;
	/** Expression to be tested for truth before each loop iteration. */
	LKAST *preCondition;
	/** Expression to be tested for truth after each loop iteration. */
	LKAST *postCondition;
}
/**
 * List of statements executed after each iteration of the loop.
 */
@property (retain, nonatomic) NSMutableArray *initStatements;
/**
 * Return a new loop with the specified statements.
 */
+ (id) loopWithStatements:(NSMutableArray*)statementList;
/**
 * Initialise a new loop with the specified statements.
 */
- (id) initWithStatements:(NSMutableArray*)statementList;
/**
 * Set a label used for break or continue in nested loops.
 */
- (void) setLabel:(NSString*)aLabel;
/**
 * Return the label used for break or continue in nested loops.
 */
- (NSString*) label;
/**
 * Set the statements for the loop body.
 */
- (void) setStatements:(NSMutableArray*)statements;
/**
 * Return the list of statements in the loop body.
 */
- (NSMutableArray*) statements;
/**
 * Set the statements executed after each iteration of the loop.
 */
- (void) setUpdateStatements:(NSMutableArray*)anArray;
/**
 * Return the list of statements executed after each iteration of the loop.
 */
- (NSMutableArray*) updateStatements;
/**
 * Set expression tested for truth before each loop iteration.
 */
- (void) setPreCondition:(LKAST*)condition;
/**
 * Return expression tested for truth before each loop iteration.
 */
- (LKAST*) preCondition;
/**
 * Set expression tested for truth after each loop iteration.
 */
- (void) setPostCondition:(LKAST*)condition;
/**
 * Return expression tested for truth after each loop iteration.
 */
- (LKAST*) postCondition;
@end

/**
 * Abstract superclass for LKBreak and LKContinue, implementing shared behavior.
 */
@interface LKLoopFlowControl : LKAST {
	NSString *label;
}
/**
 * Initialise a new break or continue statement with the given label.
 */
- (id) initWithLabel:(NSString*)aLabel;
/**
 * Return string "break" or "continue" depending on type of control statement.
 */
- (NSString*) flowControlFlavor;
@end

/**
 * AST node representing early loop termination.
 *
 * If a label is specified, the LKLoop with the same label is ended.
 * Otherwise, breaks out of the closest surrounding loop.
 */
@interface LKBreak : LKLoopFlowControl
/**
 * Return a new break statement with the given label.
 */
+ (id) breakWithLabel:(NSString*)aLabel;
@end

/**
 * AST node representing a skip to the next loop iteration.
 *
 * If a label is specified, skips to the end of the LKLoop with the same label.
 * Otherwise, does the same for the closest surrounding loop.
 */
@interface LKContinue : LKLoopFlowControl
/**
 * Return a new continue statement with the given label.
 */
+ (id) continueWithLabel:(NSString*)aLabel;
@end
