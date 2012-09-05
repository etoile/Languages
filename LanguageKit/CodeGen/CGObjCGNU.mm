//===----------------------------------------------------------------------===//
//
// This provides Objective-C code generation targetting the GNU runtime.  The
// class in this file generates structures used by the GNU Objective-C runtime
// library.  These structures are defined in objc/objc.h and objc/objc-api.h in
// the GNU runtime distribution.
//
//===----------------------------------------------------------------------===//

#import "CGObjCRuntime.h"
#import "CodeGenTypes.h"
#include "llvm/Module.h"
#include "llvm/Support/Compiler.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/Target/TargetData.h"
#include "llvm/LLVMContext.h"
#include "objc_pointers.h"
#include <map>
#include <string>
#include <algorithm>
#include <dlfcn.h>

#include "LLVMCompat.h"

#import <Foundation/NSString.h>

using namespace llvm;
using namespace std;
using namespace etoile::languagekit;

// The version of the runtime that this class targets.  Must match the version
// in the runtime.
static int RuntimeVersion = 9;
static int ProtocolVersion = 2;

namespace {
class CGObjCGNU : public CGObjCRuntime {
private:
	llvm::Module &TheModule;
	LLVMStructType *SelStructTy;
	LLVMType *SelectorTy;
	LLVMType *PtrToInt8Ty;
	LLVMType *IMPTy;
	LLVMType *IdTy;
	LLVMType *IntTy;
	LLVMType *PtrTy;
	LLVMType *LongTy;
	LLVMType *PtrToIntTy;
	std::vector<llvm::Constant*> Classes;
	std::vector<llvm::Constant*> Categories;
	std::vector<llvm::Constant*> ConstantStrings;
	object_map<NSString*, llvm::Constant*> ExistingProtocols;
	typedef std::pair<NSString*, llvm::GlobalAlias*> TypedSelector;
	typedef object_map<NSString*, SmallVector<TypedSelector, 2> > SelectorMap;
	SelectorMap SelectorTable;
	// Some zeros used for GEPs in lots of places.
	llvm::Constant *Zeros[2];
	llvm::Constant *NULLPtr;
	bool GC;
	bool JIT;
	llvm::Constant *ObjCIvarOffsetVariable(NSString *className,
		NSString *ivarName, uint64_t Offset);
	llvm::Constant *GenerateIvarList(
		NSString *ClassName,
		StringVector &IvarNames,
		StringVector &IvarTypes,
		const llvm::SmallVectorImpl<int> &IvarOffsets,
		llvm::Constant *&IvarOffsetArray);
	llvm::Constant *GenerateMethodList(NSString *ClassName,
	                                   NSString *CategoryName,
	                                   StringVector &MethodNames, 
	                                   StringVector &MethodTypes, 
	                                   bool isClassMethodList);
	llvm::Constant *GenerateProtocolList(
		StringVector &Protocols);
	llvm::Constant *GenerateClassStructure(
		llvm::Constant *MetaClass,
		llvm::Constant *SuperClass,
		unsigned info,
		NSString *Name,
		llvm::Constant *Version,
		llvm::Constant *InstanceSize,
		llvm::Constant *IVars,
		llvm::Constant *Methods,
		llvm::Constant *Protocols,
		llvm::Constant *IvarOffsets,
		bool isMeta);
	llvm::Constant *GenerateProtocolMethodList(
		const llvm::SmallVectorImpl<llvm::Constant *> &MethodNames,
		const llvm::SmallVectorImpl<llvm::Constant *> &MethodTypes);
	llvm::Constant *MakeConstantString(NSString *Str, const std::string
		&Name="");
	llvm::Constant *MakeGlobal(LLVMStructType *Ty,
		std::vector<llvm::Constant*> &V, const std::string &Name="",
		bool isPublic=false);
	llvm::Constant *MakeGlobal(LLVMArrayType *Ty,
		std::vector<llvm::Constant*> &V, const std::string &Name="");
	llvm::Value *GetWeakSymbol(const std::string &Name,
                               LLVMType *Type);
public:
	CGObjCGNU(
		CodeGenTypes *cgTypes,
		llvm::Module &Mp,
		llvm::LLVMContext &C,
		bool enableGC,
		bool isJit);
	virtual void lookupIMPAndTypes(CGBuilder &Builder,
	                               llvm::Value *Sender,
	                               llvm::Value *&Receiver,
	                               NSString *selName,
	                               llvm::Value *&imp,
	                               llvm::Value *&typeEncoding);
	virtual llvm::Constant *GenerateConstantString(NSString *String);
	virtual llvm::Value *GenerateMessageSend(CGBuilder &Builder,
	                                         llvm::Value *Sender,
	                                         llvm::Value *Receiver,
	                                         NSString *selName,
	                                         NSString *selTypes,
	                                         llvm::SmallVectorImpl<llvm::Value*> &ArgV,
	                                         llvm::BasicBlock *CleanupBlock,
	                                         NSString *ReceiverClass,
	                                         bool isClassMessage);
	virtual llvm::Value *GenerateMessageSendSuper(CGBuilder &Builder,
	                                              llvm::Value *Sender,
	                                              NSString *SuperClassName,
	                                              llvm::Value *Receiver,
	                                              NSString *selName,
	                                              NSString *selTypes,
	                                              llvm::SmallVectorImpl<llvm::Value*> &ArgV,
	                                              bool isClassMessage,
	                                              llvm::BasicBlock *CleanupBlock);
	virtual llvm::Value *LookupClass(CGBuilder &Builder,
	                                 NSString *ClassName);
	virtual llvm::Value *GetSelector(CGBuilder &Builder,
	                                 llvm::Value *SelName,
	                                 llvm::Value *SelTypes);
	virtual llvm::Value *GetSelector(CGBuilder &Builder,
	                                 NSString *SelName,
	                                 NSString *SelTypes);

