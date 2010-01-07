#include "CodeGenModule.h"
#include "CodeGenBlock.h"
#include "LLVMCodeGen.h"

#include "llvm/LinkAllPasses.h"
#include <llvm/Bitcode/ReaderWriter.h>
#include <llvm/Constants.h>
#include <llvm/DerivedTypes.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/JIT.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/GlobalVariable.h>
#include <llvm/Module.h>
#include <llvm/LLVMContext.h>
#include <llvm/ModuleProvider.h>
#include <llvm/PassManager.h>
#include "llvm/Analysis/Verifier.h"
#include <llvm/Support/IRBuilder.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Target/TargetData.h>

#include <string>
#include <algorithm>
#include <errno.h>
#include <iostream>
#include <fstream>
#include <dlfcn.h>

namespace llvm 
{
	// Flag used to indicate whether exception handling stuff should be emitted.
	extern bool DwarfExceptionHandling;
}

// A copy of the Small Int message module, used when static compiling.
static Module *SmallIntMessages = NULL;

// Remove unrequired box-then-unbox pass.
FunctionPass *createUnboxPass(void);

Constant *CodeGenModule::MakeConstantString(const std::string &Str,
                                            const std::string &Name,
                                            unsigned GEPs)
{
	Constant * ConstStr = llvm::ConstantArray::get(Context, Str);
	ConstStr = new GlobalVariable(*TheModule, ConstStr->getType(), true,
		GlobalValue::InternalLinkage, ConstStr, Name);
	return ConstantExpr::getGetElementPtr(ConstStr, Zeros, GEPs);
}

void CodeGenModule::CreateClassPointerGlobal(const char *className, const char *globalName)
{
	// Create the global
	Value *global = new GlobalVariable(*TheModule, IdTy, false,
			llvm::GlobalValue::InternalLinkage, ConstantPointerNull::get(IdTy),
			globalName);

	// Initialise it in the module load function
	InitialiseBuilder.CreateStore(InitialiseBuilder.CreateBitCast(
				Runtime->LookupClass(InitialiseBuilder,
					MakeConstantString(className)), IdTy), global);
}

// FIXME: Provide some way of crating new contexts.
CodeGenModule::CodeGenModule(const char *ModuleName, LLVMContext &C, bool jit) 
	: Context(C), InitialiseBuilder(Context)
{
	// When we JIT code, we put the Small Int message functions inside the
	// module, to allow them to be inlined by module passes.  When static
	// compiling, we reference them externally and let the link-time optimiser
	// inline them.
	if (jit)
	{
		TheModule = 
			ParseBitcodeFile(MemoryBuffer::getFile(MsgSendSmallIntFilename), 
					Context);
		SmallIntModule = TheModule;
	}
	else
	{
		if (NULL == SmallIntMessages)
		{
			SmallIntMessages = ParseBitcodeFile(
					MemoryBuffer::getFile(MsgSendSmallIntFilename), 
					Context);
		}
		TheModule = new Module(ModuleName, Context);
		SmallIntModule = SmallIntMessages;
		TheModule->setDataLayout(SmallIntModule->getDataLayout());
	}
	std::vector<const llvm::Type*> VoidArgs;
	LiteralInitFunction = llvm::Function::Create(
		llvm::FunctionType::get(llvm::Type::getVoidTy(Context), VoidArgs, false),
		llvm::GlobalValue::ExternalLinkage, string("__languagekit_constants_") +
		ModuleName, TheModule);
	BasicBlock *EntryBB = 
		llvm::BasicBlock::Create(Context, "entry", LiteralInitFunction);
	InitialiseBuilder.SetInsertPoint(EntryBB);

	Runtime = CreateObjCRuntime(*TheModule, Context, IntTy,
			IntegerType::get(Context, sizeof(long) * 8));
	// Store the class to be used for block closures in a global
	CreateClassPointerGlobal("StackBlockClosure", ".smalltalk_block_stack_class");
	CreateClassPointerGlobal("StackContext", ".smalltalk_context_stack_class");
	CreateClassPointerGlobal("RetainedStackContext", ".smalltalk_context_retained_class");
	CreateClassPointerGlobal("Symbol", ".smalltalk_symbol_class");
	CreateClassPointerGlobal("NSValue", ".smalltalk_nsvalue_class");
	CreateClassPointerGlobal("NSNumber", ".smalltalk_nsnumber_class");
	CreateClassPointerGlobal("BigInt", ".smalltalk_bigint_class");
	CreateClassPointerGlobal("BoxedFloat", ".smalltalk_boxedfloat_class");
}

