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
	Value *Context;
	SmallVector<Value*, 8> Locals;
	SmallVector<Value*, 8> Args;
	Value * RetVal;
	BasicBlock * CleanupBB;
	BasicBlock * PromoteBB;
	BasicBlock * RetBB;
	Function *CurrentFunction;
	IRBuilder<> Builder;
	Value *Self;
	Value *ScopeSelf;

	const char *ReturnType;
	bool containsBlocks;

	virtual void SetParentScope(void) {};
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
  /**
   * Debugging function - emits a printf statement with the string and the
   * extra argument.
   */
	void CreatePrintf(IRBuilder<> &Builder, const char *str, Value *val);
public:
	/**
	 * Ends the current lexical scope.
	 */
	void EndScope(void);
  CodeGenLexicalScope(CodeGenModule *Mod) : CGM(Mod), containsBlocks(false) {}
  IRBuilder<> *getBuilder() { return &Builder; }
  Value *getContext() { return Context; }
  /**
   * Load an argument at the specified index.
   */
  Value *LoadArgumentAtIndex(unsigned index, unsigned depth);

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
  virtual Value *LoadSelf(void);
  
  /**
   * Load a local in the current contest.
   */
  Value *LoadLocalAtIndex(unsigned index, unsigned depth);

  /**
   * Store a value in a local.
   */
  void StoreValueInLocalAtIndex(Value * value, unsigned index, unsigned depth);

  /**
   * Get a pointer to the class object for a specified name.
   */
  Value *LoadClass(const char *classname);

  /**
   * Load a class variable.
   */
  Value *LoadClassVariable(string className, string cVarName);
  /**
   * Store a value in a class variable.
   */
  void StoreValueInClassVariable(string className, string cVarName, Value
		  *object);

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
   * Returns the parent lexical scope.
   */
  virtual CodeGenLexicalScope *getParentScope() { return 0; }

  /**
   * Compare two pointers for equality.
   */
  Value *ComparePointers(Value *lhs, Value *rhs);
  virtual ~CodeGenLexicalScope() {
    if (0 == Builder.GetInsertBlock()->getTerminator()) {
      SetReturn();
    }
  }
};

class CodeGenMethod : public CodeGenLexicalScope {
public:
  CodeGenMethod(CodeGenModule *Mod, const char *MethodName, const char
      *MethodTypes, int locals, bool isClass=false);
};
/**
 * Offset of the variables in the context from the object start.
 * CHANGE THIS IF YOU MODIFY THE CONTEXT OBJECT!
 */
static const int CONTEXT_VARIABLE_OFFSET = 4;

#endif 
