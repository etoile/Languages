#include "CodeGenTypes.h"

namespace llvm
{
	class Value;
}

namespace etoile
{
	namespace languagekit
	{
		/**
		 * Returns the method family for a given selector.
		 */
		enum MethodFamily
		{
			Alloc,
			Copy,
			Init,
			MutableCopy,
			New,
			None
		};
		/**
		 * Returns the method family for a given method.
		 */
		MethodFamily MethodFamilyForSelector(NSString *aSelector);
		/**
		 * Returns whether a method returns a retained object.
		 */
		bool SelectorReturnsRetained(NSString *aSelector);

		/**
		 * Class used to generate read and write barriers for objects.  
		 */
		class CodeGenAssignments
		{
			protected:
			/** The module that this class will manipulate. */
			llvm::Module &Mod;
			/** Cached types. */
			CodeGenTypes &Types;
			/**
			 * Constructor.  Should not be called directly - get a concrete
			 * subclass via the Create() function.
			 */
			CodeGenAssignments(CodeGenTypes &T) : Mod(T.Mod), Types(T) {}
			/**
			 * Computes the address of an ivar from the object address and an offset.
			 */
			llvm::Value* computeOffset(CGBuilder &Builder,
			                           llvm::Value *anObject,
			                           llvm::Value *anOffset)
			{
				llvm::Value *addr = Builder.CreatePtrToInt(anObject, Types.intPtrTy);
				addr = Builder.CreateAdd(addr, anOffset);
				return Builder.CreateIntToPtr(addr, Types.ptrToIdTy);
			}
			public:
			/**
			 * Creates a new assignment helper.  Pass true as the first
			 * argument to enable garbage collection.
			 */
			static CodeGenAssignments* Create(CodeGenTypes &types, bool garbageCollection=false);
			/**
			 * Stores a value in the specified global.
			 */
			virtual void storeGlobal(CGBuilder &Builder,
			                         llvm::Value *aGlobal,
			                         llvm::Value *aValue,
			                         bool isWeak=false) = 0;
			/**
			 * Loads a value from a global.  Note that this is not required for
			 * classes or constant strings.
			 */
			virtual llvm::Value *loadGlobal(CGBuilder &Builder,
			                                llvm::Value *aGlobal,
			                                bool isWeak=false) = 0;
			/**
			 * Stores a value in an instance variable.
			 */
			virtual void storeIvar(CGBuilder &Builder,
			                       llvm::Value *anObject,
			                       llvm::Value *anOffset,
			                       llvm::Value *aValue,
			                       bool isWeak=false) = 0;
			/**
			 * Loads a value from an instance variable.
			 */
			virtual llvm::Value *loadIvar(CGBuilder &Builder,
			                              llvm::Value *anObject,
			                              llvm::Value *anOffset,
			                              bool isWeak=false) = 0;
			/**
			 * Stores a value into a local variable.  In ARC mode, we always
			 * perform retain values stored in locals, and then let the
			 * optimiser clean them up.
			 */
			virtual void storeLocal(CGBuilder &Builder,
			                        llvm::Value *aLocal,
			                        llvm::Value *aValue,
			                        bool isWeak=false) = 0;
			/**
			 * Loads a value from a local.
			 */
			virtual llvm::Value *loadLocal(CGBuilder &Builder,
			                               llvm::Value *aLocal,
			                               bool isWeak=false) = 0;
			/**
			 * Disposes of a local.  This should be called for each retained
			 * local in the cleanup section.
			 */
			virtual void disposeLocal(CGBuilder &Builder,
			                          llvm::Value *aLocal,
			                          bool isWeak=false) = 0;
			/**
			 * Retains and then autoreleases a return value.
			 */
			virtual llvm::Value *autoreleaseReturnValue(CGBuilder &Builder,
			                                            llvm::Value *aValue) = 0;
			/**
			 * Retains the result of a method return.  This lets us always
			 * retain autoreleased return values, and free them in the cleanup.
			 * The ARC optimiser will then remove a balanced retain / release
			 * later.
			 */
			virtual llvm::Value *retainResult(CGBuilder &aBuilder,
			                                  NSString* aSelector,
			                                  llvm::Value *aValue) = 0;
			/**
			 * Releases a temporary value.
			 */
			virtual void releaseValue(CGBuilder &aBuilder, llvm::Value *aValue) = 0;
			/**
			 * Retains a temporary value.
			 */
			virtual llvm::Value* retainValue(CGBuilder &aBuilder, llvm::Value *aValue) = 0;
			/**
			 * Casts a block to an object, so that it can be used with the
			 * normal memory management strategies.
			 */
			virtual llvm::Value *castBlockToObject(CGBuilder &aBuilder, llvm::Value *aValue) = 0;
			virtual ~CodeGenAssignments();
		};
	}
}
