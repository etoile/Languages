#include <EtoileFoundation/EtoileFoundation.h>
#import "SymbolTable.h"
#import "Parser.h"
#import "LLVMCodeGen.h"

@interface A: NSObject {
id anIvar;
}
@end
@implementation A
- (id) getFoo
{
	return [NSNumber new];
}
- (void) log
{
	NSLog(@"Log called");
}
@end

@implementation NSObject (log)
- (void) log
{
  NSLog(@"%@", [self description]);
}
@end
@interface TestBlock : NSObject{
}
- (id) run:(id)aBlock;
@end
@implementation TestBlock
- (id) run:(id)aBlock
{
  NSLog(@"Block %@", aBlock);
  return [aBlock value];
}
@end

int main(void)
{
	[NSAutoreleasePool new];
	Parser * p = [[Parser alloc] init];
	//id testProg = @"NSObject subclass: TestObject [ addObject:a [ | a b c| \na := NSMutableArray array. a addObject:[ :d | d addObject:b. ]. ] ]";
//	id testProg = @"A subclass: TestObject [ test [ | a b c| \n anIvar := self getFoo. self log. a + c. ^a. ] ]";
	id testProg = [NSString stringWithContentsOfFile:@"test.st"];
	[testProg print];
	printf("\n\nParsing generates:\n\n");
	AST *ast = [p parseString: testProg];
	[ast print];
	id cg = [LLVMCodeGen new];
	NSLog(@"Compiling %@", ast);
	[ast compileWith:cg];
	
	/*
	[cg startModule];
	void *n = NULL;
	[cg createSubclass:@"TestObject" subclassing:@"A" withIvarNames:&n types:&n offsets:&n];
	[cg endClass];

	[cg endModule];
	*/
	NSLog(@"TestObject class: %@", NSClassFromString(@"TestObject"));
	[[NSClassFromString(@"TestObject") new] addObject:[TestBlock new]];

	return 0;
}
