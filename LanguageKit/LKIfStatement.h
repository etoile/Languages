#import "LKAST.h"

@interface LKIfStatement : LKAST {
	LKAST *condition;
	NSMutableArray *thenStatements;
	NSMutableArray *elseStatements;
}
+ (LKIfStatement*) ifStatementWithCondition:(LKAST*) aCondition
                                       then:(NSArray*)thenClause
                                       else:(NSArray*)elseClause;
+ (LKIfStatement*) ifStatementWithCondition:(LKAST*) aCondition;
- (void)setElseStatements:(NSArray*)elseClause;
- (void)setThenStatements:(NSArray*)thenClause;
@end
