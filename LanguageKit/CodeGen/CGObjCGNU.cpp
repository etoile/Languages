//===----------------------------------------------------------------------===//
//
// This provides Objective-C code generation targetting the GNU runtime.  The
// class in this file generates structures used by the GNU Objective-C runtime
// library.  These structures are defined in objc/objc.h and objc/objc-api.h in
// the GNU runtime distribution.
//
//===----------------------------------------------------------------------===//

#include "ABI.h"
#include "CGObjCRuntime.h"
#include "llvm/Module.h"
#include "llvm/Support/Compiler.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/Target/TargetData.h"
#include <map>
#include <string>
#include <algorithm>
#include <dlfcn.h>

using namespace llvm;
using namespace std;

// The version of the runtime that this class targets.  Must match the version
// in the runtime.
static const int RuntimeVersion = 8;
static const int ProtocolVersion = 2;

static bool enable_sender_dispatch = false;

namespace {
class CGObjCGNU : public CGObjCRuntime {
private:
	llvm::LLVMContext &Context;
	llvm::Module &TheModule;
	const llvm::StructType *SelStructTy;
	const llvm::Type *SelectorTy;
	const llvm::Type *PtrToInt8Ty;
	const llvm::Type *IMPTy;
	const llvm::Type *IdTy;
	const llvm::Type *IntTy;
	const llvm::Type *PtrTy;
	const llvm::Type *LongTy;
	const llvm::Type *PtrToIntTy;
	std::vector<llvm::Constant*> Classes;
	std::vector<llvm::Constant*> Categories;
	std::vector<llvm::Constant*> ConstantStrings;
	llvm::Function *LoadFunction;
	llvm::StringMap<llvm::Constant*> ExistingProtocols;
	typedef std::pair<std::string, std::string> TypedSelector;
	std::map<TypedSelector, llvm::GlobalAlias*> TypedSelectors;
	llvm::StringMap<llvm::GlobalAlias*> UntypedSelectors;
	std::map<llvm::Value*, const char*> SelectorNames;
	// Some zeros used for GEPs in lots of places.
	llvm::Constant *Zeros[2];
	llvm::Constant *NULLPtr;
private:
	llvm::Constant *GenerateIvarList(
		const llvm::SmallVectorImpl<std::string> &IvarNames,
		const llvm::SmallVectorImpl<std::string> &IvarTypes,
		const llvm::SmallVectorImpl<int> &IvarOffsets);
	llvm::Constant *GenerateMethodList(const std::string &ClassName,
		const std::string &CategoryName,
		const llvm::SmallVectorImpl<std::string> &MethodNames, 
		const llvm::SmallVectorImpl<std::string> &MethodTypes, 
		bool isClassMethodList);
	llvm::Constant *GenerateProtocolList(
		const llvm::SmallVectorImpl<std::string> &Protocols);
	llvm::Constant *GenerateClassStructure(
		llvm::Constant *MetaClass,
		llvm::Constant *SuperClass,
		unsigned info,
		const char *Name,
		llvm::Constant *Version,
		llvm::Constant *InstanceSize,
		llvm::Constant *IVars,
		llvm::Constant *Methods,
		llvm::Constant *Protocols,
		bool isMeta);
	llvm::Constant *GenerateProtocolMethodList(
		const llvm::SmallVectorImpl<llvm::Constant *> &MethodNames,
		const llvm::SmallVectorImpl<llvm::Constant *> &MethodTypes);
	llvm::Constant *MakeConstantString(const std::string &Str, const std::string
		&Name="");
	llvm::Constant *MakeGlobal(const llvm::StructType *Ty,
		std::vector<llvm::Constant*> &V, const std::string &Name="",
		bool isPublic=false);
	llvm::Constant *MakeGlobal(const llvm::ArrayType *Ty,
		std::vector<llvm::Constant*> &V, const std::string &Name="");
	llvm::Value *GetWeakSymbol(const std::string &Name,
                               llvm::Type *Type);
public:
	CGObjCGNU(llvm::Module &Mp,
		llvm::LLVMContext &C,
		const llvm::Type *LLVMIntType,
		const llvm::Type *LLVMLongType);
	virtual llvm::Constant *GenerateConstantString(const char *String, 
		const size_t length);
	virtual llvm::Value *GenerateMessageSend(llvm::IRBuilder<> &Builder,
	                                         const llvm::Type *ReturnTy,
	                                         bool isSRet,
	                                         llvm::Value *Sender,
	                                         llvm::Value *Receiver,
	                                         llvm::Value *Selector,
	                                         llvm::SmallVectorImpl<llvm::Value*> &ArgV,
	                                         llvm::BasicBlock *CleanupBlock,
	                                         const char *ReceiverClass,
	                                         bool isClassMessage);
	virtual llvm::Value *GenerateMessageSendSuper(llvm::IRBuilder<> &Builder,
	                                              const llvm::Type *ReturnTy,
	                                              bool isSRet,
	                                              llvm::Value *Sender,
	                                              const char *SuperClassName,
	                                              llvm::Value *Receiver,
	                                              llvm::Value *Selector,
	                                              llvm::SmallVectorImpl<llvm::Value*> &ArgV,
	                                              bool isClassMessage,
	                                              llvm::BasicBlock *CleanupBlock);
	virtual llvm::Value *LookupClass(llvm::IRBuilder<> &Builder,
	                                 llvm::Value *ClassName);
	virtual llvm::Value *GetSelector(llvm::IRBuilder<> &Builder,
	                                 llvm::Value *SelName,
	                                 llvm::Value *SelTypes);
	virtual llvm::Value *GetSelector(llvm::IRBuilder<> &Builder,
	                                 const char *SelName,
	                                 const char *SelTypes);

