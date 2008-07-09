#import <EtoileFoundation/EtoileFoundation.h>

/**
 * Skeleton for an interpreter context.  Not yet used.  Might not ever be used.
 * Go away and stop looking at this code (or implement an interpreter).
 */
@interface ExecutionContext : NSObject {
	SymbolTable * symbols;
  id * args;
  NSMapTable * locals;
}
- (id) initWithArgs:(id*)argv;
- (id) getArg:(int)argIndex;
- (void) setArg:(id)arg atIndex:(int)argIndex;
@end