	virtual llvm::Function *MethodPreamble(NSString *ClassName,
	                                       NSString *CategoryName,
	                                       NSString *MethodName,
	                                       LLVMType *ReturnTy,
	                                       LLVMType *SelfTy,
	                                       const SmallVectorImpl<LLVMType*> &ArgTy,
	                                       bool isClassMethod,
	                                       bool isSRet,
	                                       bool isVarArg);
	virtual llvm::Value *OffsetOfIvar(CGBuilder &Builder,
	                                  NSString *className,
	                                  NSString *ivarName,
	                                  int offsetGuess);
	virtual void GenerateCategory(
		NSString *ClassName, NSString *CategoryName,
		StringVector  &InstanceMethodNames,
		StringVector  &InstanceMethodTypes,
		StringVector  &ClassMethodNames,
		StringVector  &ClassMethodTypes,
		StringVector &Protocols);
	virtual void DefineClassVariables(
		NSString *ClassName,
		StringVector &CvarNames,
		StringVector &CvarTypes);
	virtual void GenerateClass(
		NSString *ClassName,
		NSString *SuperClassName,
		const int instanceSize,
		StringVector &IvarNames,
		StringVector &IvarTypes,
		const llvm::SmallVectorImpl<int> &IvarOffsets,
		StringVector &InstanceMethodNames,
		StringVector &InstanceMethodTypes,
		StringVector &ClassMethodNames,
		StringVector &ClassMethodTypes,
		StringVector &Protocols);
	virtual llvm::Value *GenerateProtocolRef(CGBuilder &Builder,
	                                         NSString *ProtocolName);
	virtual void GenerateProtocol(
		NSString *ProtocolName,
		StringVector &Protocols,
		const llvm::SmallVectorImpl<llvm::Constant *> &InstanceMethodNames,
		const llvm::SmallVectorImpl<llvm::Constant *> &InstanceMethodTypes,
		const llvm::SmallVectorImpl<llvm::Constant *> &ClassMethodNames,
		const llvm::SmallVectorImpl<llvm::Constant *> &ClassMethodTypes);
	virtual llvm::Function *ModuleInitFunction();
	virtual llvm::Value *AddressOfClassVariable(CGBuilder &Builder, 
	                                            NSString *ClassName,
	                                            NSString *CvarName);
};
} // end anonymous namespace


static std::string SymbolNameForSelector(NSString *MethodName)
{
	string MethodNameColonStripped([MethodName UTF8String], [MethodName length]);
	std::replace(MethodNameColonStripped.begin(), MethodNameColonStripped.end(), ':', '_');
	return MethodNameColonStripped;
}

static std::string SymbolNameForMethod(NSString *ClassName, 
	NSString *CategoryName, NSString *MethodName, bool isClassMethod)
{
	return std::string(isClassMethod ? "_c_" : "_i_") + [ClassName UTF8String]
		+ "_" + [CategoryName UTF8String]+ "_" +
		SymbolNameForSelector(MethodName);
}

CGObjCGNU::CGObjCGNU(CodeGenTypes *cgTypes,
                     llvm::Module &M,
                     llvm::LLVMContext &C,
                     bool enableGC,
                     bool isJit) :
                      CGObjCRuntime(cgTypes, C),
                      TheModule(M)
{
	IntTy = types->intTy;
	LongTy = types->longTy;
	GC = enableGC;
	JIT = isJit;
	if (GC)
	{
		RuntimeVersion = 10;
	}
	Zeros[0] = ConstantInt::get(LLVMType::getInt32Ty(Context), 0);
	Zeros[1] = Zeros[0];

	msgSendMDKind = Context.getMDKindID("GNUObjCMessageSend");

	NULLPtr = llvm::ConstantPointerNull::get(
		llvm::PointerType::getUnqual(LLVMType::getInt8Ty(Context)));
	// C string type.  Used in lots of places.
	PtrToInt8Ty = 
		llvm::PointerType::getUnqual(LLVMType::getInt8Ty(Context));
	// Get the selector Type.
	SelStructTy = GetStructType(
		  Context,
		  PtrToInt8Ty,
		  PtrToInt8Ty,
		  NULL);
	SelectorTy = llvm::PointerType::getUnqual(SelStructTy);
	PtrToIntTy = llvm::PointerType::getUnqual(IntTy);
	PtrTy = PtrToInt8Ty;
 
	// Object type
	IdTy = PtrTy;
	// IMP type
	std::vector<LLVMType*> IMPArgs;
	IMPArgs.push_back(IdTy);
	IMPArgs.push_back(SelectorTy);
	IMPTy = llvm::FunctionType::get(IdTy, IMPArgs, true);

}

// This has to perform the lookup every time, since posing and related
// techniques can modify the name -> class mapping.
llvm::Value *CGObjCGNU::LookupClass(CGBuilder &Builder,
                                    NSString *ClassName) 
{
	if (JIT)
	{
		Class cls = NSClassFromString(ClassName);
		if (Nil != cls)
		{
			return llvm::ConstantExpr::getIntToPtr(llvm::ConstantInt::get(LongTy, (uintptr_t)cls),
			IdTy);
		}
	}
	llvm::Value *className = MakeConstantString(ClassName);
	llvm::Constant *ClassLookupFn =
		TheModule.getOrInsertFunction("objc_lookup_class", IdTy, PtrToInt8Ty,
			NULL);
	return Builder.CreateCall(ClassLookupFn, className);
}

