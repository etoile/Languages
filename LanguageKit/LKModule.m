#import "LKModule.h"

static NSMutableDictionary *SelectorConflicts = nil;


@implementation LKCompilationUnit 
+ (void) initialize
{
	if (self != [LKCompilationUnit class])
	{
		return;
	}
	// Look up potential selector conflicts.
	void *state = NULL;
	Class class;
	NSMutableDictionary *types = [NSMutableDictionary new];
	SelectorConflicts = [NSMutableDictionary new];
	while (Nil != (class = objc_next_class(&state)))
	{
		struct objc_method_list *methods = class->methods;
		while (methods != NULL)
		{
			for (unsigned i=0 ; i<methods->method_count ; i++)
			{
				Method *m = &methods->method_list[i];

				NSString *name =
				   	[NSString stringWithCString:sel_get_name(m->method_name)];
				NSString *type = [NSString stringWithCString:m->method_types];
				NSString *oldType = [types objectForKey:name];
				if (oldType == nil)
				{
					[types setObject:type forKey:name];
				}
				else
				{
					if (![type isEqualToString:oldType])
					{
						[SelectorConflicts setObject:oldType forKey:name];
					}
				}
			}
			methods = methods->method_next;
		}
	}
	// Little hack to make the default sensible for count.  Should really be
	// loaded from a plist.
	[SelectorConflicts setObject:@"I8@0:4" forKey:@"count"];
	[types release];
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
- (void) addClass:(LKAST*)aClass
{
	[classes addObject:aClass];
}
- (void) addCategory:(LKAST*)aCategory
{
	[categories addObject:aCategory];
}
- (const char*) typeForMethod:(NSString*)methodName
{
	NSString *type = nil;
	// First see if this is an overridden type
	if (nil != (type = [typeOverrides objectForKey:methodName]))
	{
		return [type UTF8String];
	}
	// If it's a conflicted type, pick the default and log a warning
	if (nil != (type = [SelectorConflicts objectForKey:methodName]))
	{
		NSLog(@"Warning: Selector '%@' is polymorphic.  Assuming %@",
				methodName, type);
		return [type UTF8String];
	}
	// Otherwise, grab the type from the runtime
	const char *types = sel_get_type(sel_get_any_typed_uid([methodName UTF8String]));
	if (NULL == types) 
	{
		int args = 0;
		for (unsigned i=0, len = [methodName length] ; i<len ; i++)
		{
			if ([methodName characterAtIndex:i] == ':')
			{
				args++;
			}
		}
		int offset = sizeof(id) + sizeof(SEL);
		NSMutableString *ty = [NSMutableString stringWithFormat:@"@%d@0:%d",
			sizeof(SEL) + sizeof(id) * (args + 2), offset];
		for (int i=0 ; i<args ; i++)
		{
			offset += sizeof(id);
			[ty appendFormat:@"@%d", offset];
		}
		types = [ty UTF8String];
	}
	return types;
}
- (void) check
{
	// We might want to get some from other sources in future and merge these.
	ASSIGN(typeOverrides, [pragmas objectForKey:@"types"]);
	FOREACH(classes, class, LKAST*)
	{
		[class setParent:self];
		[class check];
	}
	FOREACH(categories, category, LKAST*)
	{
		[category setParent:self];
		[category check];
	}
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
- (void*) compileWith:(id<LKCodeGenerator>)aGenerator
{
	[aGenerator startModule];
	FOREACH(classes, class, LKAST*)
	{
		[class compileWith:aGenerator];
	}
	FOREACH(categories, category, LKAST*)
	{
		[category compileWith:aGenerator];
	}
	[aGenerator endModule];
	[[NSNotificationCenter defaultCenter]
	   	postNotificationName:@"NewClassesCompiledNotification"
		              object:nil];
	return NULL;
}
@end
