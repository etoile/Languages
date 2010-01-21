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
	/** Types for the method */
	const char *type;
}
/**
 * Return a new message send.
 */
+ (id) message;
/**
 * Return a new message send with the specified selector.
 */
+ (id) messageWithSelectorName:(NSString*)aSelector;
/**
 * Return a new message send with the specified selector and arguments.
 */
+ (id) messageWithSelectorName:(NSString*)aSelector
                     arguments: (NSArray*)args;
/**
 * Initialize with the specified selector.
 */
- (id) initWithSelectorName:(NSString*)aSelector;
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
- (NSArray*) arguments;
/**
 * Return the selector.
 */
- (NSString*) selector;
/**
 * Return the target
 */
- (id) target;
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
