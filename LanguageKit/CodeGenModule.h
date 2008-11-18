#ifndef __CODE_GEN_MODULE__INCLUDED__
#define __CODE_GEN_MODULE__INCLUDED__
#include "CGObjCRuntime.h"

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

  Module *TheModule;
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
  // All will be "@" for now.
  llvm::SmallVector<string, 8> IvarTypes;
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


public:
  const Type *getCurrentClassTy() { return CurrentClassTy; }
  IRBuilder<> *getInitBuilder() { return &InitialiseBuilder; }
  const string& getClassName() { return ClassName; }
  const string& getSuperClassName() { return SuperClassName; }
  Module *getModule() { return TheModule; }
  CGObjCRuntime *getRuntime() { return Runtime; }
  string getCategoryName() { return CategoryName; }

  /**
   * Returns the code generator for the current scope
   */
  CodeGenLexicalScope *getCurrentScope() { return ScopeStack.back(); }
  /**
   * Initialise for the specified module.  The second argument specifies
   * whether the module should be used for static or JIT compilation.
   */
	CodeGenModule(const char *ModuleName, bool jit=true);

  /**
   * Start generating code for a class.
   */
  void BeginClass(const char *Class, const char *Super, const char ** Names,
      const char ** Types, int *Offsets, int SuperclassSize);

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
		  locals);

  /**
   * Start a method.
   */
  void BeginInstanceMethod(const char *MethodName, const char *MethodTypes, int
		  locals);

  /**
   * End the current method.
   */
  void EndMethod();

  /**
   * Begin a BlockClosure.
   */
  void BeginBlock(unsigned args, unsigned locals, Value **promoted, int count);
  
  /**
   * End the current block.  Returns a pointer to the block object.
   */
  Value *EndBlock(void);

  /**
   * Load a bound variable from a block.
   */
  Value *LoadBlockVar(unsigned index, unsigned offset);
  /**
   * Store a value in a bound variable.
   */
  void StoreBlockVar(Value *val, unsigned index, unsigned offset);

  /**
   * Set the (local) return value for a block.
   */
  void SetBlockReturn(Value *value);
  /**
   * Create an integer constant.  Either a SmallInt or a BigInt, depending on
   * the size.
   */
  Value *IntConstant(const char *value);
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
};
// Debugging macros:
extern "C" {
  extern int DEBUG_DUMP_MODULES;
}
#define DUMP(x) do { if (DEBUG_DUMP_MODULES) x->dump(); } while(0)
#define DUMPT(x) DUMP((x->getType()))
#define LOG(x,...) do { if (DEBUG_DUMP_MODULES) fprintf(stderr, x,##__VA_ARGS__); } while(0)
void SkipTypeQualifiers(const char **typestr);
const Type *LLVMTypeFromString(const char * typestr);
llvm::FunctionType *LLVMFunctionTypeFromString(const char *typestr);
    // Set up the arguments
#endif // __CODE_GEN_MODULE__INCLUDED__
