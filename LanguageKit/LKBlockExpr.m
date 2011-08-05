#import "LKBlockExpr.h"
#import "LKDeclRef.h"
#import "Runtime/LKObject.h"

@implementation LKBlockExpr 
+ (id) blockWithArguments:(NSMutableArray*)arguments locals:(NSMutableArray*)locals statements:(NSMutableArray*)statementList
{
	return [[[self alloc] initWithArguments: arguments
	                                 locals: locals
	                             statements: statementList] autorelease];
}
- (id) initWithArguments: (NSMutableArray*)arguments
                  locals: (NSMutableArray*)locals
              statements: (NSMutableArray*)statementList
{
	LKSymbolTable *table = [LKSymbolTable new];
	[table setDeclarationScope: self];
	[table addSymbolsNamed: locals ofKind: LKSymbolScopeLocal];
	[table addSymbolsNamed: arguments ofKind: LKSymbolScopeArgument];
	self = [super initWithSymbolTable: table];
	[table release];
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
	[str appendString:@"[ "];
	for (LKSymbol *s in [symbols arguments])
	{
		[str appendFormat:@":%@ ", s];
	}
	[str appendString:@"| "];
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
	NSArray *args = [symbols arguments];
	NSUInteger argCount = [args count];
	// FIXME: We should be able to generate other block signatures
	NSMutableString *sig = 
		[NSMutableString stringWithFormat: @"%s%d@?", @encode(LKObject), sizeof(id) * (argCount+1)];
	for (NSUInteger i=0 ; i<argCount ; i++)
	{
		[sig appendFormat: @"%d%s", sizeof(id)*i, @encode(LKObject)];
	}
	[aGenerator beginBlockWithArgs: args
	                        locals: [symbols locals]
	                     externals: [symbols byRefVariables]
	                     signature: sig];
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
		[aGenerator blockReturn: lastValue];
	}
	return [aGenerator endBlock];
}
- (void) inheritSymbolTable:(LKSymbolTable*)aSymbolTable
{
	[symbols setEnclosingScope: aSymbolTable];
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
