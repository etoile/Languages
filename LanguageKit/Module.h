#import "AST.h"

/**
 * AST node representing a module - a set of classes and categories compiled
 * together.
 */
@interface LKCompilationUnit : LKAST {
  /** Classes defined in this module. */
  NSMutableArray * classes;
  /** Categories defined in this module. */
  NSMutableArray * categories;
}
/**
 * Add a new class to this module.
 */
- (void) addClass:(LKAST*)aClass;
/**
 * Add a new category to this module.
 */
- (void) addCategory:(LKAST*)aCategory;
@end
