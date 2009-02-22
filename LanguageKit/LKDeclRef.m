#import "LKDeclRef.h"
#import "LKSymbolTable.h"


@implementation LKDeclRef
+ (id) reference:(NSString*)sym
{
	return [[[self alloc] initWithSymbol: sym] autorelease];
}
- (id) initWithSymbol:(NSString*)sym
{
	SELFINIT;
	ASSIGN(symbol, sym);
	return self;
}
- (void) check
{
	if([symbol characterAtIndex:0] != '#')
	{
		switch ([symbols scopeOfSymbol:symbol])
		{
			case LKSymbolScopeInvalid:
				[NSException raise:@"InvalidSymbol"
							format:@"Unrecognised symbol %@", symbol];
			case LKSymbolScopeExternal:
			{
				LKExternalSymbolScope s = 
					[(LKBlockSymbolTable*)symbols scopeOfExternalSymbol:symbol];
				if (nil == s.scope)
				{
					[NSException raise:@"InvalidSymbol"
								format:@"Unrecognised symbol %@", symbol];
				}
			}
			default:
				break;
		}
	}
}
- (NSString*) description
{
	return symbol;
}
- (void*) compileWith:(id<LKCodeGenerator>)aGenerator
{
	switch([symbols scopeOfSymbol:symbol])
	{
		case LKSymbolScopeLocal:
			return [aGenerator loadLocalAtIndex:[symbols offsetOfLocal:symbol]];
		case LKSymbolScopeBuiltin:
			if ([symbol isEqual:@"self"] || [symbol isEqual:@"super"])
			{
				return [aGenerator loadSelf];
			}
			else if ([symbol isEqual:@"nil"] || [symbol isEqual:@"Nil"])
			{
				return [aGenerator nilConstant];
			}
		case LKSymbolScopeGlobal:
			return [aGenerator loadClass:symbol];
		case LKSymbolScopeArgument:
	   		return [aGenerator loadArgumentAtIndex:
						  [symbols indexOfArgument:symbol]];
		case LKSymbolScopeExternal:
		{
			LKExternalSymbolScope s = 
				[(LKBlockSymbolTable*)symbols scopeOfExternalSymbol: symbol];
			switch([s.scope scopeOfSymbol: symbol])
			{
				case LKSymbolScopeLocal:
					return [aGenerator loadLocalAtIndex: [s.scope offsetOfLocal:symbol]
					                lexicalScopeAtDepth: s.depth];
				case LKSymbolScopeArgument:
					return [aGenerator loadArgumentAtIndex: [s.scope indexOfArgument:symbol]
					                   lexicalScopeAtDepth: s.depth];
				case LKSymbolScopeObject:
				{
					return [aGenerator loadValueOfType: [s.scope typeOfSymbol:symbol]
											  atOffset: [s.scope offsetOfIVar:symbol]
											fromObject: [aGenerator loadSelf]];
				}
				case LKSymbolScopeClass:
				{
					return [aGenerator loadClassVariable: symbol];
				}
				default:
					NSAssert(NO, @"Invalid scope for external");
			}
		}
		case LKSymbolScopeObject:
		{
			return [aGenerator loadValueOfType: [symbols typeOfSymbol:symbol]
			                          atOffset: [symbols offsetOfIVar:symbol]
			                        fromObject: [aGenerator loadSelf]];
		}
		case LKSymbolScopeClass:
		{
			return [aGenerator loadClassVariable: symbol];
		}
		default:
			NSLog(@"Compiling declref to symbol %@ of type %d",
					symbol, [symbols scopeOfSymbol:symbol]);
			return [super compileWith:aGenerator];
	}
}
- (NSString*) symbol
{
	return symbol;
}
@end
