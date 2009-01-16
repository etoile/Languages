#import "LKAST.h"
#import "LKMessageSend.h"

/**
 * AST node representing a method.
 */
@interface LKMethod : LKAST {
  /** Method signature - selector and names of arguments. */
  LKMessageSend *signature;
@public
  /** List of statements in this method. */
  NSMutableArray * statements;
}
/**
 * Return a new Method with the specified signature, locals and statements.
 */
+ (id) methodWithSignature:(LKMessageSend*)aSignature locals:(NSMutableArray*)locals statements:(NSMutableArray*)statementList;
/**
 * Initialise a new Method with the specified signature, locals and statements.
 */
- (id) initWithSignature:(LKMessageSend*)aSignature locals:(NSMutableArray*)locals statements:(NSMutableArray*)statementList;
/**
 * Set the method signature for this method.
 */
- (void) setSignature:(LKMessageSend*)aSignature;
/**
 * Returns the signature
 */
- (LKMessageSend*) signature;
/**
 * Returns the list of statements in the method
 */
- (NSMutableArray*) statements;
@end
@interface LKInstanceMethod : LKMethod {}
@end
@interface LKClassMethod : LKMethod {}
@end
/**
 * A freestanding method is a method that is not attached to any class.  It is
 * used in REPL mode, and might be useful for better support for prototypes.
 */
@interface LKFreestandingMethod : LKMethod {}
@end
