#import "LKAST.h"
#import "LKDeclRef.h"

Class DeclRefClass;
Class ModuleClass;

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
	ModuleClass = [LKModule class];
}

- (LKModule*) module
{
	while (nil != self && ModuleClass != isa)
	{
		self = parent;
	}
	return (LKModule*)self;
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
- (void) inheritSymbolTable:(LKSymbolTable*)aSymbolTable
{
	ASSIGN(symbols, aSymbolTable);
}
- (LKAST*) parent;
{
	return parent;
}
- (void) setParent:(LKAST*)aNode
{
	[self inheritSymbolTable:[aNode symbols]];
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
			case LKSymbolScopeGlobal:
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
- (void*) compileWithGenerator: (id<LKCodeGenerator>)aGenerator
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
- (BOOL) isComment
{
	return NO;
}
@end
@implementation LKAST (Visitor)
- (void) visitWithVisitor:(id<LKASTVisitor>)aVisitor
{
	// AST nodes with no children do nothing.
}
- (void) visitArray:(NSMutableArray*)anArray
        withVisitor:(id<LKASTVisitor>)aVisitor
{
	unsigned count = [anArray count];
	for (int i=0 ; i<count ; i++)
	{
		LKAST *old = [anArray objectAtIndex:i];
		LKAST *new = [aVisitor visitASTNode:old];
		if (new != old)
		{
			[anArray replaceObjectAtIndex:i withObject:new];
		}
		[new visitWithVisitor:aVisitor];
	}
}
@end
