#import "DeclRef.h"

@implementation ClosedDeclRef
@end

@implementation DeclRef
+ (DeclRef*) refForDecl:(NSString*)sym
{
	DeclRef *ref = [[[DeclRef alloc] init] autorelease];
	ref->symbol = sym;
	return ref;
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
- (void*) compileWith:(id<CodeGenerator>)aGenerator
{
	switch([symbols scopeOfSymbol:symbol])
	{
		case local:
			return [aGenerator loadLocalAtIndex:[symbols offsetOfLocal:symbol]];
		case builtin:
			if ([symbol isEqual:@"self"])
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
			return [aGenerator loadArgumentAtIndex:[symbols indexOfArgument:symbol]];
		case promoted:
		{
			ClosedDeclRef *decl = [(BlockSymbolTable*)symbols promotedLocationOfSymbol:symbol];
			return [aGenerator loadBlockVarAtIndex:decl->index offset:decl->offset];
		}
		default:
			NSLog(@"Compiling declref to symbol %@ of type %d",
					symbol, [symbols scopeOfSymbol:symbol]);
			return [super compileWith:aGenerator];
	}
}
@end