	virtual llvm::Function *MethodPreamble(const std::string &ClassName,
	                                       const std::string &CategoryName,
	                                       const std::string &MethodName,
	                                       const llvm::Type *ReturnTy,
	                                       const llvm::Type *SelfTy,
	                                       const SmallVectorImpl<const llvm::Type*> &ArgTy,
	                                       bool isClassMethod,
	                                       bool isSRet,
	                                       bool isVarArg);
	virtual void GenerateCategory(
		const char *ClassName, const char *CategoryName,
		const llvm::SmallVectorImpl<std::string>  &InstanceMethodNames,
		const llvm::SmallVectorImpl<std::string>  &InstanceMethodTypes,
		const llvm::SmallVectorImpl<std::string>  &ClassMethodNames,
		const llvm::SmallVectorImpl<std::string>  &ClassMethodTypes,
		const llvm::SmallVectorImpl<std::string> &Protocols);
	virtual void DefineClassVariables(
		const string &ClassName,
		const llvm::SmallVectorImpl<std::string> &CvarNames,
		const llvm::SmallVectorImpl<std::string> &CvarTypes);
	virtual void GenerateClass(
		const char *ClassName,
		const char *SuperClassName,
		const int instanceSize,
		const llvm::SmallVectorImpl<std::string> &IvarNames,
		const llvm::SmallVectorImpl<std::string> &IvarTypes,
		const llvm::SmallVectorImpl<int> &IvarOffsets,
		const llvm::SmallVectorImpl<std::string> &InstanceMethodNames,
		const llvm::SmallVectorImpl<std::string> &InstanceMethodTypes,
		const llvm::SmallVectorImpl<std::string> &ClassMethodNames,
		const llvm::SmallVectorImpl<std::string> &ClassMethodTypes,
		const llvm::SmallVectorImpl<std::string> &Protocols);
	virtual llvm::Value *GenerateProtocolRef(llvm::IRBuilder<> &Builder,
	                                         const char *ProtocolName);
	virtual void GenerateProtocol(
		const char *ProtocolName,
		const llvm::SmallVectorImpl<std::string> &Protocols,
		const llvm::SmallVectorImpl<llvm::Constant *> &InstanceMethodNames,
		const llvm::SmallVectorImpl<llvm::Constant *> &InstanceMethodTypes,
		const llvm::SmallVectorImpl<llvm::Constant *> &ClassMethodNames,
		const llvm::SmallVectorImpl<llvm::Constant *> &ClassMethodTypes);
	virtual llvm::Function *ModuleInitFunction();
	virtual llvm::Value *LoadClassVariable(llvm::IRBuilder<> &Builder, 
	                                       string &ClassName,
	                                       string &CvarName);
	virtual void StoreClassVariable(llvm::IRBuilder<> &Builder, 
	                                string &ClassName,
	                                string &CvarName,
	                                llvm::Value* aValue);
};
} // end anonymous namespace


static std::string SymbolNameForSelector(const std::string &MethodName)
{
	string MethodNameColonStripped = MethodName;
	std::replace(MethodNameColonStripped.begin(), MethodNameColonStripped.end(), ':', '_');
	return MethodNameColonStripped;
}

static std::string SymbolNameForMethod(const std::string &ClassName, const
	std::string &CategoryName, const std::string &MethodName, bool isClassMethod)
{
	return std::string(isClassMethod ? "_c_" : "_i_") + ClassName + "_" +
		CategoryName + "_" + SymbolNameForSelector(MethodName);
}

CGObjCGNU::CGObjCGNU(llvm::Module &M,
                     llvm::LLVMContext &C,
                      const llvm::Type *LLVMIntType,
                      const llvm::Type *LLVMLongType) : 
                      Context(C),
                      TheModule(M),
                      IntTy(LLVMIntType),
                      LongTy(LLVMLongType)
{
	Zeros[0] = ConstantInt::get(llvm::Type::getInt32Ty(Context), 0);
	Zeros[1] = Zeros[0];
	NULLPtr = llvm::ConstantPointerNull::get(
		llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(Context)));
	// C string type.  Used in lots of places.
	PtrToInt8Ty = 
		llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(Context));
	// Get the selector Type.
	SelStructTy = llvm::StructType::get(
		  Context,
		  PtrToInt8Ty,
		  PtrToInt8Ty,
		  NULL);
	SelectorTy = llvm::PointerType::getUnqual(SelStructTy);
	PtrToIntTy = llvm::PointerType::getUnqual(IntTy);
	PtrTy = PtrToInt8Ty;
 
	// Object type
	llvm::PATypeHolder OpaqueObjTy = llvm::OpaqueType::get(Context);
	llvm::Type *OpaqueIdTy = llvm::PointerType::getUnqual(OpaqueObjTy);
	IdTy = llvm::StructType::get(Context, OpaqueIdTy, NULL);
	llvm::cast<llvm::OpaqueType>(OpaqueObjTy.get())->refineAbstractTypeTo(IdTy);
	IdTy = llvm::cast<llvm::StructType>(OpaqueObjTy.get());
	IdTy = llvm::PointerType::getUnqual(IdTy);
	// IMP type
	std::vector<const llvm::Type*> IMPArgs;
	IMPArgs.push_back(IdTy);
	IMPArgs.push_back(SelectorTy);
	IMPTy = llvm::FunctionType::get(IdTy, IMPArgs, true);

}

// This has to perform the lookup every time, since posing and related
// techniques can modify the name -> class mapping.
llvm::Value *CGObjCGNU::LookupClass(llvm::IRBuilder<> &Builder,
                                    llvm::Value *ClassName) 
{
	llvm::Constant *ClassLookupFn =
		TheModule.getOrInsertFunction("objc_lookup_class", IdTy, PtrToInt8Ty,
			NULL);
	return Builder.CreateCall(ClassLookupFn, ClassName);
}

/// Statically looks up the selector for the specified name / type pair.
llvm::Value *CGObjCGNU::GetSelector(llvm::IRBuilder<> &Builder,
                                    const char *SelName,
                                    const char *SelTypes) 
{
	// For static selectors, we return an alias for now then store them all in
	// a list that the runtime will initialise later.

	// Untyped selector
	if (SelTypes == 0) 
	{
		// If it's already cached, return it.
		if (UntypedSelectors[SelName]) 
		{
			return Builder.CreateLoad(UntypedSelectors[SelName]);
		}
		// If it isn't, cache it.
		llvm::GlobalAlias *Sel = new llvm::GlobalAlias(
				llvm::PointerType::getUnqual(SelectorTy),
				llvm::GlobalValue::InternalLinkage, SymbolNameForSelector(SelName),
				NULL, &TheModule);
		SelectorNames[Sel] = SelName;
		UntypedSelectors[SelName] = Sel;
		return Builder.CreateLoad(Sel);
	}

	// Typed selectors
	TypedSelector Selector = TypedSelector(SelName,
			SelTypes);

	// If it's already cached, return it.
	if (TypedSelectors[Selector]) 
	{
		return Builder.CreateLoad(TypedSelectors[Selector]);
	}

	// If it isn't, cache it.
	llvm::GlobalAlias *Sel = new llvm::GlobalAlias(
			llvm::PointerType::getUnqual(SelectorTy),
			llvm::GlobalValue::InternalLinkage, SymbolNameForSelector(SelName),
				NULL, &TheModule);
	SelectorNames[Sel] = SelName;
	TypedSelectors[Selector] = Sel;
	return Builder.CreateLoad(Sel);
}
/// Dynamically looks up the selector for the specified name / type pair.
llvm::Value *CGObjCGNU::GetSelector(llvm::IRBuilder<> &Builder,
                                    llvm::Value *SelName,
                                    llvm::Value *SelTypes) 
{
	// Dynamically look up selectors from non-constant sources
	llvm::Value *cmd;
	if (SelTypes == 0) 
	{
		llvm::Constant *SelFunction = 
			TheModule.getOrInsertFunction("sel_register_name", 
				SelectorTy, 
				PtrToInt8Ty, 
				NULL);
		cmd = Builder.CreateCall(SelFunction, SelName);
	}
	else 
	{
		llvm::Constant *SelFunction = 
			TheModule.getOrInsertFunction("sel_register_typed_name",
					SelectorTy,
					PtrToInt8Ty,
					PtrToInt8Ty,
					NULL);
		cmd = Builder.CreateCall2(SelFunction, SelName, SelTypes);
	}
	return cmd;
}

