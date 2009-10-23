#import <EtoileFoundation/EtoileFoundation.h>
#import <EtoileFoundation/runtime.h>
#import "Symbol.h"

@implementation Symbol
+ (id) SymbolForString:(NSString*)aSymbol
{
	return [self SymbolForCString: [aSymbol UTF8String]];
}
+ (id) SymbolForCString:(const char*)aSymbol
{
	return [[Symbol alloc] initWithSelector:sel_getUid(aSymbol)];
}
+ (id) SymbolForSelector:(SEL) aSelector
{
	return [[Symbol alloc] initWithSelector:aSelector];
}
- (id) initWithSelector:(SEL) aSelector
{
	SELFINIT
	selector = aSelector;
	return self;
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
