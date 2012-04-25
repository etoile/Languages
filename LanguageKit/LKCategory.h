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

@property (readonly, nonatomic) NSString *className;
@property (readonly, nonatomic) NSString *categoryName;
@property (readonly, nonatomic) NSMutableArray *methods;

/**
 * Return a new Category with the specified name, class and list of
 * methods.  The third argument is an array of AST nodes representing methods.
 */
+ (id) categoryWithName:(NSString*)aName
           onClassNamed:(NSString*)aClass
                methods:(NSArray*)aMethodList;
/**
 * Returns a new anonymous category.
 */
+ (id) categoryOnClassNamed:(NSString*)aName methods:(NSArray*)aMethodList;

/**
 * Returns the methods.
 */
- (NSMutableArray*) methods;
@end
