#import "EScriptPreamble.h"
#import <LanguageKit/LKModule.h>
#import <LanguageKit/LKInterpreter.h>
#import "EScriptObject.h"


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
	NSString *types_new = [[module typesForMethod:@"new"] objectAtIndex: 0];

	id res;
	[symbols addSymbolsNamed: A(@"Object", @"Array") ofKind: LKSymbolScopeClass];
	res = [aGenerator loadClassNamed: @"EScriptObject"];
	res = [aGenerator sendMessage: @"new"
	                        types: types_new
	                     toObject: res
	                     withArgs: NULL
	                        count: 0];
	[aGenerator storeValue: res inVariable: [symbols symbolForName: @"Object"]];
	res = [aGenerator loadClassNamed: @"NSMutableArray"];
	res = [aGenerator sendMessage: @"new"
	                        types: types_new
	                     toObject: res
	                     withArgs: NULL
	                        count: 0];
	[aGenerator storeValue: res inVariable: [symbols symbolForName: @"Array"]];

	return NULL;
}
- (id)interpretInContext: (LKInterpreterContext*)context
{
	id o = [EScriptObject new];
	[context setValue: o
	        forSymbol: @"Object"];
	o = [NSMutableArray new];
	[context setValue: o
	        forSymbol: @"Array"];
	return nil;
}
@end
