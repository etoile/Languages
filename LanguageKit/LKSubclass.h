#import "LKAST.h"

/**
 * AST node representing a new class definition.
 */
@interface LKSubclass : LKAST {
  /** Name of this class. */
  NSString * classname;
  /** Name of the superclass. */
  NSString * superclass;
  /** Array of methods defined in this class. */
  NSMutableArray * methods;
  /** Array of class variables defined for this class. */
  NSArray * cvars;
  /** Array of instance variables defined for this class. */
  NSArray * ivars;
}
/**
 * Return a new Subclass with the specified name, superclass and list of
 * instance variables and methods.
 */
+ (id) subclassWithName:(NSString*)aName
             superclass:(NSString*)aClass
                  cvars:(NSArray*)anIvarList
                  ivars:(NSArray*)anIvarList
                methods:(NSArray*)aMethodList;
/**
 * Returns the class name
 */
- (NSString*) classname;
/**
 * Returns the superclass name
 */
- (NSString*) superclass;
/**
 * Returns the methods
 */
- (NSMutableArray*) methods;
/**
 * Returns the class variables
 */
- (NSArray*) cvars;
/**
 * Returns the instance variables
 */
- (NSArray*) ivars;
@end
