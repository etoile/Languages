#import <Foundation/Foundation.h>

#ifndef __has_feature      // Optional.
#define __has_feature(x) 0 // Compatibility with non-clang compilers.
#endif

#ifndef NS_RETURNS_RETAINED
#if __has_feature(attribute_ns_returns_retained)
#define NS_RETURNS_RETAINED __attribute__((ns_returns_retained))
#else
#define NS_RETURNS_RETAINED
#endif
#endif


@interface Symbol : NSObject
{
	SEL selector;
}
+ (id) SymbolForString:(NSString*)aSymbol NS_RETURNS_RETAINED;
+ (id) SymbolForCString:(const char*)aSymbol NS_RETURNS_RETAINED;
+ (id) SymbolForSelector:(SEL) aSelector NS_RETURNS_RETAINED;

- (id) copyWithZone: (NSZone*)aZone;
- (id) initWithSelector:(SEL) aSelector;
- (id) stringValue;
- (SEL) selValue;
@end
