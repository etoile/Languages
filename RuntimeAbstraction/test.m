#import <Foundation/Foundation.h>
//#include "runtime.h"

void foo(void)
{
	@throw ([NSObject new]);
}
int main(void)
{
	id obj = [NSObject new];
	@try {
		//@synchronized(obj)
		{
			printf("foo\n");
			foo();
		}
	}
	@catch(id a) {
		printf("Caught!\n");
	}
	[obj dealloc];
	return 0;
}
