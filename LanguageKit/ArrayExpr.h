#import "AST.h"

@interface LKArrayExpr : LKAST {
  NSArray *elements;
}
+ (id) arrayWithElements:(NSArray*)anArray;
- (id) initWithElements:(NSArray*)anArray;
@end
