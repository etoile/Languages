/***
 * C interface to the LLVM generator component.
 */

/***
 * ModuleBuilder is an opaque pointer type wrapping a C++ object which
 * interfaces with the LLVM components for generating the intermediate
 * representation code.
 */
typedef struct CodeGenModule* ModuleBuilder;
/**
 * Value of an LLVM register.
 */
typedef struct Value* LLVMValue;

/***
 * Create a new LLVM module builder.
 */
ModuleBuilder newModuleBuilder(const char *ModuleName);
/**
 * Destroy the module builder.
 */
void freeModuleBuilder(ModuleBuilder aModule);
/**
 * Begin a class, subclassing Super.  
 */
void BeginClass(ModuleBuilder B, const char *Class, const char *Super, 
    const char ** Names, const char ** Types, int *Offsets);
/**
 * End the current class.
 */
void EndClass(ModuleBuilder B);
/**
 * Generate a Smalltalk message send.
 */
LLVMValue MessageSend(ModuleBuilder B, LLVMValue receiver, const char *selname,
    const char *seltype, LLVMValue *argv, unsigned argc);
/**
 * Generate an Objective-C message send.
 */
LLVMValue MessageSendId(ModuleBuilder B, LLVMValue receiver, const char
    *selname, const char *seltype, LLVMValue *argv, unsigned argc);
/**
 * Return the value of self in the current method.
 */
LLVMValue LoadSelf(ModuleBuilder B);
/**
 * Store a value in a local variable.
 */
void StoreValueInLocalAtIndex(ModuleBuilder B, LLVMValue value, unsigned index);
/**
 * Store a value in an instance variable.
 */
void StoreValueOfTypeAtOffsetFromObject(ModuleBuilder B, LLVMValue value,
    const char* type, unsigned offset, LLVMValue object);
/**
 * Load a local variable.
 */
LLVMValue LoadLocalAtIndex(ModuleBuilder B, unsigned index);
/**
 * Load a pointer to a local variable.
 */
LLVMValue LoadPointerToLocalAtIndex(ModuleBuilder B, unsigned index);
/**
 * Load an instance variable.
 */
LLVMValue LoadValueOfTypeAtOffsetFromObject(ModuleBuilder B, const char* type,
    unsigned offset, LLVMValue object);
/**
 * Load the named class.
 */
LLVMValue LoadClass(ModuleBuilder B, const char *classname);
/**
 * Set the return value for the current method.
 */
void SetReturn(ModuleBuilder B, LLVMValue retval);
/**
 * Begin a method.
 */
void BeginMethod(ModuleBuilder B, const char *methodname, const char
    *methodTypes, unsigned args);
/**
 * End the current method.
 */
void EndMethod(ModuleBuilder B);
/**
 * Begin a block.
 */
void BeginBlock(ModuleBuilder B, unsigned args, unsigned locals, LLVMValue
    *promoted, int count);
/**
 * Load a bound variable in the current block.
 */
LLVMValue LoadBlockVar(ModuleBuilder B, unsigned index, unsigned offset);
/**
 * End the current block.
 */
LLVMValue EndBlock(ModuleBuilder B);
/**
 * Set the local return value for a block.
 */
void SetBlockReturn(ModuleBuilder B, LLVMValue value);
/**
 * Load an argument to the current block or method context.
 */
LLVMValue LoadArgumentAtIndex(ModuleBuilder B, unsigned index);
/**
 * Constructs an LLVM Constant for this integer.
 */
LLVMValue IntConstant(ModuleBuilder B, const char *value);
/**
 * Constructs an LLVM constant for this string.
 */
LLVMValue StringConstant(ModuleBuilder B, const char *value);
/**
 * Constructs an LLVM constant representing nil.
 */
LLVMValue NilConstant(void);
/**
 * Compare two pointers.  Returns a SmallInt representing the comparison result.
 */
LLVMValue ComparePointers(ModuleBuilder B, LLVMValue lhs, LLVMValue rhs);

/**
 * Compile the module and load it into the runtime system.
 */
void Compile(ModuleBuilder B);
/**
 * Initialise the code generator.  Should be called before any other LLVM
 * calls.  Takes the name of the skeleton module file as an argument.
 */
void LLVMinitialise(const char *bcFilename);

/**
 * Debug flag used to set whether excessive amounts of debugging info should be
 * spammed to stderr.
 */
extern int DEBUG_DUMP_MODULES;
#ifdef __OBJC__
#import <Foundation/NSObject.h>
#import <CodeGen.h>
/**
 * Concrete implementation of the CodeGenerator protocol using LLVM.
 */
@interface LLVMCodeGen : NSObject <CodeGenerator> {
	ModuleBuilder Builder;
}
@end
#endif