llvm::Constant *CGObjCGNU::MakeConstantString(const std::string &Str,
                                              const std::string &Name) 
{
	llvm::Constant * ConstStr = llvm::ConstantArray::get(Context, Str);
	ConstStr = new llvm::GlobalVariable(TheModule, ConstStr->getType(), true,
		llvm::GlobalValue::InternalLinkage, ConstStr, Name);
	return llvm::ConstantExpr::getGetElementPtr(ConstStr, Zeros, 2);
}

llvm::Constant *CGObjCGNU::MakeGlobal(const llvm::StructType *Ty,
                                      std::vector<llvm::Constant*> &V,
                                      const std::string &Name,
                                      bool isPublic)
{
	llvm::Constant *C = llvm::ConstantStruct::get(Ty, V);
	return new llvm::GlobalVariable(TheModule, Ty, false,
		(isPublic ? llvm::GlobalValue::ExternalLinkage :
		llvm::GlobalValue::InternalLinkage), C, Name);
}

llvm::Constant *CGObjCGNU::MakeGlobal(const llvm::ArrayType *Ty,
                                      std::vector<llvm::Constant*> &V,
                                      const std::string &Name) 
{
	llvm::Constant *C = llvm::ConstantArray::get(Ty, V);
	return new llvm::GlobalVariable(TheModule, Ty, false,
		llvm::GlobalValue::InternalLinkage, C, Name);
}
llvm::Value *CGObjCGNU::GetWeakSymbol(const std::string &Name,
                                               llvm::Type *Type)
{
	llvm::Constant *value;
	if(0 != (value = TheModule.getNamedGlobal(Name)))
	{
		if (value->getType() != Type)
		{
			value = llvm::ConstantExpr::getBitCast(value, Type);
		}
		return value;
	}
	return new llvm::GlobalVariable(TheModule, Type, false,
			llvm::GlobalValue::WeakAnyLinkage, 0, Name);
}

/// Generate an NSConstantString object.
llvm::Constant *CGObjCGNU::GenerateConstantString(const char *String,
		const size_t length)
{
	std::string Str(String, String +length);
	std::vector<llvm::Constant*> Ivars;
	Ivars.push_back(NULLPtr);
	Ivars.push_back(MakeConstantString(Str));
	Ivars.push_back(ConstantInt::get(IntTy, length));
	llvm::Constant *ObjCStr = MakeGlobal(
		llvm::StructType::get(Context, PtrToInt8Ty, PtrToInt8Ty, IntTy,
		NULL), Ivars, ".objc_str");
	ConstantStrings.push_back(
		llvm::ConstantExpr::getBitCast(ObjCStr, PtrToInt8Ty));
	return ObjCStr;
}
static llvm::Value *callIMP(LLVMContext &Context,
                            llvm::IRBuilder<> &Builder,
                            llvm::Value *imp,
                            const llvm::Type *ReturnTy,
                            bool isSRet,
                            llvm::Value *Receiver,
                            llvm::Value *Selector,
                            llvm::SmallVectorImpl<llvm::Value*> &ArgV,
                            llvm::BasicBlock *CleanupBlock)
{
	// Call the method
	llvm::SmallVector<llvm::Value*, 8> callArgs;
	llvm::Value *sret = 0;
	if (isSRet)
	{
		sret = Builder.CreateAlloca(ReturnTy);
		callArgs.push_back(sret);
	}
	callArgs.push_back(Receiver);
	callArgs.push_back(Selector);
	if (PASS_STRUCTS_AS_POINTER)
	{
		llvm::Value* callArg;
		for (unsigned int i = 0; i < ArgV.size() ; i++) {
			callArg = ArgV[i];
			if (isa<StructType>(callArg->getType()))
			{
				llvm::AllocaInst *StructPointer =
					Builder.CreateAlloca(callArg->getType());
				Builder.CreateStore(callArg, StructPointer);
				callArgs.push_back(StructPointer);
			}
			else
			{
				callArgs.push_back(callArg);
			}
		}
	}
	else
	{
		callArgs.insert(callArgs.end(), ArgV.begin(), ArgV.end());
	}
	llvm::Value *ret = 0;
	if (0 != CleanupBlock)
	{
		llvm::BasicBlock *continueBB =
			llvm::BasicBlock::Create(Context, "invoke_continue",
					Builder.GetInsertBlock()->getParent());
		ret = Builder.CreateInvoke(imp, continueBB, CleanupBlock,
			callArgs.begin(), callArgs.end());
		Builder.SetInsertPoint(continueBB);
		if (isSRet)
		{
			cast<llvm::InvokeInst>(ret)->addAttribute(1, Attribute::StructRet);
		}
	}
	else
	{
		ret = Builder.CreateCall(imp, callArgs.begin(), callArgs.end());
		if (isSRet)
		{
			cast<llvm::CallInst>(ret)->addAttribute(1, Attribute::StructRet);
		}
	}
	if (isSRet)
	{
		ret = Builder.CreateLoad(sret);
	}
	return ret;
}

