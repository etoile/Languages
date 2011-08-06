#import "LKModule.h"
#import "LKSubclass.h"
#import <EtoileFoundation/runtime.h>
#import "Runtime/LKObject.h"
#import <objc/runtime.h>

/**
 * Maps method names to type encodings, gathered by iterating through all
 * methods in all classes. Needed only on the Mac runtime, which
 * doesn't have a function for looking up the types given a selector.
 */
static NSMutableDictionary *Types = nil;
static NSMutableDictionary *SelectorConflicts = nil;
NSString *LKCompilerDidCompileNewClassesNotification = 
	@"LKCompilerDidCompileNewClassesNotification";
/**
 * Creates an NSString from a string returned by the runtime.  These strings
 * are guaranteed to persist for the duration of the program, so there's no
 * need to copy the data.
 */
static NSString *NSStringFromRuntimeString(const char *cString)
{
	return [[[NSString alloc] initWithBytesNoCopy: (char*)cString
	                                       length: strlen(cString)
	                                     encoding: NSUTF8StringEncoding
	                                 freeWhenDone: NO] autorelease];
}

SEL sel_get_any_typed_uid(const char *name);

#if defined(__GNUSTEP_RUNTIME__)
static NSArray *TypesForMethodName(NSString *methodName)
{
	const char *buffer[16];
	unsigned int count = sel_copyTypes_np([methodName UTF8String], buffer, 16);
	const char **types = buffer;
	if (count > 16)
	{
		types = calloc(count, sizeof(char*));
		sel_copyTypes_np([methodName UTF8String], buffer, count);
	}
	NSMutableArray *typeStrings = [NSMutableArray new];
	for (unsigned int i=0 ; i<count ; i++)
	{
		[typeStrings addObject: NSStringFromRuntimeString(types[i])];
	}
	if (buffer != types)
	{
		free(types);
	}
	return [typeStrings autorelease];
}
#else
static NSArray* TypesForMethodName(NSString *methodName)
{
	return [Types objectForKey: methodName];
}
#endif

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

			NSString *name = NSStringFromRuntimeString(sel_getName(method_getName(m)));
			NSString *type = NSStringFromRuntimeString(method_getTypeEncoding(m));
			[Types addObject: type forKey: name];
		}
	}
	
	if (numClasses > 0 && NULL != classes)
	{
		free(classes);
	}
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
		id value = [NSPropertyListSerialization propertyListFromData: [[aDict objectForKey:key] dataUsingEncoding:NSUTF8StringEncoding]
		                                            mutabilityOption: NSPropertyListMutableContainersAndLeaves
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
- (NSArray*) typesForMethod:(NSString*)methodName
{
	NSArray *types = TypesForMethodName(methodName);
	
	if ([types count] == 0)
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
		NSMutableString *ty = [NSMutableString stringWithFormat: @"%s%d@0:%d",
			@encode(LKObject), sizeof(SEL) + sizeof(id) * (argCount + 2),
			offset];
		for (int i=0 ; i<argCount ; i++)
		{
			offset += sizeof(id);
			[ty appendFormat: @"%s%d", @encode(LKObject), offset];
		}
		types = [NSArray arrayWithObject: ty];
	}
	return types;
}
- (BOOL)check
{
	// We might want to get some from other sources in future and merge these.
	ASSIGN(typeOverrides, [pragmas objectForKey:@"types"]);
	BOOL success = YES;
	FOREACH(classes, forwardClass, LKSubclass*)
	{
		[LKSymbolTable symbolTableForClass: [forwardClass classname]];
	}
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
	// FIXME: Get the file name from somewhere.
	[aGenerator startModule: @"Anonymous"];
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