void CodeGenModule::BeginClass(const char *Class,
                               const char *Super,
                               const char ** cVarNames,
                               const char ** cVarTypes,
                               const char ** iVarNames,
                               const char ** iVarTypes,
                               int *iVarOffsets,
                               int SuperclassSize) 
{
	ClassName = string(Class);
	SuperClassName = string(Super);
	CategoryName = "";
	InstanceMethodNames.clear();
	InstanceMethodTypes.clear();
	ClassMethodNames.clear();
	ClassMethodTypes.clear();
	IvarNames.clear();
	while (*iVarNames)
	{
		IvarNames.push_back(*iVarNames);
		iVarNames++;
	}
	IvarTypes.clear();
	while (*iVarTypes)
	{
		IvarTypes.push_back(*iVarTypes);
		iVarTypes++;
	}
	IvarOffsets.clear();
	while (*iVarOffsets)
	{
		IvarOffsets.push_back(*iVarOffsets);
		iVarOffsets++;
	}
	SmallVector<string, 8> cvarnames, cvartypes;
	while(*cVarNames)
	{
		cvarnames.push_back(*cVarNames);
		cvartypes.push_back(*cVarTypes);
		cVarTypes++;
		cVarNames++;
	}
	Runtime->DefineClassVariables(ClassName, cvarnames, cvartypes);
	
	InstanceSize = SuperclassSize + sizeof(void*) * IvarNames.size();
	CurrentClassTy = IdTy;
}

void CodeGenModule::EndClass(void)
{
	Runtime->GenerateClass(ClassName.c_str(), SuperClassName.c_str(),
		InstanceSize, IvarNames, IvarTypes, IvarOffsets, InstanceMethodNames,
		InstanceMethodTypes, ClassMethodNames, ClassMethodTypes, Protocols);
}

void CodeGenModule::BeginCategory(const char *Class, const char *Category)
{
	ClassName = string(Class);
	SuperClassName = "";
	CategoryName = string(CategoryName); 
	InstanceMethodNames.clear();
	InstanceMethodTypes.clear();
	IvarNames.clear();
	CurrentClassTy = IdTy;
}

void CodeGenModule::EndCategory(void)
{
	Runtime->GenerateCategory(ClassName.c_str(), CategoryName.c_str(),
		InstanceMethodNames, InstanceMethodTypes, ClassMethodNames,
		ClassMethodTypes, Protocols);
}

CodeGenMethod::CodeGenMethod(CodeGenModule *Mod,
                             const char *MethodName,
                             const char *MethodTypes,
                             int locals,
                             bool isClass,
                             const char **localNames)
                             : CodeGenLexicalScope(Mod) 
{
	// Generate the method function
	bool isSRet;
	const Type *realReturnType = NULL;
	FunctionType *MethodTy = CGM->LLVMFunctionTypeFromString(MethodTypes, isSRet,
		realReturnType);
	unsigned argc = MethodTy->getNumParams() - 2;
	const Type *argTypes[argc];
	FunctionType::param_iterator arg = MethodTy->param_begin();
	++arg; ++arg;
	for (unsigned i=0 ; i<argc ; ++i)
	{
		argTypes[i] = MethodTy->getParamType(i+2);
	}


	CurrentFunction = CGM->getRuntime()->MethodPreamble(CGM->getClassName(),
		CGM->getCategoryName(), MethodName, MethodTy->getReturnType(),
		CGM->getCurrentClassTy(), argTypes, argc, isClass, isSRet);

	InitialiseFunction(Args, Locals, locals, MethodTypes, isSRet, localNames);
}

