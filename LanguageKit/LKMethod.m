#import "LKMethod.h"
#import "LKReturn.h"
#import "LKModule.h"
#import "LKCompilerErrors.h"
static NSSet *ARCBannedMessages;


@implementation LKMethod
@synthesize signature, statements;
+ (void)initialize
{
	ARCBannedMessages = [[NSSet alloc] initWithObjects: @"retain", @"release", @"autorelease", @"retainCount", nil];
}
+ (id) methodWithSignature:(LKMessageSend*)aSignature
                    locals:(NSMutableArray*)locals
                statements:(NSMutableArray*)statementList
{
	return [[self alloc] initWithSignature: aSignature
	                                 locals: locals
	                             statements: statementList];
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
	if (self == nil) { return nil; }
	ASSIGN(signature, aSignature);
	ASSIGN(statements, statementList);
	return self;
}
- (BOOL)check
{
	NSString *selector = [signature selector];
	if ([ARCBannedMessages containsObject: selector])
	{
		NSDictionary *errorDetails = D(
			[NSString stringWithFormat: @"%@ may not be implemented in LanguageKit",
				selector], kLKHumanReadableDescription,
			self, kLKASTNode);
		if ([LKCompiler reportError: LKInvalidSelectorError
		                    details: errorDetails])
		{
			return [self check];
		}
		return NO;
	}
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
			LKSymbol *argSymbol = [symbols symbolForName: arg];
			[argSymbol setIndex: i];
			[argSymbol setTypeEncoding: [NSString stringWithUTF8String: [sig getArgumentTypeAtIndex: i++]]];
		}
		NSString *methodName = [signature selector];
		[self beginMethodWithGenerator: aGenerator
		              forSelectorNamed: methodName
		                  typeEncoding: type];
		for (LKAST *statement in statements)
		{
			[statement compileWithGenerator: aGenerator];
		}
		// FIXME: Don't do this if we're a root class (we don't actually
		// support root classes in Smalltalk at the moment).
		if ([@"dealloc" isEqualToString: methodName])
		{
			[aGenerator sendSuperMessage: methodName
			                       types: type
			                    withArgs: NULL
			                       count: 0];
		}
		if (![[statements lastObject] isKindOfClass: [LKReturn class]])
		{
			[aGenerator setReturn: [aGenerator loadSelf]];
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
