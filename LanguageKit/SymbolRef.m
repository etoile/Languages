#import "SymbolRef.h"

@implementation SymbolRef 
- (void) check {}
- (NSString*) description
{
	return [NSString stringWithFormat:@"#%@", symbol];
}
- (void*) compileWith:(id<CodeGenerator>)aGenerator
{
	return [aGenerator generateConstantSymbol:symbol];
}
@end
