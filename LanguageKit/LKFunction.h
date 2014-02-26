#import <LanguageKit/LKAST.h>
#import <LanguageKit/LKFunctionCall.h>

/**
 * AST node representing a function.
 */
@interface LKFunction : LKAST 
/** Function signature - selector, type encoding, and names of arguments. */
@property (retain, nonatomic) LKFunctionCall *signature;
/** List of statements in this function. */
@property (retain, nonatomic) NSMutableArray *statements;
/**
 * Return a new Function with the specified signature, locals and statements.
 */
+ (id) functionWithSignature: (LKFunctionCall*)aSignature
                      locals: (NSMutableArray*)locals
                  statements: (NSMutableArray*)statementList;
/**
 * Initialise a new Function with the specified signature, locals and statements.
 */
- (id) initWithSignature: (LKFunctionCall*)aSignature
                  locals: (NSMutableArray*)localss
              statements: (NSMutableArray*)statementList;
/**
 * Set the function signature for this function.
 */
- (void) setSignature:(LKFunctionCall*)aSignature;
/**
 * Returns the function's body
 */
- (NSString*) functionBody;
@end

