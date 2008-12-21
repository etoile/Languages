#import "LKVariableDecl.h"
#import "LKToken.h"
#import <EtoileFoundation/Macros.h>

@implementation LKVariableDecl
- (LKVariableDecl*) initWithName: (LKToken*) declName
{
	SELFINIT;
	ASSIGN(variableName, declName);
	return self;
}
+ (LKVariableDecl*) variableDeclWithName:(LKToken*) declName
{
	return [[[self alloc] initWithName:declName] autorelease];
}
- (void) setParent:(LKAST*)aParent
{
	[super setParent:aParent];
	[symbols addSymbol:variableName];
}
- (void) check {}
- (void*) compileWith:(id<LKCodeGenerator>)aGenerator
{
	return NULL;
}
@end
