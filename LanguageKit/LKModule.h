#import "LKAST.h"

/**
 * AST node representing a module - a set of classes and categories compiled
 * together.
 */
@interface LKCompilationUnit : LKAST {
  /** Classes defined in this module. */
  NSMutableArray * classes;
  /** Categories defined in this module. */
  NSMutableArray * categories;
  /** Current pragmas */
  NSMutableDictionary * pragmas;
  /** Manually-specified method types. */
  NSMutableDictionary *typeOverrides;
}
/**
 * Add compile-time pragmas.
 */
- (void) addPragmas: (NSDictionary*)aDict;
/**
 * Add a new class to this module.
 */
- (void) addClass:(LKAST*)aClass;
/**
 * Add a new category to this module.
 */
- (void) addCategory:(LKAST*)aCategory;
/**
 * Returns the type that should be used for a given selector.
 */
- (const char*) typeForMethod:(NSString*)methodName;
@end
