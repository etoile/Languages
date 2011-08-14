#import <Foundation/Foundation.h>

extern NSString *LKSmalltalkBlockNonLocalReturnException;

@interface _NSBlock : NSObject
- (id)value;
@end

@implementation _NSBlock (ExceptionHandling)
- (id) onException: (NSString*) exceptionName do: (id(^)(id)) handler
{
	@try 
	{
		return [self value];
	}
	@catch (NSException *localException)
	{
		// If this is an exception that the interpreter is using to fudge a
		// non-local return, chuck it out to the interpreter.
		if ([[localException name] isEqualToString: LKSmalltalkBlockNonLocalReturnException])
		{
			[localException raise];
		}
		if ((exceptionName == nil) || [[localException name] isEqualToString: exceptionName])
		{
			return handler(localException);
		}
		else
		{
			[localException raise];
		}
	}
	// Not reached
	return nil;
}
@end
