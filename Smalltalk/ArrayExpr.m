#import "ArrayExpr.h"

@implementation ArrayExpr
- (id) initWithElements:(NSArray*)anArray
{
	SELFINIT;
	ASSIGN(elements, anArray);
	return self;
}
+ (id) arrayWithElements:(NSArray*)anArray
{
	return [[[self alloc] initWithElements:anArray] autorelease];
}
- (void) check
{
	FOREACH(elements, element, AST*)
	{
		[element setParent:self];
		[element check];
	}
}
- (NSString*) description
{
	NSMutableString *str = [NSMutableString stringWithString:@"#("];
	NSEnumerator *e = [elements reverseObjectEnumerator];
	id obj;
	while (nil != (obj = [e nextObject]))
	{
		[str appendFormat:@"%@, ", [obj description]];
	}
	[str replaceCharactersInRange:NSMakeRange([str length] - 2, 2) withString:@")"];
	return str;
}
- (void*) compileWith:(id<CodeGenerator>)aGenerator
{
	NSEnumerator *e = [elements reverseObjectEnumerator];
	id obj;
	void *values[[elements count] + 1];
	int i = 0;
	while (nil != (obj = [e nextObject]))
	{
		values[i++] = [obj compileWith:aGenerator];
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
