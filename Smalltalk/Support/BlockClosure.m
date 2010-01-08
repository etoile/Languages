#import <Foundation/Foundation.h>
#import "BlockClosure.h"

extern NSString *LKSmalltalkBlockNonLocalReturnException;

@implementation BlockClosure (ExceptionHandling)
- (id) onException: (NSString*) exceptionName do: (BlockClosure*) handler
{
	NS_DURING
		NS_VALUERETURN([self value], id);
	NS_HANDLER
		// If this is an exception that the interpreter is using to fudge a
		// non-local return, chuck it out to the interpreter.
		if ([[localException name] isEqualToString: LKSmalltalkBlockNonLocalReturnException])
		{
			[localException raise];
		}
		if ([[localException name] isEqualToString: exceptionName])
		{
			return [handler value: localException];
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
