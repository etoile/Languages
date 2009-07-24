#import "LKAST.h"
#import "LKCompiler.h"

@class LKSubclass;
@class LKCategory;

/**
 * AST node representing a module - a set of classes and categories compiled
 * together.
 */
@interface LKModule : LKAST
{
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
 * Return a new autoreleased module.
 */
+ (id) module;
/**
 * Add compile-time pragmas.
 */
- (void) addPragmas: (NSDictionary*)aDict;
/**
 * Add a new class to this module.
 */
- (void) addClass: (LKSubclass*)aClass;
/**
 * Add a new category to this module.
 */
- (void) addCategory: (LKCategory*)aCategory;
/**
 * Returns the type that should be used for a given selector.
 */
- (const char*) typeForMethod:(NSString*)methodName;
/**
 * Returns the classes in this module
 */
- (NSArray*) allClasses;
/**
 * Returns the categories in this module
 */
- (NSArray*) allCategories;
/**
 * Returns the pragmas in this module
 */
- (NSDictionary*) pragmas;
@end

/**
 * Notification posted when new classes have been compiled.
 */
extern NSString *LKCompilerDidCompileNewClassesNotification;
