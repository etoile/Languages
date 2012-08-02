#include "CodeGenModule.h"
#include "CodeGenBlock.h"
#include "LLVMCompat.h"

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
#include <llvm/Support/MemoryBuffer.h>
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include <llvm/Target/TargetData.h>
#include <llvm/Support/system_error.h>


#include <string>
#include <algorithm>
#include <errno.h>
#include <iostream>
#include <fstream>
#include <dlfcn.h>
#include <stdlib.h>

#import <Foundation/Foundation.h>
#import "../LanguageKit.h"



using namespace etoile::languagekit;

using std::string;

// A copy of the Small Int message module, used when static compiling.
static Module *SmallIntMessages = NULL;

// Remove unrequired box-then-unbox pass.
FunctionPass *createUnboxPass(void);

Constant *CodeGenModule::MakeConstantString(NSString *Str,
                                            const std::string &Name,
                                            unsigned GEPs)
{
	Constant *ConstStr = constantStrings[Str];
	if (0 == ConstStr)
	{
#if (LLVM_MAJOR > 3) || ((LLVM_MAJOR == 3) && LLVM_MINOR > 0)
		ConstStr = llvm::ConstantDataArray::getString(Context, [Str UTF8String]);
#else
		ConstStr = llvm::ConstantArray::get(Context, [Str UTF8String]);
#endif
		ConstStr = new GlobalVariable(*TheModule, ConstStr->getType(), true,
			GlobalValue::InternalLinkage, ConstStr, Name);
		constantStrings[Str] = ConstStr;
	}
	return ConstantExpr::getGetElementPtr(ConstStr, types->zeros, GEPs);
}


CodeGenModule::CodeGenModule(NSString *ModuleName, LLVMContext &C, bool gc,
		bool jit, bool profiling) 
	: Context(C), InitialiseBuilder(Context), profilingEnabled(profiling)
{
	JIT = jit;
	CategoryName = nil;
	ClassName = nil;
	SuperClassName = nil;
	// When we JIT code, we put the Small Int message functions inside the
	// module, to allow them to be inlined by module passes.  When static
	// compiling, we reference them externally and let the link-time optimiser
	// inline them.
	if (NULL == SmallIntMessages)
	{
		OwningPtr<MemoryBuffer> buffer;
		llvm::MemoryBuffer::getFile([MsgSendSmallIntFilename UTF8String], buffer);
		SmallIntMessages = ParseBitcodeFile(buffer.get(), Context);
	}

	TheModule = new Module([ModuleName UTF8String], Context);
	SmallIntModule = SmallIntMessages;
	TheModule->setDataLayout(SmallIntModule->getDataLayout());
	types = new CodeGenTypes(*TheModule);
	assign = CodeGenAssignments::Create(*types, gc);

	std::vector<LLVMType*> VoidArgs;
	LiteralInitFunction = llvm::Function::Create(
		llvm::FunctionType::get(llvm::Type::getVoidTy(Context), VoidArgs, false),
		llvm::GlobalValue::PrivateLinkage, string("__languagekit_constants_") +
		[ModuleName UTF8String], TheModule);
	BasicBlock *EntryBB = 
		llvm::BasicBlock::Create(Context, "entry", LiteralInitFunction);
	InitialiseBuilder.SetInsertPoint(EntryBB);
	initializerPool =
		InitialiseBuilder.CreateCall(TheModule->getOrInsertFunction("objc_autoreleasePoolPush",
					types->ptrToVoidTy, NULL));

	Runtime = CreateObjCRuntime(types, *TheModule, Context, gc, jit);

	// FIXME: Leak
	//Debug = new DIFactory(*TheModule);
	// Create some metadata for this module.  Pretend that everything LK
	// compiles is Objective-C.
	//ModuleScopeDescriptor = Debug->CreateCompileUnit(llvm::dwarf::DW_LANG_ObjC,
	//		ModuleName, "path", "LanguageKit");
	//ModuleSourceFile = Debug->CreateFile(ModuleName, "path",
	//		ModuleScopeDescriptor);
}

void CodeGenModule::CreateClassPointerGlobal(NSString *className, const char *globalName)
{
assert(0 && "Dead code!");
}


