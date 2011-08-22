#import <EtoileFoundation/EtoileFoundation.h>

// HACK: Foundation.h on OS X defines true and false, which conflicts with the
// method branchOnCondition:true:false: in this class
#undef true
#undef false
@class LKSymbolTable;
@class LKSymbol;

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
 * Create a new class, with the specified superclass.  The symbol table should
 * define all of the new instance and class variables.
 */
- (void)createSubclassWithName: (NSString*)aClass
               superclassNamed: (NSString*)aSuperclass
               withSymbolTable: (LKSymbolTable*)symbolTable;
/**
 * Finish the current class. 
 */
- (void)endClass;
/**
 * Create a new category with the specified name on the named class.
 */
- (void)createCategoryWithName:(NSString*)aCategory
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
- (void) beginClassMethod: (NSString*)aName
         withTypeEncoding: (NSString*)typeEncoding
                arguments: (NSArray*)arguments
                   locals: (NSArray*)locals;
/**
 * Begin an instance method with the specified type encoding and number of local
 * variables.  Local variables and arguments are indexed by number, the symbol
 * table information is just for debugging.
 */
- (void) beginInstanceMethod: (NSString*)aName
            withTypeEncoding: (NSString*)typeEncoding
                   arguments: (NSArray*)arguments
                      locals: (NSArray*)locals;
/**
 * Begins compiling a free-standing function.  
 */
- (void)beginFunction: (NSString*)aName
     withTypeEncoding: (NSString*)typeEncoding
            arguments: (NSArray*)arguments 
               locals: (NSArray*)locals;
/**
 * Sends a message to a receiver which may be a SmallInt (a boxed Smalltalk
 * integer contained within an object pointer).
 */
- (void*) sendMessage:(NSString*)aMessage
                types:(NSArray*)types
                   to:(void*)receiver
             withArgs:(void**)argv
                count:(unsigned int)argc;
/**
 * Sends a message to the superclass.
 */
- (void*) sendSuperMessage:(NSString*)sel
                     types:(NSString*)seltypes
                  withArgs:(void**)argv
                     count:(unsigned int)argc;
/**
 * Sends a message to an object.  Similar to
 * sendMessage:type:to:withargs:count: but requires that receiver be an
 * Objective-C object, not a small integer.
 */
- (void*) sendMessage:(NSString*)aMessage
                types:(NSArray*)types
             toObject:(void*)receiver
             withArgs:(void**)argv
                count:(unsigned int)argc;
/**
 * Calls a C function, with the specified type encoding.
 */
- (void*)callFunction: (NSString*)functionName
         typeEncoding: (NSString*)typeEncoding
            arguments: (void**)arguments
                count: (int)count;
/**
 * Begin generating a block expression with the specified number of arguments
 * and locals.  The bound variables are pointers to variables declared outside
 * the block's scope.
 */
- (void) beginBlockWithArgs: (NSArray*)args
                     locals: (NSArray*)locals
                  externals: (NSArray*)externals
                  signature: (NSString*)signature;
/**
 * Returns 'self' in the current method.
 */
- (void*) loadSelf;
/**
 * Returns the block object for the current closure.  Calling this when not in
 * a block has undefined behaviour.
 */
- (void*) loadBlockContext;
/**
 * Load a pointer to the named class.
 */
- (void*) loadClassNamed:(NSString*)aClass;
/**
 * Stores a value in the specified variable.
 */
- (void)storeValue:(void*)aVal inVariable: (LKSymbol*)aVariable;
/**
 * Load the value of the specified local variable.
 */
- (void*)loadVariable: (LKSymbol*)aVariable;
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
 * Protocol for static code generators.
 */
@protocol LKStaticCodeGenerator <LKCodeGenerator>
/**
 * Initializes the code generator with the specified file.
 */
- (id<LKStaticCodeGenerator>) initWithFile:(NSString*)outFile;
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
+ (id<LKStaticCodeGenerator>) defaultStaticCompilerWithFile:(NSString*)outFile;
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
