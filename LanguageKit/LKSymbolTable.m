#import "LKSymbolTable.h"
#import <EtoileFoundation/runtime.h>
#import <SourceCodeKit/SourceCodeKit.h>

static NSMutableDictionary *NewClasses;

static LKSymbolScope lookupUnscopedSymbol(NSString *aName)
{
	if ([aName isEqualToString:@"nil"]
	   || [aName isEqualToString:@"Nil"]
	   || [aName isEqualToString:@"self"]
	   || [aName isEqualToString:@"super"])
	{
		return LKSymbolScopeBuiltin;
	}
	if (NSClassFromString(aName) != NULL || [NewClasses objectForKey:aName])
	{
		return LKSymbolScopeClassName;
	}
	return LKSymbolScopeInvalid;
}

@interface LKSymbolTable (Private)
- (LKSymbolScope) scopeOfSymbolNonrecursive:(NSString*)aName;
@end

@implementation LKObjectSymbolTable
+ (LKSymbolTable*) symbolTableForNewClassNamed:(NSString*)aClass
{
	return [NewClasses objectForKey: aClass];
}

- (LKObjectSymbolTable*) initEmptyTable
{
	SELFINIT;
	classVariables = [[NSMutableArray alloc] init];
	instanceVariables = NSCreateMapTable(NSObjectMapKeyCallBacks, NSIntMapValueCallBacks, 10);
	nextOffset = 0;
	types = [NSMutableDictionary new];
	return self;
}
+ (LKObjectSymbolTable*)symbolTable
{
	return [[[self alloc] initEmptyTable] autorelease];
}
- (int) instanceSize
{
	// Check for nil in case nil messaging does not return zero
	if (nil != enclosingScope)
	{
		return [(id)enclosingScope instanceSize] + nextOffset;
	}
	else
	{
		return nextOffset;
	}
}
- (void) registerNewClassNamed: (NSString*)aClass
{
	ASSIGN(className, aClass);
	[NewClasses setObject: self forKey: aClass];
}
- (void)dealloc
{
	NSFreeMapTable(instanceVariables);
	[classVariables release];
	[className release];
	[super dealloc];
}
- (void) addClassVariable: (NSString*) aClassVar
{
	[classVariables addObject:aClassVar];
}
- (void)setClassName: (NSString*)aName
{
	ASSIGN(className, aName);
}
- (LKObjectSymbolTable*)symbolTableForSubclassNamed: (NSString*)aString
{
	LKObjectSymbolTable *symtab = [LKObjectSymbolTable symbolTable];
	[symtab setClassName: aString];
	[symtab setScope: self];
	return symtab;
}
- (int)nextOffset
{
	return nextOffset;
}
- (void)setScope:(LKSymbolTable*)scope
{
	[super setScope: scope];
}
- (LKSymbolTable*) initForClass:(Class)aClass 
{
	SELFINIT;
	Class aSuperClass;
	classVariables = [[NSMutableArray alloc] init];
	instanceVariables = NSCreateMapTable(NSObjectMapKeyCallBacks, NSIntMapValueCallBacks, 10);
	// nextOffset = class_getInstanceSize(aClass);
	NSMutableDictionary *ivarTypes = [NSMutableDictionary new];
	className = [[NSString alloc] initWithUTF8String: class_getName(aClass)];
	
	unsigned int ivarcount = 0;
	Ivar* ivarlist = class_copyIvarList(aClass, &ivarcount);
	if(ivarlist != NULL) 
	{
		for (int i = 0 ; i < ivarcount ; i++)
		{
			int offset = ivar_getOffset(ivarlist[i]);
			NSString * name = [NSString stringWithUTF8String:
				(char*)ivar_getName(ivarlist[i])];
			NSMapInsert(instanceVariables, (void*)name,
					(void*)(uintptr_t)offset);
			NSString * type = [NSString stringWithUTF8String:
				(char*)ivar_getTypeEncoding(ivarlist[i])];
			[ivarTypes setObject:type forKey:name];
		}
		free(ivarlist);
	}
	types = ivarTypes;
	if (Nil != (aSuperClass = class_getSuperclass(aClass)))
	{
		NSString *superName = [[NSString alloc] initWithUTF8String: class_getName(aSuperClass)];
		id parentScope = [NewClasses objectForKey: superName];
		[superName release];
		if (nil == parentScope)
		{
			parentScope = [[LKObjectSymbolTable alloc] initForClass: aSuperClass];
		}
		[self setScope: parentScope];
	}

	// Calculate the next offset for adding instance variables
	// based on the class size and superclass size.
	nextOffset = class_getInstanceSize(aClass);
	if (aSuperClass != nil)
		nextOffset -= class_getInstanceSize(aSuperClass);

	[NewClasses setObject: self forKey: className];
	return self;
}

- (int) offsetOfIVar:(NSString*)aName
{
	int offset;

	if (YES == NSMapMember(instanceVariables, aName, NULL, (void**)&offset))
	{
		return [(id)enclosingScope instanceSize] + offset;
	}
	else
	{
		return [enclosingScope offsetOfIVar: aName];
	}
}
- (NSString*)classForIvar: (NSString*)aName
{
	int offset;

	if (YES == NSMapMember(instanceVariables, aName, NULL, (void**)&offset))
	{
		return className;
	}
	else
	{
		return [enclosingScope classForIvar: aName];
	}
}

- (void) addSymbol:(NSString*)aSymbol
{
	NSMapInsert(instanceVariables, (void*)aSymbol, (void*)(intptr_t)nextOffset);
	nextOffset += sizeof(id);
}

