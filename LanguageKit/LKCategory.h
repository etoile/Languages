#import "LKAST.h"

/**
 * AST node representing a new category definition.
 */
@interface LKCategoryDef : LKAST {
  /** Name of this class. */
  NSString * classname;
  /** Name of the category. */
  NSString *categoryName;
  /** Array of methods defined in this category. */
  NSMutableArray * methods;
}
/**
 * Return a new Category with the specified name, class and list of
 * methods.
 */
+ (id) categoryWithName:(NSString*)aName 
                  class:(NSString*)aClass 
                methods:(NSArray*)aMethodList;
/**
 * Returns a new anonymous category.
 */
+ (id) categoryWithClass:(NSString*)aName methods:(NSArray*)aMethodList;
@end
