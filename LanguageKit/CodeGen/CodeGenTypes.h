#import "LLVMCompat.h"
#import "ABIInfo.h"
@class NSString;
namespace llvm
{
	class Module;
	class PointerType;
	class IntegerType;
}

namespace etoile
{
namespace languagekit
{
	struct CodeGenTypes
	{
		llvm::Module &Mod;
		/**
		 * ABI Information provider.
		 */
		ABIInfo *AI;
		/**
		 * LLVM void type.
		 */
		LLVMType *voidTy;
		/**
		 * Type used for object pointers.
		 */
		LLVMPointerType *idTy;
		/**
		 * Pointer to something.
		 */
		LLVMPointerType *ptrToVoidTy;
		/**
		 * Type used for pointers to object pointers.
		 */
		LLVMPointerType *ptrToIdTy;
		/**
		 * Type used for selectors.
		 */
		LLVMPointerType *selTy;
		/**
		  * LLVM type for C char.
		  */
		LLVMIntegerType *charTy;
		/**
		  * LLVM type for C short.
		  */
		LLVMIntegerType *shortTy;
		/**
		  * LLVM type for C int.
		  */
		LLVMIntegerType *intTy;
		/**
		  * LLVM type for C long.
		  */
		LLVMIntegerType *longTy;
		/**
		  * LLVM type for C long long.
		  */
		LLVMIntegerType *longLongTy;
		/**
		  * Type of pointer-sized integers.
		  */
		LLVMIntegerType *intPtrTy;
		/**
		 * Type for pointer subtraction results.
		 */
		LLVMIntegerType *ptrDiffTy;
		/**
		 * The type for a byref structure.
		 */
		LLVMStructType *genericByRefType;
		/**
		 * Some zeros to reuse.
		 */
		llvm::Value *zeros[2];
		CodeGenTypes(llvm::Module &M);
		/**
		 * Returns a function type for the specified Objective-C type encoding.
		 */
		llvm::FunctionType* functionTypeFromString(NSString *typestr,
                                                   bool &isSRet,
		                                           LLVMType *&realRetTy);
		/**
		 * Returns a type encoding for the first value in the specified type
		 * encoding.  For example, @:@ will return the type encoding for @
		 * (id), not a function type.
		 */
		LLVMType *typeFromString(NSString *typeEncoding);
  
		/**
		 * Indicates that the current value is a block.  If we pass it to a
		 * function or a method, or store it on the heap or in a global, then we
		 * need to call objc_retainBlock().
		 */
		unsigned int valueIsBlock;
		llvm::MDNode *valueIsBlockNode;
	};
}
}
