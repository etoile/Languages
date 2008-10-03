#import "AST.h"
#import "MessageSend.h"

/**
 * AST node representing a method.
 */
@interface SmalltalkMethod : AST {
  /** Method signature - selector and names of arguments. */
  MessageSend *signature;
@public
  /** List of statements in this method. */
  NSMutableArray * statements;
}
/**
 * Return a new Method with the specified signature, locals and statements.
 */
+ (id) methodWithSignature:(MessageSend*)aSignature locals:(NSMutableArray*)locals statements:(NSMutableArray*)statementList;
/**
 * Initialise a new Method with the specified signature, locals and statements.
 */
- (id) initWithSignature:(MessageSend*)aSignature locals:(NSMutableArray*)locals statements:(NSMutableArray*)statementList;
/**
 * Set the method signature for this method.
 */
- (void) setSignature:(MessageSend*)aSignature;
@end
