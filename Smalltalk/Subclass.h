#import "AST.h"

/**
 * AST node representing a new class definition.
 */
@interface Subclass : AST {
  /** Name of this class. */
  NSString * classname;
  /** Name of the superclass. */
  NSString * superclass;
  /** Array of methods defined in this class. */
  NSMutableArray * methods;
  /** Array of instance variables defined for this class. */
  NSArray * ivars;
}
/**
 * Initialise a new Subclass with the specified name, superclass and list of
 * instance variables and methods.
 */
- (id) initWithName:(NSString*)aName superclass:(NSString*)aClass ivars:(NSArray*)anIvarList methods:(NSArray*)aMethodList;
@end
