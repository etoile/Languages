#import "EScriptPreamble.h"
#import <LanguageKit/LKModule.h>
#import <LanguageKit/LKInterpreter.h>
#import "EScriptObject.h"

#define MAKE_PROTOTYPE(name, class)\
	res = [aGenerator loadClassNamed:@#class];\
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

- (BOOL)check
{
	return YES;
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
	MAKE_PROTOTYPE(Object, EScriptObject)
	MAKE_PROTOTYPE(Array,  NSMutableArray)

	return NULL;
}
- (id)interpretInContext: (LKInterpreterContext*)context
{
	id o = [EScriptObject new];
	[o becomePrototype];
	[context setValue: o
	        forSymbol: @"Object"];
	o = [NSMutableArray new];
	[o becomePrototype];
	[context setValue: o
	        forSymbol: @"Array"];
	return nil;
}
@end
