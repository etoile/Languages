#import <EtoileFoundation/EtoileFoundation.h>
#include <setjmp.h>

@interface BlockContext : NSObject {
	int count;
	id objects[0];
}
@end

@interface BlockClosure : NSObject {
@public
  IMP function;
@protected
  /**
   * Variables that have are external to the block.
   */
  id *unbound[5];
  /**
   * Variables which have been promoted to this block.
   */
  id bound[5];
  /**
   * Number of arguments.  Used for checking when calling -value.
   */
  int32_t args;
  /**
   * Return value for explicit block returns.
   */
  id retVal;
  /**
   * Jump buffer used for non-local return.  Non-local returns are implemented
   * by a longjmp to this location.  Replacing this buffer will change the
   * return destination for explicit returns (^ in the block).
   */
  jmp_buf nonLocalReturn;
}
- (id) value;
- (id) value:(id)a1;
- (id) value:(id)a1 value:(id)a2;
- (id) value:(id)a1 value:(id)a2 value:(id)a3;
- (id) value:(id)a1 value:(id)a2 value:(id)a3 value:(id)a4;
- (id) on:(NSString*)exceptionName do:(BlockClosure*)handler;
@end

/**
 * Get a block directly, without message passing or allocation overhead.  Uses
 * a per-thread storage pool.
 */
BlockClosure *NewBlock();
void FreeBlock(BlockClosure* aBlock);
