#import "LKMethod.h"

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
		[[LKMethodSymbolTable alloc] initWithLocals:locals
		                                       args:[aSignature arguments]];
	[self initWithSymbolTable: st];
	RELEASE(st);
	ASSIGN(signature, aSignature);
	ASSIGN(statements, statementList);
	return self;
}
- (void) setSignature:(LKMessageSend*)aSignature
{
	ASSIGN(signature, aSignature);
}
- (void) check
{
	//TODO: See if this is replacing a superclass method.	If so, infer
	//argument types, otherwise assume ids.
	FOREACH(statements, statement, LKAST*)
	{
		[statement setParent:self];
		[statement check];
	}
}
- (NSString*) description
{
	NSMutableString *str = [NSMutableString string];
	LKMethodSymbolTable *st = (LKMethodSymbolTable*)symbols;
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
	FOREACH(statements, statement, LKAST*)
	{
		[str appendFormat:@"%@.\n", statement];
	}
	[str appendString:@"]"];
	return str;
}
- (void*) compileWith:(id<LKCodeGenerator>)aGenerator
{
	const char *sel = [[signature selector] UTF8String];
	// FIXME: Should get method signature from superclass
	const char *types = sel_get_type(sel_get_any_typed_uid(sel));
	if (NULL == types) 
	{
		int args = [[signature arguments] count];
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
					 locals:[[(LKMethodSymbolTable*)symbols locals] count]];
	FOREACH(statements, statement, LKAST*)
	{
		[statement compileWith:aGenerator];
	}
	[aGenerator endMethod];
	return NULL;
}
@end
