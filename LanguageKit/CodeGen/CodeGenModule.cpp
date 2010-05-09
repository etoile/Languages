#include "CodeGenModule.h"
#include "CodeGenBlock.h"

#include "llvm/LinkAllPasses.h"
#include <llvm/Bitcode/ReaderWriter.h>
#include <llvm/Constants.h>
#include <llvm/DerivedTypes.h>
#include <llvm/Linker.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/JIT.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/GlobalVariable.h>
#include <llvm/Module.h>
#include <llvm/LLVMContext.h>
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

// If we have the libobjc opts, use them
#ifdef LIBOBJC2_PASSES
ModulePass *createClassIMPCachePass(void);
FunctionPass *createClassLookupCachePass(void);
ModulePass *createClassMethodInliner(void);
FunctionPass *createGNUNonfragileIvarPass(void);
FunctionPass *createGNULoopIMPCachePass(void);
#endif

using std::string;

// A copy of the Small Int message module, used when static compiling.
static Module *SmallIntMessages = NULL;

// Remove unrequired box-then-unbox pass.
FunctionPass *createUnboxPass(void);

Constant *CodeGenModule::MakeConstantString(const string &Str,
                                            const string &Name,
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

CodeGenModule::CodeGenModule(const char *ModuleName, LLVMContext &C, bool jit) 
	: Context(C), InitialiseBuilder(Context)
{
	// When we JIT code, we put the Small Int message functions inside the
	// module, to allow them to be inlined by module passes.  When static
	// compiling, we reference them externally and let the link-time optimiser
	// inline them.
	if (NULL == SmallIntMessages)
	{
		SmallIntMessages = ParseBitcodeFile(
				MemoryBuffer::getFile(MsgSendSmallIntFilename), 
				Context);
	}

	TheModule = new Module(ModuleName, Context);
	SmallIntModule = SmallIntMessages;
	TheModule->setDataLayout(SmallIntModule->getDataLayout());

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

	// FIXME: Leak
	Debug = new DIFactory(*TheModule);
	// Create some metadata for this module.  Pretend that everything LK
	// compiles is Objective-C.
	ModuleScopeDescriptor = Debug->CreateCompileUnit(llvm::dwarf::DW_LANG_ObjC,
			ModuleName, "path", "LanguageKit");
	ModuleSourceFile = Debug->CreateFile(ModuleName, "path",
			ModuleScopeDescriptor);

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
	llvm::SmallVector<const Type *, 8> argTypes;
	FunctionType::param_iterator arg = MethodTy->param_begin();
	++arg; ++arg;
	for (unsigned i=0 ; i<argc ; ++i)
	{
		argTypes.push_back(MethodTy->getParamType(i+2));
	}


	CurrentFunction = CGM->getRuntime()->MethodPreamble(CGM->getClassName(),
		CGM->getCategoryName(), MethodName, MethodTy->getReturnType(),
		CGM->getCurrentClassTy(), argTypes, isClass, isSRet);

	InitialiseFunction(Args, Locals, locals, MethodTypes, isSRet, localNames,
			MethodName);
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

Value *CodeGenModule::GenericConstant(IRBuilder<> &Builder, 
		const string className, const string constructor, 
		const char *arg)
{
	GlobalVariable *ClassPtr = TheModule->getGlobalVariable(className, true);
	Value *Class = InitialiseBuilder.CreateLoad(ClassPtr);

	Value *V = MakeConstantString(arg);

	Value *S = Runtime->GenerateMessageSend(InitialiseBuilder, IdTy,
		false,  NULL, Class, constructor.c_str(), 0, V);
	// Retain it
	S = Runtime->GenerateMessageSend(InitialiseBuilder, IdTy, false,  NULL,
		S, "retain", 0);
	// Define a global variable and store it there.
	GlobalVariable *GS = new GlobalVariable(*TheModule, IdTy, false,
			GlobalValue::InternalLinkage, ConstantPointerNull::get(IdTy),
			arg);
	InitialiseBuilder.CreateStore(S, GS);
	// Load the global.
	return Builder.CreateLoad(GS);
}

Value *CodeGenModule::SymbolConstant(IRBuilder<> &Builder, const char *symbol)
{
	return GenericConstant(Builder, ".smalltalk_symbol_class",
			"SymbolForCString:", symbol);
}

Value *CodeGenModule::IntConstant(IRBuilder<> &Builder, const char *value)
{
	errno = 0;
	long long val = strtoll(value, NULL, 10);
	intptr_t ptrVal = (val << 1);
	if ((0 == val && errno == EINVAL) || ((ptrVal >> 1) != val))
	{
		return GenericConstant(Builder, ".smalltalk_bigint_class",
				"bigIntWithCString:", value);
	}
	ptrVal |= 1;
	Constant *Val = ConstantInt::get(IntPtrTy, ptrVal);
	Val = ConstantExpr::getIntToPtr(Val, IdTy);
	Val->setName("SmallIntConstant");
	return Val;
}
Value *CodeGenModule::FloatConstant(IRBuilder<> &Builder, const char *value)
{
	return GenericConstant(Builder, ".smalltalk_boxedfloat_class",
			"boxedFloatWithCString:", value);
}

void CodeGenModule::writeBitcodeToFile(char* filename, bool isAsm)
{
	EndModule();
	string err;
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

static void *findSymbol(const string &str)
{
	return dlsym(RTLD_DEFAULT, str.c_str());
}

void CodeGenModule::EndModule(void)
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
}

void CodeGenModule::compile(void)
{
	EndModule();
	llvm::Linker::LinkModules(TheModule, SmallIntModule, 0);
	DUMP(TheModule);
	LOG("\n\n\n Optimises to:\n\n\n");
	PassManager pm;
	pm.add(createClassIMPCachePass());
	pm.add(createClassLookupCachePass());
	pm.add(createClassMethodInliner());
	pm.add(createGNUNonfragileIvarPass());
	pm.add(createGNULoopIMPCachePass());
	pm.add(createScalarReplAggregatesPass());
	pm.add(createPromoteMemoryToRegisterPass());
	pm.add(createStripSymbolsPass(true));
	pm.add(createFunctionInliningPass(10000));
	pm.add(createAggressiveDCEPass());
	pm.add(createIPConstantPropagationPass());
	pm.add(createPartialInliningPass());
	pm.add(createSimplifyLibCallsPass());
	pm.add(createInstructionCombiningPass());
	pm.add(createTailDuplicationPass());
	pm.add(createStripDeadPrototypesPass());
	pm.add(createAggressiveDCEPass());
	pm.add(createCFGSimplificationPass());
	//pm.add(createVerifierPass());
	pm.run(*TheModule);
	DUMP(TheModule);
	if (NULL == EE)
	{
		EE = ExecutionEngine::create(TheModule);
		EE->InstallLazyFunctionCreator(findSymbol);
	}
	LOG("Compiling...\n");
	EE->runStaticConstructorsDestructors(TheModule, false);
	LOG("Loaded.\n");
}

// FIXME: This method is, basically, entirely nonsense.  It's a quick hack to
// get SOMETHING working, but it needs a lot of work before it will provide
// actually meaningful information.
DIType CodeGenModule::DebugTypeForEncoding(const string &encoding)
{
	// Get the previously-created version, if it exists
	StringMap<DIType>::const_iterator it = DebugTypeEncodings.find(encoding);
	if (it != DebugTypeEncodings.end())
	{
		return it->second;
	}
	uint64_t size;
	uint64_t align;
	unsigned dwarfEncoding;
	// FIXME: This is rubbish at the moment, fix it to work with nontrivial types.
	switch(encoding[0])
	{
		case '{':
		case '[':
		default:
			align = __alignof(void*);
			size = sizeof(void*);
			dwarfEncoding = llvm::dwarf::DW_ATE_unsigned;
			break;
			fprintf(stderr, "WRGON! %c\n", encoding[0]);
			return DIType();
#define CASE(c, type, dwarf)\
		case c:\
			align = __alignof(type);\
			size = sizeof(type);\
			dwarfEncoding = dwarf;\
			break;
		CASE('c', char, llvm::dwarf::DW_ATE_signed_char)
		CASE('C', char, llvm::dwarf::DW_ATE_unsigned_char)
		CASE('s', short, llvm::dwarf::DW_ATE_unsigned)
		CASE('S', short, llvm::dwarf::DW_ATE_signed)
		CASE('i', int, llvm::dwarf::DW_ATE_unsigned)
		CASE('I', int, llvm::dwarf::DW_ATE_signed)
		CASE('l', long, llvm::dwarf::DW_ATE_unsigned)
		CASE('L', long, llvm::dwarf::DW_ATE_signed)
		CASE('q', long long, llvm::dwarf::DW_ATE_unsigned)
		CASE('Q', long long, llvm::dwarf::DW_ATE_signed)
		CASE('f', float, llvm::dwarf::DW_ATE_float)
		CASE('d', float, llvm::dwarf::DW_ATE_float)
		CASE('B', bool, llvm::dwarf::DW_ATE_boolean)
		// FIXME: SELs are pointers, but they shouldn't be ids.
		case ':':
		case '@':
		{
			align = __alignof(void*);
			size = sizeof(void*);
			dwarfEncoding = llvm::dwarf::DW_ATE_unsigned;
			break;
			// FIXME: Make id point to something, not just be opaque?
			DIType PointerTypeInfo =
				Debug->CreateDerivedType(llvm::dwarf::DW_TAG_pointer_type,
						ModuleScopeDescriptor, "id", ModuleSourceFile, 0,
						sizeof(void*), __alignof(void*), 0, 0, DIType());
			DebugTypeEncodings[encoding] = PointerTypeInfo;
			return PointerTypeInfo;
		}
	}
	// FIXME: Type Names
	DIType TypeInfo = Debug->CreateBasicType(ModuleScopeDescriptor, "",
			ModuleSourceFile, 0, size, align, 0, 0, dwarfEncoding);
	DebugTypeEncodings[encoding] = TypeInfo;
	return TypeInfo;
}

DIArray CodeGenModule::DebugTypeArrayForEncoding(const string &encoding)
{
	llvm::SmallVector<DIDescriptor, 8> types;
	for (string::const_iterator b=encoding.begin(), e=encoding.end() ; b<e ; b++)
	{
		string::const_iterator typeEnd;
		switch (*b)
		{
			default:
				continue;
			// All primitive values are one character
			case 'B': case 'c': case 'C': case 's': case 'S': case 'i': 
			case 'I': case 'l': case 'L': case 'q': case 'Q': case 'f':
			case 'd': case 'v': case ':': case '@': case '#':
			{
				typeEnd = b;
				break;
			}
			case '[':
			{
				typeEnd = b + 1;
				for (int depth=1 ; depth > 0 ; typeEnd++)
				{
					if ('[' == *typeEnd)
					{
						depth++;
					}
					if (']' == *typeEnd)
					{
						depth--;
					}
				}
				break;
			}
			case '{':
			{
				typeEnd = b + 1;
				for (int depth=1 ; depth > 0 ; typeEnd++)
				{
					if ('{' == *typeEnd)
					{
						depth++;
					}
					if ('}' == *typeEnd)
					{
						depth--;
					}
				}
				break;
			}
		}
		const string subEncoding = string(b, typeEnd);
		b = typeEnd;
		types.push_back(DebugTypeForEncoding(subEncoding));
	}
	return Debug->GetOrCreateArray(types.data(), types.size());
}
