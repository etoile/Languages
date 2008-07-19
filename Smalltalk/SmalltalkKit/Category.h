#import "AST.h"

/**
 * AST node representing a new category definition.
 */
@interface CategoryDef : AST {
  /** Name of this class. */
  NSString * classname;
  /** Array of methods defined in this category. */
  NSMutableArray * methods;
}
/**
 * Initialise a new Category with the specified name, class and list of
 * methods.
 */
- (id) initWithClass:(NSString*)aName methods:(NSArray*)aMethodList;
@end
