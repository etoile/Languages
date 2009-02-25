#import "EScriptPreamble.h"
#import <LanguageKit/LKModule.h>

#define MAKE_PROTOTYPE(name, class)\
	res = [aGenerator loadClass:@#class];\
	res = [aGenerator sendMessage:"new"\
	                        types:types_new\
	                     toObject:res\
	                     withArgs:NULL\
	                        count:0];\
	[aGenerator sendMessage:"becomePrototype"\
	                  types:types_pro\
	               toObject:res\
	               withArgs:NULL\
	                  count:0];\
	[aGenerator storeValue:res inClassVariable:@#name];

@implementation EScriptPreamble
+ (id) preamble
{
	return AUTORELEASE([[self alloc] init]);
}

- (void) check
{
}

- (NSString*) description
{
	return @"EScript Preamble";
}

- (void*) compileWithGenerator:(id<LKCodeGenerator>)aGenerator
{
	id module = [self module];
	const char *types_new = [module typeForMethod:@"new"];
	const char *types_pro = [module typeForMethod:@"becomePrototype"];

	id res;
	MAKE_PROTOTYPE(Object, NSObject)
	MAKE_PROTOTYPE(Array,  NSMutableArray)

	return NULL;
}
@end
