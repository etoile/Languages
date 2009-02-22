#import "LKAST.h"

@interface LKCompare : LKAST {
	LKAST *lhs;
	LKAST *rhs;
}
+ (LKCompare*) comparisonWithLeftExpression: (LKAST*)expr1
					        rightExpression: (LKAST*)expr2;
@end
