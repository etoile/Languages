#import "Method.h"

@implementation SmalltalkMethod
- (void) setSignature:(NSString*)aSignature
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
		// FIXME: Make this @
		types = "@12@0:8";
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
