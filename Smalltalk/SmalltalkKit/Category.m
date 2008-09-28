#import "Category.h"

@implementation CategoryDef
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
		symbols = [[ObjectSymbolTable alloc] initForClass:class];
	}
	else
	{
		ASSIGN(symbols,
			   	[ObjectSymbolTable symbolTableForNewClassNamed:classname]);
	}
	FOREACH(methods, method, AST*)
	{
		[method setParent:self];
		[method check];
	}
}
- (NSString*) description
{
	NSMutableString *str = [NSMutableString stringWithFormat:@"%@ extend [ \n",
   		classname];
	FOREACH(methods, method, AST*)
	{
		[str appendString:[method description]];
	}
	[str appendString:@"\n]"];
	return str;
}
- (void*) compileWith:(id<CodeGenerator>)aGenerator
{
	[aGenerator createCategoryOn:classname
                         named:@"SmalltalkCategory"];
	FOREACH(methods, method, AST*)
	{
		[method compileWith:aGenerator];
	}
	[aGenerator endCategory];
	if ([[AST code] objectForKey: classname] == nil)
	{
		[[AST code] setObject: [NSMutableArray array] forKey: classname];
	}
	[[[AST code] objectForKey: classname] addObject: self];
	return NULL;
}
@end
