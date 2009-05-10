#import <Foundation/Foundation.h>
#include "runtime.h"

id obj;

void foo(void)
{
	@synchronized(obj)
	{
		printf("foo\n");
		@throw ([NSObject new]);
	}
}
int main(void)
{
	obj = [NSObject new];
	@try {
			foo();
	}
	@catch(NSObject *a) {
		printf("Caught!\n");
	}
	@finally { printf("Not broken!"); }
	[obj dealloc];
	return 0;
}
