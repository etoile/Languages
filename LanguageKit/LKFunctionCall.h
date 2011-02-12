#import "LKAST.h"

/**
 * AST node representing a function call operation.
 */
@interface LKFunctionCall : LKAST 
{
	/* The name of the function to call. */
	NSString *functionName;
	/** Array of AST nodes which evaluate to the message arguments. */
	NSMutableArray *arguments;
}
/**
 * Return a new function call with the specified arguments.
 *
 * Retains the args array, does not copy it.
 */
+ (id)functionCallWithFunction: (NSString*)function
                     arguments: (NSMutableArray*)args;
/**
 * Return all of the arguments of this call.
 */
- (NSArray*)arguments;
/**
 * Return the target
 */
- (NSString*)target;
@end
