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
	LKBlockSymbolTable *st = [[LKBlockSymbolTable alloc] initWithLocals:locals args:arguments];
	self = [self initWithSymbolTable: st];
	RELEASE(st);
	if (self != nil)
	{
		ASSIGN(statements, statementList);
	}
	return self;
}
- (void) setStatements: (NSMutableArray*)anArray
{
	ASSIGN(statements, anArray);
}
- (BOOL)check
{
	BOOL success = YES;
	FOREACH(statements, s, LKAST*)
	{
		[s setParent:self];
		success &= [s check];
	}
	return success;
}
- (NSString*) description
{
	NSMutableString *str = [NSMutableString string];
	LKMethodSymbolTable *st = (LKMethodSymbolTable*)symbols;
	[str appendString:@"[ "];
	if ([[st args] count] > 0)
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
- (void*) compileWithGenerator: (id<LKCodeGenerator>)aGenerator
{
	[aGenerator beginBlockWithArgs:[[(LKMethodSymbolTable*)symbols args] count]
	                        locals:[[(LKMethodSymbolTable*)symbols locals] count]];
	void * lastValue = NULL;
	BOOL addBranch = YES;
	FOREACH(statements, statement, LKAST*)
	{
		if (![statement isComment])
		{
			lastValue = [statement compileWithGenerator: aGenerator];
			if ([statement isBranch])
			{
				addBranch = NO;
				break;
			}
		}
	}
	if (addBranch)
	{
		[aGenerator blockReturn:lastValue];
	}
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
- (void)dealloc
{
	[statements release];
	[super dealloc];
}
@end
