#include "CodeGenBlock.h"
#include "CodeGenModule.h"
#include "LLVMCompat.h"
#include <llvm/Module.h>
#import "../LanguageKit.h"


using namespace llvm;
namespace etoile
{
namespace languagekit
{

// Store V in structure S element index
static inline Value *storeInStruct(
		IRBuilder<> *B, Value *S, Value *V, unsigned index)
{
	return B->CreateStore(V, B->CreateStructGEP(S, index));
}

llvm::Constant* CodeGenBlock::emitBlockDescriptor(NSString* signature,
                                                 llvm::StructType* blockType)
{
	llvm::SmallVector<LLVMType*, 2> argTypes;
	argTypes.push_back(llvm::PointerType::getUnqual(blockType));
	argTypes.push_back(llvm::PointerType::getUnqual(blockType));
	llvm::Function *copy =
			llvm::Function::Create(llvm::FunctionType::get(types.voidTy, argTypes, false),
			                       llvm::GlobalValue::PrivateLinkage,
			                       ".copy_block",
			                       CGM->TheModule);
	argTypes.pop_back();
	llvm::Function *dispose =
			llvm::Function::Create(llvm::FunctionType::get(types.voidTy, argTypes, false),
			                       llvm::GlobalValue::PrivateLinkage,
			                       ".dispose_block",
			                       CGM->TheModule);

	llvm::Function::arg_iterator AI = copy->arg_begin();
	llvm::Value* copyDest = AI;
	AI++;
	llvm::Value* copySrc = AI;
	llvm::Value* disposeSrc = dispose->arg_begin();
	CGBuilder copyBuilder(llvm::BasicBlock::Create(CGM->Context, "entry", copy));
	CGBuilder disposeBuilder(llvm::BasicBlock::Create(CGM->Context, "entry", dispose));
	llvm::Constant* copyFn =
		CGM->TheModule->getOrInsertFunction("_Block_object_assign", types.voidTy,
			types.ptrToVoidTy, types.ptrToVoidTy, types.intTy, NULL);
	llvm::Constant* disposeFn =
		CGM->TheModule->getOrInsertFunction("_Block_object_dispose", types.voidTy,
			types.ptrToVoidTy, types.intTy, NULL);
	// BLOCK_FIELD_IS_BYREF
	llvm::ConstantInt *byref = llvm::ConstantInt::get(types.intTy, 8);
	// BLOCK_FIELD_IS_OBJECT
	llvm::ConstantInt *object = llvm::ConstantInt::get(types.intTy, 3);
	// The self pointer is not byref, so we need a slightly different copy
	// function for it.
	llvm::Value* srcField = copyBuilder.CreateStructGEP(copySrc, 6);
	llvm::Value* destField = copyBuilder.CreateStructGEP(copyDest, 6);
	srcField = copyBuilder.CreateLoad(srcField);
	srcField = copyBuilder.CreateBitCast(srcField, types.ptrToVoidTy);
	destField = copyBuilder.CreateBitCast(destField, types.ptrToVoidTy);
	copyBuilder.CreateCall3(copyFn, destField, srcField, object);

	// ...and byref deletion
	srcField = disposeBuilder.CreateStructGEP(disposeSrc, 6);
	srcField = disposeBuilder.CreateLoad(srcField);
	srcField = disposeBuilder.CreateBitCast(srcField, types.ptrToVoidTy);
	disposeBuilder.CreateCall2(disposeFn, srcField, object);

	// Now we copy all of the byref structures.  
	for (unsigned int i=7 ; i<blockType->getNumElements() ; i++)
	{
		// We call into a helper function to do the actual byref copying.  
		srcField = copyBuilder.CreateStructGEP(copySrc, i);
		destField = copyBuilder.CreateStructGEP(copyDest, i);
		srcField = copyBuilder.CreateLoad(srcField);
		srcField = copyBuilder.CreateBitCast(srcField, types.ptrToVoidTy);
		destField = copyBuilder.CreateBitCast(destField, types.ptrToVoidTy);
		copyBuilder.CreateCall3(copyFn, destField, srcField, byref);

		// ...and byref deletion
		srcField = disposeBuilder.CreateStructGEP(disposeSrc, i);
		srcField = disposeBuilder.CreateLoad(srcField);
		srcField = disposeBuilder.CreateBitCast(srcField, types.ptrToVoidTy);
		disposeBuilder.CreateCall2(disposeFn, srcField, byref);
	}
	disposeBuilder.CreateRetVoid();
	copyBuilder.CreateRetVoid();

	llvm::StructType *descriptorTy = GetStructType(CGM->Context,
	                                               types.longTy,        // reserved
	                                               types.longTy,        // size
	                                               copy->getType(),     // copy
	                                               dispose->getType(),  // dispose
	                                               types.ptrToVoidTy,   // signature
	                                               NULL);
	llvm::Constant *sig = CGM->MakeConstantString(signature);
	llvm::Constant *size = llvm::ConstantExpr::getSizeOf(blockType);
	if (size->getType() != types.longTy)
	{
		size = llvm::ConstantExpr::getTrunc(size, types.longTy);
	}
	llvm::Constant *descriptorInit =
		llvm::ConstantStruct::get(descriptorTy, 
			llvm::ConstantInt::get(types.longTy, 0),
			size,
			copy,
			dispose,
			sig,
			NULL);
	return new GlobalVariable(*CGM->TheModule, descriptorTy, true,
		llvm::GlobalValue::PrivateLinkage, descriptorInit, "blockDescriptor");
}

CodeGenBlock::CodeGenBlock(NSArray *locals,
                           NSArray *arguments,
                           NSArray *bound,
                           NSString *signature,
                           CodeGenModule *Mod)
	: CodeGenSubroutine(Mod)
{
	// Create the block function
	LKSymbol *blockSelf = [LKSymbol new];
	[blockSelf setName: @"..block_self"];

	// First argument is the block pointer.  Don't confuse this with the self
	// variable in the enclosing scope (which may not exist).
	// TODO: Add a mechanism for front ends to expose this to the language.
	NSMutableArray *realArguments =
		[NSMutableArray arrayWithObject: blockSelf];
	[realArguments addObjectsFromArray: arguments];
	InitialiseFunction(@".block_invoke", realArguments, locals, signature);
	LLVMPointerType *invokeTy = (CurrentFunction->getType());
	// Define the layout of a block
	llvm::SmallVector<LLVMType*, 16> fields;
	fields.push_back(types.ptrToIdTy);            // 0 isa
	fields.push_back(types.intTy);                // 1 flags
	fields.push_back(types.intTy);                // 2 reserved
	fields.push_back(invokeTy);                   // 3 invoke
	fields.push_back(types.ptrToVoidTy);          // 4 Descriptor
	// TODO: Remove these if they're not required
	fields.push_back(types.ptrToVoidTy);          // 5 Context pointer
	fields.push_back(types.idTy);                 // 6 self (may be nil)
	for (LKSymbol *symbol in bound)
	{
		fields.push_back(types.idTy);
	}
	blockStructureTy = llvm::StructType::get(CGM->Context, fields);
	llvm::PointerType *blockPtrTy = llvm::PointerType::getUnqual(blockStructureTy);

	//blockContext = Builder.CreateBitCast(variables[@"..block_self"], blockPtrTy);
	blockContext = Builder.CreateBitCast(CurrentFunction->arg_begin(), blockPtrTy);
	// Self is bound directly.
	Self = Builder.CreateStructGEP(blockContext, 6);
	// Context pointer for non-local returns
	Context = Builder.CreateLoad(Builder.CreateStructGEP(blockContext, 5));

	// Create the block structure in the parent scope.
	CodeGenSubroutine *parent = CGM->ScopeStack.back();
	CGBuilder &b = parent->Builder;
	variable_map &vars = parent->indirect_variables;
	block = b.CreateAlloca(blockStructureTy);
	block->setMetadata(CGM->types->valueIsBlock, CGM->types->valueIsBlockNode);
	// Set the isa pointer to _NSConcreteStackBlock
	b.CreateStore(CGM->TheModule->getOrInsertGlobal("_NSConcreteStackBlock", types.idTy),
		b.CreateStructGEP(block, 0));
	// Flags: BLOCK_HAS_SIGNATURE, BLOCK_HAS_COPY_DISPOSE
	b.CreateStore(llvm::ConstantInt::get(types.intTy, (1 << 25) | (1<<30)),
		b.CreateStructGEP(block, 1));
	// Reserved is 0-initialised
	b.CreateStore(llvm::ConstantInt::get(types.intTy, 0),
		b.CreateStructGEP(block, 2));
	// The function pointer
	b.CreateStore(CurrentFunction, b.CreateStructGEP(block, 3));
	// The block descriptor
	b.CreateStore(b.CreateBitCast(emitBlockDescriptor(signature, blockStructureTy), types.ptrToVoidTy),
		b.CreateStructGEP(block, 4));
	b.CreateStore(b.CreateBitCast(parent->Context, types.ptrToVoidTy), b.CreateStructGEP(block, 5));
	b.CreateStore(parent->LoadSelf(), b.CreateStructGEP(block, 6));

	int i = 7;
	// All of the indirect symbols that are imported into this block are byref
	// structures, except self.  
	for (LKSymbol *symbol in bound)
	{
		NSString *name = [symbol name];
		indirect_variables[name] = 
			Builder.CreateLoad(Builder.CreateBitCast(
				Builder.CreateStructGEP(blockContext, i),
				llvm::PointerType::getUnqual(llvm::PointerType::getUnqual(types.genericByRefType))));
		llvm::Value *val = vars[name];
		val = b.CreateBitCast(val, types.idTy);
		llvm::Value *addr = b.CreateStructGEP(block, i);
		b.CreateStore(val, addr);
		i++;
	}
}
llvm::Value* CodeGenBlock::LoadBlockContext(void)
{
	return Builder.CreateBitCast(blockContext, types.idTy);
}

void CodeGenBlock::SetReturn(Value* RetVal)
{
	// FIXME: Throw the non-local return.  Requires getting some pointer from
	// the frame where the block was created.  We can probably do that by just
	// having a NULL object that's always bound into blocks as a non-byref
	// construct
	llvm::Value *returnFn = CGM->TheModule->getOrInsertFunction("__LanguageKitThrowNonLocalReturn",
		types.voidTy, types.idTy, types.idTy, NULL);
	Builder.CreateCall2(returnFn, 
		Builder.CreateBitCast(Context, types.idTy),
		Builder.CreateBitCast(RetVal, types.idTy));
}

void CodeGenBlock::SetBlockReturn(Value* RetVal) 
{
	CodeGenSubroutine::SetReturn(RetVal);
}

llvm::Value *CodeGenBlock::EndBlock(void)
{
	EndScope();
	return block;
}
}}
