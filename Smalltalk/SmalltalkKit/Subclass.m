#import "Subclass.h"

@implementation Subclass
- (id) initWithName:(NSString*)aName superclass:(NSString*)aClass ivars:(NSArray*)anIvarList methods:(NSArray*)aMethodList
{
	SELFINIT;
	ASSIGN(classname, aName);
	ASSIGN(superclass, aClass);
	ASSIGN(ivars, anIvarList);
	ASSIGN(methods, aMethodList);
	return self;
}
- (void) check
{
	Class SuperClass = NSClassFromString(superclass);
	if (Nil == SuperClass)
	{
		NSLog(@"Attempting to subclass nonexistant class %@", superclass);
	}
	if (Nil != NSClassFromString(classname))
	{
		NSLog(@"Can not create new class %@ - a class of this name already exists.", classname);
	}
	//Construct symbol table.
  [SymbolTable registerNewClass:classname];
	symbols = [[ObjectSymbolTable alloc] initForClass:SuperClass];
	FOREACH(ivars, ivar, NSString*)
	{
		[symbols addSymbol:ivar];
	}
	FOREACH(methods, method, AST*)
	{
		[method setParent:self];
		[method check];
	}
}
- (NSString*) description
{
	NSMutableString *str = [NSMutableString stringWithFormat:@"%@ subclass: %@ [ \n",
   		superclass, classname];
	if ([ivars count])
	{
		[str appendString:@"| "];
		FOREACH(ivars, ivar, NSString*)
		{
			[str appendFormat:@"%@ ", ivar];
		}
		[str appendString:@"|\n"];
	}
	FOREACH(methods, method, AST*)
	{
		[str appendString:[method description]];
	}
	[str appendString:@"\n]"];
	return str;
}
- (void*) compileWith:(id<CodeGenerator>)aGenerator
{
	void * none[1] = {NULL};
	[aGenerator createSubclass:classname
                   subclassing:superclass
                 withIvarNames:(const char**)none
                         types:(const char**)none
                       offsets:(int*)none];
	FOREACH(methods, method, AST*)
	{
		[method compileWith:aGenerator];
	}
	[aGenerator endClass];
	if ([[AST code] objectForKey: classname] == nil)
	{
		[[AST code] setObject: [NSMutableArray array] forKey: classname];
	}
	[[[AST code] objectForKey: classname] addObject: self];
	return NULL;
}
@end
