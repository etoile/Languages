#include "llvm/ADT/SmallVector.h"
#include "llvm/Support/IRBuilder.h"

namespace llvm {
  class BasicBlock;
  class Function;
  class Module;
  class Type;
  class Value;
}

class CodeGenModule;

class CodeGenBlock {
  llvm::SmallVector<llvm::Value*, 8> Args;
  llvm::SmallVector<llvm::Value*, 8> Locals;
  const llvm::Type *BlockTy;
  llvm::Module *TheModule;
  llvm::Value *BlockSelf;
public:
  llvm::Value *Block;
  llvm::Function *BlockFn;
  llvm::IRBuilder<> *Builder;
  llvm::Value *RetVal;
  llvm::BasicBlock *CleanupBB;

  CodeGenBlock(llvm::Module *M, int args, int locals, llvm::Value **promoted,
    int count, llvm::Value *Self, llvm::IRBuilder<> *MethodBuilder,
    CodeGenModule *CGM);

  llvm::Value *LoadArgumentAtIndex(unsigned index);

  void SetReturn(llvm::Value* RetVal);

  llvm::Value *LoadBlockVar(unsigned index, unsigned offset) ;

  llvm::Value *EndBlock(void); 
};
