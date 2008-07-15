#include "CodeGenBlock.h"
#include "CodeGenModule.h"
#include <llvm/Support/IRBuilder.h>
#include <llvm/Module.h>


using namespace llvm;

CodeGenBlock::CodeGenBlock(Module *M, int args, int locals, Value **promoted, int count,
    IRBuilder *MethodBuilder) {
  TheModule = M;
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

  // Create the block object
  Function *BlockCreate =
    cast<Function>(TheModule->getOrInsertFunction("NewBlock", IdTy,
          (void*)0));
  Block = MethodBuilder->CreateCall(BlockCreate);
  Block = MethodBuilder->CreateBitCast(Block, BlockTy);
  // Create the block function
  BlockFn = Function::Create(BlockFunctionTy, GlobalValue::InternalLinkage,
      "BlockFunction", TheModule);
  BasicBlock * EntryBB = llvm::BasicBlock::Create("entry", BlockFn);
  Builder = new IRBuilder(EntryBB);

  // Set up the arguments
  InitialiseFunction(Builder, BlockFn, BlockSelf, Args, Locals,
      locals, RetVal, CleanupBB);
  MethodBuilder->CreateStore(ConstantInt::get(Type::Int32Ty, args),
      MethodBuilder->CreateStructGEP(Block, 4));

  // Store the block function in the object
  Value *FunctionPtr = MethodBuilder->CreateStructGEP(Block, 1);
  MethodBuilder->CreateStore(MethodBuilder->CreateBitCast(BlockFn, IMPTy),
      FunctionPtr);
  
  //FIXME: I keep calling these promoted when I mean bound.  Change all of
  //the variable / method names to reflect this.

  //Store the promoted vars in the block
  Value *promotedArray = MethodBuilder->CreateStructGEP(Block, 2);
  // FIXME: Reference self, promote self
  for (int i=1 ; i<count ; i++) {
    MethodBuilder->CreateStore(promoted[i], 
        MethodBuilder->CreateStructGEP(promotedArray, i));
  }
}

Value *CodeGenBlock::LoadArgumentAtIndex(unsigned index) {
  return Builder->CreateLoad(Args[index]);
}

void CodeGenBlock::SetReturn(Value* RetVal) {
  const Type *RetTy = BlockFn->getReturnType();
  if (RetVal == 0) {
      Builder->CreateRet(UndefValue::get(BlockFn->getReturnType()));
  } else {
    if (RetVal->getType() != RetTy) {
      RetVal = Builder->CreateBitCast(RetVal, RetTy);
    }
    Builder->CreateRet(RetVal);
  }
}

Value *CodeGenBlock::LoadBlockVar(unsigned index, unsigned offset) {
  Value *block = Builder->CreateLoad(BlockSelf);
  // Object array
  Value *object = Builder->CreateStructGEP(block, 2);
  object = Builder->CreateStructGEP(object, index);
  object = Builder->CreateLoad(object);
  if (offset > 0)
  {
    Value *addr = Builder->CreatePtrToInt(object, IntTy);
    addr = Builder->CreateAdd(addr, ConstantInt::get(IntTy, offset));
    addr = Builder->CreateIntToPtr(addr, PointerType::getUnqual(IdTy));
    return Builder->CreateLoad(addr);
  }
  object = Builder->CreateLoad(object);
  return object;
}

Value *CodeGenBlock::EndBlock(void) {
  return Block;
}