///Generates a message send where the super is the receiver.  This is a message
///send to self with special delivery semantics indicating which class's method
///should be called.
llvm::Value *CGObjCGNU::GenerateMessageSendSuper(llvm::IRBuilder<> &Builder,
                                            const llvm::Type *ReturnTy,
                                            bool isSRet,
                                            llvm::Value *Sender,
                                            const char *SuperClassName,
                                            llvm::Value *Receiver,
                                            llvm::Value *Selector,
                                            llvm::SmallVectorImpl<llvm::Value*> &ArgV,
                                            bool isClassMessage,
                                            llvm::BasicBlock *CleanupBlock)
{
	// FIXME: Posing will break this.
	llvm::Value *ReceiverClass = LookupClass(Builder,
			MakeConstantString(SuperClassName));
	// If it's a class message, get the metaclass
	if (isClassMessage)
	{
		ReceiverClass = Builder.CreateBitCast(ReceiverClass,
				PointerType::getUnqual(IdTy));
		ReceiverClass = Builder.CreateLoad(ReceiverClass);
	}
	std::vector<const llvm::Type*> impArgTypes;
	if (isSRet)
	{
		impArgTypes.push_back(llvm::PointerType::getUnqual(ReturnTy));
	}
	impArgTypes.push_back(Receiver->getType());
	impArgTypes.push_back(SelectorTy);
	
	// Avoid an explicit cast on the IMP by getting a version that has the right
	// return type.
	llvm::FunctionType *impType = isSRet ?
		llvm::FunctionType::get(llvm::Type::getVoidTy(Context), impArgTypes, true) :
		llvm::FunctionType::get(ReturnTy, impArgTypes, true);
	// Construct the structure used to look up the IMP
	llvm::StructType *ObjCSuperTy = llvm::StructType::get(Context,
		Receiver->getType(), IdTy, NULL);
	llvm::Value *ObjCSuper = Builder.CreateAlloca(ObjCSuperTy);
	Builder.CreateStore(Receiver, Builder.CreateStructGEP(ObjCSuper, 0));
	Builder.CreateStore(ReceiverClass, Builder.CreateStructGEP(ObjCSuper, 1));

	// Get the IMP
	llvm::Constant *lookupFunction = 
		TheModule.getOrInsertFunction("objc_msg_lookup_super",
		                              llvm::PointerType::getUnqual(impType),
		                              llvm::PointerType::getUnqual(ObjCSuperTy),
		                              SelectorTy, NULL);


	llvm::Value *lookupArgs[] = {ObjCSuper, Selector};
	llvm::Value *imp = Builder.CreateCall(lookupFunction, lookupArgs,
		lookupArgs+2);
	llvm::cast<llvm::CallInst>(imp)->setOnlyReadsMemory();

	return callIMP(Context, Builder, imp, ReturnTy, isSRet, Receiver,
			Selector, ArgV, CleanupBlock);
}

/// Generate code for a message send expression.
llvm::Value *CGObjCGNU::GenerateMessageSend(llvm::IRBuilder<> &Builder,
                                            const llvm::Type *ReturnTy,
                                            bool isSRet,
                                            llvm::Value *Sender,
                                            llvm::Value *Receiver,
                                            llvm::Value *Selector,
                                            llvm::SmallVectorImpl<llvm::Value*> &ArgV,
                                            llvm::BasicBlock *CleanupBlock,
                                            const char *ReceiverClass,
                                            bool isClassMessage)
{

	// Look up the method implementation.
	std::vector<const llvm::Type*> impArgTypes;
	if (isSRet)
	{
		impArgTypes.push_back(llvm::PointerType::getUnqual(ReturnTy));
	}
	impArgTypes.push_back(Receiver->getType());
	impArgTypes.push_back(SelectorTy);
	
	// Avoid an explicit cast on the IMP by getting a version that has the right
	// return type.
	llvm::FunctionType *impType = isSRet ?
		llvm::FunctionType::get(llvm::Type::getVoidTy(Context), impArgTypes, true) :
		llvm::FunctionType::get(ReturnTy, impArgTypes, true);
	
	if (0 == Sender)
	{
		Sender = NULLPtr;
	}
	llvm::Value *ReceiverPtr = Builder.CreateAlloca(Receiver->getType());
	Builder.CreateStore(Receiver, ReceiverPtr);

	llvm::Type *SlotTy = llvm::StructType::get(Context, PtrTy, PtrTy, PtrTy,
			IntTy, llvm::PointerType::getUnqual(impType), NULL);

	llvm::Constant *lookupFunction = 
		TheModule.getOrInsertFunction("objc_msg_lookup_sender",
				llvm::PointerType::getUnqual(SlotTy), ReceiverPtr->getType(),
				Selector->getType(), Sender->getType(), NULL);
	// Lookup does not capture the receiver pointer
	if (llvm::Function *LookupFn = dyn_cast<llvm::Function>(lookupFunction)) 
	{
		LookupFn->setDoesNotCapture(1);
	}

	llvm::CallInst *slot = 
		Builder.CreateCall3(lookupFunction, ReceiverPtr, Selector, Sender);

	slot->setOnlyReadsMemory();

	llvm::Value *imp = Builder.CreateLoad(Builder.CreateStructGEP(slot, 4));

	Receiver = Builder.CreateLoad(ReceiverPtr);

	// Call the method.
	return callIMP(Context, Builder, imp, ReturnTy, isSRet, Receiver, Selector,
				ArgV, CleanupBlock);
}

/// Generates a MethodList.  Used in construction of a objc_class and 
/// objc_category structures.
llvm::Constant *CGObjCGNU::GenerateMethodList(
	const std::string &ClassName,
	const std::string &CategoryName, 
	const llvm::SmallVectorImpl<std::string>  &MethodNames, 
	const llvm::SmallVectorImpl<std::string>  &MethodTypes, 
	bool isClassMethodList) 
{
	// Get the method structure type.  
	llvm::StructType *ObjCMethodTy = llvm::StructType::get(
		Context,
		PtrToInt8Ty, // Really a selector, but the runtime creates it us.
		PtrToInt8Ty, // Method types
		llvm::PointerType::getUnqual(IMPTy), //Method pointer
		NULL);
	std::vector<llvm::Constant*> Methods;
	std::vector<llvm::Constant*> Elements;
	for (unsigned int i = 0, e = MethodTypes.size(); i < e; ++i) 
	{
		Elements.clear();
		Elements.push_back(MakeConstantString(MethodNames[i]));
		Elements.push_back(MakeConstantString(MethodTypes[i]));
		llvm::Constant *Method =
			TheModule.getFunction(SymbolNameForMethod(ClassName, CategoryName,
					 MethodNames[i], isClassMethodList));
		Method = llvm::ConstantExpr::getBitCast(Method,
				llvm::PointerType::getUnqual(IMPTy));
		Elements.push_back(Method);
		Methods.push_back(llvm::ConstantStruct::get(ObjCMethodTy, Elements));
	}

	// Array of method structures
	llvm::ArrayType *ObjCMethodArrayTy = llvm::ArrayType::get(ObjCMethodTy,
			MethodNames.size());
	llvm::Constant *MethodArray = llvm::ConstantArray::get(ObjCMethodArrayTy,
			Methods);

	// Structure containing list pointer, array and array count
	llvm::SmallVector<const llvm::Type*, 16> ObjCMethodListFields;
	llvm::PATypeHolder OpaqueNextTy = llvm::OpaqueType::get(Context);
	llvm::Type *NextPtrTy = llvm::PointerType::getUnqual(OpaqueNextTy);
	llvm::StructType *ObjCMethodListTy = llvm::StructType::get(
		Context,
		NextPtrTy, 
		IntTy, 
		ObjCMethodArrayTy,
		NULL);
	// Refine next pointer type to concrete type
	llvm::cast<llvm::OpaqueType>(
		OpaqueNextTy.get())->refineAbstractTypeTo(ObjCMethodListTy);
	ObjCMethodListTy = llvm::cast<llvm::StructType>(OpaqueNextTy.get());

	Methods.clear();
	Methods.push_back(llvm::ConstantPointerNull::get(
		llvm::PointerType::getUnqual(ObjCMethodListTy)));
	Methods.push_back(ConstantInt::get(llvm::Type::getInt32Ty(Context),
		MethodTypes.size()));
	Methods.push_back(MethodArray);
	
	// Create an instance of the structure
	return MakeGlobal(ObjCMethodListTy, Methods, ".objc_method_list");
}

