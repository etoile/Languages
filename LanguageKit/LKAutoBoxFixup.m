#import <EtoileFoundation/EtoileFoundation.h>
#import <LanguageKit/LanguageKit.h>
#import <objc/hooks.h>
#import <objc/runtime.h>

struct objc_slot* objc_get_slot(Class cls, SEL selector);

static struct objc_slot* objc_method_type_fixup(Class cls, SEL
		selector, struct objc_slot *result)
{
	LOCAL_AUTORELEASE_POOL();
	id <LKCodeGenerator> jit = defaultJIT();
	const char *selName = sel_getName(selector);
	const char *selTypes = sel_getType_np(selector);
	[jit startModule: nil];
	[jit createCategoryWithName: @"Type_Fixup"
				   onClassNamed: [NSString stringWithUTF8String: class_getName(cls)]];
	if (class_isMetaClass(cls))
	{
		[jit beginClassMethod: selName
		            withTypes: selTypes
		               locals: NULL
		                count: 0];
	}
	else
	{
		[jit beginInstanceMethod: selName
		               withTypes: selTypes
		                  locals: NULL
		                   count: 0];
	}
	SEL correctSel = sel_registerTypedName_np(selName, result->types);
	Method m = class_getInstanceMethod(cls, correctSel);
	int argc = method_getNumberOfArguments(m) - 2;
	void *argv[argc];
	for (int i=0 ; i<argc ; i++)
	{
		argv[i] = [jit loadArgumentAtIndex: i lexicalScopeAtDepth: 0];
	}
	void *ret = [jit sendMessage: selName
	                       types: result->types
	                    toObject: [jit loadSelf]
	                    withArgs: argv
	                       count: argc];
	if (selTypes[0] != 'v' && result->types[0] != 'v')
	{
		[jit setReturn: ret];
	}
	[jit endMethod];
	[jit endCategory];
	[jit endModule];
	return objc_get_slot(cls, selector);
}
