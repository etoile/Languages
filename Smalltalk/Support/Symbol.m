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
/***
 * Used by LanguageKit to construct constant boxed selectors.  Returned
 * selectors should persist for the length of the program.
 */
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
