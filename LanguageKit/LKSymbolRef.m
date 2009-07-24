#import "LKSymbolRef.h"

@implementation LKSymbolRef 
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