void CodeGenModule::BeginInstanceMethod(const char *MethodName,
                                        const char *MethodTypes,
                                        int locals,
                                        const char **localNames)
{
	// Log the method name and types so that we can use it to set up the class
	// structure.
	InstanceMethodNames.push_back(MethodName);
	InstanceMethodTypes.push_back(MethodTypes);
	inClassMethod = false;
	assert(ScopeStack.empty()
		&& "Creating a method inside something is not sensible");
	ScopeStack.push_back(new CodeGenMethod(this, MethodName, MethodTypes,
				locals, false, localNames));
}

void CodeGenModule::BeginClassMethod(const char *MethodName,
                                     const char *MethodTypes,
                                     int locals,
									 const char **localNames)
{
	// Log the method name and types so that we can use it to set up the class
	// structure.
	ClassMethodNames.push_back(MethodName);
	ClassMethodTypes.push_back(MethodTypes);
	assert(ScopeStack.empty() 
		&& "Creating a method inside something is not sensible");
	ScopeStack.push_back(new CodeGenMethod(this, MethodName, MethodTypes,
				locals, true, localNames));
	inClassMethod = true;
}

/*
void CodeGenModule::BeginFreestandingMethod(const char *MethodName, const char *MethodTypes, int locals)
{
  assert(ScopeStack.empty() 
		  && "Creating a method inside something is not sensible");
  inClassMethod = false;
  string name = string("Freestanding_Method") + MethodName;
  ScopeStack.push_back(
		  new CodeGenMethod(this, name, MethodTypes, locals, true));
}
*/

void CodeGenModule::EndMethod()
{
	//assert(isa<CodeGenMethod>(ScopeStack.back()));
	ScopeStack.back()->EndScope();
	delete ScopeStack.back();
	ScopeStack.pop_back();
}

void CodeGenModule::BeginBlock(unsigned args, unsigned locals)
{
	ScopeStack.push_back(new CodeGenBlock(args, locals, ScopeStack.back(),
		this));
}

void CodeGenModule::SetBlockReturn(Value *value)
{
	((CodeGenBlock*)ScopeStack.back())->SetBlockReturn(value);
}

Value *CodeGenModule::EndBlock(void)
{
	CodeGenBlock *block = (CodeGenBlock*)(ScopeStack.back());
	ScopeStack.pop_back();
	block->EndBlock();
	return block->Block;
}

Value *CodeGenModule::StringConstant(const char *value)
{
	return Runtime->GenerateConstantString(value, strlen(value));
}

Value *CodeGenModule::IntConstant(IRBuilder<> &Builder, const char *value)
{
	errno = 0;
	long long val = strtoll(value, NULL, 10);
	intptr_t ptrVal = (val << 1);
	if ((0 == val && errno == EINVAL) || ((ptrVal >> 1) != val))
	{
		Value *BigIntClass = InitialiseBuilder.CreateLoad(
				TheModule->getGlobalVariable(".smalltalk_bigint_class",
					true));
		Value *V = MakeConstantString(value);
		// Create the BigInt
		Value *S = Runtime->GenerateMessageSend(InitialiseBuilder, IdTy,
			false,  NULL, BigIntClass, Runtime->GetSelector(InitialiseBuilder,
				"bigIntWithCString:", NULL), &V, 1);
		// Retain it
		S = Runtime->GenerateMessageSend(InitialiseBuilder, IdTy, false,  NULL,
			S, Runtime->GetSelector(InitialiseBuilder, "retain", NULL));
		// Define a global variable and store it there.
		GlobalVariable *GS = new GlobalVariable(*TheModule, IdTy, false,
				GlobalValue::InternalLinkage, ConstantPointerNull::get(IdTy),
				value);
	   	InitialiseBuilder.CreateStore(S, GS);
		// Load the global.
		return Builder.CreateLoad(GS);
	}
	ptrVal |= 1;
	Constant *Val = ConstantInt::get(IntPtrTy, ptrVal);
	Val = ConstantExpr::getIntToPtr(Val, IdTy);
	Val->setName("SmallIntConstant");
	return Val;
}

