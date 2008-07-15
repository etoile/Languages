#ifndef __CODE_GEN_MODULE__INCLUDED__
#define __CODE_GEN_MODULE__INCLUDED__
#include "CGObjCRuntime.h"

namespace llvm {
  class BasicBlock;
  class PointerType;
  class FunctionType;
}
using clang::CodeGen::CGObjCRuntime;
class CodeGenBlock;
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

  Module *TheModule;
  CGObjCRuntime * Runtime;
  const Type *CurrentClassTy;
  Function *CurrentMethod;
  Value *Self;
  IRBuilder *Builder;
  string ClassName;
  string SuperClassName;
  string CategoryName;
  int InstanceSize;
  SmallVector<Value*, 8> Locals;
  SmallVector<Value*, 8> Args;
  Value * RetVal;
  BasicBlock * CleanupBB;
  SmallVector<CodeGenBlock*, 8> BlockStack;
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

  /**
   * Maps a selector to a SmallInt function name.
   */
  string FunctionNameFromSelector(const char *sel);

  /**
   * Constructs a Smalltalk object from the specified Objective-C type.
   */
  Value *BoxValue(IRBuilder *B, Value *V, const char *typestr);

  /**
   * Constructs a C primitive from a Smalltalk object.
   */
  void UnboxArgs(IRBuilder *B, Function *F,  Value ** argv, Value **args,
      unsigned argc, const char *selTypes);

  /**
   * Preform a real message send.  Reveicer must be a real object, not a
   * SmallInt.
   */
  Value *MessageSendId(IRBuilder *B, Value *receiver, const char *selName,
      const char *selTypes, Value **argv, unsigned argc);
  /**
   * Send a message to something that may be a SmallInt or an Objective-C
   * object.
   */
  Value *MessageSend(IRBuilder *B, Function *F, Value *receiver, const char
      *selName, const char *selTypes, Value **argv, unsigned argc);

public:
  /**
   * Initialise for the specified module.
   */
  CodeGenModule(const char *ModuleName);

  /**
   * Start generating code for a class.
   */
  void BeginClass(const char *Class, const char *Super, const char ** Names,
      const char ** Types, int *Offsets);

  /**
   * End a class.
   */
  void EndClass(void);

  /**
   * Start a method.
   */
  void BeginMethod(const char *MethodName, const char *MethodTypes, int locals);

  /**
   * Load an argument at the specified index.
   */
  Value *LoadArgumentAtIndex(unsigned index);

  /**
   * Send a message to an Objective-C object.
   */
  Value *MessageSendId(Value *receiver, const char *selName, const char
      *selTypes, Value **argv, unsigned argc);

  /**
   * Send a message to a Smalltalk object.
   */
  Value *MessageSend(Value *receiver, const char *selName, const char
      *selTypes, Value **argv, unsigned argc);

  /**
   * Set the return value for this method / block.
   */
  void SetReturn(Value * Ret = 0);

  /**
   * End the current method.
   */
  void EndMethod();

  /**
   * Load the value of self in the current context.
   */
  Value *LoadSelf(void);
  
  /**
   * Load a local in the current contest.
   */
  Value *LoadLocalAtIndex(unsigned index);

  /**
   * Get a pointer to a local variable in the current context.
   */
	Value *LoadPointerToLocalAtIndex(unsigned index);

  /**
   * Store a value in a local.
   */
  void StoreValueInLocalAtIndex(Value * value, unsigned index);

  /**
   * Get a pointer to the class object for a specified name.
   */
  Value *LoadClass(const char *classname);

  /**
   * Load an instance variable.
   */
  Value *LoadValueOfTypeAtOffsetFromObject( const char* type, unsigned offset,
      Value *object);

  /**
   * Store an instance value.
   */
  void StoreValueOfTypeAtOffsetFromObject(Value *value,
      const char* type, unsigned offset, Value *object);

  /**
   * Begin a BlockClosure.
   */
  void BeginBlock(unsigned args, unsigned locals, Value **promoted, int count);
  
  /**
   * Load a bound variable from a block.
   */
  Value *LoadBlockVar(unsigned index, unsigned offset);

  /**
   * Set the (local) return value for a block.
   */
  void SetBlockReturn(Value *value);

  /**
   * End the current block.  Returns a pointer to the block object.
   */
  Value *EndBlock(void);

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
   * Compare two pointers for equality.
   */
	Value *ComparePointers(Value *lhs, Value *rhs);

  /**
   * Compile and load this module.
   */
  void compile(void);
};
// Debugging macros:
#define DUMP(x) do { if (DEBUG_DUMP_MODULES) x->dump(); } while(0)
#define DUMPT(x) DUMP((x->getType()))
#define LOG(x,...) do { if (DEBUG_DUMP_MODULES) fprintf(stderr, x,##__VA_ARGS__); } while(0)
void SkipTypeQualifiers(const char **typestr);
const Type *LLVMTypeFromString(const char * typestr);
llvm::FunctionType *LLVMFunctionTypeFromString(const char *typestr);
void InitialiseFunction(IRBuilder *B, Function *F, Value *&Self,
    SmallVectorImpl<Value*> &Args, SmallVectorImpl<Value*> &Locals, unsigned
    locals, Value *&RetVal, BasicBlock *&CleanupBB);

    // Set up the arguments
#endif // __CODE_GEN_MODULE__INCLUDED__