- (LKSymbolScope) scopeOfSymbolNonrecursive:(NSString*)aName
{
	if (YES == NSMapMember(instanceVariables, aName, 0, 0))
	{
		return LKSymbolScopeObject;
	}
	if ([classVariables containsObject:aName])
	{
		return LKSymbolScopeClass;
	}
	return LKSymbolScopeInvalid;
}
- (NSString*)className
{
	return className;
}
@end
@implementation LKMethodSymbolTable
- (id) initWithLocals:(NSArray*)locals
                 args:(NSArray*)args
{
	SELFINIT;
	if (locals == nil)
	{
		localVariables = [[NSMutableArray alloc] init];
	}
	else
	{
		localVariables = [locals mutableCopy];
	}
	if (args == nil)
	{
		arguments = [[NSMutableArray alloc] init];
	}
	else
	{
		arguments = [args mutableCopy];
	}
	return self;
}
- (int) indexOfArgument:(NSString*)aName
{
	return [arguments indexOfObject:aName];
}
- (int) offsetOfLocal:(NSString*)aName
{
	return [localVariables indexOfObject:aName];
}

- (void) addSymbol:(NSString*)aSymbol
{
	[localVariables addObject:aSymbol];
}

- (NSMutableArray*)locals
{
	return localVariables;
}
- (NSMutableArray*) args
{
	return arguments;
}
- (LKSymbolScope) scopeOfSymbolNonrecursive:(NSString*)aName
{
	if ([localVariables containsObject:aName])
	{
		return LKSymbolScopeLocal;
	}
	if ([arguments containsObject:aName])
	{
		return LKSymbolScopeArgument;
	}
	return LKSymbolScopeInvalid;
}
- (void) dealloc
{
	[localVariables release];
	[arguments release];
	[super dealloc];
}
@end
@implementation LKBlockSymbolTable
- (LKExternalSymbolScope) scopeOfExternalSymbol:(NSString*)aSymbol
{
	LKExternalSymbolScope scope = {0, nil};
	LKSymbolTable *nextscope = enclosingScope;
	while (nil != nextscope) 
	{
		scope.depth++;
		LKSymbolScope result = [nextscope scopeOfSymbolNonrecursive:aSymbol];
		if (result != LKSymbolScopeInvalid && result != LKSymbolScopeExternal)
		{
			scope.scope = nextscope;
			break;
		}
		nextscope = nextscope->enclosingScope;
	}
	return scope;
}

- (LKSymbolScope) scopeOfSymbolNonrecursive:(NSString*)aName
{
	LKSymbolScope scope = lookupUnscopedSymbol(aName);
	if (scope == LKSymbolScopeInvalid)
	{
		scope = [super scopeOfSymbolNonrecursive:aName];
		if (scope == LKSymbolScopeInvalid)
		{
			return LKSymbolScopeExternal;
		}
	}
	return scope;
}
@end
@implementation LKSymbolTable
+ (void) initialize
{
	NewClasses = [[NSMutableDictionary alloc] init];
}
+ (void) forwardDeclareNewClass: (NSString*) className
{
	if (![NewClasses objectForKey: className])
	{
		[NewClasses setObject: [NSNull null] forKey: className];
	}
}
- (void) addSymbol:(NSString*)aSymbol {}
- (void) setScope:(LKSymbolTable*)scope
{
	ASSIGN(enclosingScope, scope);
}
- (int) indexOfArgument:(NSString*)aName
{
	return [enclosingScope indexOfArgument:aName];
}
- (LKSymbolScope) scopeOfSymbolNonrecursive:(NSString*)aName
{
	return lookupUnscopedSymbol(aName);
}

- (LKSymbolScope) scopeOfSymbol:(NSString*)aName
{
	LKSymbolTable *scope = self;
	while (nil != scope) 
	{
		LKSymbolScope result = [scope scopeOfSymbolNonrecursive:aName];
		if (result != LKSymbolScopeInvalid)
		{
			return result;
		}
		scope = scope->enclosingScope;
	}
	return lookupUnscopedSymbol(aName);
}
- (NSString*)typeOfSymbol:(NSString*)aName
{
	NSString * type = [types objectForKey: aName];
	if (nil != type)
	{
		return type;
	}
	if (nil != enclosingScope)
	{
		return [enclosingScope typeOfSymbol: aName];
	}
	// Untyped objects are untyped objects.
	return @"@";
}
- (int) offsetOfLocal:(NSString*)aName
{
	return [enclosingScope offsetOfLocal:aName];
}
- (int) offsetOfIVar:(NSString*)aName
{
	return [enclosingScope offsetOfIVar:aName];
}
- (NSString*)classForIvar: (NSString*)aName
{
	return [enclosingScope classForIvar: aName];
}
- (void) dealloc
{
	[enclosingScope release];
	[types release];
	[super dealloc];
}
@end

@implementation LKGlobalSymbolTable : LKSymbolTable
- (LKGlobalSymbolTable*)initWithSourceCollection: (SCKSourceCollection*)collection
{
	SUPERINIT;
	ASSIGN(sources, collection);
	return self;
}
+ (LKGlobalSymbolTable*)symbolTableWithSourceCollection: (SCKSourceCollection*)collection;
{
	return [[[self alloc] initWithSourceCollection: collection] autorelease];
}

- (LKSymbolScope)scopeOfSymbolNonrecursive: (NSString*)aName
{
	if ([sources.globals objectForKey: aName] != nil)
	{
		return LKSymbolScopeGlobal;
	}
	return LKSymbolScopeInvalid;
}
- (NSString*)typeOfSymbol: (NSString*)aName
{
	SCKGlobal *global = [sources.globals objectForKey: aName];
	if (global != nil)
	{
		return global.type;
	}
	if (nil != enclosingScope)
	{
		return [enclosingScope typeOfSymbol: aName];
	}
	// Untyped objects are untyped objects.
	return @"@";
}
@end
