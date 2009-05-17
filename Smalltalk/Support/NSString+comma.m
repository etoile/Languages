#import <Foundation/Foundation.h>

@implementation NSString (Comma)
- (NSString *)comma: (NSString *)aString
{
	return [self stringByAppendingString: aString];
}
@end

@implementation NSArray (Comma)
- (NSArray *)comma: (NSArray *)anArray
{
	return [self arrayByAddingObjectsFromArray: anArray];
}
@end