/// Generates an IvarList.  Used in construction of a objc_class.
llvm::Constant *CGObjCGNU::GenerateIvarList(
	const llvm::SmallVectorImpl<std::string>  &IvarNames, const
	llvm::SmallVectorImpl<std::string>  &IvarTypes, const
	llvm::SmallVectorImpl<int>  &IvarOffsets) 
{
	if (0 == IvarNames.size())
	{
		return NULLPtr;
	}
	// Get the method structure type.  
	llvm::StructType *ObjCIvarTy = llvm::StructType::get(
		Context,
		PtrToInt8Ty,
		PtrToInt8Ty,
		IntTy,
		NULL);
	std::vector<llvm::Constant*> Ivars;
	std::vector<llvm::Constant*> Elements;
	for (unsigned int i = 0, e = IvarNames.size() ; i < e ; i++)
 	{
		Elements.clear();
		Elements.push_back(MakeConstantString(IvarNames[i]));
		Elements.push_back(MakeConstantString(IvarTypes[i]));
		Elements.push_back(ConstantInt::get(IntTy, IvarOffsets[i]));
		Ivars.push_back(llvm::ConstantStruct::get(ObjCIvarTy, Elements));
	}

	// Array of method structures
	llvm::ArrayType *ObjCIvarArrayTy = llvm::ArrayType::get(ObjCIvarTy,
		IvarNames.size());

	
	Elements.clear();
	Elements.push_back(ConstantInt::get(
		 llvm::cast<llvm::IntegerType>(IntTy), (int)IvarNames.size()));
	Elements.push_back(llvm::ConstantArray::get(ObjCIvarArrayTy, Ivars));
	// Structure containing array and array count
	llvm::StructType *ObjCIvarListTy = llvm::StructType::get(Context, IntTy,
		ObjCIvarArrayTy,
		NULL);

	// Create an instance of the structure
	return MakeGlobal(ObjCIvarListTy, Elements, ".objc_ivar_list");
}

/// Generate a class structure
llvm::Constant *CGObjCGNU::GenerateClassStructure(
	llvm::Constant *MetaClass,
	llvm::Constant *SuperClass,
	unsigned info,
	const char *Name,
	llvm::Constant *Version,
	llvm::Constant *InstanceSize,
	llvm::Constant *IVars,
	llvm::Constant *Methods,
	llvm::Constant *Protocols,
	bool isMeta)
{
	// Set up the class structure
	// Note:  Several of these are char*s when they should be ids.  This is
	// because the runtime performs this translation on load.
	llvm::StructType *ClassTy = llvm::StructType::get(
		Context,
		PtrToInt8Ty,            // class_pointer
		PtrToInt8Ty,            // super_class
		PtrToInt8Ty,            // name
		LongTy,                 // version
		LongTy,                 // info
		LongTy,                 // instance_size
		IVars->getType(),       // ivars
		Methods->getType(),     // methods
		// These are all filled in by the runtime, so we pretend 
		PtrTy,                  // dtable
		PtrTy,                  // subclass_list
		PtrTy,                  // sibling_class
		PtrTy,                  // protocols
		PtrTy,                  // gc_object_type
		//LongTy,                 // abi_version
		// Ivar offset pointers, to be filled in by the runtime
		//IvarOffsets->getType(), // ivar_offsets
		NULL);
	llvm::Constant *Zero = ConstantInt::get(LongTy, 0);
	llvm::Constant *NullP =
		llvm::ConstantPointerNull::get(llvm::cast<llvm::PointerType>(PtrTy));
	// Fill in the structure
	std::vector<llvm::Constant*> Elements;
	Elements.push_back(llvm::ConstantExpr::getBitCast(MetaClass, PtrToInt8Ty));
	Elements.push_back(SuperClass);
	if (NULL == Name)
	{
		Name = "AnonymousClass";
	}
	Elements.push_back(MakeConstantString(Name));
	Elements.push_back(Zero);
	Elements.push_back(ConstantInt::get(LongTy, info));
	Elements.push_back(InstanceSize);
	Elements.push_back(IVars);
	Elements.push_back(Methods);
	Elements.push_back(NullP);
	Elements.push_back(NullP);
	Elements.push_back(NullP);
	Elements.push_back(llvm::ConstantExpr::getBitCast(Protocols, PtrTy));
	Elements.push_back(NullP);
	// Create an instance of the structure
	return MakeGlobal(ClassTy, Elements, 
			(isMeta ? "_OBJC_METACLASS_": "_OBJC_CLASS_") + std::string(Name),
			true);
}

llvm::Constant *CGObjCGNU::GenerateProtocolMethodList(
	const llvm::SmallVectorImpl<llvm::Constant *> &MethodNames,
	const llvm::SmallVectorImpl<llvm::Constant *> &MethodTypes) 
{
	// Get the method structure type.
	llvm::StructType *ObjCMethodDescTy = llvm::StructType::get(
		Context,
		PtrToInt8Ty, // Really a selector, but the runtime does the casting for us.
		PtrToInt8Ty,
		NULL);
	std::vector<llvm::Constant*> Methods;
	std::vector<llvm::Constant*> Elements;
	for (unsigned int i = 0, e = MethodTypes.size() ; i < e ; i++) {
		Elements.clear();
		Elements.push_back( llvm::ConstantExpr::getGetElementPtr(MethodNames[i],
					Zeros, 2)); 
		Elements.push_back(
					llvm::ConstantExpr::getGetElementPtr(MethodTypes[i], Zeros, 2));
		Methods.push_back(llvm::ConstantStruct::get(ObjCMethodDescTy, Elements));
	}
	llvm::ArrayType *ObjCMethodArrayTy = llvm::ArrayType::get(ObjCMethodDescTy,
			MethodNames.size());
	llvm::Constant *Array = llvm::ConstantArray::get(ObjCMethodArrayTy, Methods);
	llvm::StructType *ObjCMethodDescListTy = llvm::StructType::get(
			Context, IntTy, ObjCMethodArrayTy, NULL);
	Methods.clear();
	Methods.push_back(ConstantInt::get(IntTy, MethodNames.size()));
	Methods.push_back(Array);
	return MakeGlobal(ObjCMethodDescListTy, Methods, ".objc_method_list");
}

