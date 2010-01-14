#import <Foundation/NSArray.h>

@interface NSArray (map)
- (NSArray*) map:(id)aClosure;
- (void) foreach:(id)aClosure;
- (NSArray*) select:(id)aClosure;
- (id) detect:(id)aClosure;
- (id) fold:(id)aClosure;
@end
