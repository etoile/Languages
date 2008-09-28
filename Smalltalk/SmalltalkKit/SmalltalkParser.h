#import <EtoileFoundation/EtoileFoundation.h>
#import "AST.h"

/**
 * Smalltalk parser class.  This class implements a tokeniser and calls a
 * parser created using the Lemon parser generator.
 */
@interface SmalltalkParser : NSObject {
  AST *delegate;
}
/**
 * Parse the specified Smalltalk string and return an abstract syntax tree for
 * the program.
 */
- (AST*) parseString:(NSString*)s;
/**
 * Used by the Lemon implementation to feed the generated AST back so the
 * Parser can return it.
 */
- (void) setDelegate:(AST*)ast;
@end
