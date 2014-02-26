#import <LanguageKit/LKAST.h>
#import <LanguageKit/LKMessageSend.h>

/**
 * AST node representing a method.
 */
@interface LKMethod : LKAST 
/** Method signature - selector and names of arguments. */
@property (strong, nonatomic) LKMessageSend *signature;
/** List of statements in this method. */
@property (strong, nonatomic) NSMutableArray *statements;
/**
 * Return a new Method with the specified signature, locals and statements.
 */
+ (id) methodWithSignature: (LKMessageSend*)aSignature
                    locals: (NSMutableArray*)locals
                statements: (NSMutableArray*)statementList;
/**
 * Initialise a new Method with the specified signature, locals and statements.
 */
- (id) initWithSignature: (LKMessageSend*)aSignature
                  locals: (NSMutableArray*)localss
              statements: (NSMutableArray*)statementList;
/**
 * Set the method signature for this method.
 */
- (void) setSignature:(LKMessageSend*)aSignature;
/**
 * Returns the method's body
 */
- (NSString*) methodBody;
/**
 * Returns YES if this is a class method.
 */
- (BOOL)isClassMethod;
@end
@interface LKInstanceMethod : LKMethod {}
@end
@interface LKClassMethod : LKMethod {}
@end
/**
 * A freestanding method is a method that is not attached to any class.  It is
 * used in REPL mode, and might be useful for better support for prototypes.
 */
@interface LKFreestandingMethod : LKMethod {}
@end
