#include "llvm/ADT/SmallVector.h"
#include "llvm/Support/IRBuilder.h"
#include "CodeGenModule.h"
#include "CodeGenLexicalScope.h"

namespace llvm {
  class BasicBlock;
  class Function;
  class Module;
  class Type;
  class Value;
}

class CodeGenModule;

class CodeGenBlock : public CodeGenLexicalScope {
  const llvm::Type *BlockTy;
  CodeGenLexicalScope *parentScope;
public:
  llvm::Value *Block;

  CodeGenBlock(int args, int locals, llvm::Value **promoted,
    int count, CodeGenLexicalScope *enclosingScope, CodeGenModule *Mod);

  llvm::Value *LoadArgumentAtIndex(unsigned index);

  void SetReturn(llvm::Value* RetVal);
  virtual Value *LoadSelf(); 

  llvm::Value *LoadBlockVar(unsigned index, unsigned offset) ;

  llvm::Value *EndBlock(void); 
};
