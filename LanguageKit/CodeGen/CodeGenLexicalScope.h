#ifndef __CODE_GEN_LEXICAL_SCOPE__INCLUDED__
#define __CODE_GEN_LEXICAL_SCOPE__INCLUDED__

#include "CodeGenTypes.h"
#include "CodeGenModule.h"
#include <string>
#include "objc_pointers.h"


namespace llvm {
	class BasicBlock;
	class PointerType;
	class FunctionType;
	class Value;
}
@class LKSymbol;

namespace etoile
{
namespace languagekit
{

class CodeGenModule;
class CodeGenBlock;

class CodeGenSubroutine
{
	friend class CodeGenModule;
	friend class CodeGenBlock;
protected:
	CodeGenModule *CGM;
	Value *Context;
	DIDescriptor ScopeDescriptor;

	typedef unordered_map<id,
	                                llvm::Value*,
	                                object_hash<id>,
	                                object_equal<id> > variable_map;
	variable_map variables;
	variable_map indirect_variables;

	Value* RetVal;
	BasicBlock* CleanupBB;
	BasicBlock* CleanupEndBB;
	BasicBlock* ExceptionBB;
	BasicBlock* RetBB;
	Function* CurrentFunction;
	CGBuilder Builder;
	Value* Self;
	Value* ScopeSelf;

	CodeGenTypes &types;

	NSString *ReturnType;
	bool containsBlocks;

	virtual void SetParentScope(void) {}
	/**
	 * Intialises a Function object to be used as a Smalltalk method or block
	 * function.
	 */
	void InitialiseFunction(NSString *functionName,
	                        NSArray *arguments,
	                        NSArray *locals,
	                        NSString *typeEncoding=@"@@:",
	                        BOOL returnsRetained=false);
	/**
	* Maps a selector to a SmallInt function name.
	*/
	string FunctionNameFromSelector(NSString *sel);

	/**
	* Constructs a Smalltalk object from the specified Objective-C type.
	*/
	Value *BoxValue(CGBuilder *B, Value *V, NSString *typestr);

	/**
	* Constructs a C primitive from a Smalltalk object.
	*/
	Value *Unbox(CGBuilder *B, Function *F, Value *val, NSString *Type);

	/**
	* Construct C primitives from Smalltalk objects in an argument list.  Skips
	* the two implicit arguments in the type encoding if skipImplicit is true
	* (the argument list is then assumed to be a list of message arguments,
	* rather than function arguments).
	*/
	void UnboxArgs(CGBuilder *B, Function *F,
	               llvm::SmallVectorImpl<llvm::Value*> &argv,
	               llvm::SmallVectorImpl<llvm::Value*> &args, 
	               NSString *selTypes,
	               bool skipImplicit=true);

	/**
	 * Loads a value from a byref structure.
	 */
	llvm::Value *loadByRefPointer(llvm::Value *byRefPointer);
	/**
	 * Creates an on-stack byref structure pointing to an LKObject value.
	 */
	llvm::Value* emitByRefStructure(void);
	/**
	 * Creates and initializes a new local variable with the specified value,
	 * or with zero if there is no specified value.
	 */
	void initializeVariableWithValue(LKSymbol *aSym, llvm::Value *val);
	/**
	* Preform a real message send.  Receiver must be a real object, not a
	* SmallInt.  Assumes that there is only one possible type for the selector.
	*/
	Value *MessageSendId(CGBuilder *B, Value *receiver, NSString *selName,
		NSString *selTypes, llvm::SmallVectorImpl<llvm::Value*> &argv);
	/**
	* Preform a real message send.  Receiver must be a real object, not a
	* SmallInt.  Tries all of the possible types.
	*/
	Value *MessageSendId(CGBuilder &B, Value *receiver, NSString *selName,
			NSArray *selTypes, SmallVectorImpl<Value*> &argv);
	/**
	 * Send a message to something that may be a SmallInt or an Objective-C
	 * object.
	 */
	Value *MessageSend(CGBuilder *B, Function *F, Value *receiver, NSString 
	    *selName, NSArray *selTypes, SmallVectorImpl<Value*> &boxedArgs);
	/**
	 * Send a message with no arguments to something that is either a SmallInt or
	 * an Objective-C object.
	 */
	Value *MessageSend(CGBuilder *B, Function *F, Value *receiver, NSString 
	    *selName, NSArray *selTypes);
	/**
	 * Cleans up a variable at the end of a method.
	 */
	void releaseVariable(llvm::Value *val);
	/**
	 * Debugging function - emits a printf statement with the string and the
	 * extra argument.
	 */
	void CreatePrintf(CGBuilder &Builder, NSString *str, Value *val);
public:
	/**/
	/**
	 * Returns the block's object.
	 */
	virtual llvm::Value *LoadBlockContext(void);
	/**
	 * Ends the current lexical scope.
	 */
	void EndScope(void);
	CodeGenSubroutine(CodeGenModule *Mod) : CGM(Mod),
	Builder(Mod->Context), Self(0), ScopeSelf(0), types(*Mod->types),
	containsBlocks(false) {}
	/**
	 * Load an argument at the specified index.
	 */
	Value *LoadArgumentAtIndex(unsigned index, unsigned depth);

