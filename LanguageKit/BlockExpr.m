#import "BlockExpr.h"
#import "DeclRef.h"

// FIXME: This currently uses locals for arguments, which is completely wrong.
@implementation LKBlockExpr 
+ (id) blockWithArguments:(NSMutableArray*)arguments locals:(NSMutableArray*)locals statements:(NSMutableArray*)statementList
{
	return [[[self alloc] initWithArguments: arguments
	                                 locals: locals
	                             statements: statementList] autorelease];
}
- (id) initWithArguments:(NSMutableArray*)arguments locals:(NSMutableArray*)locals statements:(NSMutableArray*)statementList
{
	SELFINIT;
	LKBlockSymbolTable *st = [[LKBlockSymbolTable alloc] initWithLocals:locals args:arguments];
	[self initWithSymbolTable: st];
	RELEASE(st);
	ASSIGN(statements, statementList);
	return self;
}
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
	FOREACH(statements, s, LKAST*)
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
			LKClosedDeclRef * ref = [LKClosedDeclRef new];
			ref->symbol = aSymbol;
			ref->index = nextClosed++;
			ref->offset = 0;
			[SAFECAST(LKBlockSymbolTable, symbols) promoteSymbol:aSymbol toLocation:ref];
			break;
		}
		case object:
		{
			LKClosedDeclRef * ref = [LKClosedDeclRef new];
			ref->symbol = aSymbol;
			ref->index = 0;
			ref->offset = [symbols->enclosingScope offsetOfIVar:aSymbol];
			[SAFECAST(LKBlockSymbolTable, symbols) promoteSymbol:aSymbol toLocation:ref];
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
	LKMethodSymbolTable *st = (LKMethodSymbolTable*)symbols;
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
	FOREACH(statements, statement, LKAST*)
	{
		[str appendString:[statement description]];
		[str appendString:@".\n"];
	}
	[str appendString:@"]"];
	return str;
}
- (void*) compileWith:(id<LKCodeGenerator>)aGenerator
{
	// FIXME: self pointer should always be promoted.
	void *promoted[5];
	LKBlockSymbolTable *st = (LKBlockSymbolTable*)symbols;
	NSArray *promotedSymbols = [st promotedVars];
	promoted[0] = [aGenerator loadSelf];
	int index = 1;
	FOREACH(promotedSymbols, symbol, NSString*)
	{
		switch ([(LKBlockSymbolTable*)symbols scopeOfExternal:symbol])
		{
			case local:
			{
				index++;
				int location = ((LKClosedDeclRef*)[st promotedLocationOfSymbol:symbol])->index;
				promoted[location] = 
					[aGenerator loadPointerToLocalAtIndex:
						[symbols->enclosingScope offsetOfLocal:symbol]];
				break;
			}
			case argument:
			{
				index++;
				int location = ((LKClosedDeclRef*)[st promotedLocationOfSymbol:symbol])->index;
				promoted[location] = 
					[aGenerator loadPointerToArgumentAtIndex:
						[symbols->enclosingScope indexOfArgument:symbol]];
				break;
			}
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
	[aGenerator beginBlockWithArgs:[[(LKMethodSymbolTable*)symbols args] count]
	                        locals:0
						 boundVars:promoted
							 count:index];
	void * lastValue = NULL;
	FOREACH(statements, statement, LKAST*)
	{
		lastValue = [statement compileWith:aGenerator];
	}
	[aGenerator blockReturn:lastValue];
	return [aGenerator endBlock];
}
@end
