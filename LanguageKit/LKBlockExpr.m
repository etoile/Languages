#import "LKBlockExpr.h"
#import "LKDeclRef.h"

@implementation LKBlockExpr 
+ (id) blockWithArguments:(NSMutableArray*)arguments locals:(NSMutableArray*)locals statements:(NSMutableArray*)statementList
{
	return [[[self alloc] initWithArguments: arguments
	                                 locals: locals
	                             statements: statementList] autorelease];
}
- (id) initWithArguments:(NSMutableArray*)arguments locals:(NSMutableArray*)locals statements:(NSMutableArray*)statementList
{
	SELFINIT;
	LKBlockSymbolTable *st = [[LKBlockSymbolTable alloc] initWithLocals:locals args:arguments];
	[self initWithSymbolTable: st];
	RELEASE(st);
	ASSIGN(statements, statementList);
	return self;
}
- (id) init
{
	SUPERINIT;
	nextClosed = 1;
	return self;
}
- (void) setStatements: (NSMutableArray*)anArray
{
	ASSIGN(statements, anArray);
}
- (void) check
{
	FOREACH(statements, s, LKAST*)
	{
		[s setParent:self];
		[s check];
	}
}
- (NSString*) description
{
	NSMutableString *str = [NSMutableString string];
	LKMethodSymbolTable *st = (LKMethodSymbolTable*)symbols;
	[str appendString:@"[ "];
	if ([[st args] count])
	{
		FOREACH([st args], symbol, NSString*)
		{
			[str appendFormat:@":%@ ", symbol];
		}
		[str appendString:@"| "];
	}
	[str appendString:@"\n"];
	FOREACH(statements, statement, LKAST*)
	{
		[str appendString:[statement description]];
		[str appendString:@".\n"];
	}
	[str appendString:@"]"];
	return str;
}
- (void*) compileWith:(id<LKCodeGenerator>)aGenerator
{
	[aGenerator beginBlockWithArgs:[[(LKMethodSymbolTable*)symbols args] count]
	                        locals:[[(LKMethodSymbolTable*)symbols locals] count]];
	void * lastValue = NULL;
	FOREACH(statements, statement, LKAST*)
	{
		if (![statement isComment])
		{
			lastValue = [statement compileWith:aGenerator];
		}
	}
	[aGenerator blockReturn:lastValue];
	return [aGenerator endBlock];
}
- (void) inheritSymbolTable:(LKSymbolTable*)aSymbolTable
{
	[symbols setScope:aSymbolTable];
}
- (void) visitWithVisitor:(id<LKASTVisitor>)aVisitor
{
	[self visitArray:statements withVisitor:aVisitor];
}
- (NSMutableArray*) statements
{
	return statements;
}
@end
