/**
 * Compatibility header that wraps LLVM API breakage and lets us compile with
 * old and new versions of LLVM.
 *
 * First LLVM version supported is 2.9.
 */

#ifndef __LANGUAGEKIT_LLVM_HACKS__
#define __LANGUAGEKIT_LLVM_HACKS__
#if (LLVM_MAJOR > 3) || (LLVM_MAJOR == 3 && LLVM_MINOR >= 3)
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Metadata.h>
#include <llvm/IR/Intrinsics.h>
#include <llvm/IR/Module.h>
#else
#include <llvm/Instructions.h>
#include <llvm/Metadata.h>
#include <llvm/Intrinsics.h>
#include <llvm/Module.h>
#endif
#if (LLVM_MAJOR == 3) && (LLVM_MINOR > 1) || (LLVM_MAJOR > 3)
#if (LLVM_MAJOR > 3) || (LLVM_MAJOR == 3 && LLVM_MINOR >= 3)
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#else
#include <llvm/IRBuilder.h>
#include <llvm/Module.h>
#include <llvm/LLVMContext.h>
#endif
#include <llvm/DebugInfo.h>
#else
#include <llvm/Support/IRBuilder.h>
#include <llvm/Analysis/DebugInfo.h>
#endif

// Only preserve names in a debug build.  This simplifies the
// IR in a release build, but makes it much harder to debug.
#ifndef DEBUG
	typedef llvm::IRBuilder<false> CGBuilder;
#else
	typedef llvm::IRBuilder<> CGBuilder;
#endif

__attribute((unused)) static inline
llvm::PHINode* CreatePHI(llvm::Type *Ty,
                         unsigned NumReservedValues,
                         const llvm::Twine &NameStr="",
                         llvm::Instruction *InsertBefore=0) {
#if LLVM_MAJOR < 3
    llvm::PHINode *phi = llvm::PHINode::Create(Ty, NameStr, InsertBefore);
    phi->reserveOperandSpace(NumReservedValues);
    return phi;
#else
    return llvm::PHINode::Create(Ty, NumReservedValues, NameStr, InsertBefore);
#endif
}

__attribute((unused)) static inline
llvm::PHINode* IRBuilderCreatePHI(CGBuilder *Builder,
                                  llvm::Type *Ty,
                                  unsigned NumReservedValues,
                                  const llvm::Twine &NameStr="")
{
#if LLVM_MAJOR < 3
	llvm::PHINode *phi = Builder->CreatePHI(Ty, NameStr);
	phi->reserveOperandSpace(NumReservedValues);
	return phi;
#else
	return Builder->CreatePHI(Ty, NumReservedValues, NameStr);
#endif
}



__attribute((unused)) static inline
llvm::MDNode* CreateMDNode(llvm::LLVMContext &C,
                           llvm::Value **V,
                            unsigned length=1) {
#if LLVM_MAJOR < 3
	return llvm::MDNode::get(C, V, length);
#else
	llvm::ArrayRef<llvm::Value*> val(V, length);
	return llvm::MDNode::get(C, val);
#endif
}

template<typename T>
static inline
llvm::InvokeInst* IRBuilderCreateInvoke(CGBuilder *Builder,
                                        llvm::Value *callee,
                                        llvm::BasicBlock *dest,
                                        llvm::BasicBlock *dest2,
                                        T values,
                                        const llvm::Twine &NameStr="")
{
#if LLVM_MAJOR < 3
	return Builder->CreateInvoke(callee, dest, dest2, values.begin(), values.end(), NameStr);
#else
	return Builder->CreateInvoke(callee, dest, dest2, values, NameStr);
#endif
}

template<typename T>
static inline
llvm::CallInst* IRBuilderCreateCall(CGBuilder *Builder,
                                    llvm::Value *callee,
                                    T values,
                                    const llvm::Twine &NameStr="")
{
#if LLVM_MAJOR < 3
	return Builder->CreateCall(callee, values.begin(), values.end(), NameStr);
#else
	return Builder->CreateCall(callee, values, NameStr);
#endif
}


#if LLVM_MAJOR < 3
#define GetStructType(context, ...) StructType::get(context, __VA_ARGS__)
#else
#define GetStructType(context, ...) StructType::get(__VA_ARGS__)
#endif

#if LLVM_MAJOR < 3
typedef const llvm::Type LLVMType;
typedef const llvm::StructType LLVMStructTy;
typedef const llvm::ArrayType LLVMArrayTy;
typedef const llvm::PointerType LLVMPointerTy;
typedef const llvm::IntegerType LLVMIntegerTy;
#else
typedef llvm::Type LLVMType;
typedef llvm::StructType LLVMStructTy;
typedef llvm::ArrayType LLVMArrayTy;
typedef llvm::PointerType LLVMPointerTy;
typedef llvm::IntegerType LLVMIntegerTy;
#endif

#endif