void CodeGenModule::BeginClass(NSString *aClassName,
                               NSString *aSuperClassName,
                               LKSymbolTable *symbolTable)
{
	ASSIGN(ClassName, aClassName);
	ASSIGN(SuperClassName, aSuperClassName);
	CategoryName = @"";
	InstanceMethodNames.clear();
	InstanceMethodTypes.clear();
	ClassMethodNames.clear();
	ClassMethodTypes.clear();
	CvarNames.clear();
	CvarTypes.clear();
	IvarNames.clear();
	IvarTypes.clear();
	IvarOffsets.clear();
	NSUInteger nextOffset = 0;
	for (LKSymbol *s in [[symbolTable symbols] objectEnumerator])
	{
		switch ([s scope])
		{
			default: continue;
			case LKSymbolScopeObject:
			{
				NSUInteger size, alignment;
				NSGetSizeAndAlignment([[s typeEncoding] UTF8String], &size, &alignment);
				nextOffset += nextOffset % alignment;
				IvarNames.push_back([s name]);
				IvarTypes.push_back([s typeEncoding]);
				IvarOffsets.push_back(nextOffset);
				nextOffset += size;
			}
			case LKSymbolScopeClass:
			{
				CvarNames.push_back([s name]);
				CvarTypes.push_back([s typeEncoding]);
			}
		}
	}
	Runtime->DefineClassVariables(ClassName, CvarNames, CvarTypes);
	InstanceSize = sizeof(void*) * IvarNames.size();
	CurrentClassTy = types->idTy;
}

void CodeGenModule::EndClass(void)
{
	Runtime->GenerateClass(ClassName, SuperClassName,
		InstanceSize, IvarNames, IvarTypes, IvarOffsets, InstanceMethodNames,
		InstanceMethodTypes, ClassMethodNames, ClassMethodTypes, Protocols);
}

void CodeGenModule::BeginCategory(NSString *aClass, NSString *Category)
{
	ASSIGN(ClassName, aClass);
	SuperClassName = @"";
	ASSIGN(CategoryName, Category);
	InstanceMethodNames.clear();
	InstanceMethodTypes.clear();
	ClassMethodNames.clear();
	ClassMethodTypes.clear();
	IvarNames.clear();
	CurrentClassTy = types->idTy;
}

void CodeGenModule::EndCategory(void)
{
	Runtime->GenerateCategory(ClassName, CategoryName,
		InstanceMethodNames, InstanceMethodTypes, ClassMethodNames,
		ClassMethodTypes, Protocols);
}

CodeGenFunction::CodeGenFunction(NSString *functionName,
                                 NSArray *locals,
                                 NSArray *arguments,
                                 NSString *signature,
                                 CodeGenModule *Mod)
                               : CodeGenSubroutine(Mod) 
{
	CleanupBB = 0;
	// FIXME: Should allow the front end to set this.
	bool returnsRetained = NO;
	InitialiseFunction(functionName, arguments, locals, signature, returnsRetained);
	Self = 0;
}

CodeGenMethod::CodeGenMethod(NSString *methodName,
                             NSString *functionName,
                             NSArray *locals,
                             NSArray *arguments,
                             NSString *signature,
                             bool isClass,
                             CodeGenModule *Mod)
                             : CodeGenSubroutine(Mod) 
{
	LKSymbol *selfSymbol = [LKSymbol new];
	[selfSymbol setName: @"self"];
	[selfSymbol setTypeEncoding: @"@"];
	LKSymbol *cmdSymbol = [LKSymbol new];
	[cmdSymbol setName: @"_cmd"];
	// FIXME: This is technically wrong, but since we don't actually expose
	// _cmd to front ends yet it's better than boxing them here.
	// Eventually we should make the AST emit the hidden arguments and allow
	// front ends to name self, rather than have it magic (so EScript, for
	// example, could use this instead of self)
	[cmdSymbol setTypeEncoding: @"@"];
	// TODO: Add a mechanism for front ends to expose this to the language.
	NSMutableArray *realArguments =
		[NSMutableArray arrayWithObjects: selfSymbol, cmdSymbol, nil];
	[realArguments addObjectsFromArray: arguments];
	CleanupBB = 0;
	bool returnsRetained = SelectorReturnsRetained(methodName);
	InitialiseFunction(functionName, realArguments, locals, signature, returnsRetained);
	// FIXME: self and _cmd should be implicit arguments and they should be
	// handled as special things.
	Self = variables[@"self"];
	assert(Self);
	// If the return type is an object it defaults to self.  If it's something
	// else, it's NULL.
	// FIXME: This retain of self is superfluous.  If the optimiser doesn't
	// make it go away then we should move this up to the AST which knows that
	// this is redundant
	if (RetVal && (RetVal->getType() == types.ptrToIdTy))
	{
		// Methods in the -init family are passed an owning reference to self
		// that they consume and then return a new owning reference.
		llvm::Value *selfRet = Builder.CreateLoad(Self);
		selfRet = CGM->assign->retainValue(Builder, selfRet);
		Builder.CreateStore(selfRet, RetVal);
	}
}

