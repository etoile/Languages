#import <EtoileFoundation/EtoileFoundation.h>
#import "AST.h"
#import "SymbolTable.h"

/**
 * Smalltalk parser class.  This class implements a tokeniser and calls a
 * parser created using the Lemon parser generator.
 */
@interface Parser : NSObject {
@public
  AST *delegate;
}
/**
 * Parse the specified Smalltalk string and return an abstract syntax tree for
 * the program.
 */
- (AST*) parseString:(NSString*)s;
@end
