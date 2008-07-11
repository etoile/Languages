#import "AST.h"

@interface ArrayExpr : AST {
  NSArray *elements;
}
+ (id) arrayWithElements:(NSArray*)anArray;
@end
