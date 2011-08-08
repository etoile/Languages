#import "LKDeclRef.h"
#import "LKSymbolTable.h"
#import "LKCompiler.h"
#import "LKCompilerErrors.h"


@implementation LKDeclRef
- (id) initWithSymbol:(NSString*)sym
{
	SUPERINIT;
	ASSIGN(symbol, sym);
	return self;
}
+ (id) referenceWithSymbol:(NSString*)sym
{
	return [[[self alloc] initWithSymbol: sym] autorelease];
}
- (BOOL)check
{
	// If we've already done this check, reset the symbol and do it again
	if (![symbol isKindOfClass: [NSString class]])
	{
		ASSIGN(symbol, [symbol name]);
	}
	if ([symbol characterAtIndex: 0] == '#') { return YES; }

	LKSymbol *s = [symbols symbolForName: symbol];
	if (nil == s)
	{
		NSDictionary *errorDetails = D(
			[NSString stringWithFormat: @"Unrecognised symbol: %@",
				symbol], kLKHumanReadableDescription,
			self, kLKASTNode);
		if ([LKCompiler reportError: LKUndefinedSymbolError
		                    details: errorDetails])
		{
			return [self check];
		}
		return NO;
	}
	else
	{
		ASSIGN(symbol, s);
	}
	return YES;
}
- (NSString*) description
{
	return [symbol description];
}
- (void*) compileWithGenerator: (id<LKCodeGenerator>)aGenerator
{
	NSString *symbolName = [symbol name];
	switch([symbol scope])
	{
		case LKSymbolScopeLocal:
		case LKSymbolScopeExternal:
		case LKSymbolScopeArgument:
		case LKSymbolScopeObject:
		case LKSymbolScopeClass:
			return [aGenerator loadVariable: symbol];
		case LKSymbolScopeBuiltin:
			// FIXME: It's ugly hard-coding language-specific names into the
			// symbol table.  
			if ([symbolName isEqual:@"self"] || [symbolName isEqual:@"super"])
			{
				return [aGenerator loadSelf];
			}
			else if ([symbolName isEqual:@"blockContext"])
			{
				// FIXME! This should only be valid inside a block
				return [aGenerator loadBlockContext];
			}
			else if ([symbolName isEqual:@"nil"] || [symbolName isEqual:@"Nil"])
			{
				return [aGenerator nilConstant];
			}
		case LKSymbolScopeGlobal:
			return [aGenerator loadClassNamed: symbolName];
		default:
			NSLog(@"Compiling declref to symbol %@ of type %d",
					symbol, [symbol scope]);
			return [super compileWithGenerator: aGenerator];
	}
}
- (NSString*) symbol
{
	return symbol;
}
- (void)dealloc
{
	[symbol release];
	[super dealloc];
}
@end
