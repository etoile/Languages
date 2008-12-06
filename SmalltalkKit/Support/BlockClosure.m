#import <Foundation/Foundation.h>
#import "BlockClosure.h"

@implementation BlockClosure
- (id) value
{
	if (args > 0)
	{
		[NSException raise:@"InvalidBlockValueCall" format:@"Block expects %d arguments", args];
	}
	return function(self, _cmd);
}
- (id) value:(id)a1
{
	if (args != 1)
	{
		[NSException raise:@"InvalidBlockValueCall" format:@"Block expects %d arguments", args];
	}
	return function(self, _cmd, a1);
}
- (id) value:(id)a1 value:(id)a2;
{
	if (args != 2)
	{
		[NSException raise:@"InvalidBlockValueCall" format:@"Block expects %d arguments", args];
	}
	return function(self, _cmd, a1, a2);
}
- (id) value:(id)a1 value:(id)a2 value:(id)a3;
{
	if (args != 3)
	{
		[NSException raise:@"InvalidBlockValueCall" format:@"Block expects %d arguments", args];
	}
	return function(self, _cmd, a1, a2, a3);
}
- (id) value:(id)a1 value:(id)a2 value:(id)a3 value:(id)a4;
{
	if (args != 4)
	{
		[NSException raise:@"InvalidBlockValueCall" format:@"Block expects %d arguments", args];
	}
	return function(self, _cmd, a1, a2, a3, a4);
}
- (id) whileTrue:(id)anotherBlock
{
	if (args > 0)
	{
		[NSException raise:@"InvalidBlockValueCall" format:@"Block expects %d arguments", args];
	}
	id last = nil;
	while(nil != function(self, _cmd))
	{
		last = [anotherBlock value];
	}
	return last;
}

- (id) on: (NSString*) exceptionName do: (BlockClosure*) handler
{
  NS_DURING
    NS_VALUERETURN([self value], id);
  NS_HANDLER
    if ([[localException name] isEqualToString: exceptionName])
      {
	return [handler value: localException];
      }
    else
      {
	[localException raise];
	return nil; // won't happen.
      }
  NS_ENDHANDLER
}

@end
