#import "LKAST.h"

@interface LKCompare : LKAST {
	LKAST *lhs;
	LKAST *rhs;
}
+ (LKCompare*) compare:(LKAST*)expr1 to:(LKAST*)expr2;
- (id) initComparing:(LKAST*)expr1 to:(LKAST*)expr2;
@end
