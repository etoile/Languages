#import <Foundation/Foundation.h>

@interface Symbol : NSObject
{
	SEL selector;
}
+ (id) SymbolForString:(NSString*)aSymbol;
+ (id) SymbolForCString:(const char*)aSymbol;
+ (id) SymbolForSelector:(SEL) aSelector;

- (id) initWithSelector:(SEL) aSelector;
- (id) stringValue;
- (SEL) selValue;
@end
