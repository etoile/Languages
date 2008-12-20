#import <EtoileFoundation/EtoileFoundation.h>
#include <setjmp.h>

@class BlockContext;

@interface BlockClosure : NSObject {
@public
  IMP function;
@protected
	/**
	 * Number of arguments.  Used for checking when calling -value.
	 */
	int32_t args;
	/** The context for this block. */
	BlockContext *context;
}
- (int32_t) argumentCount;
- (id) value;
- (id) value:(id)a1;
- (id) value:(id)a1 value:(id)a2;
- (id) value:(id)a1 value:(id)a2 value:(id)a3;
- (id) value:(id)a1 value:(id)a2 value:(id)a3 value:(id)a4;
- (id) onException:(NSString*)exceptionName do:(BlockClosure*)handler;
@end