	/**
	 * Send a message to the superclass.
	 */
	Value *MessageSendSuper(NSString *selName, NSString *selTypes,
			SmallVectorImpl<Value*> &argv);

	/**
	 * Send a message to an Objective-C object.  
	 */
	Value *MessageSendId(Value *receiver, NSString *selName,
		NSArray *selTypes, llvm::SmallVectorImpl<llvm::Value*> &argv);

	/**
	 * Send a message to a Smalltalk object.
	 */
	Value *MessageSend(Value *receiver, NSString *selName, NSArray
	    *selTypes, SmallVectorImpl<Value*> &boxedArgs);

	/**
	 * Call a C function.
	 */
	Value *CallFunction(NSString *functionName, NSString *argTypes,
	    SmallVectorImpl<Value*> &boxedArgs);
	/**
	 * Set the return value for this method / block.
	 */
	virtual void SetReturn(Value * Ret = 0);

	/**
	 * Load the value of self in the current context.
	 */
	virtual Value *LoadSelf(void);
	
	/**
	 * Get a pointer to the class object for a specified name.
	 */
	Value *LoadClass(NSString *classname);

	/**
	 * Load a class variable.
	 */
	Value *LoadClassVariable(NSString *className, NSString *cVarName);
	/**
	 * Store a value in a class variable.
	 */
	void StoreValueInClassVariable(NSString *className, NSString *cVarName, Value
			*object);

	/**
	 * Load an instance variable.
	 */
	Value *LoadValueOfTypeAtOffsetFromObject(NSString *className, NSString 
			*ivarName, NSString* type, unsigned offset, Value *object);


	/**
	 * Store an instance value.
	 */
	void StoreValueOfTypeAtOffsetFromObject(Value *value, NSString *className,
			NSString *ivarName, NSString *type, unsigned offset, Value
			*object);

	/**
	 * Stores a value in a local variable.
	 */
	void storeValueInVariable(llvm::Value *value, NSString *aVariable);
	/**
	 * Returns the value in the named variable.
	 */
	llvm::Value* loadVariable(NSString *aVariable);
	/**
	 * Clean up after a block.
	 */
	void EndChildBlock(CodeGenBlock *block);
	/**
	 * Create a symbol object.
	 */
	Value *SymbolConstant(NSString *symbol);
	/**
	 * Create a floating point constant.
	 */
	Value *FloatConstant(NSString *value);
	/**
	 * Create an integer constant.
	 */
	Value *IntConstant(NSString *value);
	/**
	 * Returns the parent lexical scope.
	 */
	virtual CodeGenSubroutine *getParentScope() { return 0; }

	/**
	 * Compare two pointers for equality.
	 */
	Value *ComparePointers(Value *lhs, Value *rhs);


	/**
	 * Creates a new basic block with the specified name and returns a pointer
	 * to the block.
	 */
	BasicBlock *StartBasicBlock(NSString* BBName);
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
	/**
	 * Split code flow depending on whether an object is a real object or a
	 * small integer.  Sets the insert point for aBuilder to the new basic
	 * block for the object cast, and sets the insert point for smallIntBuilder
	 * to the basic block for the small integer case.
	 */
	void splitSmallIntCase(llvm::Value *anObject,
	                       CGBuilder &aBuilder,
	                       CGBuilder &smallIntBuilder);
	/**
	 * Combines two basic blocks, one for an object case and one for a small
	 * int case.  If anObject is not NULL, then a PHI node will be created in
	 * the basic block combining the two cases and will be set to the object /
	 * small int.
	 *
	 * This function sets the insert point of the object builder to the start
	 * of the continuation block, after the PHI nodes.
	 */
	void combineSmallIntCase(llvm::Value *anObject,
	                         llvm::Value *aSmallInt,
	                         llvm::PHINode *&phi,
	                         CGBuilder &objectBuilder,
	                         CGBuilder &smallIntBuilder);
	virtual ~CodeGenSubroutine();
};

class CodeGenMethod : public CodeGenSubroutine {
public:
	CodeGenMethod(NSString *methodName,
	              NSString *functionName,
	              NSArray *locals,
	              NSArray *arguments,
	              NSString *signature,
	              bool isClass,
	              CodeGenModule *Mod);
};
class CodeGenFunction : public CodeGenSubroutine
{
public:
	CodeGenFunction(NSString *functionName,
                    NSArray *locals,
                    NSArray *arguments,
                    NSString *signature,
                    CodeGenModule *Mod);
};
}}
#endif 
