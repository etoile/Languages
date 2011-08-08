#import "LKSymbolTable.h"
#import "Runtime/LKObject.h"
#import <EtoileFoundation/runtime.h>

static NSMutableDictionary *NewClasses;

static LKSymbolScope lookupUnscopedSymbol(NSString *aName)
{
	// FIXME: Hard-coding the names of builtins is REALLY ugly!
	if ([aName isEqualToString:@"nil"]
	   || [aName isEqualToString:@"Nil"]
	   || [aName isEqualToString:@"blockContext"]
	   || [aName isEqualToString:@"self"]
	   || [aName isEqualToString:@"super"])
	{
		return LKSymbolScopeBuiltin;
	}
	if(NSClassFromString(aName) != NULL || [NewClasses objectForKey:aName])
	{
		return LKSymbolScopeGlobal;
	}
	return LKSymbolScopeInvalid;
}

@implementation LKSymbolTable
@synthesize enclosingScope, tableScope, symbols, declarationScope;
- (void)dealloc
{
	[symbols release];
	[super dealloc];
}
+ (void)initialize
{
	NewClasses = [NSMutableDictionary new];
}
- (LKSymbolTable*)initInScope: (LKSymbolTable*)aTable
{
	SUPERINIT;
	enclosingScope = aTable;
	symbols = [NSMutableDictionary new];
	return self;
}
- (id)init
{
	return [self initInScope: nil];
}
+ (LKSymbolTable*) symbolTableForClass: (NSString*)aClassName
{
	LKSymbolTable *table = [NewClasses objectForKey: aClassName];
	if (nil != table) { return table; }

	table = [self new];
	if (nil == table) { return nil; }

	[NewClasses setObject: table forKey: aClassName];

	Class class = NSClassFromString(aClassName);
	if (class)
	{
		unsigned int ivarcount = 0;
		Ivar* ivarlist = class_copyIvarList(class, &ivarcount);
		if(ivarlist != NULL) 
		{
			for (int i = 0 ; i < ivarcount ; i++)
			{
				LKSymbol *sym = [LKSymbol new];
				NSString *name = NSStringFromRuntimeString(ivar_getName(ivarlist[i]));
				[sym setName: name];
				[sym setTypeEncoding: NSStringFromRuntimeString(ivar_getTypeEncoding(ivarlist[i]))];
				[sym setScope: LKSymbolScopeObject];
				[sym setOwner: class];
				[table addSymbol: sym];
				[sym release];
				[name release];
			}
			free(ivarlist);
		}
		class = class_getSuperclass(class);
		if (Nil != class)
		{
			[table setEnclosingScope: [self symbolTableForClass: [class className]]];
		}
	}
	return table;
}
- (void)addSymbol: (LKSymbol*)aSymbol
{
	[symbols setObject: aSymbol forKey: [aSymbol name]];
}
- (LKSymbol*)symbolForName: (NSString*)aName
{
	LKSymbol *s = [symbols objectForKey: aName];
	if (nil == s)
	{
		if (nil != enclosingScope)
		{
			LKSymbol *sym = [enclosingScope symbolForName: aName];
			// If this is an argument or local from the enclosing scope, then
			// bind it into this symbol table as an external symbol.
			switch ([sym scope])
			{
				case LKSymbolScopeExternal:
				case LKSymbolScopeArgument:
				case LKSymbolScopeLocal:
					[sym setReferencingScopes: [sym referencingScopes] + 1];
					sym = [sym copy];
					[sym setReferencingScopes: 0];
					[sym setScope: LKSymbolScopeExternal];
					[self addSymbol: sym];
					[sym release];
				default:
					return sym;
			}
		}
		LKSymbolScope scope = lookupUnscopedSymbol(aName);
		if (scope != LKSymbolScopeInvalid)
		{
			LKSymbol *sym = [LKSymbol new];
			[sym setName: aName];
			[sym setTypeEncoding: @"@"];
			[sym setScope: scope];
			[self addSymbol: sym];
			[sym release];
			return sym;
		}
	}
	return s;
}
static inline NSMutableArray* collectSymbolsOfType(NSDictionary *symbols,
                                            LKSymbolScope scope)
{
	NSMutableArray *args = nil;
	for (LKSymbol *s in [symbols objectEnumerator])
	{
		if ([s scope] == scope)
		{
			if (nil == args)
			{
				args = [NSMutableArray new];
			}
			[args addObject: s];
		}
	}
	return [args autorelease];
}

static NSInteger compareSymbolOrder(LKSymbol *a, LKSymbol *b, void *c)
{
	NSInteger i1 = [a index];
	NSInteger i2 = [b index];
	if (i1 < i2) { return NSOrderedAscending; }
	if (i1 > i2) { return NSOrderedDescending; }
	return NSOrderedSame;
}

- (NSArray*)arguments
{
	return collectSymbolsOfType(symbols, LKSymbolScopeArgument);
}
- (NSArray*)locals
{
	NSMutableArray *locals = collectSymbolsOfType(symbols, LKSymbolScopeLocal);
	[locals sortUsingFunction: compareSymbolOrder context: NULL];
	return locals;
}
- (NSArray*)byRefVariables;
{
	return collectSymbolsOfType(symbols, LKSymbolScopeExternal);
}
- (NSArray*)classVariables
{
	return collectSymbolsOfType(symbols, LKSymbolScopeClass);
}
- (NSArray*)instanceVariables
{
	return collectSymbolsOfType(symbols, LKSymbolScopeObject);
}
- (void)addSymbolsNamed: (NSArray*)anArray ofKind: (LKSymbolScope)kind;
{
	NSUInteger i = 0;
	for (NSString *symbol in anArray)
	{
		LKSymbol *sym = [LKSymbol new];
		[sym setName: symbol];
		[sym setTypeEncoding: NSStringFromRuntimeString(@encode(LKObject))];
		[sym setScope: kind];
		[sym setIndex: i++];
		[self addSymbol: sym];
		[sym release];
	}
}
@end

@implementation LKSymbol
@synthesize name, typeEncoding, owner, scope, index, referencingScopes;
- (id)init
{
	SUPERINIT;
	index = -1;
	return self;
}
- (void)dealloc
{
	[name release];
	[typeEncoding release];
	[super dealloc];
}
- (NSString*)stringValue
{
	return name;
}
- (NSString*)description
{
	return [name description];
}
- (NSUInteger)hash
{
	return [name hash];
}
- (BOOL)isEqual: (id) other
{
	return [name isEqualToString: [other name]];
}
- (id)copyWithZone: (NSZone*)aZone
{
	LKSymbol *c = [LKSymbol allocWithZone: aZone];
	[c setName: name];
	[c setTypeEncoding: typeEncoding];
	[c setOwner: owner];
	[c setScope: scope];
	[c setIndex: index];
	[c setReferencingScopes: referencingScopes];
	return c;
}
@end
