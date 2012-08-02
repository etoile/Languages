#import "LKAST.h"

@interface LKArrayExpr : LKAST {
  NSMutableArray *elements;
}
@property (strong, nonatomic) NSMutableArray *elements;
+ (id) arrayWithElements:(NSArray*)anArray;
- (id) initWithElements:(NSArray*)anArray;
@end
