#import <EtoileFoundation/EtoileFoundation.h>

@class AST;
/**
 * EScript parser class.  This class implements a tokeniser and calls a
 * parser created using the Lemon parser generator.
 */
@interface EScriptParser : NSObject {
  AST *delegate;
}
/**
 * Parse the specified EScript string and return an abstract syntax tree for
 * the program.
 */
- (AST*) parseString:(NSString*)s;
/**
 * Used by the Lemon implementation to feed the generated AST back so the
 * Parser can return it.
 */
- (void) setDelegate:(AST*)ast;
@end
