#import "LKArrayExpr.h"

@implementation LKArrayExpr
+ (id) arrayWithElements:(NSArray*)anArray
{
	return [[[self alloc] initWithElements: anArray] autorelease];
}
- (id) initWithElements:(NSArray*)anArray
{
	SELFINIT;
	ASSIGN(elements, anArray);
	return self;
}
- (void) check
{
	FOREACH(elements, element, LKAST*)
	{
		[element setParent:self];
		[element check];
	}
}
- (NSString*) description
{
	NSMutableString *str = [NSMutableString stringWithString:@"#("];
	FOREACH(elements, element, LKAST*)
	{
		[str appendFormat:@"%@, ", [element description]];
	}
	[str replaceCharactersInRange:NSMakeRange([str length] - 2, 2) withString:@")"];
	return str;
}
- (void*) compileWith:(id<LKCodeGenerator>)aGenerator
{
	void *values[[elements count] + 1];
	int i = 0;
	FOREACH(elements, element, LKAST*)
	{
		values[i++] = [element compileWith:aGenerator];
	}
	values[i++] = [aGenerator nilConstant];
	void *arrayClass = [aGenerator loadClass:@"NSMutableArray"];
	return [aGenerator sendMessage:"arrayWithObjects:"
							 types:NULL
						  toObject:arrayClass
						  withArgs:values
							 count:i];
}
@end
