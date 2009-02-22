#import "LKSymbolTable.h"

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
	if(NSClassFromString(aName) != NULL || [NewClasses objectForKey:aName])
	{
		return LKSymbolScopeGlobal;
	}
	return LKSymbolScopeInvalid;
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
- (void) registerNewClassNamed:(NSString*)aClass
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
- (void)dealloc
{
	NSFreeMapTable(instanceVariables);
	[super dealloc];
}
- (id) copyWithZone:(NSZone*)aZone
{
	return [[LKObjectSymbolTable allocWithZone:aZone] 
	                               initWithMap:instanceVariables
	                                      next:nextOffset
	                                    inZone:aZone];
}
- (void) addClassVariable: (NSString*) aClassVar
{
	[classVariables addObject:aClassVar];
}
- (LKSymbolTable*) initForClass:(Class)aClass 
{
	SELFINIT;
	classVariables = [[NSMutableArray alloc] init];
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
	if (NSMapMember(instanceVariables, aName, 0, 0))
	{
		return LKSymbolScopeObject;
	}
	if ([classVariables containsObject:aName])
	{
		return LKSymbolScopeClass;
	}
	return LKSymbolScopeInvalid;
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
		LKSymbolScope result = [nextscope scopeOfSymbol:aSymbol];
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
//TODO You can't insert LKSymbolScopeGlobal symbols yet.
- (void) addSymbol:(NSString*)aSymbol {}
- (void) setScope:(LKSymbolTable*)scope
{
	enclosingScope = scope;
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
	[types release];
	[super dealloc];
}
@end
