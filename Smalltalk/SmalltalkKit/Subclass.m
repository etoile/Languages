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
	symbols = [[ObjectSymbolTable alloc] initForClass:SuperClass];
	FOREACH(ivars, ivar, NSString*)
	{
		[symbols addSymbol:ivar];
	}
    [(ObjectSymbolTable*)symbols registerNewClass:classname];
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
	const char *ivarNames[[ivars count] + 1];
	const char *ivarTypes[[ivars count] + 1];
	int ivarOffsets[[ivars count] + 1];
	for (int i=0; i<[ivars count]; i++)
	{
		ivarNames[i] = [[ivars objectAtIndex: i] UTF8String];
		ivarTypes[i] = "@";
		ivarOffsets[i] = [symbols offsetOfIVar: [ivars objectAtIndex: i]];
	}
	ivarNames[[ivars count]] = NULL;
	ivarTypes[[ivars count]] = NULL;
	ivarOffsets[[ivars count]] = 0;

	[aGenerator createSubclass:classname
                   subclassing:superclass
                 withIvarNames:ivarNames
                         types:ivarTypes
                       offsets:ivarOffsets];
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
