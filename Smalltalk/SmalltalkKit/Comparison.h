#import "AST.h"

@interface Compare : AST {
	AST *lhs;
	AST *rhs;
}
+ (Compare*) compare:(AST*)expr1 to:(AST*)expr2;
@end