void CodeGenModule::BeginFunction(NSString *functionName,
                                  NSString *functionTypes,
                                  NSArray *locals,
                                  NSArray *arguments)
{
	inClassMethod = false;
	assert(ScopeStack.empty()
		&& "Creating a function inside something is not sensible");
	ScopeStack.push_back(new CodeGenFunction(functionName, locals,
		arguments, functionTypes, this));
}
void CodeGenModule::BeginInstanceMethod(NSString *methodName,
                                        NSString *methodTypes,
                                        NSArray *locals,
                                        NSArray *arguments)
{
	// Log the method name and types so that we can use it to set up the class
	// structure.
	InstanceMethodNames.push_back(methodName);
	InstanceMethodTypes.push_back(methodTypes);
	inClassMethod = false;
	NSString *functionName = [methodName stringByReplacingOccurrencesOfString: @":" withString: @"_"];
	functionName = [NSString stringWithFormat: @"_i_%@_%@_%@", ClassName, CategoryName, functionName];
	assert(ScopeStack.empty()
		&& "Creating a method inside something is not sensible");
	ScopeStack.push_back(new CodeGenMethod(methodName, functionName, locals,
		arguments, methodTypes, false, this));
}

void CodeGenModule::BeginClassMethod(NSString *methodName,
                                     NSString *methodTypes,
                                     NSArray *locals,
                                     NSArray *arguments)
{
	// Log the method name and types so that we can use it to set up the class
	// structure.
	ClassMethodNames.push_back(methodName);
	ClassMethodTypes.push_back(methodTypes);
	inClassMethod = true;
	NSString *functionName = [methodName stringByReplacingOccurrencesOfString: @":" withString: @"_"];
	functionName = [NSString stringWithFormat: @"_c_%@_%@_%@", ClassName, CategoryName, functionName];
	assert(ScopeStack.empty() 
		&& "Creating a method inside something is not sensible");
	ScopeStack.push_back(new CodeGenMethod(methodName, functionName, locals,
		arguments, methodTypes, true, this));
}

void CodeGenModule::EndFunction()
{
	ScopeStack.back()->EndScope();
	delete ScopeStack.back();
	ScopeStack.pop_back();
}
void CodeGenModule::EndMethod()
{
	//assert(isa<CodeGenMethod>(ScopeStack.back()));
	ScopeStack.back()->EndScope();
	delete ScopeStack.back();
	ScopeStack.pop_back();
}

