#import "AST.h"
#import "MessageSend.h"

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
@end
