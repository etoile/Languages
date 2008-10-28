
#import <Foundation/Foundation.h>

@implementation NSObject (Introspection)

/**
 * Returns the names of all methods supported by the receiver.
 */
- (NSArray*) allMethodNames
{
  return GSObjCMethodNames(self);
}

@end
