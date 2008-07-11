#import "AST.h"

/**
 * AST node representing a message send operation.
 */
@interface MessageSend : AST {
  /** Receiver of the message. */
	id target;
  /** The message selector. */
	NSString * selector;
  /** Array of AST nodes which evaluate to the message arguments. */
	NSMutableArray * arguments;
}
/**
 * Set the receiver of the expression.
 */
- (void) setTarget:(id)anObject;
/**
 * Add a component of the selector.
 */
- (void) addSelectorComponent:(NSString*)aSelector;
/**
 * Add an argument.
 */
- (void) addArgument:(id)anObject;
/**
 * Return all of the arguments of this message.
 */
- (NSMutableArray*) arguments;
/**
 * Return the selector.
 */
- (NSString*) selector;
@end
