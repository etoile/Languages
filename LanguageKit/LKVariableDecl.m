#import "LKVariableDecl.h"
#import "LKToken.h"
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
	[symbols addSymbolsNamed: variableName];
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
