#import <EtoileFoundation/EtoileFoundation.h>

// HACK: Foundation.h on OS X defines true and false, which conflicts with the
// method branchOnCondition:true:false: in this class
#undef true
#undef false

/**
 * Code generator protocol.  Each AST node calls methods in a class conforming
 * to this protocol.  Methods which return a value return a pointer to a
 * generator-specific type, which can then be passed in to the generator later
 * when it expects a value.
 */
@protocol LKCodeGenerator <NSObject>
/**
 * Begin generating a module.  A module is a set of classes and categories
 * which are compiled and optimised at once.
 */
- (void) startModule: (NSString*)fileName;
/**
 * Finish generating a module.
 */
- (void) endModule;
/**
 * Create a new class, with the specified superclass and instance variables.
 * The types should be Objective-C type encoding strings.
 */
- (void) createSubclassWithName:(NSString*)aClass
                superclassNamed:(NSString*)aSuperclass
                  withCvarNames:(const char**)cVarNames 
                          types:(const char**)cVarTypes
                  withIvarNames:(const char**)iVarNames 
                          types:(const char**)iVarTypes
                        offsets:(int*)offsets;
/**
 * Finish the current class. 
 */
- (void) endClass;
/**
 * Create a new category with the specified name on the named class.
 */
- (void) createCategoryWithName:(NSString*)aCategory
                   onClassNamed:(NSString*)aClass;
/**
 * Finish the current category. 
 */
- (void) endCategory;
/**
 * Begin a class method with the specified type encoding and number of local
 * variables.  Local variables and arguments are indexed by number, the symbol
 * table information is just for debugging.
 */
- (void) beginClassMethod:(const char*) aName
                withTypes:(const char*)types
                   locals: (const char**)locals
                    count:(unsigned int)localsCount;
/**
 * Begin an instance method with the specified type encoding and number of local
 * variables.  Local variables and arguments are indexed by number, the symbol
 * table information is just for debugging.
 */
- (void) beginInstanceMethod: (const char*) aName
                   withTypes: (const char*)types
                      locals: (const char**)locals
                       count: (unsigned int)localsCount;
/**
 * Sends a message to a receiver which may be a SmallInt (a boxed Smalltalk
 * integer contained within an object pointer).
 */
- (void*) sendMessage:(const char*)aMessage
                types:(const char*)types
                   to:(void*)receiver
             withArgs:(void**)argv
                count:(unsigned int)argc;
/**
 * Sends a message to the superclass.
 */
- (void*) sendSuperMessage:(const char*)sel
                     types:(const char*)seltypes
                  withArgs:(void**)argv
                     count:(unsigned int)argc;
/**
 * Sends a message to an object.  Similar to
 * sendMessage:type:to:withargs:count: but requires that receiver be an
 * Objective-C object, not a small integer.
 */
- (void*) sendMessage:(const char*)aMessage
                types:(const char*)types
             toObject:(void*)receiver
             withArgs:(void**)argv
                count:(unsigned int)argc;
/**
 * Store the specified value in the named class variable.
 */
- (void) storeValue:(void*)rval 
    inClassVariable:(NSString*) aClassVar;
/**
 * Stores a value in a local variable somewhere up the stack.
 */
- (void) storeValue:(void*)aVal 
     inLocalAtIndex:(unsigned int)index
lexicalScopeAtDepth:(unsigned int) scope;
/**
 * Load a local value from up the stack.
 */
- (void*) loadLocalAtIndex:(unsigned int)index
	   lexicalScopeAtDepth:(unsigned int) scope;
/**
 * Load an argument from up the stack.
 */
- (void*) loadArgumentAtIndex:(unsigned int) index
		  lexicalScopeAtDepth:(unsigned int) scope;
- (void*) loadClassVariable:(NSString*) aSymbol;
/**
 * Stores a value at a specific offset from an object.  Used for instance
 * variables.  The type is an Objective-C type encoding.
 */
- (void) storeValue: (void*)aValue
             inIvar: (NSString*)anIvar
             ofType: (NSString*)aType
           atOffset: (unsigned)anOffset
         fromObject: (void*)anObject
            ofClass: (NSString*)className;
/**
 * Loads a value at a specific offset from an object.  Used for instance
 * variables. The type is an Objective-C type encoding.  
 */
- (void*) loadValueOfType: (NSString*)aType
                 fromIvar: (NSString*)anIvar
                 atOffset: (unsigned)anOffset
               fromObject: (void*)anObject
                 ofClass: (NSString*)className;