void CodeGenModule::writeBitcodeToFile(char* filename, bool isAsm)
{
	InitialiseBuilder.CreateRetVoid();
	// Set the module init function to be a global ctor
	llvm::Function *init = Runtime->ModuleInitFunction();
	llvm::StructType* CtorStructTy = llvm::StructType::get(Context,
		llvm::Type::getInt32Ty(Context), init->getType(), NULL);

	std::vector<llvm::Constant*> Ctors;

	std::vector<llvm::Constant*> S;
	S.push_back(ConstantInt::get(llvm::Type::getInt32Ty(Context), 0xffff, false));
	S.push_back(init);
	Ctors.push_back(llvm::ConstantStruct::get(CtorStructTy, S));
	// Add the constant initialisation function
	S.clear();
	S.push_back(ConstantInt::get(llvm::Type::getInt32Ty(Context), 0xffff, false));
	S.push_back(LiteralInitFunction);
	Ctors.push_back(llvm::ConstantStruct::get(CtorStructTy, S));

	llvm::ArrayType *AT = llvm::ArrayType::get(CtorStructTy, Ctors.size());
	new llvm::GlobalVariable(*TheModule, AT, false,
			llvm::GlobalValue::AppendingLinkage, llvm::ConstantArray::get(AT,
			Ctors), "llvm.global_ctors");

	PassManager pm;
	pm.add(createVerifierPass());
	pm.add(new TargetData(TheModule));
	pm.run(*TheModule);
	DUMP(TheModule);

	std::string err;
	llvm::raw_fd_ostream os(filename, err);
	WriteBitcodeToFile(TheModule, os);
}

void CodeGenModule::StoreClassVar(const char *cVarName, Value *value)
{
	getCurrentScope()->StoreValueInClassVariable(ClassName, cVarName, value);
}
Value *CodeGenModule::LoadClassVar(const char *cVarName)
{
	return getCurrentScope()->LoadClassVariable(ClassName, cVarName);
}

static ExecutionEngine *EE = NULL;

static void *findSymbol(const std::string &str)
{
	return dlsym(RTLD_DEFAULT, str.c_str());
}

void CodeGenModule::compile(void)
{
	InitialiseBuilder.CreateRetVoid();
	llvm::Function *init = Runtime->ModuleInitFunction();
	// Make the init function external so the optimisations won't remove it.
	init->setLinkage(GlobalValue::ExternalLinkage);
	DUMP(TheModule);
	LOG("\n\n\n Optimises to:\n\n\n");
	PassManager pm;
	pm.add(createVerifierPass());
	pm.add(new TargetData(TheModule));
	pm.add(createScalarReplAggregatesPass());
	pm.add(createPromoteMemoryToRegisterPass());
	pm.add(createAggressiveDCEPass());
	pm.add(createFunctionInliningPass());
	pm.add(createIPConstantPropagationPass());
	pm.add(createSimplifyLibCallsPass());
	//pm.add(createPredicateSimplifierPass());
	//pm.add(createCondPropagationPass());
	pm.add(createInstructionCombiningPass());
	//FIXME: Seems broken in current LLVM - reenable when it's fixed
	//pm.add(createTailDuplicationPass());
	pm.add(createStripDeadPrototypesPass());
	pm.add(createAggressiveDCEPass());
	pm.add(createCFGSimplificationPass());
	pm.run(*TheModule);
	DUMP(TheModule);
	if (NULL == EE)
	{
		DwarfExceptionHandling = true;
		EE = ExecutionEngine::create(TheModule);
		EE->InstallLazyFunctionCreator(findSymbol);
	}
	else
	{
		EE->addModuleProvider(new ExistingModuleProvider(TheModule));
	}
	LOG("Compiling...\n");
	EE->runStaticConstructorsDestructors(TheModule, false);
	void(*f)(void) = (void(*)(void))EE->getPointerToFunction(init);
	LOG("Loading %x...\n", (unsigned)(unsigned long)f);
	f();
	((void(*)(void))EE->getPointerToFunction(LiteralInitFunction))();
	LOG("Loaded.\n");
}
