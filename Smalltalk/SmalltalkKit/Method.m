#import "Method.h"

@implementation SmalltalkMethod
+ (id) methodWithSignature:(MessageSend*)aSignature locals:(NSMutableArray*)locals statements:(NSMutableArray*)statementList
{
	return [[[self alloc] initWithSignature: aSignature
	                                 locals: locals
	                             statements: statementList] autorelease];
}
- (id) initWithSignature:(MessageSend*)aSignature locals:(NSMutableArray*)locals statements:(NSMutableArray*)statementList
{
	MethodSymbolTable *st = [[MethodSymbolTable alloc] initWithLocals:locals args:[aSignature arguments]];
	[self initWithSymbolTable: st];
	RELEASE(st);
	ASSIGN(signature, aSignature);
	ASSIGN(statements, statementList);
	return self;
}
- (void) setSignature:(MessageSend*)aSignature
{
	ASSIGN(signature, aSignature);
}
- (void) check
{
	//TODO: See if this is replacing a superclass method.	If so, infer
	//argument types, otherwise assume ids.
	FOREACH(statements, statement, AST*)
	{
		[statement setParent:self];
		[statement check];
	}
}
- (NSString*) description
{
	NSMutableString *str = [NSMutableString string];
	MethodSymbolTable *st = (MethodSymbolTable*)symbols;
	[str appendString:[signature description]];
	[str appendString:@"[ "];
	if ([[st locals] count])
	{
		[str appendString:@"\n| "];
		FOREACH([st locals], symbol, NSString*)
		{
			[str appendFormat:@"%@ ", symbol];
		}
		[str appendString:@"|"];
	}
	[str appendString:@"\n"];
	FOREACH(statements, statement, AST*)
	{
		[str appendFormat:@"%@.\n", statement];
	}
	[str appendString:@"]"];
	return str;
}
- (void*) compileWith:(id<CodeGenerator>)aGenerator
{
	const char *sel = [[signature selector] UTF8String];
	// FIXME: Should get method signature from superclass
	const char *types = sel_get_type(sel_get_any_typed_uid(sel));
	if (NULL == types) {
		int args = [[signature arguments] count];
		// FIXME: Make this @
		int offset = sizeof(id) + sizeof(SEL);
		NSMutableString *ty = [NSMutableString stringWithFormat:@"@%d@0:%d",
			sizeof(SEL) + sizeof(id) * (args + 2), offset];
		for (int i=0 ; i<args ; i++)
		{
			offset += sizeof(id);
			[ty appendFormat:@"@%d", offset];
		}
		types = [ty UTF8String];
	}
	[aGenerator beginMethod:sel
				  withTypes:types
					 locals:[[(MethodSymbolTable*)symbols locals] count]];
	FOREACH(statements, statement, AST*)
	{
		[statement compileWith:aGenerator];
	}
	[aGenerator endMethod];
	return NULL;
}
@end
