#import <Foundation/Foundation.h>
#import "CodeGenAssignments.h"
#include <ctype.h>
#include <llvm/Module.h>

using namespace etoile::languagekit;

static inline bool isInMethodFamily(const char *sel, const char *prefix, size_t selSize)
{
	size_t len = strlen(prefix);
	if (selSize < len) { return false; }
	// If this value doesn't start with the prefix, give up
	if (strncmp(sel, prefix, len) != 0) { return false; }
	// If it does stat with the prefix, then it must either be the correct
	// length, or it must be either an exact match, or the next character must
	// be uppercase.
	if (selSize > len)
	{
		if (isupper(sel[len])) { return true; }
		return false;
	}
	return true;
}

namespace etoile
{
	namespace languagekit
	{
MethodFamily MethodFamilyForSelector(NSString *aSelector)
{
	NSUInteger length = [aSelector length];
	char buffer[13];
	[aSelector getCString: buffer maxLength: 13 encoding: NSUTF8StringEncoding];
	char *sel = buffer;
	if (buffer[0] == '_')
	{
		sel++;
		length--;
	}
	if (isInMethodFamily(sel, "new", length)) { return New; }
	if (isInMethodFamily(sel, "copy", length)) { return Copy; }
	if (isInMethodFamily(sel, "init", length)) { return Init; }
	if (isInMethodFamily(sel, "alloc", length)) { return Alloc; }
	if (isInMethodFamily(sel, "mutableCopy", length)) { return MutableCopy; }
	return None;
}
bool SelectorReturnsRetained(NSString *aSelector)
{
	switch (MethodFamilyForSelector(aSelector))
	{
		case Alloc:
		case Copy:
		case MutableCopy:
		case New:
		case Init:
			return true;
		case None:
		default:
			return false;
	}
}
}}

static inline llvm::Value* ensureType(CGBuilder &aBuilder,
                                     llvm::Value *aValue,
                                     llvm::Type *aType)
{
	if (aValue->getType() == aType) { return aValue; }
	return aBuilder.CreateBitCast(aValue, aType);
}

etoile::languagekit::CodeGenAssignments::~CodeGenAssignments() {}

namespace {

/**
 * Assignment class that implements the barriers required for ARC.
 */
struct ARCAssignments : public CodeGenAssignments
{
	/**
	 * ARC strong assignment:
	 *
	 * id objc_storeStrong(id *addr, id value);
	 */
	llvm::Constant *storeStrong;
	/**
	 * ARC weak assignment:
	 *
	 * id objc_storeWeak(id *addr, id value);
	 */
	llvm::Constant *storeWeak;
	/**
	  * ARC weak read barrier:
	  *
	  * id objc_loadWeak(id* object);
	  */
	llvm::Constant *loadWeak;
	/**
	  * Retain then autorelease a value for returning.
	  *
	  * id objc_retainAutoreleaseReturnValue(id)
	  */
	llvm::Constant *autoreleaseRV;
	/**
	  * Retain a value that was previously autoreleased and then returned.
	  *
	  * id objc_retainAutoreleasedReturnValue(id)
	  */
	llvm::Constant *retainAutoreleasedRV;
	/**
	 * Retains a value.
	 *
	 * id objc_retain(id)
	 */
	llvm::Constant *retain;
	/**
	 * Release a value.
	 *
	 * void objc_release(id)
	 */
	llvm::Constant *release;
	/**
	 * Autorelease a value.
	 *
	 * id objc_autorelease(id)
	 */
	llvm::Constant *autorelease;


