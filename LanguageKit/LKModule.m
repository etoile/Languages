#import "LKModule.h"

@implementation LKCompilationUnit 
- (id) init
{
	SUPERINIT;
	classes = [[NSMutableArray alloc] init];
	categories = [[NSMutableArray alloc] init];
	return self;
}
- (void) addClass:(LKAST*)aClass
{
	[classes addObject:aClass];
}
- (void) addCategory:(LKAST*)aCategory
{
	[categories addObject:aCategory];
}
- (void) check
{
	FOREACH(classes, class, LKAST*)
	{
		[class check];
	}
	FOREACH(categories, category, LKAST*)
	{
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
