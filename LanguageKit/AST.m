#import "AST.h"
#import "DeclRef.h"

Class DeclRefClass;

static NSMutableDictionary *ASTSubclassAndCategoryNodes = nil;

@implementation LKAST

+ (NSMutableDictionary *) code
{
	if (ASTSubclassAndCategoryNodes == nil)
	{
		ASTSubclassAndCategoryNodes = [[NSMutableDictionary alloc] init];
	}
	return ASTSubclassAndCategoryNodes;
}

+ (void) initialize
{
	DeclRefClass = [LKDeclRef class];
}
- (id) initWithSymbolTable:(LKSymbolTable*)aSymbolTable
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
- (void) setParent:(LKAST*)aNode
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
		LKDeclRef *child = aChild;
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
- (void*) compileWith:(id<LKCodeGenerator>)aGenerator
{
	NSLog(@"Compiling...");
	[NSException raise:@"NotImplementedException"
	            format:@"Code generation not yet implemented for %@", 
		[self class]];
	return NULL;
}
- (LKSymbolTable*) symbols
{
	return symbols;
}
@end
