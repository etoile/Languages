#include "CodeGenBlock.h"
#include "CodeGenModule.h"
#include <llvm/Support/IRBuilder.h>
#include <llvm/Module.h>


using namespace llvm;

// Store V in structure S element index
static inline Value *storeInStruct(
		IRBuilder<> *B, Value *S, Value *V, unsigned index)
{
	return B->CreateStore(V, B->CreateStructGEP(S, index));
}

CodeGenBlock::CodeGenBlock(int args, int locals, CodeGenLexicalScope
		*enclosingScope, CodeGenModule *Mod) 
	: CodeGenLexicalScope(Mod), parentScope(enclosingScope) 
{
	Value *enclosingContext = enclosingScope->getContext();
	// Define the layout of a block
	BlockTy = StructType::get(
		Mod->Context,
		IdTy,                          // 0 - isa.
		IMPTy,                         // 1 - Function pointer.
		Type::getInt32Ty(Mod->Context),// 2 - Number of args.
		enclosingContext->getType(),   // 3 - Context.
		(void*)0);
	std::vector<const Type*> argTy;
	argTy.push_back(PointerType::getUnqual(BlockTy));

	// FIXME: Broken on Etoile runtime - _cmd needs to be a GEP on _call
	argTy.push_back(SelTy);
	for (int i=0 ; i<args ; ++i) 
	{
		argTy.push_back(IdTy);
	}
	FunctionType *BlockFunctionTy = FunctionType::get(IdTy, argTy, false);

	IRBuilder<> *MethodBuilder = enclosingScope->getBuilder();

	// Create the block object
	
	// The NewBlock function gets a block from a pool.  It should really be
	// inlined.
	Block = MethodBuilder->CreateAlloca(BlockTy);

	Module *TheModule = CGM->getModule();
	// Create the block function
	CurrentFunction = Function::Create(BlockFunctionTy,
		GlobalValue::InternalLinkage, "BlockFunction", TheModule);
	InitialiseFunction(Args, Locals, locals);

	// Set the isa pointer
	Value *isa = MethodBuilder->CreateLoad(
		TheModule->getGlobalVariable(".smalltalk_block_stack_class", true));
	storeInStruct(MethodBuilder, Block, isa, 0);

	// Store the block function in the object
	storeInStruct(MethodBuilder, Block,
		MethodBuilder->CreateBitCast(CurrentFunction, IMPTy), 1);
	// Store the number of arguments
	storeInStruct(MethodBuilder, Block, 
			ConstantInt::get(Type::getInt32Ty(Mod->Context), args), 2);
	// Set the context
	storeInStruct(MethodBuilder, Block, enclosingScope->getContext(), 3);


}
void CodeGenBlock::SetParentScope(void)
{
	// Link the context to its parent
	Value *parentContext = Builder.CreateLoad(Builder.CreateStructGEP(ScopeSelf, 3));
	Builder.CreateStore(parentContext, Builder.CreateStructGEP(Context, 1));
}

void CodeGenBlock::SetReturn(Value* RetVal)
{
	CGObjCRuntime *Runtime = CGM->getRuntime();
	SmallVector<Value*, 1> args = SmallVector<Value*, 1>(1, RetVal);
	Runtime->GenerateMessageSend(Builder, Type::getVoidTy(CGM->Context), false, NULL, ScopeSelf,
		Runtime->GetSelector(Builder, "nonLocalReturn:", 0), args, ExceptionBB);
}

void CodeGenBlock::SetBlockReturn(Value* RetVal) 
{
	CodeGenLexicalScope::SetReturn(RetVal);
}

Value *CodeGenBlock::EndBlock(void)
{
	BasicBlock *block = Builder.GetInsertBlock();
	if (NULL != block && NULL == block->getTerminator())
	{
		Builder.CreateBr(CleanupBB);
	}
	EndScope();
	parentScope->EndChildBlock(this);
	return Block;
}
