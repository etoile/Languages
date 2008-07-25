#import "AST.h"
#import "DeclRef.h"

Class DeclRefClass;

@implementation AST
+ (void) initialize
{
	DeclRefClass = [DeclRef class];
}
- (id) initWithSymbolTable:(SymbolTable*)aSymbolTable
{
	if(nil == (self = [self init]))
	{
		return nil;
	}
	ASSIGN(symbols, aSymbolTable);
	return self;
}
- (void) print
{
	printf("%s", [[self description] UTF8String]);
}
- (void) setParent:(AST*)aNode
{
	if (nil == symbols)
	{
		ASSIGN(symbols, [aNode symbols]);
	}
	else
	{
		[symbols setScope:[aNode symbols]];
	}
	ASSIGN(parent, aNode);
}
- (void) setBracketed:(BOOL)aFlag
{
	isBracket = aFlag;
}
- (BOOL) isBracketed
{
	return isBracket;
}
- (void) check
{
	// Subclasses should implement this.
	[self doesNotRecognizeSelector:_cmd];
}
- (void) resolveScopeOf:(NSString*)aSymbol
{
	[parent resolveScopeOf:aSymbol];
}
- (void) checkLValue:(id) aChild
{
	if (nil == aChild)
	{
		// nil is valid as a target since it signifies the result is ignored
		return;
	}
	if ([aChild isKindOfClass:DeclRefClass])
	{
		DeclRef *child = aChild;
		switch ([symbols scopeOfSymbol:child->symbol])
		{
			case 0:
			{
				[NSException raise:@"InvalidSymbol"
							format:@"Unrecognised symbol %@", aChild];
			}
			case global:
			{
				[NSException raise:@"InvalidLValue"
							format:@"Global symbol %@ is not a valid l-value", aChild];
			}
			default: 
			{
				return;
			}
		}
	}
	else
	{
		[NSException raise:@"InvalidLValue"
					format:@"Result of an expression (%@) may not be used as an l-value", aChild];
	}
}
- (void) checkRValue:(id) aChild
{
	//FIXME: This does not need to be called always here.
	[aChild setParent:self];
	[aChild check];
}
- (void*) compileWith:(id<CodeGenerator>)aGenerator
{
	NSLog(@"Compiling...");
	[NSException raise:@"NotImplementedException"
	            format:@"Code generation not yet implemented for %@", 
		[self class]];
	return NULL;
}
- (SymbolTable*) symbols
{
	return symbols;
}
@end
