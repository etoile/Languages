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
	virtual void SetParentScope(void);
public:
  virtual CodeGenLexicalScope *getParentScope() { return parentScope; }
  llvm::Value *Block;

  CodeGenBlock(int args, int locals, CodeGenLexicalScope *enclosingScope,
		  CodeGenModule *Mod);

  void SetReturn(llvm::Value* RetVal);

  llvm::Value *EndBlock(void); 
};
