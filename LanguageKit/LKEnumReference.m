#import "LKEnumReference.h"
#import "LKCompiler.h"
#import "LKCompilerErrors.h"
#import "SourceCodeKit/SCKIntrospection.h"

@interface LKCompiler (PrivateStuff)
+ (id)valueOf: (NSString*)enumName inEnumeration: (NSString*)anEnumeration;
@end


@implementation LKEnumReference
- (id)initWithValue: (NSString*)aValue inEnumeration: (NSString*)anEnum
{
	enumName = anEnum;
	enumValue = aValue;
	return self;
}

- (BOOL)check
{
	id val = [LKCompiler valueOf: enumValue inEnumeration: enumName];
	if (val == nil || [val isKindOfClass: [NSArray class]])
	{
		NSDictionary *errorDetails = D(
			[NSString stringWithFormat: @"Value %@ not found in enumeration %@", enumValue, enumName],
				kLKHumanReadableDescription,
			self, kLKASTNode);
		if ([LKCompiler reportError: LKEnumError
		                    details: errorDetails])
		{
			return [self check];
		}
		return NO;
	}
	SCKEnumerationValue *ev = val;
	value = [ev longLongValue];
	return YES;
}
- (void*) compileWithGenerator: (id<LKCodeGenerator>)aGenerator
{
	return [aGenerator intConstant: [NSString stringWithFormat: @"%lld", value]];
}
@end
