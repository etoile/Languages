#import <LanguageKit/LKAST.h>

/**
 * Generic superclass for AST visitors.  Visitors subclassing this implement
 * methods of the form:
 *
 * - (LKAST) visitComment:(LKComment*)aNode;
 *
 * Where Comment can be the suffix of any AST node subclass
 */
@interface LKASTVisitor : NSObject<LKASTVisitor>
@end
