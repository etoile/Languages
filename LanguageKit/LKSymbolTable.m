#import "LKSymbolTable.h"

static NSMutableDictionary *NewClasses;

static LKSymbolScope lookupUnscopedSymbol(NSString *aName)
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

@implementation LKObjectSymbolTable
+ (void) initialize
{
	NewClasses = [[NSMutableDictionary alloc] init];
}
+ (LKSymbolTable*) symbolTableForNewClassNamed:(NSString*)aClass
{
	return [NewClasses objectForKey:aClass];
}
- (int) instanceSize
{
	return nextOffset;
}
- (void) registerNewClass:(NSString*)aClass
{
	[NewClasses setObject:self forKey:aClass];
}
- (LKObjectSymbolTable*) initWithMap:(NSMapTable*)map 
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
	return [[LKObjectSymbolTable allocWithZone:aZone] 
	                               initWithMap:instanceVariables
	                                      next:nextOffset
	                                    inZone:aZone];
}
- (LKSymbolTable*) initForClass:(Class)aClass 
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
				NSString * name = [NSString stringWithUTF8String:
					(char*)ivarlist->ivar_list[i].ivar_name];
				NSMapInsert(instanceVariables, (void*)name,
					   	(void*)(uintptr_t)offset);
				NSString * type = [NSString stringWithUTF8String:
					(char*)ivarlist->ivar_list[i].ivar_type];
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
	return (int)(intptr_t)NSMapGet(instanceVariables, aName);
}

- (void) addSymbol:(NSString*)aSymbol
{
	NSMapInsert(instanceVariables, (void*)aSymbol, (void*)(intptr_t)nextOffset);
	nextOffset += sizeof(id);
}

- (LKSymbolScope) scopeOfSymbolNonrecursive:(NSString*)aName
{
	if(NSMapMember(instanceVariables, aName, 0, 0))
	{
		return object;
	}
	return invalid;
}

@end
@implementation LKMethodSymbolTable
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
- (LKSymbolScope) scopeOfSymbolNonrecursive:(NSString*)aName
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
- (void) dealloc
{
	[localVariables release];
	[arguments release];
	[super dealloc];
}
@end
@implementation LKBlockSymbolTable
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
- (LKSymbolScope) scopeOfExternal:(NSString*)aSymbol
{
	LKSymbolTable *nextscope = enclosingScope;
	while (nextscope) 
	{
		LKSymbolScope result = [nextscope scopeOfSymbol:aSymbol];
		if (result != invalid)
		{
			return result;
		}
		nextscope = nextscope->enclosingScope;
	}
	return invalid;
}
- (LKSymbolScope) scopeOfSymbolNonrecursive:(NSString*)aName
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
- (void) dealloc
{
	[promotedVars release];
	[super dealloc];
}
@end
@implementation LKSymbolTable
// You can't insert global symbols in Smalltalk.  
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
	while (scope) 
	{
		LKSymbolScope result = [scope scopeOfSymbolNonrecursive:aName];
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
- (void) dealloc
{
	[enclosingScope release];
	[super dealloc];
}
@end
