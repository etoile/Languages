
#import <Foundation/Foundation.h>

@implementation NSObject (Introspection)

- (NSArray*) allMethodNames
{
  "Returns the names of all methods supported by the receiver."
  return GSObjCMethodNames(self);
}

@end
