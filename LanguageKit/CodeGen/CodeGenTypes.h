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
		LLVMPointerTy *idTy;
		/**
		 * Pointer to something.
		 */
		LLVMPointerTy *ptrToVoidTy;
		/**
		 * Type used for pointers to object pointers.
		 */
		LLVMPointerTy *ptrToIdTy;
		/**
		 * Type used for selectors.
		 */
		LLVMPointerTy *selTy;
		/**
		  * LLVM type for C char.
		  */
		LLVMIntegerTy *charTy;
		/**
		  * LLVM type for C short.
		  */
		LLVMIntegerTy *shortTy;
		/**
		  * LLVM type for C int.
		  */
		LLVMIntegerTy *intTy;
		/**
		  * LLVM type for C long.
		  */
		LLVMIntegerTy *longTy;
		/**
		  * LLVM type for C long long.
		  */
		LLVMIntegerTy *longLongTy;
		/**
		  * Type of pointer-sized integers.
		  */
		LLVMIntegerTy *intPtrTy;
		/**
		 * Type for pointer subtraction results.
		 */
		LLVMIntegerTy *ptrDiffTy;
		/**
		 * The type for a byref structure.
		 */
		LLVMStructTy *genericByRefType;
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
