#import <EtoileFoundation/EtoileFoundation.h>
#import <EtoileFoundation/ETTranscript.h>
#include <time.h>
#undef ETLog
@class ETTranscript;

const int FibRuns = 100;
const int MessageRuns = 1000000;
int FibValue = 30;


@interface Fibonacci : NSObject
- (id)fib: (id)i;
- (id)makePrototype;
- (id)block;
@end

@interface ObjCObject : NSObject {}
- (id) value:(id)aValue;
- (int) fibonacci:(int) i;
@end
@implementation ObjCObject
- (id) value:(id)aValue
{
	return aValue;
}
- (int) fibonacci:(int) i
{
	switch (i)
	{
		case 0:
		case 1:
			return 1;
		default:
			return [self fibonacci:i-1] + [self fibonacci:i-2];
	}
}
@end
int fibonacci(int i)
{
	switch (i)
	{
		case 0:
		case 1:
			return 1;
		default:
			return fibonacci(i-1) + fibonacci(i-2);
	}
}

double timeFibonacciC(void)
{
	clock_t c1 = clock();
	int result;
	for (unsigned i=0 ; i<FibRuns ; i++)
	{
		result = fibonacci(FibValue);
	}
	clock_t c2 = clock();
	return ((double)c2 - (double)c1) / (double)CLOCKS_PER_SEC;
}
double timeFibonacci(id object)
{
	clock_t c1, c2;
	@autoreleasepool {
	c1 = clock();
	int result;
	for (unsigned i=0 ; i<FibRuns ; i++)
	{
		result = [object fibonacci: FibValue];
	}
	c2 = clock();
	}
	return ((double)c2 - (double)c1) / (double)CLOCKS_PER_SEC;
}
double timeFibonacciSmalltalk(id object)
{
	clock_t c1, c2;
	@autoreleasepool {
	id num = [NSNumber numberWithInt: FibValue];
	c1 = clock();
	for (unsigned i=0 ; i<FibRuns ; i++)
	{
		[object fib: num];
	}
	c2 = clock();
	}
	return ((double)c2 - (double)c1) / (double)CLOCKS_PER_SEC;
}
double timeMessageSend(id object)
{
	clock_t c1 = clock();
	for (unsigned i=0 ; i<MessageRuns ; i++)
	{
		[object value:@"a string"];
	}
	clock_t c2 = clock();
	return ((double)c2 - (double)c1) / (double)CLOCKS_PER_SEC;
}


void ETLog(id string, float f)
{
	[ETTranscript show:[NSString stringWithFormat:string, f]];
	[ETTranscript cr];
}

int main(int argc, char**argv)
{
	id pool = [NSAutoreleasePool new];
	NSDictionary *opts = ETGetOptionsDictionary("vr:p", argc, argv);
	double sttime2, sttime, octime, ctime;
	if ([[opts objectForKey: @"p"] boolValue])
	{
		id proto = [[NSClassFromString(@"SmalltalkPrototype") new] makePrototype];
		sttime = timeMessageSend(proto);
		ETLog(@"Smalltalk prototype execution took %f seconds.  ", sttime);
		proto = [ObjCObject new];
		octime = timeMessageSend(proto);
		ETLog(@"ObjC execution took %f seconds.  ", octime);
		ETLog(@"Ratio: %f", sttime / octime);
		sttime = timeMessageSend([NSClassFromString(@"SmalltalkPrototype") new]);
		ETLog(@"Smalltalk method execution took %f seconds.  ", sttime);
		ETLog(@"Ratio: %f", sttime / octime);
		sttime = timeMessageSend([[NSClassFromString(@"SmalltalkPrototype") new] block]);
		ETLog(@"Smalltalk block execution took %f seconds.  ", sttime);
		ETLog(@"Ratio: %f", sttime / octime);
	}

	int repeats = 1;
	id r = [opts objectForKey: @"r"];
	if (r) repeats = [r intValue];
	for (unsigned int i=0 ; i<repeats ; i++)
	{
		@autoreleasepool
		{
			ctime = timeFibonacciC();
			id objc = [ObjCObject new];
			octime = timeFibonacci(objc);
			sttime = timeFibonacci([NSClassFromString(@"SmalltalkFibonacci") new]);
			sttime2 = timeFibonacciSmalltalk([NSClassFromString(@"SmalltalkFibonacci") new]);
			if ([[opts objectForKey: @"v"] boolValue])
			{
				ETLog(@"C fibonacci execution took %f seconds.  ", ctime);
				ETLog(@"ObjC fibonacci execution took %f seconds.  ", octime);
				ETLog(@"Ratio: %f", octime / ctime);
				ETLog(@"Smalltalk fibonacci execution took %f seconds.  ", sttime);
				ETLog(@"Ratio: %f", sttime / octime);
				ETLog(@"Smalltalk fibonacci SmallInt version execution took %f seconds.  ", sttime);
				ETLog(@"Ratio: %f", sttime / octime);
			}
			printf("%f	%f	%f	%f\n", ctime, octime, sttime, sttime2); 
		}
	}
	[pool release];
	return 0;
}
