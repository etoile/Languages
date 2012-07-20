#include "llvm/ADT/SmallVector.h"
#include "CodeGenModule.h"
#include "CodeGenLexicalScope.h"

namespace llvm {
  class BasicBlock;
  class Function;
  class Module;
  class Type;
  class Value;
  class StructureType;
  class FunctionType;
}

namespace etoile
{
namespace languagekit
{

class CodeGenModule;

/**
 * Class responsible for emitting blocks.  The outer scope is responsible for
 * generating any block byref structures for bound arguments, this will
 * generate the types for the block and 
 */
class CodeGenBlock : public CodeGenSubroutine {
	/**
	 * The type of this block's structure.
	 */
	LLVMStructType *blockStructureTy;
	/**
	 * A pointer to the block object, in its own scope.
	 */
	llvm::Value *blockContext;
	/**
	 * A pointer to the block object, in the parent's scope.
	 */
	llvm::Value *block;
	/**
	 * Emits the descriptor for this block.  The descriptor contains the block
	 * type encoding, along with the functions required to copy and dispose of
	 * the block.
	 */
	llvm::Constant* emitBlockDescriptor(NSString *signature,
	                                    llvm::StructType *blockType);
public:
	virtual void SetReturn(Value* RetVal);
	/**
	 * Returns the block's object.
	 */
	virtual llvm::Value *LoadBlockContext(void);

	/**
	 * Begins generating a block.  The arguments and locals contain an array of
	 * LKSymbol objects representing the local and argument values in this
	 * block.  Bound variables are passed in when the block is created.
	 */
	CodeGenBlock(NSArray *locals,
	             NSArray *arguments,
	             NSArray *bound,
	             NSString *signature,
	             CodeGenModule *Mod);

	void SetBlockReturn(llvm::Value* RetVal);

	/**
	 * Creates an on-stack instance of the block, with all of the bound
	 * variables attached.
	 */
	llvm::Value *EndBlock(void);
};
}}
