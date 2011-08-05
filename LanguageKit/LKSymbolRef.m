#import "LKSymbolRef.h"

@implementation LKSymbolRef 
- (id) initWithSymbol:(NSString*)sym
{
	SUPERINIT;
	ASSIGN(symbol, sym);
	return self;
}
+ (id) referenceWithSymbol:(NSString*)sym
{
	return [[[self alloc] initWithSymbol: sym] autorelease];
}
- (void)dealloc
{
	[symbol release];
	[super dealloc];
}
- (BOOL) check { return YES; }
- (NSString*) description
{
	return [NSString stringWithFormat:@"#%@", symbol];
}
- (void*) compileWithGenerator: (id<LKCodeGenerator>)aGenerator
{
	return [aGenerator generateConstantSymbol:symbol];
}
@end
