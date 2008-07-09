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
 * Set the method signature for this method.
 */
- (void) setSignature:(NSString*)aSignature;
@end
