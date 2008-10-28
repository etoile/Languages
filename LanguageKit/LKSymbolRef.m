#import "LKSymbolRef.h"

@implementation LKSymbolRef 
- (void) check {}
- (NSString*) description
{
	return [NSString stringWithFormat:@"#%@", symbol];
}
- (void*) compileWith:(id<LKCodeGenerator>)aGenerator
{
	return [aGenerator generateConstantSymbol:symbol];
}
@end
