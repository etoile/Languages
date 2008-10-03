#import "NSObject+ReplaceMethods.h"

extern void __objc_update_dispatch_table_for_class (Class);
extern void __objc_register_selectors_from_list (MethodList_t); 

@implementation NSObject (RuntimeHackery)
+ (int) replaceMethodForSelector:(SEL)aSelector with:(IMP)aMethod
{
	MethodList_t methods = ((Class)self)->methods;
	while(methods != NULL)
	{
		for(unsigned int i=0 ; i<methods->method_count ; i++)
		{
			Method_t method = &methods->method_list[i];
			//We perform a string comparison, because == does not work on SEL
			if([NSStringFromSelector(method->method_name) isEqualToString:NSStringFromSelector(aSelector)])
			{
				NSLog(@"Replacing method...");
				method->method_imp = aMethod;
				__objc_update_dispatch_table_for_class(self);
				return 0;
			}
		}
		NSLog(@"Looking in next list");
		methods = methods->method_next;
	}
	return -1;
}
+ (int) addMethod:(IMP)aMethod forSelectorNamed:(char*)aSelector
{
	MethodList_t methods = ((Class)self)->methods;
	//Find the last method list
	while(methods->method_next != NULL)
	{
		methods = methods->method_next;
	}
	methods->method_next = calloc(sizeof(struct objc_method_list), 1);
	methods->method_next->method_count = 1;
	Method_t method =  &methods->method_next->method_list[0];
	method->method_name = sel_register_name(aSelector);
	method->method_types = NULL;
	method->method_imp = aMethod;
	__objc_update_dispatch_table_for_class(self);
	return 0;
}
@end
