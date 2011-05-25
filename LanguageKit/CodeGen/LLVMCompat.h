/**
 * Compatibility header that wraps LLVM API breakage and lets us compile with
 * old and new versions of LLVM.
 */

#include <llvm/Instructions.h>

__attribute((unused)) static inline 
llvm::PHINode* CreatePHI(const llvm::Type *Ty,
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
