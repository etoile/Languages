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
			case invalid:
				[NSException raise:@"InvalidSymbol"
							format:@"Unrecognised symbol %@", symbol];
			case external:
			{
				LKExternalSymbolScope s = 
					[(LKBlockSymbolTable*)symbols scopeOfExternal:symbol];
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
		case local:
			return [aGenerator loadLocalAtIndex:[symbols offsetOfLocal:symbol]];
		case builtin:
			if ([symbol isEqual:@"self"] || [symbol isEqual:@"super"])
			{
				return [aGenerator loadSelf];
			}
			else if ([symbol isEqual:@"nil"] || [symbol isEqual:@"Nil"])
			{
				return [aGenerator nilConstant];
			}
		case global:
			return [aGenerator loadClass:symbol];
		case argument:
	   		return [aGenerator loadArgumentAtIndex:
						  [symbols indexOfArgument:symbol]];
		case external:
		{
			LKExternalSymbolScope s = [(LKBlockSymbolTable*)symbols scopeOfExternal: symbol];
			switch([s.scope scopeOfSymbol: symbol])
			{
				case local:
					return [aGenerator loadLocalAtIndex: [s.scope offsetOfLocal:symbol]
					                lexicalScopeAtDepth: s.depth];
				case argument:
					return [aGenerator loadArgumentAtIndex: [s.scope indexOfArgument:symbol]
					                   lexicalScopeAtDepth: s.depth];
				case object:
				{
					return [aGenerator loadValueOfType: [s.scope typeOfSymbol:symbol]
											  atOffset: [s.scope offsetOfIVar:symbol]
											fromObject: [aGenerator loadSelf]];
				}
				case class:
				{
					return [aGenerator loadClassVariable: symbol];
				}
				default:
					NSAssert(NO, @"Invalid scope for external");
			}
		}
		case object:
		{
			return [aGenerator loadValueOfType: [symbols typeOfSymbol:symbol]
			                          atOffset: [symbols offsetOfIVar:symbol]
			                        fromObject: [aGenerator loadSelf]];
		}
		case class:
		{
			return [aGenerator loadClassVariable: symbol];
		}
		default:
			NSLog(@"Compiling declref to symbol %@ of type %d",
					symbol, [symbols scopeOfSymbol:symbol]);
			return [super compileWith:aGenerator];
	}
}
@end