// Create the protocol list structure used in classes, categories and so on
llvm::Constant *CGObjCGNU::GenerateProtocolList(
	const llvm::SmallVectorImpl<std::string> &Protocols)
{
	llvm::ArrayType *ProtocolArrayTy = llvm::ArrayType::get(PtrToInt8Ty,
		Protocols.size());
	llvm::StructType *ProtocolListTy = llvm::StructType::get(
		Context,
		PtrTy, //Should be a recurisve pointer, but it's always NULL here.
		LongTy,//FIXME: Should be size_t
		ProtocolArrayTy,
		NULL);
	std::vector<llvm::Constant*> Elements; 
	for (const std::string *iter=Protocols.begin(), *endIter=Protocols.end();
	    iter != endIter ; iter++) 
	{
		llvm::Constant *Ptr = llvm::ConstantExpr::getBitCast(
				ExistingProtocols[*iter], PtrToInt8Ty);
		Elements.push_back(Ptr);
	}
	llvm::Constant * ProtocolArray = llvm::ConstantArray::get(ProtocolArrayTy,
		Elements);
	Elements.clear();
	Elements.push_back(NULLPtr);
	Elements.push_back(ConstantInt::get(
		llvm::cast<llvm::IntegerType>(LongTy), Protocols.size()));
	Elements.push_back(ProtocolArray);
	return MakeGlobal(ProtocolListTy, Elements, ".objc_protocol_list");
}

llvm::Value *CGObjCGNU::GenerateProtocolRef(llvm::IRBuilder<> &Builder,
	const char *ProtocolName) 
{
	return ExistingProtocols[ProtocolName];
}

void CGObjCGNU::GenerateProtocol(
	const char *ProtocolName,
	const llvm::SmallVectorImpl<std::string> &Protocols,
	const llvm::SmallVectorImpl<llvm::Constant *> &InstanceMethodNames,
	const llvm::SmallVectorImpl<llvm::Constant *> &InstanceMethodTypes,
	const llvm::SmallVectorImpl<llvm::Constant *> &ClassMethodNames,
	const llvm::SmallVectorImpl<llvm::Constant *> &ClassMethodTypes)
{
	llvm::Constant *ProtocolList = GenerateProtocolList(Protocols);
	llvm::Constant *InstanceMethodList =
		GenerateProtocolMethodList(InstanceMethodNames, InstanceMethodTypes);
	llvm::Constant *ClassMethodList =
		GenerateProtocolMethodList(ClassMethodNames, ClassMethodTypes);
	// Protocols are objects containing lists of the methods implemented and
	// protocols adopted.
	llvm::StructType *ProtocolTy = llvm::StructType::get(
		Context,
		IdTy,
		PtrToInt8Ty,
		ProtocolList->getType(),
		InstanceMethodList->getType(),
		ClassMethodList->getType(),
		NULL);
	std::vector<llvm::Constant*> Elements; 
	// The isa pointer must be set to a magic number so the runtime knows it's
	// the correct layout.
	Elements.push_back(llvm::ConstantExpr::getIntToPtr(
		ConstantInt::get(llvm::Type::getInt32Ty(Context), ProtocolVersion), IdTy));
	Elements.push_back(MakeConstantString(ProtocolName, ".objc_protocol_name"));
	Elements.push_back(ProtocolList);
	Elements.push_back(InstanceMethodList);
	Elements.push_back(ClassMethodList);
	ExistingProtocols[ProtocolName] = 
		llvm::ConstantExpr::getBitCast(MakeGlobal(ProtocolTy, Elements,
			".objc_protocol"), IdTy);
}

void CGObjCGNU::GenerateCategory(
	const char *ClassName,
	const char *CategoryName,
	const llvm::SmallVectorImpl<std::string> &InstanceMethodNames,
	const llvm::SmallVectorImpl<std::string> &InstanceMethodTypes,
	const llvm::SmallVectorImpl<std::string> &ClassMethodNames,
	const llvm::SmallVectorImpl<std::string> &ClassMethodTypes,
	const llvm::SmallVectorImpl<std::string> &Protocols)
{
	std::vector<llvm::Constant*> Elements;
	Elements.push_back(MakeConstantString(CategoryName));
	Elements.push_back(MakeConstantString(ClassName));
	// Instance method list 
	Elements.push_back(llvm::ConstantExpr::getBitCast(GenerateMethodList(
			ClassName, CategoryName, InstanceMethodNames, InstanceMethodTypes,
			false), PtrTy));
	// Class method list
	Elements.push_back(llvm::ConstantExpr::getBitCast(GenerateMethodList(
			ClassName, CategoryName, ClassMethodNames, ClassMethodTypes, true),
		  PtrTy));
	// Protocol list
	Elements.push_back(llvm::ConstantExpr::getBitCast(
		  GenerateProtocolList(Protocols), PtrTy));
	Categories.push_back(llvm::ConstantExpr::getBitCast(
		  MakeGlobal(llvm::StructType::get(Context, PtrToInt8Ty, PtrToInt8Ty, PtrTy,
			  PtrTy, PtrTy, NULL), Elements), PtrTy));
}
void CGObjCGNU::GenerateClass(
	const char *ClassName,
	const char *SuperClassName,
	const int instanceSize,
	const llvm::SmallVectorImpl<std::string>  &IvarNames,
	const llvm::SmallVectorImpl<std::string>  &IvarTypes,
	const llvm::SmallVectorImpl<int>  &IvarOffsets,
	const llvm::SmallVectorImpl<std::string>  &InstanceMethodNames,
	const llvm::SmallVectorImpl<std::string>  &InstanceMethodTypes,
	const llvm::SmallVectorImpl<std::string>  &ClassMethodNames,
	const llvm::SmallVectorImpl<std::string>  &ClassMethodTypes,
	const llvm::SmallVectorImpl<std::string> &Protocols) 
{
	// Get the superclass pointer.
	llvm::Constant *SuperClass;
	if (SuperClassName)
	{
		SuperClass = MakeConstantString(SuperClassName, ".super_class_name");
	}
	else 
	{
		SuperClass = llvm::ConstantPointerNull::get(
			llvm::cast<llvm::PointerType>(PtrToInt8Ty));
	}
	llvm::Constant * Name = llvm::ConstantArray::get(Context, ClassName);
	Name = new llvm::GlobalVariable(TheModule, Name->getType(), true,
		llvm::GlobalValue::InternalLinkage, Name, ".class_name");
	// Empty vector used to construct empty method lists
	llvm::SmallVector<std::string, 1> empty;
	llvm::SmallVector<int, 1> empty2;
	// Generate the method and instance variable lists
	llvm::Constant *MethodList = GenerateMethodList(ClassName, "",
		InstanceMethodNames, InstanceMethodTypes, false);
	llvm::Constant *ClassMethodList = GenerateMethodList(ClassName, "",
		ClassMethodNames, ClassMethodTypes, true);
	llvm::Constant *IvarList = GenerateIvarList(IvarNames, IvarTypes,
		IvarOffsets);
	//Generate metaclass for class methods
	llvm::Constant *MetaClassStruct = GenerateClassStructure(NULLPtr,
		SuperClass, 0x2L, ClassName, 0, ConstantInt::get(LongTy, 0),
		GenerateIvarList(empty, empty, empty2), ClassMethodList, NULLPtr, true);
	// Generate the class structure
	llvm::Constant *ClassStruct = GenerateClassStructure(MetaClassStruct,
		SuperClass, 0x1L, ClassName, 0,
		ConstantInt::get(LongTy, instanceSize), IvarList,
		MethodList, GenerateProtocolList(Protocols), false);
	// Add class structure to list to be added to the symtab later
	ClassStruct = llvm::ConstantExpr::getBitCast(ClassStruct, PtrToInt8Ty);
	Classes.push_back(ClassStruct);
}

