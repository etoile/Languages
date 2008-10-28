#import "LKAST.h"

/**
 * AST node representing a new category definition.
 */
@interface LKCategoryDef : LKAST {
  /** Name of this class. */
  NSString * classname;
  /** Array of methods defined in this category. */
  NSMutableArray * methods;
}
/**
 * Return a new Category with the specified name, class and list of
 * methods.
 */
+ (id) categoryWithClass:(NSString*)aName methods:(NSArray*)aMethodList;
/**
 * Initialise a new Category with the specified name, class and list of
 * methods.
 */
- (id) initWithClass:(NSString*)aName methods:(NSArray*)aMethodList;
@end
