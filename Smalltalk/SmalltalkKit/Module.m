#import "Module.h"

@implementation CompilationUnit 
- (id) init
{
	SUPERINIT;
	classes = [[NSMutableArray alloc] init];
	categories = [[NSMutableArray alloc] init];
	return self;
}
- (void) addClass:(AST*)aClass
{
	[classes addObject:aClass];
}
- (void) addCategory:(AST*)aCategory
{
	[categories addObject:aCategory];
}
- (void) check
{
	FOREACH(classes, class, AST*)
	{
		[class check];
	}
	FOREACH(categories, category, AST*)
	{
		[category check];
	}
}
- (NSString*) description
{
	NSMutableString *str = [NSMutableString string];
	FOREACH(classes, class, AST*)
	{
		[str appendString:[class description]];
	}
	FOREACH(categories, category, AST*)
	{
		[str appendString:[category description]];
	}
	return str;
}
- (void*) compileWith:(id<CodeGenerator>)aGenerator
{
	[aGenerator startModule];
	FOREACH(classes, class, AST*)
	{
		[class compileWith:aGenerator];
	}
	FOREACH(categories, category, AST*)
	{
		[category compileWith:aGenerator];
	}
	[aGenerator endModule];
	return NULL;
}
@end