llvm::Function *CGObjCGNU::ModuleInitFunction()
{ 
	// Only emit an ObjC load function if no Objective-C stuff has been called
	if (Classes.empty() && Categories.empty() && ConstantStrings.empty() &&
	    ExistingProtocols.empty() && TypedSelectors.empty() &&
	    UntypedSelectors.empty())
	{
		return NULL;
	}

	// Name the ObjC types to make the IR a bit easier to read
	TheModule.addTypeName(".objc_selector", SelectorTy);
	TheModule.addTypeName(".objc_id", IdTy);
	TheModule.addTypeName(".objc_imp", IMPTy);

	std::vector<llvm::Constant*> Elements;
	// Generate statics list:
	llvm::Constant *Statics = NULLPtr;
	if (0 != ConstantStrings.size()) {
		llvm::ArrayType *StaticsArrayTy = llvm::ArrayType::get(PtrToInt8Ty,
			ConstantStrings.size() + 1);
		ConstantStrings.push_back(NULLPtr);
		Elements.push_back(MakeConstantString("NSConstantString",
			".objc_static_class_name"));
		Elements.push_back(llvm::ConstantArray::get(StaticsArrayTy, ConstantStrings));
		llvm::StructType *StaticsListTy = 
			llvm::StructType::get(Context, PtrToInt8Ty, StaticsArrayTy, NULL);
		llvm::PointerType *StaticsListPtrTy = 
			llvm::PointerType::getUnqual(StaticsListTy);
		Statics = MakeGlobal(StaticsListTy, Elements, ".objc_statics");
		llvm::ArrayType *StaticsListArrayTy =
			llvm::ArrayType::get(StaticsListPtrTy, 2);
		Elements.clear();
		Elements.push_back(Statics);
		Elements.push_back(llvm::ConstantPointerNull::get(StaticsListPtrTy));
		Statics = MakeGlobal(StaticsListArrayTy, Elements, ".objc_statics_ptr");
		Statics = llvm::ConstantExpr::getBitCast(Statics, PtrTy);
	}
	// Array of classes, categories, and constant objects
	llvm::ArrayType *ClassListTy = llvm::ArrayType::get(PtrToInt8Ty,
		Classes.size() + Categories.size() + 2);
	llvm::StructType *SymTabTy = llvm::StructType::get(
		Context,
		LongTy,
		SelectorTy,
		llvm::Type::getInt16Ty(Context),
		llvm::Type::getInt16Ty(Context),
		ClassListTy,
		NULL);

	Elements.clear();
	// Pointer to an array of selectors used in this module.
	std::vector<llvm::Constant*> Selectors;
	for (std::map<TypedSelector, llvm::GlobalAlias*>::iterator
	     iter = TypedSelectors.begin(), iterEnd = TypedSelectors.end();
	     iter != iterEnd ; ++iter) 
	{
		Elements.push_back(MakeConstantString(iter->first.first, ".objc_sel_name"));
		Elements.push_back(MakeConstantString(iter->first.second,
			".objc_sel_types"));
		Selectors.push_back(llvm::ConstantStruct::get(SelStructTy, Elements));
		Elements.clear();
	}
	for (llvm::StringMap<llvm::GlobalAlias*>::iterator
	     iter = UntypedSelectors.begin(), iterEnd = UntypedSelectors.end();
	     iter != iterEnd; ++iter)
	{
		Elements.push_back(
			MakeConstantString(iter->getKeyData(), ".objc_sel_name"));
		Elements.push_back(NULLPtr);
		Selectors.push_back(llvm::ConstantStruct::get(SelStructTy, Elements));
		Elements.clear();
	}
	Elements.push_back(NULLPtr);
	Elements.push_back(NULLPtr);
	Selectors.push_back(llvm::ConstantStruct::get(SelStructTy, Elements));
	Elements.clear();
	// Number of static selectors
	Elements.push_back(ConstantInt::get(LongTy, Selectors.size() ));
	llvm::Constant *SelectorList = MakeGlobal(
		llvm::ArrayType::get(SelStructTy, Selectors.size()), Selectors,
		".objc_selector_list");
	Elements.push_back(llvm::ConstantExpr::getBitCast(SelectorList, SelectorTy));

	// Now that all of the static selectors exist, create pointers to them.
	int index = 0;
	for (std::map<TypedSelector, llvm::GlobalAlias*>::iterator
	     iter=TypedSelectors.begin(), iterEnd =TypedSelectors.end();
	     iter != iterEnd; ++iter)
	{
		llvm::Constant *Idxs[] = {Zeros[0],
		ConstantInt::get(llvm::Type::getInt32Ty(Context), index++), Zeros[0]};

		llvm::GlobalVariable *SelPtr = new llvm::GlobalVariable(TheModule,
				SelectorTy, true, llvm::GlobalValue::InternalLinkage,
				llvm::ConstantExpr::getGetElementPtr(SelectorList, Idxs, 2),
				".objc_sel_ptr");

		(*iter).second->setAliasee(SelPtr);
	}

	for (llvm::StringMap<llvm::GlobalAlias*>::iterator
	     iter=UntypedSelectors.begin(), iterEnd = UntypedSelectors.end();
	     iter != iterEnd; iter++)
	{
		llvm::Constant *Idxs[] = {Zeros[0],
		ConstantInt::get(llvm::Type::getInt32Ty(Context), index++), Zeros[0]};

		llvm::GlobalVariable *SelPtr = new llvm::GlobalVariable(TheModule,
				SelectorTy, true, llvm::GlobalValue::InternalLinkage,
				llvm::ConstantExpr::getGetElementPtr(SelectorList, Idxs, 2),
				".objc_sel_ptr");
		
		(*iter).second->setAliasee(SelPtr);
	}
	// Number of classes defined.
	Elements.push_back(ConstantInt::get(llvm::Type::getInt16Ty(Context), 
		Classes.size()));
	// Number of categories defined
	Elements.push_back(ConstantInt::get(llvm::Type::getInt16Ty(Context), 
		Categories.size()));
	// Create an array of classes, then categories, then static object instances
	Classes.insert(Classes.end(), Categories.begin(), Categories.end());
	//  NULL-terminated list of static object instances (mainly constant strings)
	Classes.push_back(Statics);
	Classes.push_back(NULLPtr);
	llvm::Constant *ClassList = llvm::ConstantArray::get(ClassListTy, Classes);
	Elements.push_back(ClassList);
	// Construct the symbol table 
	llvm::Constant *SymTab= MakeGlobal(SymTabTy, Elements);

	// The symbol table is contained in a module which has some version-checking
	// constants
	llvm::StructType * ModuleTy = llvm::StructType::get(Context, LongTy, LongTy,
		PtrToInt8Ty, llvm::PointerType::getUnqual(SymTabTy), NULL);
	Elements.clear();
	// Runtime version used for compatibility checking.
	Elements.push_back(ConstantInt::get(LongTy, RuntimeVersion));
	llvm::TargetData td = 
		llvm::TargetData::TargetData(&TheModule);
	Elements.push_back(ConstantInt::get(LongTy, 
				td.getTypeSizeInBits(ModuleTy)/8));
	//FIXME: Should be the path to the file where this module was declared
	Elements.push_back(NULLPtr);
	Elements.push_back(SymTab);
	llvm::Value *Module = MakeGlobal(ModuleTy, Elements);

	// Create the load function calling the runtime entry point with the module
	// structure
	std::vector<const llvm::Type*> VoidArgs;
	llvm::Function * LoadFunction = llvm::Function::Create(
		llvm::FunctionType::get(llvm::Type::getVoidTy(Context), VoidArgs, false),
		llvm::GlobalValue::InternalLinkage, ".objc_load_function",
		&TheModule);

	llvm::BasicBlock *EntryBB = llvm::BasicBlock::Create(Context, "entry", LoadFunction);
	llvm::IRBuilder<> Builder(Context);
	Builder.SetInsertPoint(EntryBB);

	llvm::Value *Register = TheModule.getOrInsertFunction("__objc_exec_class",
		llvm::Type::getVoidTy(Context), llvm::PointerType::getUnqual(ModuleTy), NULL);

	Builder.CreateCall(Register, Module);
	Builder.CreateRetVoid();
	return LoadFunction;
}
llvm::Function *CGObjCGNU::MethodPreamble(
	const std::string &ClassName,
	const std::string &CategoryName,
	const std::string &MethodName,
	const llvm::Type *ReturnTy,
	const llvm::Type *SelfTy,
	const SmallVectorImpl<const llvm::Type*> &ArgTy,
	bool isClassMethod,
	bool isSRet,
	bool isVarArg)
{
	std::vector<const llvm::Type*> Args;
	Args.push_back(SelfTy);
	Args.push_back(SelectorTy);
	Args.insert(Args.end(), ArgTy.begin(), ArgTy.end());

	llvm::FunctionType *MethodTy = llvm::FunctionType::get(ReturnTy,
		Args,
		isVarArg);
	std::string FunctionName = SymbolNameForMethod(ClassName, CategoryName,
		MethodName, isClassMethod);
	
	llvm::Function *Method = llvm::Function::Create(MethodTy,
		llvm::GlobalValue::InternalLinkage,
		FunctionName,
		&TheModule);
	llvm::Function::arg_iterator AI = Method->arg_begin();
	if (isSRet)
	{
		Method->addAttribute(1, Attribute::StructRet);
		AI->setName("retval");
		++AI;
	}
	AI->setName("self");
	++AI;
	AI->setName("_cmd");
	return Method;
}

