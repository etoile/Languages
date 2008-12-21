#import "LKAST.h"

@interface LKIfStatement : LKAST {
	LKAST *condition;
	NSArray *thenStatements;
	NSArray *elseStatements;
}
+ (LKIfStatement*) ifStatementWithCondition:(LKAST*) aCondition
                                       then:(NSArray*)thenClause
                                       else:(NSArray*)elseClause;
@end
