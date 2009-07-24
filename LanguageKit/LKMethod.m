#import "LKMethod.h"
#import "LKModule.h"

@implementation LKMethod
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
	LKMethodSymbolTable *st = 
		[[LKMethodSymbolTable alloc] initWithLocals: locals
		                                       args: [aSignature arguments]];
	self = [self initWithSymbolTable: st];
	if (self == nil) { return nil; }
	RELEASE(st);
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
- (void) setSignature:(LKMessageSend*)aSignature
{
	ASSIGN(signature, aSignature);
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
	LKMethodSymbolTable *st = (LKMethodSymbolTable*)symbols;
	if ([[st locals] count])
	{
		[str appendString:@"| "];
		FOREACH([st locals], symbol, NSString*)
		{
			[str appendFormat:@"%@ ", symbol];
		}
		[str appendString:@"|\n"];
	}
	FOREACH(statements, statement, LKAST*)
	{
		[str appendFormat:@"%@.\n", statement];
	}
	return str;
}
- (NSString*) description
{
	NSMutableString *str = [NSMutableString string];
	//LKMethodSymbolTable *st = (LKMethodSymbolTable*)symbols;
	[str appendString:[signature description]];
	[str appendString:@"[\n"];
	[str appendString: [self methodBody]];
	[str appendString:@"]"];
	return str;
}
- (void) beginMethodWithGenerator:(id)aGenerator
                 forSelectorNamed:(const char*)sel
			            withTypes:(const char*)types
{}
- (void*) compileWithGenerator: (id<LKCodeGenerator>)aGenerator
{
	const char *sel = [[signature selector] UTF8String];
	// FIXME: Should get method signature from superclass
	const char *types = [[self module] typeForMethod:[signature selector]];
	// If the types don't come from somewhere else, then assume all arguments
	// are id and the return type is id
	if (NULL == types) 
	{
		int argCount = [[signature arguments] count];
		int offset = sizeof(id) + sizeof(SEL);
		NSMutableString *ty = [NSMutableString stringWithFormat:@"@%d@0:%d",
			sizeof(SEL) + sizeof(id) * (argCount + 2), offset];
		for (int i=0 ; i<argCount ; i++)
		{
			offset += sizeof(id);
			[ty appendFormat:@"@%d", offset];
		}
		types = [ty UTF8String];
	}
	[self beginMethodWithGenerator:aGenerator forSelectorNamed:sel withTypes:types];
	FOREACH(statements, statement, LKAST*)
	{
		[statement compileWithGenerator: aGenerator];
	}
	[aGenerator endMethod];
	return NULL;
}
- (void) inheritSymbolTable:(LKSymbolTable*)aSymbolTable
{
	[symbols setScope:aSymbolTable];
}
- (void) visitWithVisitor:(id<LKASTVisitor>)aVisitor
{
	[self visitArray: statements withVisitor: aVisitor];
}
- (LKMessageSend*) signature
{
	return signature;
}
- (NSMutableArray*) statements
{
	return statements;
}
@end
@implementation LKInstanceMethod
- (void) beginMethodWithGenerator:(id)aGenerator
                 forSelectorNamed:(const char*)sel
			            withTypes:(const char*)types
{
	NSArray *localNames = [(LKMethodSymbolTable*)symbols locals];
	unsigned count = [localNames count];
	const char *locals[count];
	for (unsigned i=0 ; i<count ; i++)
	{
		locals[i] = [[localNames objectAtIndex: i] UTF8String];
	}
	[aGenerator beginInstanceMethod: sel
	                      withTypes: types
	                         locals: locals
	                          count: count];
}
@end
@implementation LKClassMethod
- (void) beginMethodWithGenerator:(id)aGenerator
                 forSelectorNamed:(const char*)sel
			            withTypes:(const char*)types
{
	NSArray *localNames = [(LKMethodSymbolTable*)symbols locals];
	unsigned count = [localNames count];
	const char *locals[count];
	for (unsigned i=0 ; i<count ; i++)
	{
		locals[i] = [[localNames objectAtIndex: i] UTF8String];
	}
	[aGenerator beginClassMethod: sel
	                   withTypes: types
	                      locals: locals
	                       count: count];
}
@end