void CodeGenModule::BeginBlock(NSArray *locals,
                               NSArray *arguments,
                               NSArray *bound,
                               NSString *signature)
{
	ScopeStack.push_back(new CodeGenBlock(locals, arguments, bound, signature,
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
	llvm::Value *b = block->EndBlock();
	delete block;
	return b;
}

Value *CodeGenModule::StringConstant(NSString *value)
{
	return Runtime->GenerateConstantString(value);
}

Value *CodeGenModule::GenericConstant(CGBuilder &Builder, 
		NSString *className, NSString *constructor, 
		NSString *arg)
{
	if (JIT)
	{
		IMP msg = [NSClassFromString(className) methodForSelector: NSSelectorFromString(constructor)];
		id constant = msg(NSClassFromString(className), NSSelectorFromString(constructor), [arg UTF8String]);
		return llvm::ConstantExpr::getIntToPtr(llvm::ConstantInt::get(types->intPtrTy, (uintptr_t)(__bridge_retained void*)[constant copy]),
			types->idTy);
	}
	Value *Class = Runtime->LookupClass(InitialiseBuilder, className);

	Value *V = MakeConstantString(arg);

	Value *S = Runtime->GenerateMessageSend(InitialiseBuilder, 
		NULL, Class, constructor, 0, V);
	// Define a global variable and store it there.
	GlobalVariable *GS = new GlobalVariable(*TheModule, types->idTy, false,
			GlobalValue::InternalLinkage, ConstantPointerNull::get(types->idTy),
			[arg UTF8String]);
	assign->storeGlobal(InitialiseBuilder, GS, S);
	// Load the global.
	return Builder.CreateLoad(GS);
}

Value *CodeGenModule::SymbolConstant(CGBuilder &Builder, NSString *symbol)
{
	return GenericConstant(Builder, @"Symbol",
			@"SymbolForCString:", symbol);
}

Value *CodeGenModule::IntConstant(CGBuilder &Builder, NSString *value)
{
	errno = 0;
	long long val = strtoll([value UTF8String], NULL, 10);
	intptr_t ptrVal = (val << OBJC_SMALL_OBJECT_SHIFT);
	if ((0 == val && errno == EINVAL) || ((ptrVal >> OBJC_SMALL_OBJECT_SHIFT) != val))
	{
		return GenericConstant(Builder, @"BigInt",
				@"bigIntWithCString:", value);
	}
	ptrVal |= 1;
	Constant *Val = ConstantInt::get(types->intPtrTy, ptrVal);
	Val = ConstantExpr::getIntToPtr(Val, types->idTy);
	Val->setName("SmallIntConstant");
	return Val;
}
Value *CodeGenModule::FloatConstant(CGBuilder &Builder, NSString *value)
{
	return GenericConstant(Builder, @"BoxedFloat",
			@"boxedFloatWithCString:", value);
}

void CodeGenModule::writeBitcodeToFile(NSString *filename, bool isAsm)
{
	EndModule();
	string err;
	llvm::raw_fd_ostream os([filename UTF8String], err);
	WriteBitcodeToFile(TheModule, os);
}

void CodeGenModule::StoreCVar(NSString *cVarName, Value *value)
{
	getCurrentScope()->StoreValueInClassVariable(ClassName, cVarName, value);
}
Value *CodeGenModule::LoadCvar(LKSymbol *cvar)
{
	NSString *className = nil;
	id owner = [cvar owner];
	if (class_isMetaClass(object_getClass(owner)))
	{
		className = [owner className];
	}
	else
	{
		className = [owner classname];
	}
	return getCurrentScope()->LoadClassVariable(className, [cvar name]);
}
void CodeGenModule::StoreIVar(LKSymbol *ivar, Value *value)
{
	CodeGenSubroutine *scope = getCurrentScope();
	NSString *className = nil;
	id owner = [ivar owner];
	if (class_isMetaClass(object_getClass(owner)))
	{
		className = [owner className];
	}
	else
	{
		className = [owner classname];
	}
	scope->StoreValueOfTypeAtOffsetFromObject(value,
	                                          className,
	                                          [ivar name],
	                                          [ivar typeEncoding],
	                                          0,
	                                          scope->LoadSelf());
}
void CodeGenModule::StoreScopedValue(NSString *variable, Value *value)
{
	getCurrentScope()->storeValueInVariable(value, variable);
}
llvm::Value *CodeGenModule::LoadIvar(LKSymbol *ivar)
{
	CodeGenSubroutine *scope = getCurrentScope();
	NSString *className = nil;
	id owner = [ivar owner];
	if (class_isMetaClass(object_getClass(owner)))
	{
		className = [owner className];
	}
	else
	{
		className = [owner classname];
	}
	return scope->LoadValueOfTypeAtOffsetFromObject(className,
	                                                [ivar name],
	                                                [ivar typeEncoding],
	                                                0,
	                                                scope->LoadSelf());
}
llvm::Value *CodeGenModule::LoadScopedValue(NSString *variable)
{
	return getCurrentScope()->loadVariable(variable);
}

static ExecutionEngine *EE = NULL;

#if LLVM_MAJOR < 3
static void *findSymbol(const string &str)
{
	return dlsym(RTLD_DEFAULT, str.c_str());
}
#endif

#if LLVM_MAJOR >= 3
// Create the two ARC optimisation passes
static void addObjCARCExpandPass(const PassManagerBuilder &Builder, PassManagerBase &PM)
{
	PM.add(createObjCARCExpandPass());
}

static void addObjCARCOptPass(const PassManagerBuilder &Builder, PassManagerBase &PM)
{
	PM.add(createObjCARCOptPass());
}
#endif

void CodeGenModule::EndModule(void)
{
	InitialiseBuilder.CreateCall(TheModule->getOrInsertFunction("objc_autoreleasePoolPop",
				types->voidTy, types->ptrToVoidTy, NULL), initializerPool);
	InitialiseBuilder.CreateRetVoid();
	// Set the module init function to be a global ctor
	llvm::Function *init = Runtime->ModuleInitFunction();
	llvm::StructType* CtorStructTy = GetStructType(Context, 
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
	OwningPtr<MemoryBuffer> buffer;
	MemoryBuffer::getFile([MsgSendSmallIntFilename UTF8String], buffer);
	Module *smallIntModule = ParseBitcodeFile(buffer.get(), Context);
	DUMP(TheModule);
#if LLVM_MAJOR < 3
	llvm::Linker::LinkModules(TheModule, smallIntModule, 0);
	LOG("\n\n\n Optimises to:\n\n\n");
	PassManager pm;
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
#else
	std::string error;
	if (llvm::Linker::LinkModules(TheModule, smallIntModule, 0, &error))
	{
		[NSException raise: @"LKCodeGenException"
		            format: @"Linking failed: %s", error.c_str()];
	}
	LOG("\n\n\n Optimises to:\n\n\n");
	static BOOL RuntimePassesLoaded = NO;

	NSMutableArray *paths = [NSMutableArray arrayWithObjects:
		@"/lib", @"/usr/lib", @"/usr/local/lib", nil];
	[paths addObjectsFromArray: [[[[NSProcessInfo processInfo] environment] objectForKey: @"LD_LIBRARY_PATH"] componentsSeparatedByString: @":"]];
	NSFileManager *fm = [NSFileManager defaultManager];
	for (NSString *path in paths)
	{
		NSString *lib = [path stringByAppendingPathComponent: @"libGNUObjCRuntime.so"];
		if ([fm fileExistsAtPath: lib isDirectory: NULL])
		{
			dlopen([lib UTF8String], RTLD_NOW);
			RuntimePassesLoaded = YES;
			break;
		}
	}

	PassManagerBuilder PMBuilder;
	// TODO: Allow this to be configured by the driver
	PMBuilder.OptLevel = 3;
	PMBuilder.addExtension(PassManagerBuilder::EP_EarlyAsPossible,
	                       addObjCARCExpandPass);
	PMBuilder.addExtension(PassManagerBuilder::EP_ScalarOptimizerLate,
	                       addObjCARCOptPass);
	// Inlining threshold copied from clang.  May be silly...
	PMBuilder.Inliner = createFunctionInliningPass(275);
	FunctionPassManager *PerFunctionPasses= new FunctionPassManager(TheModule);
	PerFunctionPasses->add(new TargetData(TheModule));

	PMBuilder.populateFunctionPassManager(*PerFunctionPasses);

	for (Module::iterator I = TheModule->begin(), E = TheModule->end() ;
	     I != E ; ++I)
	{
		if (!I->isDeclaration())
		{
			PerFunctionPasses->run(*I);
		}
	}
	PerFunctionPasses->doFinalization();
	delete PerFunctionPasses;
	// Run the per-module passes
	PassManager *PerModulePasses = new PassManager();
	PerModulePasses->add(new TargetData(TheModule));
	PerModulePasses->add(createVerifierPass());
	PMBuilder.populateModulePassManager(*PerModulePasses);
	// Add the ARC cleanup pass at the end.
	PerModulePasses->add(createObjCARCContractPass());
	PerModulePasses->add(createVerifierPass());
	PerModulePasses->add(createStripSymbolsPass(true));
	// Something in LICM is broken in trunk currently.  Disable until it's fixed.
#if LLVM_MINOR < 2
	PerModulePasses->run(*TheModule);
#endif
	delete PerModulePasses;
#endif
	DUMP(TheModule);
	if (NULL == EE)
	{
		LOG("Creating execution engine...\n");
#if (LLVM_MAJOR > 3) || (LLVM_MAJOR == 3 && LLVM_MINOR >= 1)
		EngineBuilder EB = EngineBuilder(TheModule);
		TargetOptions TO;
		TO.JITExceptionHandling = 1;
		// Note: mathk, turn on debug info generation here too!
		EB.setTargetOptions(TO);
		EE = EB.create();
#else
		EE = ExecutionEngine::create(TheModule);
		assert(EE && "Created execution engine");
#	if LLVM_MAJOR < 3
		EE->InstallLazyFunctionCreator(findSymbol);
#	endif
#endif
	}
	LOG("Compiling...\n");
	EE->runStaticConstructorsDestructors(TheModule, false);
	LOG("Loaded.\n");
}


// FIXME: This method is, basically, entirely nonsense.  It's a quick hack to
// get SOMETHING working, but it needs a lot of work before it will provide
// actually meaningful information.
#if 0
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
	// FIXME: This is rubbish at the moment, fix it to work with nontrivial types->
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
		types->push_back(DebugTypeForEncoding(subEncoding));
	}
	return Debug->GetOrCreateArray(types->data(), types->size());
}
#endif
