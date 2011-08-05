#import "LKMethod.h"
#import "LKModule.h"

@implementation LKMethod
@synthesize signature, statements;
+ (id) methodWithSignature:(LKMessageSend*)aSignature
                    locals:(NSMutableArray*)locals
                statements:(NSMutableArray*)statementList
{
	return [[[self alloc] initWithSignature: aSignature
	                                 locals: locals
	                             statements: statementList] autorelease];
}
- (id) initWithSignature:(LKMessageSend*)aSignature
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
	BOOL success = YES;
	FOREACH(statements, statement, LKAST*)
	{
		[statement setParent:self];
		success &= [statement check];
	}
	return success;
}
- (NSString*) methodBody
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
	[str appendString: [self methodBody]];
	[str appendString:@"]"];
	return str;
}
- (void) beginMethodWithGenerator: (id)aGenerator
                 forSelectorNamed: (NSString*)sel
                     typeEncoding: (NSString*)typeEncoding
{}
- (void*) compileWithGenerator: (id<LKCodeGenerator>)aGenerator
{
	NSArray *types = [[self module] typesForMethod: [signature selector]];
	// We emit one copy of the method for every possible type encoding.
	// FIXME: We should only do this if we are not inheriting the method from a
	// superclass.
	for (NSString *type in types)
	{
		// FIXME: This is really ugly.  It also means that the type is not
		// intrinsic to the AST, it's not defined until run time, which is a
		// bit horrible.
		NSMethodSignature *sig = [NSMethodSignature signatureWithObjCTypes: [type UTF8String]];
		// Skip implicit args
		NSInteger i = 2;
		for (NSString *arg in [signature arguments])
		{
			[[symbols symbolForName: arg] setTypeEncoding: [NSString stringWithUTF8String: [sig getArgumentTypeAtIndex: i++]]];
		}
		[self beginMethodWithGenerator: aGenerator
		              forSelectorNamed: [signature selector]
		                  typeEncoding: type];
		for (LKAST *statement in statements)
		{
			[statement compileWithGenerator: aGenerator];
		}
		[aGenerator endMethod];
	}
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
@implementation LKInstanceMethod
- (void) beginMethodWithGenerator: (id)aGenerator
                 forSelectorNamed: (NSString*)sel
                     typeEncoding: (NSString*)typeEncoding
{
	NSArray *localNames = [symbols locals];
	[aGenerator beginInstanceMethod: sel
	               withTypeEncoding: typeEncoding
	                      arguments: [symbols arguments]
	                         locals: [symbols locals]];
}
@end
@implementation LKClassMethod
- (void) beginMethodWithGenerator: (id)aGenerator
                 forSelectorNamed: (NSString*)sel
                     typeEncoding: (NSString*)typeEncoding
{
	[aGenerator beginClassMethod: sel
	            withTypeEncoding: typeEncoding
	                   arguments: [symbols arguments]
	                      locals: [symbols locals]];
}
@end
