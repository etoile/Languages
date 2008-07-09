#include <EtoileFoundation/EtoileFoundation.h>
@interface NSObject (RuntimeHackery)
+ (int) replaceMethodForSelector:(SEL)aSelector with:(IMP)aMethod;
+ (int) addMethod:(IMP)aMethod forSelectorNamed:(char*)aSelector;
@end
