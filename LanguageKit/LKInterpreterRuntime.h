#import <Foundation/Foundation.h>
/**
 * These functions allow the interpreter to interact with the Objective-C
 * runtime.
 */

/**
 * Send a message to an object. receiverClass is the class to start looking
 * for the method implementation in; normally receiver's class, but if it is
 * a super message send it should be the appropriate superclass of receiver.
 *
 * Arguments are unboxed to the correct type, and the returned value
 * is boxed to an object.
 *
 * Throws an exception if any of the arguments can't be unboxed to the
 * required type for the message send.
 */
id LKSendMessage(NSString *className, id receiver, NSString *selName,
                 unsigned int argc, id *args);

/**
 * Gets the value of an instance variable, boxing it as necessary.
 */
id LKGetIvar(id receiver, NSString *name);

/**
 * Sets the value of an instance variable, unboxing it as necessary.
 * 
 * For object typed ivars, the existing value is released and the new value
 * is retained.
 */
BOOL LKSetIvar(id receiver, NSString *name, id value);

/**
 * Creates and returns a "trampoline" Objective-C method implementation for a 
 * method with the given type string.
 * 
 * When invoked, this method implementation will call ASTForMethod() which will
 * look up the method's AST node using the slector and receiver's class, and
 * finally interpret the AST node using
 * -[LKMethod executeWithReciever:arguments:count:].
 *
 * Note that the method IMPs returned by LKInterpreterMakeIMP can never be
 * freed (because they might be cached).
 */
IMP LKInterpreterMakeIMP(Class cls, const char *objctype);
