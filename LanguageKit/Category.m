#import "Category.h"

@implementation LKCategoryDef
+ (id) categoryWithClass:(NSString*)aName methods:(NSArray*)aMethodList
{
	return [[[self alloc] initWithClass: aName
	                            methods: aMethodList] autorelease];
}
- (id) initWithClass:(NSString*)aName methods:(NSArray*)aMethodList
{
	SELFINIT;
	ASSIGN(classname, aName);
	ASSIGN(methods, aMethodList);
	return self;
}
- (void) check
{
	Class class = NSClassFromString(classname);
	//Construct symbol table.
	if (Nil != class)
	{
		symbols = [[LKObjectSymbolTable alloc] initForClass:class];
	}
	else
	{
		ASSIGN(symbols,
			   	[LKObjectSymbolTable symbolTableForNewClassNamed:classname]);
	}
	FOREACH(methods, method, LKAST*)
	{
		[method setParent:self];
		[method check];
	}
}
- (NSString*) description
{
	NSMutableString *str = [NSMutableString stringWithFormat:@"%@ extend [ \n",
   		classname];
	FOREACH(methods, method, LKAST*)
	{
		[str appendString:[method description]];
	}
	[str appendString:@"\n]"];
	return str;
}
- (void*) compileWith:(id<LKCodeGenerator>)aGenerator
{
	[aGenerator createCategoryOn:classname
                         named:@"SmalltalkCategory"];
	FOREACH(methods, method, LKAST*)
	{
		[method compileWith:aGenerator];
	}
	[aGenerator endCategory];
	if ([[LKAST code] objectForKey: classname] == nil)
	{
		[[LKAST code] setObject: [NSMutableArray array] forKey: classname];
	}
	[[[LKAST code] objectForKey: classname] addObject: self];
	return NULL;
}
@end
