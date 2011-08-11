#import "LKAST.h"

/**
 * AST node representing a call to a C function.
 */
@interface LKFunctionCall : LKAST
/**
 * The name of the called function.
 */
@property (nonatomic, retain) NSString *functionName;
/**
 * The type encoding of the function.
 */
@property (nonatomic, retain) NSString *typeEncoding;
/**
 * The function arguments.
 */
@property (nonatomic, retain) NSMutableArray *arguments;
@end

