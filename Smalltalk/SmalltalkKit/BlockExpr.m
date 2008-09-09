#import "BlockExpr.h"
#import "DeclRef.h"

// FIXME: This currently uses locals for arguments, which is completely wrong.
@implementation BlockExpr 
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
	FOREACH(statements, s, AST*)
	{
		[s setParent:self];
		[s check];
	}
}
- (void) resolveScopeOf:(NSString*)aSymbol
{
	switch ([symbols->enclosingScope scopeOfSymbol:aSymbol])
	{
		//FIXME: Check nextClosed < 5
		case external:
		{
			[parent resolveScopeOf:aSymbol];
			[self resolveScopeOf:aSymbol];
			return;
		}
		case argument:
		case local:
		{
			NSLog(@"Promoting symbol %@ to offset %d", aSymbol, nextClosed);
			ClosedDeclRef * ref = [ClosedDeclRef new];
			ref->symbol = aSymbol;
			ref->index = nextClosed++;
			ref->offset = 0;
			[SAFECAST(BlockSymbolTable, symbols) promoteSymbol:aSymbol toLocation:ref];
			break;
		}
		case object:
		{
			ClosedDeclRef * ref = [ClosedDeclRef new];
			ref->symbol = aSymbol;
			ref->index = 0;
			ref->offset = [symbols->enclosingScope offsetOfIVar:aSymbol];
			[SAFECAST(BlockSymbolTable, symbols) promoteSymbol:aSymbol toLocation:ref];
			break;
		}
		case builtin:
		{
			return;
		}
		case promoted:
			  //FIXME: Reference the enclosing block
		default:
		{
			[NSException raise:@"InvalidBindingScope"
						format:@"Unable to bind %@", aSymbol];
		}
	}
}
- (NSString*) description
{
	NSMutableString *str = [NSMutableString string];
	MethodSymbolTable *st = (MethodSymbolTable*)symbols;
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
	FOREACH(statements, statement, AST*)
	{
		[str appendString:[statement description]];
	}
	[str appendString:@"]"];
	return str;
}
- (void*) compileWith:(id<CodeGenerator>)aGenerator
{
	// FIXME: self pointer should always be promoted.
	void *promoted[5];
	NSArray *promotedSymbols = [(BlockSymbolTable*)symbols promotedVars];
	promoted[0] = [aGenerator loadSelf];
	int index = 1;
	FOREACH(promotedSymbols, symbol, NSString*)
	{
		switch ([(BlockSymbolTable*)symbols scopeOfExternal:symbol])
		{
			case local:
				promoted[index++] = 
					[aGenerator loadPointerToLocalAtIndex:
						[symbols->enclosingScope offsetOfLocal:symbol]];
				break;
			case argument:
				promoted[index++] = 
					[aGenerator loadPointerToArgumentAtIndex:
						[symbols->enclosingScope indexOfArgument:symbol]];

			case object:
				// Instance variables are accessed relative to the self pointer.
				break;
			default:
				NSAssert1(NO, @"Don't know how to promote %@.", symbol);
		}
		NSAssert(index < 5, 
				@"Too many promoted variables to fit in block object");
	}
	// FIXME: Locals
	[aGenerator beginBlockWithArgs:[[(MethodSymbolTable*)symbols args] count]
	                        locals:0
						 boundVars:promoted
							 count:index];
	void * lastValue = NULL;
	FOREACH(statements, statement, AST*)
	{
		lastValue = [statement compileWith:aGenerator];
	}
	[aGenerator blockReturn:lastValue];
	return [aGenerator endBlock];
}
@end
