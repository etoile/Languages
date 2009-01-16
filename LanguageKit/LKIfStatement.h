#import "LKAST.h"

@interface LKIfStatement : LKAST {
	LKAST *condition;
	NSMutableArray *thenStatements;
	NSMutableArray *elseStatements;
}
+ (LKIfStatement*) ifStatementWithCondition:(LKAST*) aCondition
                                       then:(NSArray*)thenClause
                                       else:(NSArray*)elseClause;
@end
