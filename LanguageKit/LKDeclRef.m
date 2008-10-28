#import "LKDeclRef.h"

@implementation LKClosedDeclRef
@end

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
				[parent resolveScopeOf:symbol];
				[self check];
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
		case promoted:
		{
			LKClosedDeclRef *decl = 
				[(LKBlockSymbolTable*)symbols promotedLocationOfSymbol:symbol];
			return [aGenerator loadBlockVarAtIndex:decl->index 
			                                offset:decl->offset];
		}
		case object:
		{
			return [aGenerator loadValueOfType:[symbols typeOfSymbol:symbol]
			                           atOffset:[symbols offsetOfIVar:symbol]
			                         fromObject:[aGenerator loadSelf]];
		}
		default:
			NSLog(@"Compiling declref to symbol %@ of type %d",
					symbol, [symbols scopeOfSymbol:symbol]);
			return [super compileWith:aGenerator];
	}
}
@end
