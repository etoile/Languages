#import "LKCategory.h"

@implementation LKCategoryDef

@synthesize classname, categoryName, methods;

- (id) initWithName:(NSString*)aName
              class:(NSString*)aClass
            methods:(NSArray*)aMethodList
{
	SUPERINIT;
	ASSIGN(classname, aClass);
	ASSIGN(categoryName, aName);
	ASSIGN(methods, [aMethodList mutableCopy]);
	return self;
}
+ (id) categoryWithName:(NSString*)aName
           onClassNamed:(NSString*)aClass
                methods:(NSArray*)aMethodList
{
	return [[self alloc] initWithName:aName
	                             class:aClass
	                           methods:aMethodList];
}
+ (id) categoryOnClassNamed:(NSString*)aName methods:(NSArray*)aMethodList
{
	return [self categoryWithName:@"AnonymousCategory"
	                 onClassNamed:aName
	                      methods:aMethodList];
}
- (BOOL)check
{
	symbols = [LKSymbolTable symbolTableForClass: classname];
	BOOL success = YES;
	FOREACH(methods, method, LKAST*)
	{
		[method setParent:self];
		success &= [method check];
	}
	return success;
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

- (void*) compileWithGenerator: (id<LKCodeGenerator>)aGenerator
{
	[aGenerator createCategoryWithName:categoryName
	                      onClassNamed:classname];
	FOREACH(methods, method, LKAST*)
	{
		[method compileWithGenerator: aGenerator];
	}
	[aGenerator endCategory];
	if ([[LKAST code] objectForKey: classname] == nil)
	{
		[[LKAST code] setObject: [NSMutableArray array] forKey: classname];
	}
	[[[LKAST code] objectForKey: classname] addObject: self];
	return NULL;
}

- (void) visitWithVisitor:(id<LKASTVisitor>)aVisitor
{
	[self visitArray:methods withVisitor:aVisitor];
}
@end
