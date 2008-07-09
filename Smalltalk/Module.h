#import "AST.h"

/**
 * AST node representing a module - a set of classes and categories compiled
 * together.
 */
@interface CompilationUnit : AST {
  /** Classes defined in this module. */
  NSMutableArray * classes;
  /** Categories defined in this module. */
  NSMutableArray * categories;
}
/**
 * Add a new class to this module.
 */
- (void) addClass:(AST*)aClass;
/**
 * Add a new category to this module.
 */
- (void) addCategory:(AST*)aCategory;
@end