	ARCAssignments(CodeGenTypes &T) : CodeGenAssignments(T)
	{
		storeWeak = 
			Mod.getOrInsertFunction("objc_storeWeak", Types.idTy, Types.ptrToIdTy, Types.idTy, NULL);
		loadWeak = 
			Mod.getOrInsertFunction("objc_loadWeak", Types.idTy, Types.ptrToIdTy, NULL);
		autoreleaseRV = 
			Mod.getOrInsertFunction("objc_autoreleaseReturnValue", Types.idTy, Types.idTy, NULL);
		retainAutoreleasedRV = 
			Mod.getOrInsertFunction("objc_retainAutoreleasedReturnValue", Types.idTy, Types.idTy, NULL);
		retain = 
			Mod.getOrInsertFunction("objc_retain", Types.idTy, Types.idTy, NULL);
		release = 
			Mod.getOrInsertFunction("objc_release", Types.voidTy, Types.idTy, NULL);
		autorelease = 
			Mod.getOrInsertFunction("objc_autorelease", Types.idTy, Types.idTy, NULL);
		// None of the ARC functions can throw.
		llvm::cast<llvm::Function>(storeWeak->stripPointerCasts())->setDoesNotThrow();
		llvm::cast<llvm::Function>(loadWeak->stripPointerCasts())->setDoesNotThrow();
		llvm::cast<llvm::Function>(autoreleaseRV->stripPointerCasts())->setDoesNotThrow();
		llvm::cast<llvm::Function>(retainAutoreleasedRV->stripPointerCasts())->setDoesNotThrow();
		llvm::cast<llvm::Function>(release->stripPointerCasts())->setDoesNotThrow();
		llvm::cast<llvm::Function>(autorelease->stripPointerCasts())->setDoesNotThrow();
		llvm::cast<llvm::Function>(retain->stripPointerCasts())->setDoesNotThrow();
	}
	void storeValue(CGBuilder &aBuilder,
	                llvm::Value *anAddr,
	                llvm::Value *aValue,
	                bool isWeak)
	{
		anAddr = ensureType(aBuilder, anAddr, Types.ptrToIdTy);
		aValue = ensureType(aBuilder, aValue, Types.idTy);
		if (isWeak)
		{
			aBuilder.CreateCall2(storeWeak, anAddr, aValue);
		}
		else
		{
			// We could use objc_storeStrong() here, but the ARC optimiser will
			// generate that when it makes sense.  It is easier for the
			// optimiser to spot and remove balanced retain / release
			// operations if they are separate.
			llvm::Value *retained = aBuilder.CreateCall(retain, aValue);
			aBuilder.CreateCall(release, aBuilder.CreateLoad(anAddr));
			aBuilder.CreateStore(retained, anAddr);
		}
	}
	llvm::Value* loadValue(CGBuilder &aBuilder,
	                       llvm::Value *anAddr,
	                       bool isWeak)
	{
		anAddr = ensureType(aBuilder, anAddr, Types.ptrToIdTy);
		if (isWeak)
		{
			return aBuilder.CreateCall(loadWeak, anAddr);
		}
		return aBuilder.CreateLoad(anAddr);
	}
	virtual void storeGlobal(CGBuilder &aBuilder,
	                         llvm::Value *aGlobal,
	                         llvm::Value *aValue,
	                         bool isWeak)
	{
		storeValue(aBuilder, aGlobal, aValue, isWeak);
	}
	virtual llvm::Value *loadGlobal(CGBuilder &aBuilder,
	                                llvm::Value *aGlobal,
	                                bool isWeak)
	{
		return loadValue(aBuilder, aGlobal, isWeak);
	}
	virtual void storeIvar(CGBuilder &aBuilder,
	                       llvm::Value *anObject,
	                       llvm::Value *anOffset,
	                       llvm::Value *aValue,
	                       bool isWeak)
	{
		storeValue(aBuilder,
		           computeOffset(aBuilder, anObject, anOffset),
		           aValue,
		           isWeak);
	}
	virtual llvm::Value *loadIvar(CGBuilder &aBuilder,
	                              llvm::Value *anObject,
	                              llvm::Value *anOffset,
	                              bool isWeak)
	{
		return loadValue(aBuilder,
		                 computeOffset(aBuilder, anObject, anOffset),
		                 isWeak);
	}
	virtual void storeLocal(CGBuilder &aBuilder,
	                        llvm::Value *aLocal,
	                        llvm::Value *aValue,
	                        bool isWeak)
	{
		storeValue(aBuilder, aLocal, aValue, isWeak);
	}
	virtual llvm::Value *loadLocal(CGBuilder &aBuilder,
	                               llvm::Value *aLocal,
	                               bool isWeak)
	{
		return loadValue(aBuilder, aLocal, isWeak);
	}
	virtual void disposeLocal(CGBuilder &aBuilder,
	                          llvm::Value *aLocal,
	                          bool isWeak)
	{
		storeValue(aBuilder, aLocal, llvm::ConstantPointerNull::get(Types.idTy), isWeak);
	}
	virtual llvm::Value *autoreleaseReturnValue(CGBuilder &aBuilder,
	                                            llvm::Value *aValue)
	{
		return aBuilder.CreateCall(autoreleaseRV,
		                           ensureType(aBuilder, aValue, Types.idTy));
	}
	virtual llvm::Value *retainResult(CGBuilder &aBuilder,
	                                  NSString* aSelector,
	                                  llvm::Value *aValue)
	{
		if (!SelectorReturnsRetained(aSelector))
		{
			ensureType(aBuilder, aValue, Types.idTy);
			aValue = aBuilder.CreateCall(retainAutoreleasedRV, aValue);
		}
		ensureType(aBuilder, aValue, Types.idTy);
		aValue = aBuilder.CreateCall(autorelease, aValue);
		return aValue;
	}
	virtual void releaseValue(CGBuilder &aBuilder, llvm::Value *aValue)
	{
		ensureType(aBuilder, aValue, Types.idTy);
		aBuilder.CreateCall(release, aValue);
	}
	virtual llvm::Value* retainValue(CGBuilder &aBuilder, llvm::Value *aValue)
	{
		ensureType(aBuilder, aValue, Types.idTy);
		return aBuilder.CreateCall(retain, aValue);
	}
};
/**
  * Assignment class that implements the 
  */
struct GCAssignments : public CodeGenAssignments
{
	/**
	  * Global write barrier function:
	  * 
	  * id objc_assign_global(id val, id *ptr);
	  */
	llvm::Constant *assignGlobal;
	/**
	  * Ivar assignment write barrier:
	  *
	  * id objc_assign_ivar(id val, id dest, ptrdiff_t offset);
	  */
	llvm::Constant *assignIvar;
	/**
	  * Weak write barrier.
	  *
	  * id objc_assign_weak(id value, id *location);
	  */
	llvm::Constant *assignWeak;
	/**
	  * Weak read barrier.
	  * 
	  * id objc_read_weak(id *location);
	  */
	llvm::Constant *readWeak;

