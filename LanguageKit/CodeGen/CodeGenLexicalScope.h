#ifndef __CODE_GEN_LEXICAL_SCOPE__INCLUDED__
#define __CODE_GEN_LEXICAL_SCOPE__INCLUDED__

#include "llvm/Support/IRBuilder.h"
#include "CodeGenModule.h"
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
	DIDescriptor ScopeDescriptor;
	SmallVector<Value*, 8> Locals;
	SmallVector<Value*, 8> Args;
	SmallVector<DIVariable, 8> DebugLocals;
	SmallVector<DIVariable, 8> DebugArgs;
	Value * RetVal;
	BasicBlock * CleanupBB;
	BasicBlock * ExceptionBB;
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
     SmallVectorImpl<Value*> &Locals, unsigned locals, 
	 const char *MethodTypes="@", bool isSRet=false,
	 const char **symbols=0, const std::string &humanName="AnonymousBlock");
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
	void UnboxArgs(IRBuilder<> *B, Function *F,
			llvm::SmallVectorImpl<llvm::Value*> &argv,
			llvm::SmallVectorImpl<llvm::Value*> &args, const char *selTypes);

	/**
	* Send a message to the superclass.
	*/
	Value *MessageSendSuper(IRBuilder<> *B, Function *F, const char *selName,
			const char *selTypes, llvm::SmallVectorImpl<llvm::Value*> &argv);
	/**
	* Preform a real message send.  Reveicer must be a real object, not a
	* SmallInt.
	*/
	Value *MessageSendId(IRBuilder<> *B, Value *receiver, const char *selName,
		const char *selTypes, llvm::SmallVectorImpl<llvm::Value*> &argv);
  /**
   * Send a message to something that may be a SmallInt or an Objective-C
   * object.
   */
  Value *MessageSend(IRBuilder<> *B, Function *F, Value *receiver, const char
      *selName, const char *selTypes, SmallVectorImpl<Value*> &boxedArgs);
  /**
   * Send a message with no arguments to something that is either a SmallInt or
   * an Objective-C object.
   */
  Value *MessageSend(IRBuilder<> *B, Function *F, Value *receiver, const char
      *selName, const char *selTypes);
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
  CodeGenLexicalScope(CodeGenModule *Mod) : CGM(Mod),
	Builder(Mod->Context), containsBlocks(false) {}
  IRBuilder<> *getBuilder() { return &Builder; }
  Value *getContext() { return Context; }
  DIDescriptor getScopeDescriptor() { return ScopeDescriptor; }
  /**
   * Load an argument at the specified index.
   */
  Value *LoadArgumentAtIndex(unsigned index, unsigned depth);

  /**
   * Send a message to the superclass.
   */
  Value *MessageSendSuper(const char *selName, const char *selTypes, 
		  SmallVectorImpl<Value*> &argv);

  /**
   * Send a message to an Objective-C object.
   */
  Value *MessageSendId(Value *receiver, const char *selName, const char
      *selTypes, SmallVectorImpl<Value*> &argv);

  /**
   * Send a message to a Smalltalk object.
   */
  Value *MessageSend(Value *receiver, const char *selName, const char
      *selTypes, SmallVectorImpl<Value*> &boxedArgs);

  /**
   * Set the return value for this method / block.
   */
  virtual void SetReturn(Value * Ret = 0);

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
  Value *LoadValueOfTypeAtOffsetFromObject(const char *className, const char
		  *ivarName, const char* type, unsigned offset, Value *object);

  /**
   * Store an instance value.
   */
  void StoreValueOfTypeAtOffsetFromObject(Value *value, const char *className,
		  const char *ivarName, const char* type, unsigned offset, Value
		  *object);
  /**
   * Clean up after a block.
   */
  void EndChildBlock(CodeGenBlock *block);
  /**
   * Create a symbol object.
   */
  Value *SymbolConstant(const char *symbol);
  /**
   * Create a floating point constant.
   */
  Value *FloatConstant(const char *value);
  /**
   * Create an integer constant.
   */
  Value *IntConstant(const char *value);
  /**
   * Returns the parent lexical scope.
   */
  virtual CodeGenLexicalScope *getParentScope() { return 0; }

  /**
   * Compare two pointers for equality.
   */
  Value *ComparePointers(Value *lhs, Value *rhs);


	/**
	 * Creates a new basic block with the specified name and returns a pointer
	 * to the block.
	 */
	BasicBlock *StartBasicBlock(const char* BBName);
	/**
	 * Returns the current basic block.
	 */
	BasicBlock *CurrentBasicBlock(void);
	/**
	 * Moves the insert point to the specified basic block.
	 */
	void MoveInsertPointToBasicBlock(BasicBlock *BB);
	/**
	 * Ends the current basic block with an unconditional jump to the specified
	 * basic block and sets the insert point to that block.
	 */
	void GoTo(BasicBlock *BB);
	/**
	 * Ends the current basic block with a conditional branch, to FalseBB if
	 * condition is the SmallInt value corresponding to the Objective-C 'NO' or
	 * to TrueBB if it is any other value.
	 */
	void BranchOnCondition(Value *condition, BasicBlock *TrueBB, BasicBlock
		*FalseBB);
	virtual ~CodeGenLexicalScope()
	{
		BasicBlock *BB = Builder.GetInsertBlock();
		if (0 != BB && 0 == BB->getTerminator())
		{
			SetReturn();
		}
	}
};

class CodeGenMethod : public CodeGenLexicalScope {
public:
  CodeGenMethod(CodeGenModule *Mod, const char *MethodName, const char
      *MethodTypes, int locals, bool isClass, const char **localNames);
};
/**
 * Offset of the variables in the context from the object start.
 * CHANGE THIS IF YOU MODIFY THE CONTEXT OBJECT!
 */
static const int CONTEXT_VARIABLE_OFFSET = 4;

#endif 
