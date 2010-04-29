#import <Foundation/Foundation.h>
#include <time.h>
@class ETTranscript;

const int FibRuns = 1;
const int FibValue = 47;

@interface Fibonacci : NSObject
- (void)runNative: (int)i;
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
	NSLog(@"%d", result);
	clock_t c2 = clock();
	return ((double)c2 - (double)c1) / (double)CLOCKS_PER_SEC;
}
double timeFibonacci(id object)
{
	clock_t c1 = clock();
	int result;
	for (unsigned i=0 ; i<FibRuns ; i++)
	{
		result = [object fibonacci: FibValue];
	}
	NSLog(@"%d", result);
	clock_t c2 = clock();
	return ((double)c2 - (double)c1) / (double)CLOCKS_PER_SEC;
}
double timeFibonacciSmalltalk(id object)
{
	clock_t c1 = clock();
	for (unsigned i=0 ; i<FibRuns ; i++)
	{
		[object runNative: FibValue];
	}
	clock_t c2 = clock();
	return ((double)c2 - (double)c1) / (double)CLOCKS_PER_SEC;
}
double timeMessageSend(id object)
{
	clock_t c1 = clock();
	for (unsigned i=0 ; i<1000000 ; i++)
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

int main(void)
{
	id pool = [NSAutoreleasePool new];
	id proto = [[NSClassFromString(@"SmalltalkPrototype") new] makePrototype];
	double sttime = timeMessageSend(proto);
	ETLog(@"Smalltalk prototype execution took %f seconds.  ", sttime);
	proto = [ObjCObject new];
	double octime = timeMessageSend(proto);
	ETLog(@"ObjC execution took %f seconds.  ", octime);
	ETLog(@"Ratio: %f", sttime / octime);
	sttime = timeMessageSend([NSClassFromString(@"SmalltalkPrototype") new]);
	ETLog(@"Smalltalk method execution took %f seconds.  ", sttime);
	ETLog(@"Ratio: %f", sttime / octime);
	sttime = timeMessageSend([[NSClassFromString(@"SmalltalkPrototype") new] block]);
	ETLog(@"Smalltalk block execution took %f seconds.  ", sttime);
	ETLog(@"Ratio: %f", sttime / octime);

	double ctime = timeFibonacciC();
	ETLog(@"C fibonacci execution took %f seconds.  ", ctime);
	proto = [ObjCObject new];
	octime = timeFibonacci(proto);
	ETLog(@"ObjC fibonacci execution took %f seconds.  ", octime);
	ETLog(@"Ratio: %f", octime / ctime);
	sttime = timeFibonacci([NSClassFromString(@"SmalltalkFibonacci") new]);
	ETLog(@"Smalltalk fibonacci execution took %f seconds.  ", sttime);
	ETLog(@"Ratio: %f", sttime / octime);
	sttime = timeFibonacciSmalltalk([NSClassFromString(@"SmalltalkFibonacci") new]);
	ETLog(@"Smalltalk fibonacci SmallInt version execution took %f seconds.  ", sttime);
	ETLog(@"Ratio: %f", sttime / octime);
	[pool release];
	return 0;
}
