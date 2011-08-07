#import "LKVariableDecl.h"
#import "LKToken.h"
#import "Runtime/LKObject.h"
#import <EtoileFoundation/Macros.h>

@implementation LKVariableDecl
- (LKVariableDecl*) initWithName: (LKToken*) declName
{
	SUPERINIT;
	ASSIGN(variableName, declName);
	return self;
}
+ (LKVariableDecl*) variableDeclWithName:(LKToken*) declName
{
	return [[[self alloc] initWithName:declName] autorelease];
}
- (NSString*) description
{
	return [NSString stringWithFormat:@"var %@", variableName];
}
- (void) setParent:(LKAST*)aParent
{
	[super setParent:aParent];
	LKSymbol *sym = [LKSymbol new];
	[sym setName: variableName];
	[sym setTypeEncoding: NSStringFromRuntimeString(@encode(LKObject))];
	[sym setScope: LKSymbolScopeLocal];
	[symbols addSymbol: sym];
	[sym release];
}
- (NSString*)name
{
	return (NSString*)variableName;
}
- (BOOL) check { return YES; }
- (void*) compileWithGenerator: (id<LKCodeGenerator>)aGenerator
{
	return NULL;
}
@end
