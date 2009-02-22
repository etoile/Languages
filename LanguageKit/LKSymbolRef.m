#import "LKSymbolRef.h"

@implementation LKSymbolRef 
- (void) check {}
- (NSString*) description
{
	return [NSString stringWithFormat:@"#%@", symbol];
}
- (void*) compileWithGenerator: (id<LKCodeGenerator>)aGenerator
{
	return [aGenerator generateConstantSymbol:symbol];
}
@end
