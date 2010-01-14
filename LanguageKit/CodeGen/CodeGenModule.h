#ifndef __CODE_GEN_MODULE__INCLUDED__
#define __CODE_GEN_MODULE__INCLUDED__
#include "CGObjCRuntime.h"
#include <llvm/Analysis/DebugInfo.h>
#include "llvm/ADT/StringMap.h"
#include <stdio.h>

namespace llvm {
  class BasicBlock;
  class PointerType;
  class FunctionType;
}
class CodeGenLexicalScope;
using std::string;
using namespace::llvm;

extern PointerType *IdTy;
extern const Type *IntTy;
extern const Type *IntPtrTy;
extern const Type *SelTy;
extern const PointerType *IMPTy;
extern const char *MsgSendSmallIntFilename;
extern Constant *Zeros[2];

/**
 * This class implements a streaming code generation interface designed to be
 * called directly from an AST.  
 */
class CodeGenModule {
private:
  friend class CodeGenBlock;
  friend class CodeGenLexicalScope;

  LLVMContext &Context;
  Module *TheModule;

  DIFactory *Debug;
  DICompileUnit ModuleScopeDescriptor;
  llvm::StringMap<DIType> DebugTypeEncodings;

  Module *SmallIntModule;
  Function *LiteralInitFunction;
  IRBuilder<>InitialiseBuilder;
  CGObjCRuntime * Runtime;
  bool inClassMethod;
  const Type *CurrentClassTy;
  IRBuilder<> *MethodBuilder;
  string ClassName;
  string SuperClassName;
  string CategoryName;
  int InstanceSize;
  SmallVector<CodeGenLexicalScope*, 8> ScopeStack;
  llvm::SmallVector<string, 8> IvarNames;
  llvm::SmallVector<string, 8> CvarNames;
  // All will be "@" for now.
  llvm::SmallVector<string, 8> IvarTypes;
  llvm::SmallVector<string, 8> CvarTypes;
  llvm::SmallVector<int, 8> IvarOffsets;
  llvm::SmallVector<string, 8> InstanceMethodNames;
  llvm::SmallVector<string, 8> InstanceMethodTypes;
  llvm::SmallVector<string, 8> ClassMethodNames;
  llvm::SmallVector<string, 8> ClassMethodTypes;
  llvm::SmallVector<std::string, 8> Protocols;

  /**
   * Returns a constant C string using Str as an initialiser.
   */
  Constant *MakeConstantString(const std::string &Str, const
          std::string &Name="", unsigned GEPs=2);

  /**
   * Creates a generic constant.  This will be defined in the module load
   * function by sending a message to the specified class.
   */
	Value *GenericConstant(IRBuilder<> &Builder, const std::string className,
			const std::string constructor, const char *argument);
	/**
	 * Creates a global value containing a pointer to a class.
	 */
	void CreateClassPointerGlobal(const char *className, const char *globalName);

public:
  const Type *getCurrentClassTy() { return CurrentClassTy; }
  IRBuilder<> *getInitBuilder() { return &InitialiseBuilder; }
  const string& getClassName() { return ClassName; }
  const string& getSuperClassName() { return SuperClassName; }
  Module *getModule() { return TheModule; }
  DIFactory *getDebugFactory() { return Debug; }
  CGObjCRuntime *getRuntime() { return Runtime; }
  string getCategoryName() { return CategoryName; }
  DICompileUnit getModuleDescriptor() { return ModuleScopeDescriptor; }

  /**
   * Returns the debug info node for an Objective-C type encoding.
   */
  DIType DebugTypeForEncoding(const std::string &encoding);

  /**
   * Returns the code generator for the current scope
   */
  CodeGenLexicalScope *getCurrentScope() { return ScopeStack.back(); }
  /**
   * Initialise for the specified module.  The second argument specifies
   * whether the module should be used for static or JIT compilation.
   */
	CodeGenModule(const char *ModuleName, LLVMContext &C, bool jit=true);

  /**
   * Start generating code for a class.
   */
	void BeginClass(const char *Class, const char *Super, const
		char ** cVarNames, const char ** cVarTypes, const char ** iVarNames,
		const char ** iVarTypes, int *iVarOffsets, int SuperclassSize);

  /**
   * End a class.
   */
  void EndClass(void);

  /**
   * Start generating code for a category.
   */
  void BeginCategory(const char *Class, const char *CategoryName);

  /**
   * Finish generating a category.
   */
  void EndCategory(void);

  /**
   * Start a class method.
   */
  void BeginClassMethod(const char *MethodName, const char *MethodTypes, int
		  locals, const char **localNames);

  /**
   * Start a method.
   */
  void BeginInstanceMethod(const char *MethodName, const char *MethodTypes, int
		  locals, const char **localNames);

  /**
   * End the current method.
   */
  void EndMethod();

  /**
   * Begin a BlockClosure.
   */
  void BeginBlock(unsigned args, unsigned locals);
  
  /**
   * End the current block.  Returns a pointer to the block object.
   */
  Value *EndBlock(void);
	/**
	 * Store the class variable for the current class.
	 */
	void StoreClassVar(const char *cVarName, Value *value);
	/**
	 * Load the class variable for the current class.
	 */
	Value *LoadClassVar(const char *cVarName);

  /**
   * Set the (local) return value for a block.
   */
  void SetBlockReturn(Value *value);
	/**
	* Create an integer constant.  Either a SmallInt or a BigInt, depending on
	* the size.  
	*/
	Value *IntConstant(IRBuilder<> &Builder, const char *value);
	/**
	 * Creates a floating point constant.
	 */
	Value *FloatConstant(IRBuilder<> &Builder, const char *value);
	/**
	 * Create a symbol (selector) constant.
	 */
	Value *SymbolConstant(IRBuilder<> &Builder, const char *symbol);
  /**
   * Create a string (object) constant.
   */
  Value *StringConstant(const char *value);

  /**
   * Get the module which provides static definitions of small int messages.
   */
  Module *getSmallIntModule() { return SmallIntModule; };

  /**
   * Compile and load this module.
   */
  void compile(void);
  /**
   * Write the module as a bitcode file.  If isAsm is true then this writes
   * LLVM 'assembly language' instead of bitcode.
   */
	void writeBitcodeToFile(char* filename, bool isAsm=false);
   /**
   * Returns an LLVM type from a type string.
   */
   const Type *LLVMTypeFromString(const char * typestr);
   /**
   * Returns an LLVM function type from a string.  Sets isSRet if the function
   * contains a structure which should be returned on the stack.  
   */
   llvm::FunctionType *LLVMFunctionTypeFromString(const char *typestr,
   												bool &isSRet,
   												const Type *&realRetTy);
};
// Debugging macros:
extern "C" {
  extern int DEBUG_DUMP_MODULES;
}
/** Debugging macro: dumps the object if the debug flag is set */
#define DUMP(x) do { if (DEBUG_DUMP_MODULES) x->dump(); } while(0)
/** Debugging macro: dumps the object's type if the debug flag is set */
#define DUMPT(x) DUMP((x->getType()))
/** Debugging macro: logs an error message to stderr if the debug flag is set. */
#define LOG(x,...) \
	do { if (DEBUG_DUMP_MODULES) fprintf(stderr, x,##__VA_ARGS__); } while(0)
/**
 * Skip method type qualifiers.  Increments typestr to skip past qualifiers
 * that are not needed by LanguageKit (e.g. oneway).
 */
void SkipTypeQualifiers(const char **typestr);



#endif // __CODE_GEN_MODULE__INCLUDED__
