#include "CodeGenBlock.h"
#include "CodeGenModule.h"
#include <llvm/Support/IRBuilder.h>
#include <llvm/Module.h>


using namespace llvm;


CodeGenBlock::CodeGenBlock(int args, int locals, llvm::Value **promoted, int
		count, CodeGenLexicalScope *enclosingScope, CodeGenModule *Mod) :
	CodeGenLexicalScope(Mod), parentScope(enclosingScope) {

  const Type *IdPtrTy = PointerType::getUnqual(IdTy);
  BlockTy = StructType::get(
      IdTy,                          // 0 - isa.
      IMPTy,                         // 1 - Function pointer.
      ArrayType::get(IdPtrTy, 5),    // 2 - Bound variables.
      ArrayType::get(IdTy, 5),       // 3 - Promoted variables.
      Type::Int32Ty,                 // 4 - Number of args.
      IdTy,                          // 5 - Return value.
      Type::Int8Ty,                  // 6 - Start of jump buffer.
      (void*)0);
  BlockTy = PointerType::getUnqual(BlockTy);
  std::vector<const Type*> argTy;
  argTy.push_back(BlockTy);

  // FIXME: Broken on Etoile runtime.
  argTy.push_back(SelTy);
  for (int i=0 ; i<args ; ++i) {
    argTy.push_back(IdTy);
  }
  FunctionType *BlockFunctionTy = FunctionType::get(IdTy, argTy, false);

  IRBuilder<> *MethodBuilder = enclosingScope->getBuilder();
  // Create the block object
  Function *BlockCreate =
    cast<Function>(CGM->getModule()->getOrInsertFunction("NewBlock", IdTy,
          (void*)0));
  Block = MethodBuilder->CreateCall(BlockCreate);
  Block = MethodBuilder->CreateBitCast(Block, BlockTy);
  // Create the block function
  CurrentFunction = Function::Create(BlockFunctionTy,
		  GlobalValue::InternalLinkage, "BlockFunction", CGM->getModule());

  // Set up the arguments
  InitialiseFunction(Args, Locals, locals);
  MethodBuilder->CreateStore(ConstantInt::get(Type::Int32Ty, args),
      MethodBuilder->CreateStructGEP(Block, 4));

  // Store the block function in the object
  Value *FunctionPtr = MethodBuilder->CreateStructGEP(Block, 1);
  MethodBuilder->CreateStore(MethodBuilder->CreateBitCast(CurrentFunction,
			  IMPTy), FunctionPtr);
  
  //FIXME: I keep calling these promoted when I mean bound.  Change all of
  //the variable / method names to reflect this.

  //Store pointers to the bound vars in the block
  Value *boundArray = MethodBuilder->CreateStructGEP(Block, 2);
  Value *promotedArray = MethodBuilder->CreateStructGEP(Block, 3);

  for (int i=1 ; i<count ; i++) {
    MethodBuilder->CreateStore(promoted[i], 
        MethodBuilder->CreateStructGEP(boundArray, i));
  }
  //Reference self, promote self
  MethodBuilder->CreateStore(enclosingScope->LoadSelf(),
      MethodBuilder->CreateStructGEP(promotedArray, 0));
  MethodBuilder->CreateStore(MethodBuilder->CreateStructGEP(promotedArray, 0),
      MethodBuilder->CreateStructGEP(boundArray, 0));
}

Value *CodeGenBlock::LoadArgumentAtIndex(unsigned index) {
  return Builder.CreateLoad(Args[index]);
}

void CodeGenBlock::SetReturn(Value* RetVal) {
  const Type *RetTy = CurrentFunction->getReturnType();
  if (RetVal == 0) {
      Builder.CreateRet(UndefValue::get(CurrentFunction->getReturnType()));
  } else {
    if (RetVal->getType() != RetTy) {
      RetVal = Builder.CreateBitCast(RetVal, RetTy);
    }
    Builder.CreateRet(RetVal);
  }
}

Value *CodeGenBlock::LoadSelf() {
	return LoadBlockVar(0, 0);
}
Value *CodeGenBlock::LoadBlockVar(unsigned index, unsigned offset) {
  Value *block = Builder.CreateLoad(Self);
  // Object array
  Value *object = Builder.CreateStructGEP(block, 2);
  object = Builder.CreateStructGEP(object, index);
  object = Builder.CreateLoad(object);
  if (offset > 0)
  {
    Value *addr = Builder.CreatePtrToInt(object, IntTy);
    addr = Builder.CreateAdd(addr, ConstantInt::get(IntTy, offset));
    addr = Builder.CreateIntToPtr(addr, PointerType::getUnqual(IdTy));
    return Builder.CreateLoad(addr);
  }
  object = Builder.CreateLoad(object);
  return object;
}

Value *CodeGenBlock::EndBlock(void) {
  parentScope->EndChildBlock(this);
  return Block;
}
