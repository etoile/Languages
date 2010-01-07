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

/**
 * Create a new LLVM module builder.
 */
ModuleBuilder newModuleBuilder(const char *ModuleName);
/**
 * Creates a new module builder for static compilation.
 */
ModuleBuilder newStaticModuleBuilder(const char *ModuleBuilder);
/**
 * Destroy the module builder.
 */
void freeModuleBuilder(ModuleBuilder aModule);
/**
 * Begin a class, subclassing Super.  
 */
void BeginClass(ModuleBuilder B, const char *Class, const char *Super, const
		char ** cVarNames, const char ** cVarTypes, const char ** iVarNames,
		const char ** iVarTypes, int *iVarOffsets, int SuperclassSize);
/**
 * End the current class.
 */
void EndClass(ModuleBuilder B);
void BeginCategory(ModuleBuilder B, const char *cls, const char *cat);
void EndCategory(ModuleBuilder B);
/**
 * Generate message send to the superclass.
 */
LLVMValue MessageSendSuper(ModuleBuilder B, const char *selName, const char
	  *selTypes, LLVMValue *argv, unsigned argc);
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
void StoreValueInLocalAtIndex(ModuleBuilder B, LLVMValue value, unsigned index,
		unsigned depth);
/**
 * Store a value in an instance variable.
 */
void StoreValueOfTypeAtOffsetFromObject(ModuleBuilder B, LLVMValue value,
    const char* type, unsigned offset, LLVMValue object);
/**
 * Load a class variable.
 */
LLVMValue LoadClassVar(ModuleBuilder B, const char *cVarName);
/**
 * Store a value in a class variable.
 */
void StoreClassVar(ModuleBuilder B, const char *cVarName, LLVMValue value);
/**
 * Load a local variable.
 */
LLVMValue LoadLocalAtIndex(ModuleBuilder B, unsigned index, unsigned depth);
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
 * Begin a class method.
 */
void BeginClassMethod(ModuleBuilder B, const char *methodname, const char
		*methodTypes, unsigned args, const char **localNames);
/**
 * Begin an instance method.
 */
void BeginInstanceMethod(ModuleBuilder B, const char *methodname, const char
		*methodTypes, unsigned args, const char **localNames);
/**
 * End the current method.
 */
void EndMethod(ModuleBuilder B);
/**
 * Begin a block.
 */
void BeginBlock(ModuleBuilder B, unsigned args, unsigned locals);
/**
 * Load a bound variable in the current block.
 */
LLVMValue LoadBlockVar(ModuleBuilder B, unsigned index, unsigned offset);
/**
 * Store a value in the location pointed to by a block variable.
 */
void StoreBlockVar(ModuleBuilder B, LLVMValue value, unsigned index,
		unsigned offset);
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
LLVMValue LoadArgumentAtIndex(ModuleBuilder B, unsigned index, unsigned depth);
/**
 * Constructs an LLVM Constant for this integer.
 */
LLVMValue IntConstant(ModuleBuilder B, const char *value);
/**
 * Constructs an LLVM constant for this floating point value.
 */
LLVMValue FloatConstant(ModuleBuilder B, const char *value);
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
 * Emit the file as LLVM bitcode.
 */
void EmitBitcode(ModuleBuilder B, char *filename, bool isAsm);
/**
 * Initialise the code generator.  Should be called before any other LLVM
 * calls.  Takes the name of the skeleton module file as an argument.
 */
void LLVMinitialise(const char *bcFilename);
/**
 * Construct a Smalltalk symbol (a boxed selector)
 */
LLVMValue SymbolConstant(ModuleBuilder B, const char *symbol);

/**
 * Creates a new basic block with the specified name and returns a pointer
 * to the block.
 */
void *StartBasicBlock(ModuleBuilder B, const char* BBName);
/**
 * Returns a pointer to the current basic block.
 */
void *CurrentBasicBlock(ModuleBuilder B);
/**
 * Moves the insert point to the specified basic block.
 */
void MoveInsertPointToBasicBlock(ModuleBuilder B, void *BB);
/**
 * Ends the current basic block with an unconditional jump to the specified
 * basic block and sets the insert point to that block.
 */
void GoTo(ModuleBuilder B, void *BB);
/**
 * Ends the current basic block with a conditional branch, to FalseBB if
 * condition is the SmallInt value corresponding to the Objective-C 'NO' or
 * to TrueBB if it is any other value.
 */
void BranchOnCondition(ModuleBuilder B, LLVMValue condition, void *TrueBB,
	void *FalseBB);
/**
 * Debug flag used to set whether excessive amounts of debugging info should be
 * spammed to stderr.
 */
extern int DEBUG_DUMP_MODULES;
#ifdef __OBJC__
#import <Foundation/NSObject.h>
#import <LanguageKit/LKCodeGen.h>
/**
 * Concrete implementation of the CodeGenerator protocol using LLVM.
 */
@interface LLVMCodeGen : NSObject <LKCodeGenerator> {
	ModuleBuilder Builder;
}
+ (NSString*) smallIntBitcodeFile;
@end
#endif
