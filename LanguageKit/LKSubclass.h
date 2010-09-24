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
  NSMutableArray * cvars;
  /** Array of instance variables defined for this class. */
  NSMutableArray * ivars;
}
/**
 * Return a new Subclass with the specified name, superclass and list of
 * instance variables and methods.  The instance and class variable lists
 * should be strings and the method list is an array of AST nodes representing
 * methods.
 */
+ (id) subclassWithName:(NSString*)aName
        superclassNamed:(NSString*)aClass
                  cvars:(NSArray*)anIvarList
                  ivars:(NSArray*)anIvarList
                methods:(NSArray*)aMethodList;
/**
 * Returns the class name for the represented class.
 */
- (NSString*) classname;
/**
 * Returns the superclass name for the class represented by this AST node.
 */
- (NSString*) superclassname;
/**
 * Returns the methods
 */
- (NSMutableArray*)methods;
/**
 * Returns the class variables
 */
- (NSMutableArray*)cvars;
/**
 * Returns the instance variables
 */
- (NSMutableArray*)ivars;
/**
 * Adds an instance variable to this class definition. 
 */
- (void)addInstanceVariable: (NSString*)aName;
/**
 * Adds a class variable to this class definition. 
 */
- (void)addClassVariable: (NSString*)aName;
@end
