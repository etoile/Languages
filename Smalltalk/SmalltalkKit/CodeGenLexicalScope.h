#ifndef __CODE_GEN_LEXICAL_SCOPE__INCLUDED__
#define __CODE_GEN_LEXICAL_SCOPE__INCLUDED__

#include "llvm/Support/IRBuilder.h"
#include <string>

namespace llvm {
  class BasicBlock;
  class PointerType;
  class FunctionType;
  class Value;
}
class CodeGenBlock;
using std::string;
using namespace::llvm;
class CodeGenModule;

class CodeGenLexicalScope {
protected:
  CodeGenModule *CGM;
  SmallVector<Value*, 8> Locals;
  SmallVector<Value*, 8> Args;
  Value * RetVal;
  BasicBlock * CleanupBB;
  Function *CurrentFunction;
  IRBuilder<> Builder;
  Value *Self;
  /**
   * Intialises a Function object to be used as a Smalltalk method or block
   * function.
   */
  void InitialiseFunction( SmallVectorImpl<Value*> &Args,
     SmallVectorImpl<Value*> &Locals, unsigned locals, const char *MethodTypes="@");
  /**
   * Maps a selector to a SmallInt function name.
   */
  string FunctionNameFromSelector(const char *sel);

  /**
   * Constructs a Smalltalk object from the specified Objective-C type.
   */
  Value *BoxValue(IRBuilder<> *B, Value *V, const char *typestr);

  /**
   * Constructs a C primitive from a Smalltalk object.
   */
  Value *Unbox(IRBuilder<> *B, Function *F, Value *val, const char *Type);

  /**
   * Construct C primitives from Smalltalk objects in an argument list.
   */
  void UnboxArgs(IRBuilder<> *B, Function *F,  Value ** argv, Value **args,
      unsigned argc, const char *selTypes);

  /**
   * Send a message to the superclass.
   */
  Value *MessageSendSuper(IRBuilder<> *B, Function *F, const char
		*selName, const char *selTypes, Value **argv, unsigned argc);
  /**
   * Preform a real message send.  Reveicer must be a real object, not a
   * SmallInt.
   */
  Value *MessageSendId(IRBuilder<> *B, Value *receiver, const char *selName,
      const char *selTypes, Value **argv, unsigned argc);
  /**
   * Send a message to something that may be a SmallInt or an Objective-C
   * object.
   */
  Value *MessageSend(IRBuilder<> *B, Function *F, Value *receiver, const char
      *selName, const char *selTypes, Value **argv=0, Value **boxedArgs=0,
      unsigned argc=0);
public:
  CodeGenLexicalScope(CodeGenModule *Mod) : CGM(Mod){}
  IRBuilder<> *getBuilder() { return &Builder; }
  /**
   * Load an argument at the specified index.
   */
  Value *LoadArgumentAtIndex(unsigned index);

  /**
   * Send a message to the superclass.
   */
  Value *MessageSendSuper(const char *selName, const char *selTypes, Value
		  **argv, unsigned argc);

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
   * Loads a pointer to an argument in the current context.
   */
  Value *LoadPointerToArgumentAtIndex(unsigned index);

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
   * Clean up after a block.
   */
  void EndChildBlock(CodeGenBlock *block);
  /**
   * Create a symbol object.
   */
  Value *SymbolConstant(const char *symbol);

  /**
   * Compare two pointers for equality.
   */
  Value *ComparePointers(Value *lhs, Value *rhs);
  ~CodeGenLexicalScope() {
    if (0 == Builder.GetInsertBlock()->getTerminator()) {
      SetReturn();
    }
  }
};

class CodeGenMethod : public CodeGenLexicalScope {
public:
  CodeGenMethod(CodeGenModule *Mod, const char *MethodName, const char
      *MethodTypes, int locals);
};

#endif 
