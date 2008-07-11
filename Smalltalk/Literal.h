#import "AST.h"

@interface Literal : AST {
  /** String representation of value.  Used because this can be a literal too
   * big to fit in a SmallInt. */
  NSString * value;
}
+ (id) literalFromString:(NSString*)aString;
@end

@interface NumberLiteral : Literal {}
@end

@interface StringLiteral : Literal {}
@end
