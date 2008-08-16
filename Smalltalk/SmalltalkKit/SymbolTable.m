#import "SymbolTable.h"

static NSMutableDictionary *NewClasses;

static SymbolScope lookupUnscopedSymbol(NSString *aName)
{
	if ([aName isEqualToString:@"nil"]
	   || [aName isEqualToString:@"Nil"]
	   || [aName isEqualToString:@"self"]
	   || [aName isEqualToString:@"super"])
	{
		return builtin;
	}
	if(NSClassFromString(aName) != NULL || [NewClasses objectForKey:aName])
	{
		return global;
	}
	return invalid;
}

@implementation ObjectSymbolTable
+ (void) initialize
{
	NewClasses = [[NSMutableDictionary alloc] init];
}
+ (SymbolTable*) symbolTableForNewClassNamed:(NSString*)aClass
{
	return [NewClasses objectForKey:aClass];
}
- (void) registerNewClass:(NSString*)aClass
{
	[NewClasses setObject:self forKey:aClass];
}
- (ObjectSymbolTable*) initWithMap:(NSMapTable*)map 
							  next:(int) next 
							inZone:(NSZone*)aZone
{
	SELFINIT;
	instanceVariables = NSCopyMapTableWithZone(map, aZone);
	nextOffset = next;
	return self;
}
- (id) copyWithZone:(NSZone*)aZone
{
	return [[ObjectSymbolTable allocWithZone:aZone] 
		initWithMap:instanceVariables
			   next:nextOffset
			 inZone:aZone];
}
- (SymbolTable*) initForClass:(Class)aClass 
{
	SELFINIT;
	instanceVariables = NSCreateMapTable(NSObjectMapKeyCallBacks, NSIntMapValueCallBacks, 10);
	nextOffset = aClass->instance_size;
	NSMutableDictionary *ivarTypes = [NSMutableDictionary new];
	// FIXME: Move this into a runtime-specific category.
	while (aClass != Nil && aClass != aClass->super_class)
	{
		struct objc_ivar_list* ivarlist = aClass->ivars;
		if(ivarlist != NULL) 
		{
			for (int i = 0 ; i < ivarlist->ivar_count ; i++)
			{
				int offset = ivarlist->ivar_list[i].ivar_offset;
				NSString * name = [NSString stringWithUTF8String:(char*)ivarlist->ivar_list[i].ivar_name];
				NSMapInsert(instanceVariables, (void*)name, (void*)offset);
				NSString * type = [NSString stringWithUTF8String:(char*)ivarlist->ivar_list[i].ivar_type];
				[ivarTypes setObject:type forKey:name];
			}
		}
		//Add ivars declared in the superaClass too.
		aClass = aClass->super_class;
	}
	types = ivarTypes;
	return self;
}

- (int) offsetOfIVar:(NSString*)aName
{
	return (int)NSMapGet(instanceVariables, aName);
}

- (void) addSymbol:(NSString*)aSymbol
{
	NSMapInsert(instanceVariables, (void*)aSymbol, (void*)nextOffset);
	nextOffset += sizeof(id);
}

- (SymbolScope) scopeOfSymbolNonrecursive:(NSString*)aName
{
	if(NSMapMember(instanceVariables, aName, 0, 0))
	{
		return object;
	}
	return invalid;
}

//FIXME: dealloc leaks.
@end
@implementation MethodSymbolTable
- (id) initWithLocals:(NSMutableArray*)locals
                 args:(NSMutableArray*)args
{
	SELFINIT;
	ASSIGN(localVariables, locals);
	ASSIGN(arguments, args);
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

- (NSArray*) locals
{
	return localVariables;
}
- (NSArray*) args
{
	return arguments;
}
- (SymbolScope) scopeOfSymbolNonrecursive:(NSString*)aName
{
	if ([localVariables containsObject:aName])
	{
		return local;
	}
	if ([arguments containsObject:aName])
	{
		return argument;
	}
	return invalid;
}
@end
@implementation BlockSymbolTable
- (id) init
{
	SUPERINIT;
	promotedVars = [[NSMutableDictionary alloc] init];
	return self;
}
- (NSArray*) promotedVars
{
	return [promotedVars allKeys];
}

- (id) promotedLocationOfSymbol:(NSString*)aName
{
	return [promotedVars objectForKey:aName];
}
- (void) promoteSymbol:(NSString*)aName toLocation:(id)aLocation
{
	[promotedVars setObject:aLocation forKey:aName];
}
- (SymbolScope) scopeOfExternal:(NSString*)aSymbol
{
	SymbolTable *nextscope = enclosingScope;
	while (nextscope) 
	{
		SymbolScope result = [nextscope scopeOfSymbol:aSymbol];
		if (result != invalid)
		{
			return result;
		}
		nextscope = nextscope->enclosingScope;
	}
	return invalid;
}
- (SymbolScope) scopeOfSymbolNonrecursive:(NSString*)aName
{
	switch (lookupUnscopedSymbol(aName))
	{
		case builtin:
			return builtin;
		case global:
			return global;
		default: {}
	}
	if ([localVariables containsObject:aName])
	{
		return local;
	}
	if ([arguments containsObject:aName])
	{
		return argument;
	}
	if (nil != [promotedVars objectForKey:aName])
	{
		return promoted;
	}
	return external;
}
@end
@implementation SymbolTable
// You can't insert global symbols in Smalltalk.  
- (void) addSymbol:(NSString*)aSymbol {}
- (void) setScope:(SymbolTable*)scope
{
	ASSIGN(enclosingScope, scope);
}
- (int) indexOfArgument:(NSString*)aName
{
	return [enclosingScope indexOfArgument:aName];
}
- (SymbolScope) scopeOfSymbolNonrecursive:(NSString*)aName
{
	return lookupUnscopedSymbol(aName);
}

- (SymbolScope) scopeOfSymbol:(NSString*)aName
{
	SymbolTable *scope = self;
	while (scope) 
	{
		SymbolScope result = [scope scopeOfSymbolNonrecursive:aName];
		if (result != invalid)
		{
			return result;
		}
		scope = scope->enclosingScope;
	}
	return lookupUnscopedSymbol(aName);
}
- (NSString*) typeOfSymbol:(NSString*)aName
{
	NSString * type = [types objectForKey:aName];
	if (nil != type)
	{
		return type;
	}
	if (nil != enclosingScope)
	{
		return [enclosingScope typeOfSymbol:aName];
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
@end
