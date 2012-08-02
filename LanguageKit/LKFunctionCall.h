#import "LKAST.h"

/**
 * AST node representing a call to a C function.
 */
@interface LKFunctionCall : LKAST
/**
 * The name of the called function.
 */
@property (nonatomic, strong) NSString *functionName;
/**
 * The type encoding of the function.
 */
@property (nonatomic, strong) NSString *typeEncoding;
/**
 * The function arguments.
 */
@property (nonatomic, strong) NSMutableArray *arguments;
@end

