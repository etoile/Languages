#import <Foundation/Foundation.h>
#import "BlockClosure.h"
#import "BlockContext.h"
#include <string.h>

NSString *LKSmalltalkBlockNonLocalReturnException =
    @"LKSmalltalkBlockNonLocalReturnException";
@interface _NSBlock : NSObject
-value;
@end

@implementation _NSBlock (BlockClosure)
- (id) whileTrue:(id)anotherBlock
{
	id last = nil;
	for (id ret = [self value] ;
		(uintptr_t)ret != 1 && [ret boolValue] ;
		ret = [self value])
	{
		last = [anotherBlock value];
	}
	return last;
}
@end
