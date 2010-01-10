#import "LKModule.h"
#import <EtoileFoundation/runtime.h>

/**
 * Maps method names to type encodings, gathered by iterating through all
 * methods in all classes. Needed only on the Mac runtime, which
 * doesn't have a function for looking up the types given a selector.
 */
static NSMutableDictionary *Types = nil;
static NSMutableDictionary *SelectorConflicts = nil;
NSString *LKCompilerDidCompileNewClassesNotification = 
	@"LKCompilerDidCompileNewClassesNotification";


#if defined(GNU_RUNTIME)
static const char *TypesForMethodName(NSString *methodName)
{
	return sel_get_type(sel_get_any_typed_uid([methodName UTF8String]));
}
#else
static const char *TypesForMethodName(NSString *methodName)
{
	return [[Types objectForKey:methodName] UTF8String];
}
#endif

static NSString *typeEncodingRemovingQualifiers(NSString *str)
{
	NSMutableString *simplified = [[str mutableCopy] autorelease];
	NSRange r = {0,1};
	while(r.location < [simplified length])
	{
		switch ([simplified characterAtIndex: r.location])
		{
			case 'r':
			case 'n':
			case 'N':
			case 'o':
			case 'O':
			case 'R':
			case 'V':
				[simplified deleteCharactersInRange: r];
				break;
			default:
				r.location++;
		}
	}
	return simplified;
}


@implementation LKModule 
+ (void) initialize
{
	if (self != [LKModule class])
	{
		return;
	}
	// Look up potential selector conflicts.
	Types = [NSMutableDictionary new];
	SelectorConflicts = [NSMutableDictionary new];
	
	unsigned int numClasses = objc_getClassList(NULL, 0);
	Class *classes = NULL;
	if (numClasses > 0 )
	{
		classes = malloc(sizeof(Class) * numClasses);
		numClasses = objc_getClassList(classes, numClasses);
	}
	
	for (unsigned int classIndex=0; classIndex<numClasses; classIndex++)
	{
		unsigned int methodCount;
		Method *methods = class_copyMethodList(classes[classIndex], &methodCount);
		for (unsigned int i=0 ; i<methodCount ; i++)
		{
			Method m = methods[i];

			NSString *name =
				[NSString stringWithCString: sel_getName(method_getName(m))];
			NSString *type =
				[NSString stringWithCString: method_getTypeEncoding(m)];
			NSString *oldType = [Types objectForKey:name];
			if (oldType == nil)
			{
				[Types setObject: typeEncodingRemovingQualifiers(type) forKey:name];
			}
			else
			{
				if (![typeEncodingRemovingQualifiers(type) isEqualToString:oldType])
				{
					[SelectorConflicts setObject:oldType forKey:name];
				}
			}
		}
	}
	
	if (numClasses > 0 && NULL != classes)
	{
		free(classes);
	}
	
	// Little hack to make the default sensible for count.  Should really be
	// loaded from a plist.
	[SelectorConflicts setObject:@"I8@0:4" forKey:@"count"];
}
+ (id) module
{
	return AUTORELEASE([[self alloc] init]);
}
- (id) init
{
	SUPERINIT;
	classes = [[NSMutableArray alloc] init];
	categories = [[NSMutableArray alloc] init];
	pragmas = [[NSMutableDictionary alloc] init];
	return self;
}
- (void) addPragmas: (NSDictionary*)aDict
{
	NSEnumerator *e = [aDict keyEnumerator];
	for (id key = [e nextObject] ; nil != key ; key = [e nextObject])
	{
		id value = [NSPropertyListSerialization propertyListFromData:
			[[aDict objectForKey:key] dataUsingEncoding:NSUTF8StringEncoding]
		                                            mutabilityOption:
			NSPropertyListMutableContainersAndLeaves	
		                                                      format: NULL
		                                            errorDescription: NULL];
		id oldValue = [pragmas objectForKey: key];
		if (nil == oldValue)
		{
			[pragmas setObject: value forKey: key];
		}
		else
		{
			NSAssert(NO, @"Code for merging pragmas not yet implemented");
		}
	}
}
- (void) addClass:(LKSubclass*)aClass
{
	[classes addObject:aClass];
}
- (void) addCategory:(LKCategory*)aCategory
{
	[categories addObject:aCategory];
}
- (BOOL)isSelectorPolymorphic: (NSString*)methodName
{
	return ([typeOverrides objectForKey: methodName] == nil)
		&&
		(nil != [SelectorConflicts objectForKey:methodName]);
}
- (const char*) typeForMethod:(NSString*)methodName
{
	NSString *type = [typeOverrides objectForKey:methodName];
	// First see if this is an overridden type
	if (nil != type)
	{
		return [type UTF8String];
	}
	// If it's a conflicted type, pick the default and log a warning
	if (nil != (type = [SelectorConflicts objectForKey:methodName]))
	{
		return [type UTF8String];
	}
	// Otherwise, grab the type from the runtime (for the GNU runtime)
	// or from the mapping set up in +initialize (for the Mac runtime)
	
	const char *types = TypesForMethodName(methodName);
	
	if (NULL == types) 
	{
		int argCount = 0;
		for (unsigned i=0, len = [methodName length] ; i<len ; i++)
		{
			if ([methodName characterAtIndex:i] == ':')
			{
				argCount++;
			}
		}
		int offset = sizeof(id) + sizeof(SEL);
		NSMutableString *ty = [NSMutableString stringWithFormat:@"@%d@0:%d",
			sizeof(SEL) + sizeof(id) * (argCount + 2), offset];
		for (int i=0 ; i<argCount ; i++)
		{
			offset += sizeof(id);
			[ty appendFormat:@"@%d", offset];
		}
		types = [ty UTF8String];
	}
	return types;
}
- (BOOL)check
{
	// We might want to get some from other sources in future and merge these.
	ASSIGN(typeOverrides, [pragmas objectForKey:@"types"]);
	BOOL success = YES;
	FOREACH(classes, class, LKAST*)
	{
		[class setParent:self];
		success &= [class check];
	}
	FOREACH(categories, category, LKAST*)
	{
		[category setParent:self];
		success &= [category check];
	}
	return success;
}
- (NSString*) description
{
	NSMutableString *str = [NSMutableString string];
	FOREACH(classes, class, LKAST*)
	{
		[str appendString:[class description]];
	}
	FOREACH(categories, category, LKAST*)
	{
		[str appendString:[category description]];
	}
	return str;
}
- (void*) compileWithGenerator: (id<LKCodeGenerator>)aGenerator
{
	[aGenerator startModule];
	FOREACH(classes, class, LKAST*)
	{
		[class compileWithGenerator: aGenerator];
	}
	FOREACH(categories, category, LKAST*)
	{
		[category compileWithGenerator: aGenerator];
	}
	[aGenerator endModule];
	[[NSNotificationCenter defaultCenter]
	   	postNotificationName: LKCompilerDidCompileNewClassesNotification
		              object: nil];
	return NULL;
}
- (void) visitWithVisitor:(id<LKASTVisitor>)aVisitor
{
	[self visitArray: classes withVisitor: aVisitor];
	[self visitArray: categories withVisitor: aVisitor];
}
- (NSArray*)allClasses
{
	return classes;
}
- (NSArray*)allCategories
{
	return categories;
}
- (NSDictionary*) pragmas
{
	return pragmas;
}
- (void)dealloc
{
	[classes release];
	[categories release];
	[pragmas release];
	[typeOverrides release];
	[super dealloc];
}
@end
