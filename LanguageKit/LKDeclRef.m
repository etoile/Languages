#import "LKDeclRef.h"
#import "LKSymbolTable.h"
#import "LKCompiler.h"
#import "LKCompilerErrors.h"


@implementation LKDeclRef
- (id) initWithSymbol:(NSString*)sym
{
	SELFINIT;
	ASSIGN(symbol, sym);
	return self;
}
+ (id) referenceWithSymbol:(NSString*)sym
{
	return [[[self alloc] initWithSymbol: sym] autorelease];
}
- (BOOL)check
{
	if([symbol characterAtIndex:0] != '#')
	{
		switch ([symbols scopeOfSymbol:symbol])
		{
			case LKSymbolScopeInvalid:
			{
				NSDictionary *errorDetails = D(
					[NSString stringWithFormat: @"Unrecognised symbol: %@",
						symbol], kLKHumanReadableDesciption,
					self, kLKASTNode);
				if ([LKCompiler reportError: LKUndefinedSymbolError
				                    details: errorDetails])
				{
					return [self check];
				}
				return NO;
			}
			case LKSymbolScopeExternal:
			{
				LKExternalSymbolScope s = 
					[(LKBlockSymbolTable*)symbols scopeOfExternalSymbol:symbol];
				if (nil == s.scope)
				{
					NSDictionary *errorDetails = D(
						[NSString stringWithFormat: @"Unrecognised symbol: %@",
							symbol], kLKHumanReadableDesciption,
						self, kLKASTNode);
					if ([LKCompiler reportError: LKUndefinedSymbolError
					                    details: errorDetails])
					{
						return [self check];
					}
					return NO;
				}
			}
			default:
				break;
		}
	}
	return YES;
}
- (NSString*) description
{
	return symbol;
}
- (void*) compileWithGenerator: (id<LKCodeGenerator>)aGenerator
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
			return [aGenerator loadClassNamed:symbol];
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