	GCAssignments(CodeGenTypes &T) : CodeGenAssignments(T)
	{

		assignIvar = Mod.getOrInsertFunction( "objc_assign_ivar",
				Types.idTy, Types.idTy, Types.idTy, Types.ptrDiffTy, NULL);
		assignGlobal = Mod.getOrInsertFunction("objc_assign_global",
				Types.idTy, Types.idTy, Types.ptrToIdTy, NULL);
		assignWeak = Mod.getOrInsertFunction("objc_assign_weak",
				Types.idTy, Types.idTy, Types.ptrToIdTy, NULL);
		readWeak = Mod.getOrInsertFunction("objc_read_weak",
				Types.idTy, Types.ptrToIdTy, NULL);
	}
	void storeWeak(CGBuilder &aBuilder,
	               llvm::Value *anAddr,
	               llvm::Value *aValue)
	{
		anAddr = ensureType(aBuilder, anAddr, Types.ptrToIdTy);
		aValue = ensureType(aBuilder, aValue, Types.idTy);
		aBuilder.CreateCall2(assignWeak, aValue, anAddr);
	}
	llvm::Value* loadValue(CGBuilder &aBuilder,
	                       llvm::Value *anAddr,
	                       bool isWeak)
	{
		anAddr = ensureType(aBuilder, anAddr, Types.ptrToIdTy);
		if (isWeak)
		{
			return aBuilder.CreateCall(readWeak, anAddr);
		}
		return aBuilder.CreateLoad(anAddr);
	}
	virtual void storeGlobal(CGBuilder &aBuilder,
	                         llvm::Value *aGlobal,
	                         llvm::Value *aValue,
	                         bool isWeak)
	{
		if (isWeak)
		{
			storeWeak(aBuilder, aGlobal, aValue);
		}
		aBuilder.CreateCall2(assignGlobal,
		                     ensureType(aBuilder, aValue, Types.idTy),
		                     ensureType(aBuilder, aGlobal, Types.ptrToIdTy));
	}
	virtual llvm::Value *loadGlobal(CGBuilder &aBuilder,
	                                llvm::Value *aGlobal,
	                                bool isWeak)
	{
		return loadValue(aBuilder, aGlobal, isWeak);
	}
	virtual void storeIvar(CGBuilder &aBuilder,
	                       llvm::Value *anObject,
	                       llvm::Value *anOffset,
	                       llvm::Value *aValue,
	                       bool isWeak)
	{
		if (isWeak)
		{
			storeWeak(aBuilder,
			          computeOffset(aBuilder, anObject, anOffset),
			          aValue);
			return;
		}
		aBuilder.CreateCall3(assignIvar, 
		                     ensureType(aBuilder, aValue, Types.idTy),
		                     ensureType(aBuilder, anObject, Types.idTy),
		                     ensureType(aBuilder, anOffset, Types.ptrDiffTy));
	}
	virtual llvm::Value *loadIvar(CGBuilder &aBuilder,
	                              llvm::Value *anObject,
	                              llvm::Value *anOffset,
	                              bool isWeak)
	{
		return loadValue(aBuilder,
		                 computeOffset(aBuilder, anObject, anOffset),
		                 isWeak);
	}
	virtual void storeLocal(CGBuilder &aBuilder,
	                        llvm::Value *aLocal,
	                        llvm::Value *aValue,
	                        bool isWeak)
	{
		aLocal = ensureType(aBuilder, aLocal, Types.ptrToIdTy);
		aValue = ensureType(aBuilder, aValue, Types.idTy);
		aBuilder.CreateStore(aValue, aLocal);
	}
	virtual llvm::Value *loadLocal(CGBuilder &aBuilder,
	                               llvm::Value *aLocal,
	                               bool isWeak)
	{
		aLocal = ensureType(aBuilder, aLocal, Types.ptrToIdTy);
		return aBuilder.CreateLoad(aLocal);
	}
	virtual void disposeLocal(CGBuilder &aBuilder,
	                          llvm::Value *aLocal,
	                          bool isWeak) {}
	virtual llvm::Value *autoreleaseReturnValue(CGBuilder &aBuilder,
	                                            llvm::Value *aValue)
	{
		return aValue;
	}
	virtual llvm::Value *retainResult(CGBuilder &aBuilder,
	                                  NSString* aSelector,
	                                  llvm::Value *aValue)
	{
		return aValue;
	}
	virtual void releaseValue(CGBuilder &aBuilder, llvm::Value *aValue) {}
	virtual llvm::Value* retainValue(CGBuilder &aBuilder, llvm::Value *aValue)
	{
		return aValue;
	}
};
}
CodeGenAssignments*
etoile::languagekit::CodeGenAssignments::Create(CodeGenTypes &types,
                                                bool garbageCollection)
{
	if (garbageCollection)
	{
		return new GCAssignments(types);
	}
	return new ARCAssignments(types);
}