/**
 * Begin generating a block expression with the specified number of arguments
 * and locals.  The bound variables are pointers to variables declared outside
 * the block's scope.
 */
- (void) beginBlockWithArgs:(unsigned int)args
					 locals:(unsigned int)locals;
/**
 * Returns 'self' in the current method.
 */
- (void*) loadSelf;
/**
 * Load a pointer to the named class.
 */
- (void*) loadClassNamed:(NSString*)aClass;
/**
 * Stores a value in the specified local.
 */
- (void) storeValue:(void*)aVal inLocalAtIndex:(unsigned int)index;
/**
 * Load the value of the specified local variable.
 */
- (void*) loadLocalAtIndex:(unsigned int)index;
/**
 * Load the value of the specified argument.
 */
- (void*) loadArgumentAtIndex:(unsigned int)index;
/**
 * End the current method.
 */
- (void) endMethod;
/**
 * End the current block.  Subsequent calls will insert instructions into the
 * containing scope (the method, or another block).
 */
- (void*) endBlock;
/**
 * Specify the return value for a block.  For Smalltalk, this is the result of
 * the last statement in a block.  
 */
- (void) blockReturn:(void*)aValue;
/**
 * Set the return value for a method.
 */
- (void) setReturn:(void*)aValue;
/**
 * Returns a constant representing the string as an integer (either a SmallInt or a BigInt).
 */
- (void*) intConstant:(NSString*)aString;
/**
 * Return a constant representing the passed string.
 */
- (void*) floatConstant:(NSString*)aString;
/**
 * Return a constant representing the passed string.
 */
- (void*) stringConstant:(NSString*)aString;
/**
 * Returns nil as a constant.
 */
- (void*) nilConstant;
/**
 * Compares two pointers.
 */
- (void*) comparePointer:(void*)lhs to:(void*)rhs;
/**
 * Generates a constant symbol (a boxed selector).
 */
- (void*) generateConstantSymbol:(NSString*)aSymbol;
/**
 * Starts a new basic block and returns a pointer to the basic block.
 */
- (void*) startBasicBlock:(NSString*)aName;
/**
 * Returns a pointer to the current basic block.
 */
- (void*) currentBasicBlock;
/**
 * Sets the current insert point to the specified basic block.  
 */
- (void) moveInsertPointToBasicBlock:(void*)aBasicBlock;
/**
 * Compares aCondition to the SmallInt value for NO (1) and executes the first
 * block if it matches, the second if it doesn't.
 */
- (void) branchOnCondition:(void*)aCondition
                      true:(void*)trueBlock
                     false:(void*)falseBlock;
/**
 * Ends the current basic block with an unconditional jump to the specified
 * basic block
 */
- (void) goToBasicBlock:(void*)aBasicBlock;
/**
 * Associate a basic block with the specified label for later retrieval.
 */
- (void) setBasicBlock:(void*)aBasicBlock forLabel:(NSString*)aLabel;
/**
 * Returns the basic block associated with the specified label.
 */
- (void*) basicBlockForLabel:(NSString*)aLabel;
/**
 * Ends the current basic block with an unconditional jump to the basic block
 * that has been associated with the specified label.
 */
- (void) goToLabelledBasicBlock:(NSString*)aLabel;
@end
/** 
 * Class used to instantiate the default code generators.  Lazily loads the code
 * generator components the first time it receives a message.
 */
@interface LKCodeGenLoader : NSObject {}
/**
 * Returns the default code generator for JIT compilation.
 */
+ (id<LKCodeGenerator>) defaultJIT;
/**
 * Returns the default code generator for static compilation, outputting to the
 * file specified in the argument.
 */
+ (id<LKCodeGenerator>) defaultStaticCompilerWithFile:(NSString*)outFile;
@end
/**
 * Protocol for static code generators.
 */
@protocol LKStaticCodeGenerator <LKCodeGenerator>
/**
 * Initializes the code generator with the specified file.
 */
- (id<LKStaticCodeGenerator>) initWithFile:(NSString*)outFile;
@end
/**
 * Returns the default code generator for JIT compilation.
 *
 * Deprecated.  Use [LKCodeGenLoader defaultJIT] in new code.
 */
id <LKCodeGenerator> defaultJIT(void);
/**
 * Returns the default code generator for static compilation, outputting to the
 * file specified in the argument.
 *
 * Deprecated.  Use [LKCodeGenLoader defaultStaticCompilerWithFile:] in new
 * code.
 */
id <LKCodeGenerator> defaultStaticCompilterWithFile(NSString*);