static string ClassVariableName(const string &ClassName, const string
		&CvarName)
{
	return string(".class_variable_") + ClassName + "_" + CvarName;
}

void CGObjCGNU::DefineClassVariables(
	const string &ClassName,
	const llvm::SmallVectorImpl<std::string>  &CvarNames,
	const llvm::SmallVectorImpl<std::string>  &CvarTypes)
{
	// TODO: Store class variable metadata somewhere.
	// FIXME: Support non-object cvars
	for (unsigned int i = 0, e = CvarNames.size() ; i < e ; i++) 
	{
		string cvarname = ClassVariableName(ClassName, CvarNames[i]);

		new llvm::GlobalVariable(TheModule, IdTy, false,
				llvm::GlobalValue::InternalLinkage,
				ConstantPointerNull::get(cast<PointerType>(IdTy)), cvarname);
	}
}

llvm::Value *CGObjCGNU::LoadClassVariable(llvm::IRBuilder<> &Builder,
                                          string &ClassName,
                                          string &CvarName)
{
	string cvarname = ClassVariableName(ClassName, CvarName);
	GlobalVariable *var = TheModule.getNamedGlobal(cvarname);
	return Builder.CreateLoad(var);
}

void CGObjCGNU::StoreClassVariable(llvm::IRBuilder<> &Builder, string
	&ClassName, string &CvarName, llvm::Value* aValue)
{
	string cvarname = ClassVariableName(ClassName, CvarName);
	GlobalVariable *var = TheModule.getNamedGlobal(cvarname);
	aValue = Builder.CreateBitCast(aValue, IdTy);
	Builder.CreateStore(aValue, var);
}

CGObjCRuntime *CreateObjCRuntime(llvm::Module &M,
                                 llvm::LLVMContext &C,
                                 const llvm::Type *LLVMIntType,
                                 const llvm::Type *LLVMLongType)
{
  return new CGObjCGNU(M, C, LLVMIntType, LLVMLongType);
}

