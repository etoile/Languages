#import <EtoileFoundation/EtoileFoundation.h>

@interface Symbol : NSObject {
	SEL selector;
}
@end 
@implementation Symbol
- (id) initWithSelector:(SEL) aSelector
{
	SELFINIT
	selector = aSelector;
	return self;
}
+ (id) SymbolForCString:(char*)aSymbol
{
	return [[Symbol alloc] initWithSelector:sel_get_uid(aSymbol)];
}
- (id) stringValue
{
	return NSStringFromSelector(selector);
}
- (SEL) selValue
{
	return selector;
}
@end
