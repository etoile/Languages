#import "MessageSend.h"
#import "DeclRef.h"

static Class NSStringClass = Nil;
static NSDictionary *MangledSelectors = nil;

@implementation NSString (Print)
- (void) print
{
  printf("%s", [self UTF8String]);
}
@end
@implementation MessageSend 
+ (void) initialize
{
	NSStringClass = [NSString class];
	MangledSelectors = [D(
			@"add:", @"+:",
			@"sub:", @"-:",
			@"div:", @"/:",
			@"mul:", @"*:") retain];
	[super initialize];
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
	//[self checkRValue:target];
	//SEL sel = sel_get_any_typed_uid([selector UTF8String]);
	//NSLog(@"Selector %s types: %s", sel_get_name(sel), sel_get_type(sel));
	FOREACH(arguments, arg, AST*)
	{
		[arg setParent:self];
		[arg check];
	//	[self checkRValue:arg];
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
	// If the receiver is a global symbol, it is guaranteed to be an object.
	// TODO: The same is arguments if their type is @
	if ([target isKindOfClass:[DeclRef class]])
	{
		DeclRef *ref = SAFECAST(DeclRef, target);
		NSString *symbol = ref->symbol;
		if ([symbols scopeOfSymbol:symbol] == global)
		{
			return [aGenerator sendMessage:sel
			                         types:seltypes
			                       toObject:receiver
			                      withArgs:argv
			                         count:argc];
		}
	}
	return [aGenerator sendMessage:sel
	                         types:seltypes
	                            to:receiver
	                      withArgs:argv
	                         count:argc];
}
@end