/// Statically looks up the selector for the specified name / type pair.
llvm::Value *CGObjCGNU::GetSelector(CGBuilder &Builder,
                                    NSString *SelName,
                                    NSString *SelTypes) 
{
	// In JIT mode, we hard-code selectors.
	if (JIT)
	{
		SEL sel = sel_registerTypedName_np([SelName UTF8String], [SelTypes UTF8String]);
		return llvm::ConstantExpr::getIntToPtr(llvm::ConstantInt::get(LongTy, (uintptr_t)sel),
			SelectorTy);
	}


	// For static selectors, we return an alias for now then store them all in
	// a list that the runtime will initialise later.

	SmallVector<TypedSelector, 2> &Types = SelectorTable[SelName];
	llvm::GlobalAlias *SelValue = 0;

	// Check if we've already cached this selector
	for (SmallVectorImpl<TypedSelector>::iterator i = Types.begin(),
	     e = Types.end() ; i!=e ; i++)
	{
		if (i->first == SelTypes || [i->first isEqualToString: SelTypes])
		{
			return i->second;
		}
	}

	// If not, create a new one.
	SelValue = new llvm::GlobalAlias(SelectorTy,
	                                 llvm::GlobalValue::PrivateLinkage,
	                                 string(".objc_selector_")+[SelName UTF8String], NULL,
	                                 &TheModule);
	Types.push_back(TypedSelector(SelTypes, SelValue));


	return SelValue;
}
/// Dynamically looks up the selector for the specified name / type pair.
llvm::Value *CGObjCGNU::GetSelector(CGBuilder &Builder,
                                    llvm::Value *SelName,
                                    llvm::Value *SelTypes) 
{
// Dead code?
assert(0);
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

llvm::Constant *CGObjCGNU::MakeConstantString(NSString *Str,
                                              const std::string &Name) 
{
	std::string str([Str UTF8String], [Str length]);
#if (LLVM_MAJOR > 3) || ((LLVM_MAJOR == 3) && LLVM_MINOR > 0)
	llvm::Constant *ConstStr = llvm::ConstantDataArray::getString(Context,
		str, true);
#else
	llvm::Constant * ConstStr = llvm::ConstantArray::get(Context, str);
#endif
	ConstStr = new llvm::GlobalVariable(TheModule, ConstStr->getType(), true,
		llvm::GlobalValue::InternalLinkage, ConstStr, Name);
	return llvm::ConstantExpr::getGetElementPtr(ConstStr, Zeros, 2);
}

llvm::Constant *CGObjCGNU::MakeGlobal(LLVMStructType *Ty,
                                      std::vector<llvm::Constant*> &V,
                                      const std::string &Name,
                                      bool isPublic)
{
	llvm::Constant *C = llvm::ConstantStruct::get(Ty, V);
	return new llvm::GlobalVariable(TheModule, Ty, false,
		(isPublic ? llvm::GlobalValue::ExternalLinkage :
		llvm::GlobalValue::InternalLinkage), C, Name);
}

llvm::Constant *CGObjCGNU::MakeGlobal(LLVMArrayType *Ty,
                                      std::vector<llvm::Constant*> &V,
                                      const std::string &Name) 
{
	llvm::Constant *C = llvm::ConstantArray::get(Ty, V);
	return new llvm::GlobalVariable(TheModule, Ty, false,
		llvm::GlobalValue::InternalLinkage, C, Name);
}
llvm::Value *CGObjCGNU::GetWeakSymbol(const std::string &Name,
                                               LLVMType *Type)
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
llvm::Constant *CGObjCGNU::GenerateConstantString(NSString *String)
{
	// In JIT mode, just reuse the string.
	if (JIT)
	{
		// Copy instead of retaining, in case we got passed a mutable string.
		return llvm::ConstantExpr::getIntToPtr(llvm::ConstantInt::get(LongTy, (uintptr_t)(__bridge_retained void*)[String copy]),
			IdTy);
	}

	NSUInteger length = [String length];
	std::vector<llvm::Constant*> Ivars;
	Ivars.push_back(NULLPtr);
	Ivars.push_back(MakeConstantString(String));
	Ivars.push_back(ConstantInt::get(IntTy, length));
	llvm::Constant *ObjCStr = MakeGlobal(
		GetStructType(Context, PtrToInt8Ty, PtrToInt8Ty, IntTy,
		NULL), Ivars, ".objc_str");
	ConstantStrings.push_back(
		llvm::ConstantExpr::getBitCast(ObjCStr, PtrToInt8Ty));
	return ObjCStr;
}
llvm::Value *CGObjCRuntime::callIMP(
                            CGBuilder &Builder,
                            llvm::Value *imp,
                            NSString *typeEncoding,
                            llvm::Value *Receiver,
                            llvm::Value *Selector,
                            llvm::SmallVectorImpl<llvm::Value*> &ArgV,
                            llvm::BasicBlock *CleanupBlock,
                            llvm::MDNode *metadata)
{
	bool isSRet;
	LLVMType *ReturnTy;
	llvm::FunctionType *fTy = types->functionTypeFromString(typeEncoding, isSRet, ReturnTy);
	llvm::AttrListPtr attributes = types->AI->attributeListForFunctionType(fTy, ReturnTy);
	imp = Builder.CreateBitCast(imp, llvm::PointerType::getUnqual(fTy));

	// Call the method
	llvm::SmallVector<llvm::Value*, 8> callArgs;
	llvm::Value *sret = 0;
	unsigned param = 0;
	if (isSRet)
	{
		sret = Builder.CreateAlloca(ReturnTy);
		callArgs.push_back(sret);
		param++;
	}
	callArgs.push_back(Receiver);
	callArgs.push_back(Selector);
	llvm::Value* callArg;
	for (unsigned int i = 0; i < ArgV.size() ; i++) {
		callArg = ArgV[i];
		if (types->AI->willPassTypeAsPointer(callArg->getType()))
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
	for (unsigned int i=0 ; i<fTy->getNumParams() ; i++)
	{
		LLVMType *argTy = fTy->getParamType(i);
		if (callArgs[i]->getType() != argTy)
		{
			callArgs[i] = Builder.CreateBitCast(callArgs[i], argTy);
		}
	}
	llvm::Value *ret = 0;
	if (0 != CleanupBlock)
	{
		llvm::BasicBlock *continueBB =
			llvm::BasicBlock::Create(Context, "invoke_continue",
					Builder.GetInsertBlock()->getParent());
		llvm::InvokeInst *inv = IRBuilderCreateInvoke(&Builder, imp, continueBB, CleanupBlock,
			callArgs);
		if (0 != metadata)
		{
			inv->setMetadata(msgSendMDKind, metadata);
		}
		Builder.SetInsertPoint(continueBB);
		inv->setAttributes(attributes);
		ret = inv;
	}
	else
	{
		llvm::CallInst *call = IRBuilderCreateCall(&Builder, imp, callArgs);
		if (0 != metadata)
		{
			call->setMetadata(msgSendMDKind, metadata);
		}
		call->setAttributes(attributes);
		ret = call;
	}
	if (isSRet)
	{
		ret = Builder.CreateLoad(sret);
	}
	if (ret->getType() != ReturnTy)
	{
		if (ret->getType()->canLosslesslyBitCastTo(ReturnTy))
		{
			ret = Builder.CreateBitCast(ret, ReturnTy);
		}
		else
		{
			llvm::Value *tmp = Builder.CreateAlloca(ReturnTy);
			llvm::Value *storePtr =
				Builder.CreateBitCast(tmp, llvm::PointerType::getUnqual(ret->getType()));
			Builder.CreateStore(ret, storePtr);
			ret = Builder.CreateLoad(tmp);
		}
	}
	return ret;
}

///Generates a message send where the super is the receiver.  This is a message
///send to self with special delivery semantics indicating which class's method
///should be called.
llvm::Value *CGObjCGNU::GenerateMessageSendSuper(CGBuilder &Builder,
                                            llvm::Value *Sender,
                                            NSString *SuperClassName,
                                            llvm::Value *Receiver,
                                            NSString *selName,
                                            NSString *selTypes,
                                            llvm::SmallVectorImpl<llvm::Value*> &ArgV,
                                            bool isClassMessage,
                                            llvm::BasicBlock *CleanupBlock)
{
	llvm::Value *Selector = GetSelector(Builder, selName, selTypes);
	// FIXME: Posing will break this.
	llvm::Value *ReceiverClass = LookupClass(Builder, SuperClassName);
	// If it's a class message, get the metaclass
	if (isClassMessage)
	{
		ReceiverClass = Builder.CreateBitCast(ReceiverClass,
				PointerType::getUnqual(IdTy));
		ReceiverClass = Builder.CreateLoad(ReceiverClass);
	}
	// Construct the structure used to look up the IMP
	llvm::StructType *ObjCSuperTy = GetStructType(Context, 
		Receiver->getType(), IdTy, NULL);
	llvm::Value *ObjCSuper = Builder.CreateAlloca(ObjCSuperTy);
	Builder.CreateStore(Receiver, Builder.CreateStructGEP(ObjCSuper, 0));
	Builder.CreateStore(ReceiverClass, Builder.CreateStructGEP(ObjCSuper, 1));

	std::vector<LLVMType*> Params;
	Params.push_back(llvm::PointerType::getUnqual(ObjCSuperTy));
	Params.push_back(SelectorTy);


	// The lookup function returns a slot, which can be safely cached.
	LLVMType *SlotTy = GetStructType(Context, PtrTy, PtrTy, PtrTy,
		IntTy, PtrTy, NULL);

	llvm::Constant *lookupFunction =
	TheModule.getOrInsertFunction("objc_slot_lookup_super",
			llvm::FunctionType::get( llvm::PointerType::getUnqual(SlotTy),
				Params, true));

	llvm::SmallVector<llvm::Value*, 2> lookupArgs;
	lookupArgs.push_back(ObjCSuper);
	lookupArgs.push_back(Selector);
	llvm::CallInst *slot = IRBuilderCreateCall(&Builder, lookupFunction, lookupArgs);
	slot->setOnlyReadsMemory();

	llvm::Value *imp = Builder.CreateLoad(Builder.CreateStructGEP(slot, 4));

	llvm::Value *impMD[] = {
		llvm::MDString::get(Context, [selName UTF8String]), 
		llvm::MDString::get(Context, [SuperClassName UTF8String]),
		llvm::ConstantInt::get(LLVMType::getInt1Ty(Context), isClassMessage)
		};
	llvm::MDNode *node = CreateMDNode(Context, impMD, 3);

	return callIMP(Builder, imp, selTypes, Receiver,
			Selector, ArgV, CleanupBlock, node);
}
void CGObjCGNU::lookupIMPAndTypes(CGBuilder &Builder,
                                  llvm::Value *Sender,
                                  llvm::Value *&Receiver,
                                  NSString *selName,
                                  llvm::Value *&imp,
                                  llvm::Value *&typeEncoding)
{
	llvm::Value *Selector = GetSelector(Builder, selName, 0);

	if (0 == Sender)
	{
		Sender = NULLPtr;
	}
	llvm::Value *ReceiverPtr = Builder.CreateAlloca(Receiver->getType());
	Builder.CreateStore(Receiver, ReceiverPtr, true);

	LLVMType *SlotTy = GetStructType(Context, PtrTy, PtrTy, PtrTy,
			IntTy, PtrTy, NULL);

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

	imp = Builder.CreateLoad(Builder.CreateStructGEP(slot, 4));
	typeEncoding = Builder.CreateLoad(Builder.CreateStructGEP(slot, 2));
	Receiver = Builder.CreateLoad(ReceiverPtr, true);
}

/// Generate code for a message send expression.
llvm::Value *CGObjCGNU::GenerateMessageSend(CGBuilder &Builder,
                                            llvm::Value *Sender,
                                            llvm::Value *Receiver,
                                            NSString *selName,
                                            NSString *selTypes,
                                            llvm::SmallVectorImpl<llvm::Value*> &ArgV,
                                            llvm::BasicBlock *CleanupBlock,
                                            NSString *ReceiverClass,
                                            bool isClassMessage)
{
	llvm::Value *Selector = GetSelector(Builder, selName, selTypes);
#if !defined(__arm__) && !defined(__i386__) && !defined(__x86_64__)
	if (0 == Sender)
	{
		Sender = NULLPtr;
	}
	llvm::Value *ReceiverPtr = Builder.CreateAlloca(Receiver->getType());
	Builder.CreateStore(Receiver, ReceiverPtr, true);

	LLVMType *SlotTy = GetStructType(Context, PtrTy, PtrTy, PtrTy,
			IntTy, PtrTy, NULL);

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

	Receiver = Builder.CreateLoad(ReceiverPtr, true);
#else
	char ret = [selTypes characterAtIndex: 0];
	const char *msgFuncName = "objc_msgSend";
	switch (ret)
	{
		case 'f': case 'd': case 'D':
			msgFuncName = "objc_msgSend_fpret";
	}
	bool isSRet;
	LLVMType *ReturnTy;
	llvm::FunctionType *fTy = types->functionTypeFromString(selTypes, isSRet, ReturnTy);
	llvm::AttrListPtr attributes = types->AI->attributeListForFunctionType(fTy, ReturnTy);
	if (isSRet)
	{
			msgFuncName = "objc_msgSend_stret";
	}
	llvm::Value *imp = TheModule.getOrInsertFunction(msgFuncName, fTy, attributes);
#endif
	llvm::Value *impMD[] = {
		llvm::MDString::get(Context, [selName UTF8String]),
		llvm::MDString::get(Context, [ReceiverClass UTF8String]),
		llvm::ConstantInt::get(LLVMType::getInt1Ty(Context), isClassMessage)
		};
	llvm::MDNode *node = CreateMDNode(Context, impMD, 3);

	// Call the method.
	return callIMP(Builder, imp, selTypes, Receiver, Selector,
				ArgV, CleanupBlock, node);
}

/// Generates a MethodList.  Used in construction of a objc_class and 
/// objc_category structures.
llvm::Constant *CGObjCGNU::GenerateMethodList(
	NSString *ClassName,
	NSString *CategoryName, 
	StringVector  &MethodNames, 
	StringVector  &MethodTypes, 
	bool isClassMethodList) 
{
	// Get the method structure type.  
	llvm::StructType *ObjCMethodTy = GetStructType(
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
	llvm::SmallVector<LLVMType*, 16> ObjCMethodListFields;
	LLVMType *OpaqueNextTy = PtrTy;
	LLVMType *NextPtrTy = llvm::PointerType::getUnqual(OpaqueNextTy);
	llvm::StructType *ObjCMethodListTy = GetStructType(
		Context,
		NextPtrTy, 
		IntTy, 
		ObjCMethodArrayTy,
		NULL);

	Methods.clear();
	Methods.push_back(llvm::Constant::getNullValue(NextPtrTy));
	Methods.push_back(ConstantInt::get(LLVMType::getInt32Ty(Context),
		MethodTypes.size()));
	Methods.push_back(MethodArray);
	
	// Create an instance of the structure
	return MakeGlobal(ObjCMethodListTy, Methods, ".objc_method_list");
}

/// Generates an IvarList.  Used in construction of a objc_class.
llvm::Constant *CGObjCGNU::GenerateIvarList(
	NSString *ClassName,
	StringVector  &IvarNames, const
	llvm::SmallVectorImpl<NSString*>  &IvarTypes, const
	llvm::SmallVectorImpl<int>  &IvarOffsets, 
	llvm::Constant *&IvarOffsetArray)
{
	if (0 == IvarNames.size())
	{
		return NULLPtr;
	}
	// Get the method structure type.  
	llvm::StructType *ObjCIvarTy = GetStructType(
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

	// Array of ivar structures
	llvm::ArrayType *ObjCIvarArrayTy = llvm::ArrayType::get(ObjCIvarTy,
		IvarNames.size());
	
	Elements.clear();
	Elements.push_back(ConstantInt::get(
		 llvm::cast<llvm::IntegerType>(IntTy), (int)IvarNames.size()));
	Elements.push_back(llvm::ConstantArray::get(ObjCIvarArrayTy, Ivars));
	// Structure containing array and array count
	llvm::StructType *ObjCIvarListTy = GetStructType(Context, IntTy,
		ObjCIvarArrayTy,
		NULL);

	// Create an instance of the structure
	llvm::Constant *IvarList =
		MakeGlobal(ObjCIvarListTy, Elements, ".objc_ivar_list");

	// Generate the non-fragile ABI offset variables.
	LLVMType *IndexTy = LLVMType::getInt32Ty(Context);
	llvm::Constant *offsetPointerIndexes[] = {Zeros[0],
		llvm::ConstantInt::get(IndexTy, 1), 0,
		llvm::ConstantInt::get(IndexTy, 2) };

	std::vector<llvm::Constant*> IvarOffsetValues;
	const char *className = [ClassName UTF8String];

	for (unsigned int i = 0, e = IvarNames.size() ; i < e ; i++)
	{
		const std::string Name = std::string("__objc_ivar_offset_") + className
			+ '.' + [IvarNames[i] UTF8String];
		offsetPointerIndexes[2] = llvm::ConstantInt::get(IndexTy, i);
		// Get the correct ivar field
		llvm::Constant *offsetValue = llvm::ConstantExpr::getGetElementPtr(
				IvarList, offsetPointerIndexes, 4);
		// Get the existing alias, if one exists.
		llvm::GlobalVariable *offset = TheModule.getNamedGlobal(Name);
		if (offset)
		{
			offset->setInitializer(offsetValue);
			// If this is the real definition, change its linkage type so that
			// different modules will use this one, rather than their private
			// copy.
			offset->setLinkage(llvm::GlobalValue::ExternalLinkage);
		}
		else 
		{
			// Add a new alias if there isn't one already.
			offset = new llvm::GlobalVariable(TheModule, offsetValue->getType(),
			false, llvm::GlobalValue::ExternalLinkage, offsetValue, Name);
		}

		IvarOffsetValues.push_back(new llvm::GlobalVariable(TheModule, IntTy,
					false, llvm::GlobalValue::ExternalLinkage,
					llvm::ConstantInt::get(IntTy, IvarOffsets[i]),
					std::string("__objc_ivar_offset_value_") + className +'.' +
					[IvarNames[i] UTF8String]));
	}

	if (IvarOffsetArray)
	{
		llvm::Constant *IvarOffsetArrayInit =
			llvm::ConstantArray::get(llvm::ArrayType::get(PtrToIntTy,
						IvarOffsetValues.size()), IvarOffsetValues);
		IvarOffsetArray = new llvm::GlobalVariable(TheModule,
				IvarOffsetArrayInit->getType(), false,
				llvm::GlobalValue::InternalLinkage, IvarOffsetArrayInit,
				".ivar.offsets");
	}

	return IvarList;
}

/// Generate a class structure
llvm::Constant *CGObjCGNU::GenerateClassStructure(
	llvm::Constant *MetaClass,
	llvm::Constant *SuperClass,
	unsigned info,
	NSString *Name,
	llvm::Constant *Version,
	llvm::Constant *InstanceSize,
	llvm::Constant *IVars,
	llvm::Constant *Methods,
	llvm::Constant *Protocols,
	llvm::Constant *IvarOffsets,
	bool isMeta)
{
	// Set up the class structure
	// Note:  Several of these are char*s when they should be ids.  This is
	// because the runtime performs this translation on load.
	llvm::StructType *ClassTy = GetStructType(
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
		LongTy,                 // abi_version
		// Ivar offset pointers, to be filled in by the runtime
		IvarOffsets->getType(), // ivar_offsets
		PtrTy,                  // properties (not used by LK yet)
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
		Name = @"AnonymousClass";
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
	Elements.push_back(Zero);
	Elements.push_back(IvarOffsets);
	Elements.push_back(NullP);
	// Create an instance of the structure
	return MakeGlobal(ClassTy, Elements, 
			std::string(isMeta ? "_OBJC_METACLASS_": "_OBJC_CLASS_") + [Name UTF8String],
			true);
}

llvm::Constant *CGObjCGNU::GenerateProtocolMethodList(
	const llvm::SmallVectorImpl<llvm::Constant *> &MethodNames,
	const llvm::SmallVectorImpl<llvm::Constant *> &MethodTypes) 
{
	// Get the method structure type.
	llvm::StructType *ObjCMethodDescTy = GetStructType(
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
	llvm::StructType *ObjCMethodDescListTy = GetStructType(Context,
			IntTy, ObjCMethodArrayTy, NULL);
	Methods.clear();
	Methods.push_back(ConstantInt::get(IntTy, MethodNames.size()));
	Methods.push_back(Array);
	return MakeGlobal(ObjCMethodDescListTy, Methods, ".objc_method_list");
}

// Create the protocol list structure used in classes, categories and so on
llvm::Constant *CGObjCGNU::GenerateProtocolList(
	StringVector &Protocols)
{
	llvm::ArrayType *ProtocolArrayTy = llvm::ArrayType::get(PtrToInt8Ty,
		Protocols.size());
	llvm::StructType *ProtocolListTy = GetStructType(
		Context,
		PtrTy, //Should be a recurisve pointer, but it's always NULL here.
		LongTy,//FIXME: Should be size_t
		ProtocolArrayTy,
		NULL);
	std::vector<llvm::Constant*> Elements; 
	for (StringVector::const_iterator iter=Protocols.begin(), endIter=Protocols.end();
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

llvm::Value *CGObjCGNU::GenerateProtocolRef(CGBuilder &Builder,
	NSString *ProtocolName) 
{
	return ExistingProtocols[ProtocolName];
}

void CGObjCGNU::GenerateProtocol(
	NSString *ProtocolName,
	StringVector &Protocols,
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
	llvm::StructType *ProtocolTy = GetStructType(
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
		ConstantInt::get(LLVMType::getInt32Ty(Context), ProtocolVersion), IdTy));
	Elements.push_back(MakeConstantString(ProtocolName, ".objc_protocol_name"));
	Elements.push_back(ProtocolList);
	Elements.push_back(InstanceMethodList);
	Elements.push_back(ClassMethodList);
	ExistingProtocols[ProtocolName] = 
		llvm::ConstantExpr::getBitCast(MakeGlobal(ProtocolTy, Elements,
			".objc_protocol"), IdTy);
}

void CGObjCGNU::GenerateCategory(
	NSString *ClassName,
	NSString *CategoryName,
	StringVector &InstanceMethodNames,
	StringVector &InstanceMethodTypes,
	StringVector &ClassMethodNames,
	StringVector &ClassMethodTypes,
	StringVector &Protocols)
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
		  MakeGlobal(GetStructType(Context, PtrToInt8Ty, PtrToInt8Ty, PtrTy,
			  PtrTy, PtrTy, NULL), Elements), PtrTy));
}
void CGObjCGNU::GenerateClass(
	NSString *ClassName,
	NSString *SuperClassName,
	const int instanceSize,
	StringVector  &IvarNames,
	StringVector  &IvarTypes,
	const llvm::SmallVectorImpl<int>  &IvarOffsets,
	StringVector  &InstanceMethodNames,
	StringVector  &InstanceMethodTypes,
	StringVector  &ClassMethodNames,
	StringVector  &ClassMethodTypes,
	StringVector &Protocols) 
{
	std::string classSymbolName = "__objc_class_name_";
	classSymbolName += [ClassName UTF8String];
	if (llvm::GlobalVariable *symbol =
	  TheModule.getGlobalVariable(classSymbolName))
	{
		symbol->setInitializer(llvm::ConstantInt::get(LongTy, 0));
	}
	else
	{
		new llvm::GlobalVariable(TheModule, LongTy, false,
			llvm::GlobalValue::ExternalLinkage, llvm::ConstantInt::get(LongTy, 0),
			classSymbolName);
	}

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
#if (LLVM_MAJOR > 3) || ((LLVM_MAJOR == 3) && LLVM_MINOR > 0)
	llvm::Constant * Name = llvm::ConstantDataArray::getString(Context, [ClassName UTF8String], true);
#else
	llvm::Constant * Name = llvm::ConstantArray::get(Context, [ClassName UTF8String]);
#endif
	Name = new llvm::GlobalVariable(TheModule, Name->getType(), true,
		llvm::GlobalValue::InternalLinkage, Name, ".class_name");
	// Empty vector used to construct empty method lists
	llvm::SmallVector<NSString*, 1> empty;
	llvm::SmallVector<int, 1> empty2;
	// Generate the method and instance variable lists
	llvm::Constant *MethodList = GenerateMethodList(ClassName, @"",
		InstanceMethodNames, InstanceMethodTypes, false);
	llvm::Constant *ClassMethodList = GenerateMethodList(ClassName, @"",
		ClassMethodNames, ClassMethodTypes, true);
	llvm::Constant *IvarOffsetValues=NULLPtr, *ignored;
	llvm::Constant *IvarList = GenerateIvarList(ClassName, IvarNames,
			IvarTypes, IvarOffsets, IvarOffsetValues);
	//Generate metaclass for class methods
	llvm::Constant *MetaClassStruct = GenerateClassStructure(NULLPtr,
		SuperClass, 0x12L, ClassName, 0, ConstantInt::get(LongTy, 0),
		GenerateIvarList(ClassName, empty, empty, empty2, ignored),
		ClassMethodList,
		NULLPtr, NULLPtr, true);
	// Generate the class structure
	llvm::Constant *ClassStruct = GenerateClassStructure(MetaClassStruct,
		SuperClass, 0x11L, ClassName, 0,
		ConstantInt::get(LongTy, 0-instanceSize), IvarList,
		MethodList, GenerateProtocolList(Protocols), IvarOffsetValues, false);
	// Add class structure to list to be added to the symtab later
	ClassStruct = llvm::ConstantExpr::getBitCast(ClassStruct, PtrToInt8Ty);
	Classes.push_back(ClassStruct);
}

llvm::Function *CGObjCGNU::ModuleInitFunction()
{ 
	// Only emit an ObjC load function if no Objective-C stuff has been called
	if (Classes.empty() && Categories.empty() && ConstantStrings.empty() &&
	    ExistingProtocols.empty() && SelectorTable.empty())
	{
		return NULL;
	}

	std::vector<llvm::Constant*> Elements;
	// Generate statics list:
	llvm::Constant *Statics = NULLPtr;
	if (0 != ConstantStrings.size()) {
		llvm::ArrayType *StaticsArrayTy = llvm::ArrayType::get(PtrToInt8Ty,
			ConstantStrings.size() + 1);
		ConstantStrings.push_back(NULLPtr);
		Elements.push_back(MakeConstantString(@"NSConstantString",
			".objc_static_class_name"));
		Elements.push_back(llvm::ConstantArray::get(StaticsArrayTy, ConstantStrings));
		llvm::StructType *StaticsListTy = 
			GetStructType(Context, PtrToInt8Ty, StaticsArrayTy, NULL);
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
	llvm::StructType *SymTabTy = GetStructType(
		Context,
		LongTy,
		SelectorTy,
		LLVMType::getInt16Ty(Context),
		LLVMType::getInt16Ty(Context),
		ClassListTy,
		NULL);

	Elements.clear();
	// Pointer to an array of selectors used in this module.
	std::vector<llvm::Constant*> Selectors;
	std::vector<llvm::GlobalAlias*> SelectorAliases;

	for (SelectorMap::iterator iter = SelectorTable.begin(),
	     iterEnd = SelectorTable.end(); iter != iterEnd ; ++iter)
	{
		llvm::Constant *SelName = MakeConstantString(iter->first, "objc_sel_name");
		SmallVectorImpl<TypedSelector> &Types = iter->second;
		for (SmallVectorImpl<TypedSelector>::iterator i = Types.begin(),
		     e = Types.end() ; i!=e ; i++)
		{
			llvm::Constant *SelectorTypeEncoding = NULLPtr;
			if ([i->first length] != 0)
			{
				SelectorTypeEncoding = MakeConstantString(i->first, ".objc_sel_types");
			}
			Elements.push_back(SelName);
			Elements.push_back(SelectorTypeEncoding);
			Selectors.push_back(llvm::ConstantStruct::get(SelStructTy, Elements));
			Elements.clear();
			// Store the selector alias for later replacement
			SelectorAliases.push_back(i->second);
		}
	}
	// Number of static selectors, not including the NULL terminator
	unsigned SelectorCount = Selectors.size();
	// Add a NULL terminator to the list
	Elements.push_back(NULLPtr);
	Elements.push_back(NULLPtr);
	Selectors.push_back(llvm::ConstantStruct::get(SelStructTy, Elements));
	Elements.clear();
	Elements.push_back(ConstantInt::get(LongTy, SelectorCount));
	llvm::Constant *SelectorList = MakeGlobal(
		llvm::ArrayType::get(SelStructTy, Selectors.size()), Selectors,
		".objc_selector_list");
	Elements.push_back(llvm::ConstantExpr::getBitCast(SelectorList, SelectorTy));

	// Now that all of the static selectors exist, create pointers to them.
	for (unsigned int i=0 ; i<SelectorCount ; i++)
	{
		llvm::Constant *Idxs[] = {Zeros[0],
			llvm::ConstantInt::get(llvm::Type::getInt32Ty(Context), i),
			Zeros[0]};
		llvm::Constant *SelPtr = llvm::ConstantExpr::getGetElementPtr(SelectorList,
			llvm::makeArrayRef(Idxs, 2));
		// If selectors are defined as an opaque type, cast the pointer to this
		// type.
		SelPtr = llvm::ConstantExpr::getBitCast(SelPtr, SelectorTy);
		SelectorAliases[i]->replaceAllUsesWith(SelPtr);
		SelectorAliases[i]->eraseFromParent();
	}


	// Number of classes defined.
	Elements.push_back(ConstantInt::get(LLVMType::getInt16Ty(Context), 
		Classes.size()));
	// Number of categories defined
	Elements.push_back(ConstantInt::get(LLVMType::getInt16Ty(Context), 
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
	llvm::StructType * ModuleTy = GetStructType(Context, LongTy, LongTy,
		PtrToInt8Ty, llvm::PointerType::getUnqual(SymTabTy), 
		GC ? IntTy : NULL, NULL);
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
	if (GC)
	{
		// GC required.
		Elements.push_back(ConstantInt::get(IntTy, 2));
	}
	llvm::Value *Module = MakeGlobal(ModuleTy, Elements);

	// Create the load function calling the runtime entry point with the module
	// structure
	std::vector<LLVMType*> VoidArgs;
	llvm::Function * LoadFunction = llvm::Function::Create(
		llvm::FunctionType::get(LLVMType::getVoidTy(Context), VoidArgs, false),
		llvm::GlobalValue::InternalLinkage, ".objc_load_function",
		&TheModule);

	llvm::BasicBlock *EntryBB = llvm::BasicBlock::Create(Context, "entry", LoadFunction);
	CGBuilder Builder(Context);
	Builder.SetInsertPoint(EntryBB);

	llvm::Value *Register = TheModule.getOrInsertFunction("__objc_exec_class",
		LLVMType::getVoidTy(Context), llvm::PointerType::getUnqual(ModuleTy), NULL);

	Builder.CreateCall(Register, Module);
	Builder.CreateRetVoid();
	return LoadFunction;
}

llvm::Function *CGObjCGNU::MethodPreamble(
	NSString *ClassName,
	NSString *CategoryName,
	NSString *MethodName,
	LLVMType *ReturnTy,
	LLVMType *SelfTy,
	const SmallVectorImpl<LLVMType*> &ArgTy,
	bool isClassMethod,
	bool isSRet,
	bool isVarArg)
{
	assert(0);
	std::vector<LLVMType*> Args;
	Args.push_back(SelfTy);
	Args.push_back(SelectorTy);
	Args.insert(Args.end(), ArgTy.begin(), ArgTy.end());

	llvm::FunctionType *MethodTy = llvm::FunctionType::get(ReturnTy,
		Args,
		isVarArg);
	llvm::AttrListPtr attributes = types->AI->attributeListForFunctionType(MethodTy, ReturnTy);
	std::string FunctionName = SymbolNameForMethod(ClassName, CategoryName,
		MethodName, isClassMethod);
	
	llvm::Function *Method = llvm::Function::Create(MethodTy,
		llvm::GlobalValue::InternalLinkage,
		FunctionName,
		&TheModule);
	Method->setAttributes(attributes);
	llvm::Function::arg_iterator AI = Method->arg_begin();
	if (isSRet)
	{
		AI->setName("retval");
		++AI;
	}
	AI->setName("self");
	++AI;
	AI->setName("_cmd");
	return Method;
}

static string ClassVariableName(NSString *ClassName, NSString *CvarName)
{
	return string(".class_variable_") + [ClassName UTF8String] + "_" +
		[CvarName UTF8String];
}

void CGObjCGNU::DefineClassVariables(
	NSString *ClassName,
	StringVector &CvarNames,
	StringVector &CvarTypes)
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

llvm::Value *CGObjCGNU::AddressOfClassVariable(CGBuilder &Builder,
                                               NSString *ClassName,
                                               NSString *CvarName)
{
	string cvarname = ClassVariableName(ClassName, CvarName);
	GlobalVariable *var = TheModule.getNamedGlobal(cvarname);
	return var;
}


llvm::Constant *CGObjCGNU::ObjCIvarOffsetVariable(NSString *className,
		NSString *ivarName, uint64_t Offset)
{
	const std::string Name = std::string("__objc_ivar_offset_") +
		[className UTF8String] + '.' + [ivarName UTF8String];
	// Emit the variable and initialize it with what we think the correct value
	// is.  This allows code compiled with non-fragile ivars to work correctly
	// when linked against code which isn't (most of the time).
	llvm::GlobalVariable *IvarOffsetPointer = TheModule.getNamedGlobal(Name);
	if (!IvarOffsetPointer)
	{
		if (Offset == 0)
		{
			return TheModule.getOrInsertGlobal(Name, llvm::PointerType::getUnqual(LLVMType::getInt32Ty(Context)));
		}
		llvm::ConstantInt *OffsetGuess =
			llvm::ConstantInt::get(LLVMType::getInt32Ty(Context), Offset,
					"ivar");
		llvm::GlobalVariable *IvarOffsetGV = new
			llvm::GlobalVariable(TheModule, LLVMType::getInt32Ty(Context),
					false, llvm::GlobalValue::PrivateLinkage, OffsetGuess,
					Name+".guess");
		IvarOffsetPointer = new llvm::GlobalVariable(TheModule,
		IvarOffsetGV->getType(), false, llvm::GlobalValue::LinkOnceAnyLinkage,
		IvarOffsetGV, Name);
	}
	return IvarOffsetPointer;
}
llvm::Value *CGObjCGNU::OffsetOfIvar(CGBuilder &Builder,
                                     NSString *className,
                                     NSString *ivarName,
                                     int offsetGuess)
{
	return Builder.CreateLoad(Builder.CreateLoad(
		ObjCIvarOffsetVariable(className, ivarName, offsetGuess), false,
		"ivar"));
}

CGObjCRuntime *etoile::languagekit::CreateObjCRuntime(CodeGenTypes *types,
                                                      llvm::Module &M,
                                                      llvm::LLVMContext &C,
                                                      bool enableGC,
                                                      bool isJit)
{
  return new CGObjCGNU(types, M, C, enableGC, isJit);
}
CGObjCRuntime::~CGObjCRuntime() {}
