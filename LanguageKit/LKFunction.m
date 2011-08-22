#import "LKFunction.h"
#import "LKModule.h"
#import "LKCompilerErrors.h"


@implementation LKFunction
@synthesize signature, statements;
+ (id) functionWithSignature:(LKFunctionCall*)aSignature
                    locals:(NSMutableArray*)locals
                statements:(NSMutableArray*)statementList
{
	return [[[self alloc] initWithSignature: aSignature
	                                 locals: locals
	                             statements: statementList] autorelease];
}
- (id) initWithSignature:(LKFunctionCall*)aSignature
                  locals:(NSMutableArray*)locals
              statements:(NSMutableArray*)statementList
{
	LKSymbolTable *st = [LKSymbolTable new];
	[st setDeclarationScope: self];
	[st addSymbolsNamed: locals ofKind: LKSymbolScopeLocal];
	[st addSymbolsNamed: [aSignature arguments] ofKind: LKSymbolScopeArgument];
	self = [self initWithSymbolTable: st];
	[st release];
	if (self == nil) { return nil; }
	ASSIGN(signature, aSignature);
	ASSIGN(statements, statementList);
	return self;
}
- (void)dealloc
{
	[signature release];
	[statements release];
	[super dealloc];
}
- (BOOL)check
{
	NSInteger i = 0;
	// Make sure that the arguments are all in the right order in the symbol table.
	for (NSString *arg in [signature arguments])
	{
		LKSymbol *argSymbol = [symbols symbolForName: arg];
		[argSymbol setIndex: i++];
	}

	BOOL success = YES;
	FOREACH(statements, statement, LKAST*)
	{
		[statement setParent:self];
		success &= [statement check];
	}
	return success;
}
- (NSString*) functionBody
{
	NSMutableString *str = [NSMutableString string];
	if ([[symbols locals] count])
	{
		[str appendString:@"| "];
		for (NSString *symbol in [symbols locals])
		{
			[str appendFormat:@"%@ ", symbol];
		}
		[str appendString:@"|\n"];
	}
	for (LKAST *statement in statements)
	{
		[str appendFormat:@"%@.\n", statement];
	}
	return str;
}
- (NSString*) description
{
	NSMutableString *str = [NSMutableString string];

	[str appendString:[signature description]];
	[str appendString:@"[\n"];
	[str appendString: [self functionBody]];
	[str appendString:@"]"];
	return str;
}
- (void*) compileWithGenerator: (id<LKCodeGenerator>)aGenerator
{
	[aGenerator beginFunction: [signature functionName]
	         withTypeEncoding: [signature typeEncoding];
	                arguments: [symbols arguments]
	                   locals: [symbols locals]];
	for (LKAST *statement in statements)
	{
		[statement compileWithGenerator: aGenerator];
	}
	[aGenerator endFunction];
	return NULL;
}
- (void) inheritSymbolTable:(LKSymbolTable*)aSymbolTable
{
	[symbols setEnclosingScope: aSymbolTable];
}
- (void) visitWithVisitor:(id<LKASTVisitor>)aVisitor
{
	[self visitArray: statements withVisitor: aVisitor];
}
@end

