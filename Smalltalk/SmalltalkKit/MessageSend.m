#import "MessageSend.h"
#import "DeclRef.h"

static Class NSStringClass = Nil;
static NSDictionary *MangledSelectors = nil;
static NSMutableDictionary *SelectorConflicts = nil;


@implementation NSString (Print)
- (void) print
{
  printf("%s", [self UTF8String]);
}
@end
@implementation MessageSend 
+ (void) initialize
{
	if (self != [MessageSend class])
	{
		return;
	}
	NSStringClass = [NSString class];
	MangledSelectors = [D(
			@"add:", @"+:",
			@"isLessThan:", @"<:",
			@"isGreaterThan:", @">:",
			@"sub:", @"-:",
			@"div:", @"/:",
			@"mul:", @"*:") retain];
	// Look up potential selector conflicts.
	void *state = NULL;
	Class nextClass;
	NSMutableDictionary *types = [NSMutableDictionary new];
	SelectorConflicts = [NSMutableDictionary new];
	while(Nil != (nextClass = objc_next_class(&state)))
	{
		Class class = nextClass;
		struct objc_method_list *methods = class->methods;
		if (methods != NULL)
		{
			for (unsigned i=0 ; i<methods->method_count ; i++)
			{
				Method *m = &methods->method_list[i];

				NSString *name =
				   	[NSString stringWithCString:sel_get_name(m->method_name)];
				NSString *type = [NSString stringWithCString:m->method_types];
				NSString *oldType = [types objectForKey:name];
				if (oldType && ![type isEqualToString:oldType])
				{
					[SelectorConflicts setObject:oldType forKey:name];
				}
				else
				{
					[types setObject:type forKey:name];
				}
			}
		}
	}
	[types release];
}

- (void) setTarget:(id)anObject
{
	ASSIGN(target, anObject);
}

- (void) addSelectorComponent:(NSString*)aSelector
{
	if(selector == nil)
	{
		ASSIGN(selector, aSelector);
	}
	else
	{
		NSString * sel = [selector stringByAppendingString:aSelector];
		ASSIGN(selector, sel);
	}
}

- (void) addArgument:(id)anObject
{
	if (arguments == nil)
	{
		arguments = [[NSMutableArray alloc] init];
	}
	[arguments addObject:anObject];
}
- (NSMutableArray*) arguments
{
  return arguments;
}
- (NSString*) selector
{
	return selector;
}

- (void) check
{
	[target setParent:self];
	[target check];
	NSString *types = [SelectorConflicts objectForKey:selector];
	if (nil != types)
	{
		NSLog(@"Warning: Selector '%@' is polymorphic.  Assuming %@", selector,
				types);
	}
	//SEL sel = sel_get_any_typed_uid([selector UTF8String]);
	//NSLog(@"Selector %s types: %s", sel_get_name(sel), sel_get_type(sel));
	FOREACH(arguments, arg, AST*)
	{
		[arg setParent:self];
		[arg check];
	}
}

- (NSString*) description
{
	NSMutableString *str = [NSMutableString string];
	[str appendString:[target description]];
	[str appendString:@" "];
	NSArray *sel = [selector componentsSeparatedByString:@":"];
	int i;
	if ([sel count] == 1)
	{
		[str appendString:selector];
	}
	else
	{
		[str appendString:[sel objectAtIndex:0]];
	}
	if ([arguments count])
	{
		[str appendFormat:@": %@", [arguments objectAtIndex:0]];
	}
	for (i=1 ; i<[arguments count] ; i++)
	{
		if (i<[sel count])
		{
			[str appendString:[sel objectAtIndex:i]];
			printf(" %s", [[sel objectAtIndex:i] UTF8String]);
		}
		[str appendFormat:@": %@", [arguments objectAtIndex:i]];
	}
	return str;
}
- (void*) compileWith:(id<CodeGenerator>)aGenerator
{
	unsigned argc = [arguments count];
	void *argv[argc];
	for (unsigned i=0 ; i<argc ; i++)
	{
		argv[i] = [[arguments objectAtIndex:i] compileWith:aGenerator];
	}
	void * receiver;
	receiver = [target compileWith:aGenerator];
	NSString * mangledSelector = [MangledSelectors objectForKey:selector];
	const char *sel;
	if (nil == mangledSelector)
	{
		sel = [selector UTF8String];
	}
	else
	{
		sel = [mangledSelector UTF8String];
	}
	// FIXME: Use methodSignatureForSelector in inferred target type if possible.
	const char *seltypes = sel_get_type(sel_get_any_typed_uid(sel));
	// FIXME: This is a really ugly hack.
	if ([selector isEqualToString:@"count"])
	{
		seltypes = "I8@0:4";
	}
	// If the receiver is a global symbol, it is guaranteed to be an object.
	// TODO: The same is arguments if their type is @
	if ([target isKindOfClass:[DeclRef class]])
	{
		DeclRef *ref = SAFECAST(DeclRef, target);
		NSString *symbol = ref->symbol;
		SymbolScope scope = [symbols scopeOfSymbol:symbol];
		if (scope == global)
		{
			return [aGenerator sendMessage:sel
			                         types:seltypes
			                       toObject:receiver
			                      withArgs:argv
			                         count:argc];
		}
		if (scope == builtin)
		{
			if ([symbol isEqualToString:@"self"])
			{
				return [aGenerator sendMessage:sel
										 types:seltypes
									   toObject:receiver
									  withArgs:argv
										 count:argc];
			}
			if ([symbol isEqualToString:@"super"])
			{
				return [aGenerator sendSuperMessage:sel
										      types:seltypes
									       withArgs:argv
										      count:argc];
			}
		}
	}
	return [aGenerator sendMessage:sel
	                         types:seltypes
	                            to:receiver
	                      withArgs:argv
	                         count:argc];
}
@end
