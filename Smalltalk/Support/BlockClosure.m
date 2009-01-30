#import <Foundation/Foundation.h>
#import "BlockClosure.h"

@implementation BlockClosure (ExceptionHandling)
- (id) onException: (NSString*) exceptionName do: (BlockClosure*) handler
{
	NS_DURING
		NS_VALUERETURN([self value], id);
	NS_HANDLER
		if ([[localException name] isEqualToString: exceptionName])
		{
			NS_VALUERETURN([handler value: localException], id);
		}
		else
		{
			[localException raise];
		}
	NS_ENDHANDLER
	// Not reached
	return nil;
}
@end
