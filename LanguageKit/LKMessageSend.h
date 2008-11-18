#import "LKAST.h"

/**
 * AST node representing a message send operation.
 */
@interface LKMessageSend : LKAST {
  /** Receiver of the message. */
	id target;
  /** The message selector. */
	NSString * selector;
  /** Array of AST nodes which evaluate to the message arguments. */
	NSMutableArray * arguments;
}
/**
 * Set the receiver of the message.
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
/**
 * Send an array of messages to the same receiver.  The receiver expression
 * will be evaluated once and each message will be sent to this receiver.
 */
@interface LKMessageCascade : LKAST {
	LKAST *receiver;
	NSMutableArray *messages;
}
+ (LKMessageCascade*) messageCascadeWithTarget:(LKAST*) aTarget
                                      messages:(NSMutableArray*) messageArray;
- (void) addMessage:(LKMessageSend*)aMessage;
@end
