#include "CodeGenLexicalScope.h"
#include "CodeGenBlock.h"
#include "CodeGenModule.h"
#include <llvm/Module.h>
#include <llvm/Intrinsics.h>
#include <llvm/LLVMContext.h>

extern const char *LKObjectEncoding;

static bool isLKObject(const char *encoding)
{
	return strncmp(LKObjectEncoding, encoding, strlen(LKObjectEncoding)) == 0;
}

static bool isObject(const char *encoding)
{
	return (encoding[0] == '@') || (encoding[0] == '#') || isLKObject(encoding);
}

static Function *getSmallIntModuleFunction(CodeGenModule *CGM, string name)
{
	// If the function already exists, return it
	Function *fn = CGM->getModule()->getFunction(name);
	if (NULL != fn)
	{
		return fn;
	}
	Function *smallIntFn = CGM->getSmallIntModule()->getFunction(name);
	if (NULL == smallIntFn)
	{
		return NULL;
	}
	// Create an extern reference to the function in the Small Int module
	return Function::Create(smallIntFn->getFunctionType(),
			GlobalVariable::ExternalLinkage, name, CGM->getModule());
}

string CodeGenLexicalScope::FunctionNameFromSelector(const char *sel)
{
	// Special cases
	switch (*sel) {
		case '+':
			return "SmallIntMsgplus_";
		case '-':
			return "SmallIntMsgsub_";
		case '/':
			return "SmallIntMsgdiv_";
		case '*':
			return "SmallIntMsgmul_";
		default: {
			string str = "SmallIntMsg" + string(sel);
			replace(str.begin(), str.end(), ':', '_');
			return str;
		}
	}
}

Value *CodeGenLexicalScope::BoxValue(IRBuilder<> *B, Value *V, const char *typestr)
{
	CGObjCRuntime *Runtime = CGM->getRuntime();
	// Untyped selectors return id
	if (NULL == typestr || '\0' == *typestr) return V;
	// Special case for LKObjects
	if (isLKObject(typestr))
	{
		return V;
	}
	// FIXME: Other function type qualifiers
	while(*typestr == 'V' || *typestr == 'r')
	{
		typestr++;
	}
	switch(*typestr)
	{
		// All integer primitives smaller than a 64-bit value
		case 'B': case 'c': case 'C': case 's': case 'S': case 'i': case 'I':
		case 'l': case 'L':
			LOG("Boxing return value %s\n", typestr);
			V = B->CreateSExt(V, Type::getInt64Ty(CGM->Context));
		// Now V is sign-extended to 64-bits.
		case 'q': case 'Q':
		{
			// This will return a SmallInt or a promoted integer.
			Constant *BoxFunction = getSmallIntModuleFunction(CGM, "MakeSmallInt");
			CallInst *boxed = B->CreateCall(BoxFunction, V);
			boxed->setOnlyReadsMemory();
			return boxed;
		}
		case 'f': case 'd':
		{
			// Box float/double
			// TODO: On 64-bit platforms hide floats inside pointers, leave
			// doubles boxed
			Value *BoxedFloatClass = B->CreateLoad(
				CGM->getModule()->getGlobalVariable(".smalltalk_boxedfloat_class",
					true));
			const char *castSelName;
			if (*typestr == 'f')
			{
				castSelName = "boxedFloatWithFloat:";
			}
			else
			{
				castSelName = "boxedFloatWithDouble:";
			}
			Value *boxed = Runtime->GenerateMessageSend(*B, IdTy, false,
					NULL, BoxedFloatClass, castSelName, NULL, V);
			if (CallInst *call = dyn_cast<llvm::CallInst>(boxed))
			{
				call->setOnlyReadsMemory();
			}
			return boxed;
		}
		case ':': 
		{
			Value *SymbolCalss = B->CreateLoad(
				CGM->getModule()->getGlobalVariable(
					".smalltalk_symbol_class", true));
			return Runtime->GenerateMessageSend(*B, IdTy, false, NULL,
					SymbolCalss, "SymbolForSelector:", NULL, V);
		}
		case '{':
		{
			Value *NSValueClass = B->CreateLoad(
				CGM->getModule()->getGlobalVariable(
					".smalltalk_nsvalue_class", true));
			const char * castSelName = "valueWithBytes:objCType:";
			bool passValue = false;
			if (0 == strncmp(typestr, "{_NSRect", 8))
			{
				castSelName = "valueWithRect:";
				passValue = true;
			} 
			else if (0 == strncmp(typestr, "{_NSRange", 9))
			{
				castSelName = "valueWithRange:";
				passValue = true;
			}
			else if (0 == strncmp(typestr, "{_NSPoint", 9))
			{
				castSelName = "valueWithPoint:";
				passValue = true;
			}
			else if (0 == strncmp(typestr, "{_NSSize", 8))
			{
				castSelName = "valueWithSize:";
				passValue = true;
			}
			if (passValue)
			{
				Value *boxed = Runtime->GenerateMessageSend(*B, IdTy, false,
						NULL, NSValueClass, castSelName, NULL, V);
				if (CallInst *call = dyn_cast<llvm::CallInst>(boxed))
				{
						call->setOnlyReadsMemory();
				}
				return boxed;
			}
			assert(0 && "Boxing arbitrary structures doesn't work yet");
		}
		// Other types, just wrap them up in an NSValue
		default:
		{
			Value *NSValueClass = B->CreateLoad(
				CGM->getModule()->getGlobalVariable(
					".smalltalk_nsvalue_class",	true));
			// TODO: We should probably copy this value somewhere, maybe with a
			// custom object instead of NSValue?
			const char *end = typestr;
			while (!isdigit(*end)) { end++; }
			string typestring = string(typestr, end - typestr);
			SmallVector<Value*,2> args;
			args.push_back(V);
			args.push_back(CGM->MakeConstantString(typestring.c_str()));
			return Runtime->GenerateMessageSend(*B, IdTy, false, LoadSelf(),
					NSValueClass, "valueWithBytesOrNil:objCType:", NULL, args);
		}
		// Map void returns to nil
		case 'v':
		{
			return ConstantPointerNull::get(IdTy);
		}
		// If it's already an object, we don't need to do anything
		case '@': case '#':
			return V;
	}
}

#define NEXT(typestr) \
	while (!(*typestr == '\0') && !isdigit(*typestr)) { typestr++; }\
	while (isdigit(*typestr)) { typestr++; }

Value *CodeGenLexicalScope::Unbox(IRBuilder<> *B,
                                  Function *F,
                                  Value *val,
                                  const char *Type)
{
	// Special case for LKObjects
	if (isLKObject(Type))
	{
		return val;
	}
	string returnTypeString = string(1, *Type);
	const char *castSelName;
	switch(*Type)
	{
		case 'c':
			castSelName = "charValue";
			break;
		case 'C':
			castSelName = "unsignedCharValue";
			break;
		case 's':
			castSelName = "shortValue";
			break;
		case 'S':
			castSelName = "unsignedShortValue";
			break;
		case 'i':
			castSelName = "intValue";
			break;
		case 'I':
			castSelName = "unsignedIntValue";
			break;
		case 'l':
			castSelName = "longValue";
			break;
		case 'L':
			castSelName = "unsignedLongValue";
			break;
		case 'q':
			castSelName = "longLongValue";
			break;
		case 'Q':
			castSelName = "unsignedLongLongValue";
			break;
		case 'f':
			castSelName = "floatValue";
			break;
		case 'd':
			castSelName = "doubleValue";
			break;
		case 'B':
			castSelName = "boolValue";
			break;
		case ':':
			castSelName = "selValue";
			break;
		case '#':
		case '@':
		{
			Value *BoxFunction = getSmallIntModuleFunction(CGM, "BoxObject");
			val = B->CreateBitCast(val, IdTy);
			return B->CreateCall(BoxFunction, val, "boxed_small_int");
		}
		case 'v':
			return val;
		case '{':
		{
			const char *end = Type;
			while(!isdigit(*end)) { end++; }
			returnTypeString = string(Type, (int)(end - Type));
			//Special cases for NSRect and NSPoint
			if (0 == strncmp(Type, "{_NSRect", 8))
			{
				castSelName = "rectValue";
				break;
			}
			if (0 == strncmp(Type, "{_NSRange", 9))
			{
				castSelName = "rangeValue";
				break;
			}
			if (0 == strncmp(Type, "{_NSPoint", 9))
			{
				castSelName = "pointValue";
				break;
			}
			if (0 == strncmp(Type, "{_NSSize", 8))
			{
				castSelName = "sizeValue";
				break;
			}
		}
		default:
			LOG("Found type value: %s\n", Type);
			castSelName = "";
			assert(false && "Unable to transmogriy object to compound type");
	}
	//TODO: We don't actually use the size numbers for anything, but someone else
	//does, so make these sensible:
	returnTypeString += "12@0:4";
	return MessageSend(B, F, val, castSelName, returnTypeString.c_str());
}

void CodeGenLexicalScope::InitialiseFunction(SmallVectorImpl<Value*> &Args,
	SmallVectorImpl<Value*> &Locals, unsigned locals, const char *MethodTypes,
	bool isSRet, const char **symbols, const std::string &humanName)
{
	// FIXME: This is a very long function and difficult to follow.  Split it
	// up into more sensibly-sized chunks.
	Module *TheModule = CGM->getModule();
	
	DIFactory *DebugFactory = CGM->getDebugFactory();
	DIDescriptor context = CGM->getModuleDescriptor();
	if (CodeGenLexicalScope *parent = getParentScope())
	{
		context = parent->getScopeDescriptor();
	}

	DICompileUnit ModuleScopeDescriptor  = CGM->getModuleDescriptor();
	DIFile ModuleSourceFile  = CGM->getSourceFileDescriptor();
	DIArray MethodArgDebugTypes = CGM->DebugTypeArrayForEncoding(MethodTypes);
	DIType MethodDebugType =
		DebugFactory->CreateCompositeType(llvm::dwarf::DW_TAG_subroutine_type,
				ModuleScopeDescriptor, "", ModuleSourceFile, 0, 0, 0, 0, 0, DIType(),
				MethodArgDebugTypes);

	// FIXME: Line number info.
	DIDescriptor ScopeDebugContext = DebugFactory->CreateSubprogram(context,
			humanName, humanName, CurrentFunction->getName(), ModuleSourceFile, 0,
			MethodDebugType, true, true);

	const PointerType *Int8PtrTy = PointerType::getUnqual(Type::getInt8Ty(CGM->Context));
	ReturnType = MethodTypes;
	llvm::Function::arg_iterator AI = CurrentFunction->arg_begin();
	if (isSRet)
	{
		++AI;
	}
	ScopeSelf = AI;
	const Type *Int1Ty = Type::getInt1Ty(CGM->Context);
	// Create the skeleton
	BasicBlock * EntryBB = llvm::BasicBlock::Create(CGM->Context, "entry", CurrentFunction);
	Builder.SetInsertPoint(EntryBB);
	// Flag indicating if we are in an exception handler.  Used for branching
	// later - should be removed by mem2reg and subsequent passes.
	Value *inException = Builder.CreateAlloca(Int1Ty, 0, "inException");
	Value *is_catch = 
		Builder.CreateAlloca(Int1Ty, 0, "is_catch");
	Value *exceptionPtr = 
		Builder.CreateAlloca(Int8PtrTy, 0, "exception_pointer");

	Builder.CreateStore(ConstantInt::get(Type::getInt1Ty(CGM->Context), 0), inException);
	Builder.CreateStore(ConstantPointerNull::get(Int8PtrTy), exceptionPtr);

	Type *PtrTy = PointerType::getUnqual(IntegerType::getInt8Ty(CGM->Context));
	// Create the context type
	std::vector<const Type*> contextTypes;
	contextTypes.push_back(IdTy);                 // 0 - isa
	if (CodeGenLexicalScope *parent = getParentScope())
	{
		// Set the parent context type correctly so we can use GEPs later
		contextTypes.push_back(parent->getContext()->getType());
	}
	else
	{
		contextTypes.push_back(IdTy);             // 1 - parent
	}
	contextTypes.push_back(IntTy);                // 2 - count
	contextTypes.push_back(PtrTy);                // 3 - Symbol table
	contextTypes.push_back(ScopeSelf->getType()); // 4 - Self
	// What else?	Some kind of 'promote me' flag?  Can I squeeze this in to
	// some other value?  Maybe use the sign bit on the count?

	// Space for self, the locals, and the args
	unsigned contextSize = locals +
		(CurrentFunction->getFunctionType()->getNumParams() - 2);
	for (unsigned i = 0 ; i < contextSize ; i++) 
	{
		contextTypes.push_back(IdTy);
	}
	StructType *contextType = StructType::get(CGM->Context, contextTypes);

	int contextOffset = CONTEXT_VARIABLE_OFFSET;

	/// We need a pointer hidden away in front of the context for storing
	// pointers to pointers to the context.
	std::vector<const Type*> frameTypes;
	frameTypes.push_back(PtrTy);
	frameTypes.push_back(contextType);
	StructType *frameType = StructType::get(CGM->Context, frameTypes);

	// Create the frame 
	Value *frame = Builder.CreateAlloca(frameType, 0, "frame");
	// Initialise the pointer to NULL
	Builder.CreateStore(ConstantPointerNull::get(cast<PointerType>(PtrTy)),
			Builder.CreateStructGEP(frame, 0));
	// Set the context to be the real context
	Context = Builder.CreateStructGEP(frame, 1);
	
	// Set the isa pointer
	Builder.CreateStore(
		Builder.CreateLoad(
			CGM->getModule()->getGlobalVariable(".smalltalk_context_stack_class", true)),
		Builder.CreateStructGEP(Context, 0, "context_isa"));

	// Set the symbol table
	if (0 != symbols && locals > 0)
	{
		std::vector<llvm::Constant*> symbolTableInitialiser;
		llvm::Constant *Zero = ConstantInt::get(IntTy, 0);
		for (unsigned i=0 ; i<locals ; i++)
		{
			llvm::Constant *symbol = CGM->MakeConstantString(symbols[i]);
			symbol = llvm::ConstantExpr::getGetElementPtr(symbol, (llvm::Constant*const*)&Zero, 1);
			symbolTableInitialiser.push_back(symbol);
		}
		llvm::ArrayType *tableTy = llvm::ArrayType::get(PtrTy, locals);
		llvm::Constant *initialiser = 
			llvm::ConstantArray::get(tableTy, symbolTableInitialiser);
		llvm::Value *symbolTable = new llvm::GlobalVariable(*TheModule,
				tableTy, true, llvm::GlobalValue::InternalLinkage,
				initialiser, "symbol_table");
		Builder.CreateStore(
			Builder.CreateBitCast(symbolTable, PtrTy),
			Builder.CreateStructGEP(Context, 3, "context_symbols"));
	}
	else
	{
		Builder.CreateStore(
			llvm::ConstantPointerNull::get(cast<llvm::PointerType>(PtrTy)),
			Builder.CreateStructGEP(Context, 3, "context_symbols"));
	}

	//// Set up the arguments

	// Set the number of arguments
	Builder.CreateStore(
		ConstantInt::get(IntTy, contextSize + 1),
		Builder.CreateStructGEP(Context, 2, "context_argc"));
	

	// Store the self pointer in context 0
	Builder.CreateStore(AI, 
			Builder.CreateStructGEP(Context, contextOffset++, "self_ptr"));
	++AI; ++AI; // Currently we don't expose _cmd / _call

	// Skip return value, self, _cmd
	NEXT(MethodTypes);
	NEXT(MethodTypes);
	NEXT(MethodTypes);

	int argumentIndex = 0;
	for (Function::arg_iterator end = CurrentFunction->arg_end() ; 
		AI != end ; ++AI) 
	{
		// FIXME: Pass the names of arguments in from the front end.
		// FIXME: Sensible line information
		// FIXME: This can't be the correct way of casting a DIDescriptor to a
		// DIType...
		DIVariable DebugArg =
			DebugFactory->CreateVariable(llvm::dwarf::DW_TAG_arg_variable,
					ScopeDebugContext, "Argument", ModuleSourceFile, 0,
					DIType(MethodArgDebugTypes.getElement(argumentIndex++)));
		Value * arg = Builder.CreateStructGEP(Context, contextOffset++, "arg");
		Args.push_back(arg);
		DebugArgs.push_back(DebugArg);
		//DebugFactory->InsertDeclare(arg, DebugArg, Builder.GetInsertBlock());
		Builder.CreateStore(BoxValue(&Builder, AI, MethodTypes), arg);
		NEXT(MethodTypes);
	}
	// Create the locals and initialise them to nil
	for (unsigned i = 0 ; i < locals ; i++) 
	{
		Value * local = 
			Builder.CreateStructGEP(Context, contextOffset++, "local");
		Locals.push_back(local);
		// Initialise the local to nil
		Builder.CreateStore(ConstantPointerNull::get(IdTy),
			local);

		if (0 != symbols)
		{
			DIVariable DebugLocal =
				DebugFactory->CreateVariable(llvm::dwarf::DW_TAG_auto_variable,
						ScopeDebugContext, symbols[i], ModuleSourceFile, 0,
						CGM->DebugTypeForEncoding("@"));
			DebugLocals.push_back(DebugLocal);
		}
		//DebugFactory->InsertDeclare(local, DebugLocal, Builder.GetInsertBlock());
	}
	
	/// Put self in a register so we can easily get at it later.

	// If this is the top-level scope then self is argument 0
	if (0 == getParentScope())
	{
		Self = Builder.CreateAlloca(ScopeSelf->getType());
		Builder.CreateStore(ScopeSelf, Self);
		Value *contextPtr = Builder.CreateStructGEP(Context, 1);
		Builder.CreateStore(ConstantPointerNull::get(IdTy), contextPtr);
	}
	else
	{
		// Navigate up to the top scope and look for self.
		
		SetParentScope();
		CodeGenLexicalScope *scope = this;
		Value *context = Context;
		while(0 != scope->getParentScope())
		{
			// Get a pointer to the parent in the context
			context = Builder.CreateStructGEP(context, 1);
			// Load it.
			context = Builder.CreateLoad(context);
			scope = scope->getParentScope();
		}
		Value *topScopeSelf =
			Builder.CreateLoad(Builder.CreateStructGEP(context,
						CONTEXT_VARIABLE_OFFSET), "selfval");
		Self = Builder.CreateAlloca(topScopeSelf->getType(), 0, "self");
		Builder.CreateStore(topScopeSelf, Self, true);
	}

	// Create a basic block for returns, reached only from the cleanup block
	const Type *RetTy = CGM->LLVMTypeFromString(ReturnType);
	RetVal = 0;
	if (RetTy != Type::getVoidTy(CGM->Context))
	{
		if (isSRet)
		{
			RetVal = CurrentFunction->arg_begin();
		}
		else
		{
			RetVal = Builder.CreateAlloca(RetTy, 0, "return_value");
		}
		// On id returns, default to returning self, otherwise default to 0.
		if (isObject(ReturnType))
		{
			Builder.CreateStore(LoadSelf(), RetVal);
		}
		else
		{
			Builder.CreateStore(Constant::getNullValue(RetTy), RetVal);
		}
	}
	/// Handle returns
	
	// Create the real return handler
	BasicBlock *realRetBB = llvm::BasicBlock::Create(CGM->Context, "return", CurrentFunction);
	IRBuilder<> ReturnBuilder = IRBuilder<>(realRetBB);
	const Type *functionRetTy =
		CurrentFunction->getFunctionType()->getReturnType();
	if (functionRetTy != llvm::Type::getVoidTy(CGM->Context)) 
	{
		Value * R = ReturnBuilder.CreateLoad(RetVal);
		if (functionRetTy != RetTy)
		{
			R = ReturnBuilder.CreateBitCast(R, functionRetTy);
		}
		ReturnBuilder.CreateRet(R);
	}
	else
	{
		ReturnBuilder.CreateRetVoid();
	}
	RetBB = llvm::BasicBlock::Create(CGM->Context, "finish", CurrentFunction);

	//// Setup the cleanup block

	CleanupBB = BasicBlock::Create(CGM->Context, "cleanup", CurrentFunction);
	IRBuilder<> CleanupBuilder = IRBuilder<>(CleanupBB);
	ExceptionBB = CleanupBB;

	// If we are returning a block that is currently on the stack, we need to
	// promote it first.  For now, we -retain / -autorelease every object
	// return.
	if (RetTy != Type::getVoidTy(CGM->Context) && isObject(ReturnType))
	{
		Value *retObj = CleanupBuilder.CreateLoad(RetVal);
		CGObjCRuntime *Runtime = CGM->getRuntime();
		if (isLKObject(ReturnType))
		{
			// Note: We can't use MessageSend() here, beucase it will try to
			// set the current BB as the unwind destination and things get
			// horribly confused.  We simple assume retain and release won't
			// throw for this case.  We also special-case the small int
			// versions to do nothing.
			Value *Int = CleanupBuilder.CreatePtrToInt(retObj, IntPtrTy);
			Value *IsSmallInt = CleanupBuilder.CreateTrunc(Int,
					Type::getInt1Ty(CGM->Context), "is_small_int");

			// Basic blocks for messages to SmallInts and ObjC objects:
			BasicBlock *continueBB = BasicBlock::Create(CGM->Context, "retained", CurrentFunction);
			BasicBlock *retainBB = BasicBlock::Create(CGM->Context, "retain_return", CurrentFunction);

			// Skip the message sends if this is a small int
			CleanupBuilder.CreateCondBr(IsSmallInt, continueBB, retainBB);

			CleanupBuilder.SetInsertPoint(retainBB);
			Value *retained = Runtime->GenerateMessageSend(CleanupBuilder,
					IdTy, false, NULL, retObj, "retain", NULL);
			Runtime->GenerateMessageSend(CleanupBuilder,
					Type::getVoidTy(CGM->Context), false, NULL, retObj,
					"autorelease", NULL);
			CleanupBuilder.CreateBr(continueBB);
			CleanupBuilder.SetInsertPoint(continueBB);

			PHINode *phi = CleanupBuilder.CreatePHI(retObj->getType(), "retained_return");
			phi->reserveOperandSpace(2);
			phi->addIncoming(retObj, CleanupBB);
			phi->addIncoming(retained, retainBB);
			retObj = phi;
		}
		else
		{
			retObj = Runtime->GenerateMessageSend(CleanupBuilder, IdTy, false,
					NULL, retObj, "retain", NULL);
			Runtime->GenerateMessageSend(CleanupBuilder,
					Type::getVoidTy(CGM->Context), false, NULL, retObj,
					"autorelease", NULL);
		}
		CleanupBuilder.CreateStore(retObj, RetVal);
	}
	CleanupEndBB = CleanupBuilder.GetInsertBlock();

	PromoteBB = BasicBlock::Create(CGM->Context, "promote", CurrentFunction);
	IRBuilder<> PromoteBuilder = IRBuilder<>(PromoteBB);

	// Get the current class of the context and the class of retained contexts
	// and cast both to integers for comparison.
	Value *retainedClass = PromoteBuilder.CreateLoad(
		CGM->getModule()->getGlobalVariable(".smalltalk_context_retained_class", true));
	retainedClass = PromoteBuilder.CreatePtrToInt(retainedClass, IntPtrTy);
	Value *contextClass = PromoteBuilder.CreateLoad(
			PromoteBuilder.CreateStructGEP(Context, 0));
	contextClass = PromoteBuilder.CreatePtrToInt(contextClass, IntPtrTy);

	// See whether the context has been retained
	Value *isContextRetained = PromoteBuilder.CreateICmpEQ(contextClass, retainedClass);

	// If so, we need to promote it to the heap, if not then jump to the return block
	BasicBlock *PromotionBB = BasicBlock::Create(CGM->Context, "promote_context", CurrentFunction);
	PromoteBuilder.CreateCondBr(isContextRetained, PromotionBB, RetBB);

	// Promote the context, if required
	PromoteBuilder.SetInsertPoint(PromotionBB);
	CGM->getRuntime()->GenerateMessageSend(PromoteBuilder, Type::getVoidTy(CGM->Context), false,
		Context, Context, "promote", NULL);

	//// Handle an exception


	//// Set up the exception landing pad.

	ExceptionBB = 
		BasicBlock::Create(CGM->Context, "non_local_return_handler", CurrentFunction);
	IRBuilder<> ExceptionBuilder = IRBuilder<>(ExceptionBB);
	Value *exception = ExceptionBuilder.CreateCall(
			llvm::Intrinsic::getDeclaration(TheModule,
				llvm::Intrinsic::eh_exception, 0, 0));
	ExceptionBuilder.CreateStore(exception, exceptionPtr);
	Value *ehPersonality =
		ExceptionBuilder.CreateBitCast(TheModule->getOrInsertFunction(
			"__LanguageKitEHPersonalityRoutine", Type::getVoidTy(CGM->Context), NULL),
			Int8PtrTy);

	Value *eh_selector = ExceptionBuilder.CreateCall4(
		llvm::Intrinsic::getDeclaration(TheModule,
			llvm::Intrinsic::eh_selector, 0, 0),
		exception, ehPersonality, 
		ConstantExpr::getIntToPtr(ConstantInt::get(IntPtrTy, 1), Int8PtrTy),
		ConstantPointerNull::get(Int8PtrTy));

	ExceptionBuilder.CreateStore(ExceptionBuilder.CreateTrunc(eh_selector, Int1Ty), is_catch);
	ExceptionBuilder.CreateStore(ConstantInt::get(Type::getInt1Ty(CGM->Context), 1), inException);
	ExceptionBuilder.CreateBr(CleanupBB);
	ExceptionBuilder.ClearInsertionPoint();


	BasicBlock *EHBlock = BasicBlock::Create(CGM->Context, "exception_handler", CurrentFunction);
	// Set the return block to jump to the EH block instead of the real return block
	// if we are unwinding.
	ReturnBuilder.SetInsertPoint(RetBB);
	ReturnBuilder.CreateCondBr(ReturnBuilder.CreateLoad(inException),
		EHBlock, realRetBB);
	// Jump to the exception handler if we did a cleanup after
	PromoteBuilder.CreateBr(RetBB);
	ExceptionBuilder.SetInsertPoint(EHBlock);

	BasicBlock *CatchBlock = BasicBlock::Create(CGM->Context,
			"non_local_return", CurrentFunction);
	BasicBlock *rethrowBB = BasicBlock::Create(CGM->Context, "rethrow", CurrentFunction);
	// If we've caught a non-local return, then we want to test if it's meant
	// to land here, otherwise we continue unwinding.
	ExceptionBuilder.CreateCondBr(ExceptionBuilder.CreateLoad(is_catch),
			CatchBlock, rethrowBB);

	ExceptionBuilder.SetInsertPoint(CatchBlock);
	// This function will rethrow if the frames do not match.  Otherwise it will
	// insert the correct 
	Value *RetPtr = RetVal;
	if (0 != RetVal)
	{
		RetPtr = ExceptionBuilder.CreateBitCast(RetVal, PtrTy);
	}
	else
	{
		RetPtr = ConstantPointerNull::get(cast<PointerType>(PtrTy));
	}

	Function *EHFunction = cast<Function>(
		TheModule->getOrInsertFunction("__LanguageKitTestNonLocalReturn",
			Type::getInt8Ty(CGM->Context), PtrTy, PtrTy, PtrTy, NULL));
	// Note: This is not an invoke - if this throws we want it to unwind up the
	// stack past the current frame.  If it didn't, we'd get an infinite loop,
	// with the function continually catching the non-local return exception
	// and rethrowing it.
	Value *isRet = ExceptionBuilder.CreateCall3(EHFunction, 
		ExceptionBuilder.CreateBitCast(Context, PtrTy),
		ExceptionBuilder.CreateLoad(exceptionPtr),
	   	RetPtr);
	isRet = ExceptionBuilder.CreateTrunc(isRet, Type::getInt1Ty(CGM->Context));
	ExceptionBuilder.CreateCondBr(isRet, realRetBB, rethrowBB);

	// Rethrow the exception
	ExceptionBuilder.SetInsertPoint(rethrowBB);
	Function *rethrowFunction = cast<Function>(
		TheModule->getOrInsertFunction("_Unwind_Resume_or_Rethrow",
			Type::getVoidTy(CGM->Context), PtrTy, NULL));
	ExceptionBuilder.CreateCall(rethrowFunction,
			ExceptionBuilder.CreateLoad(exceptionPtr));
	ExceptionBuilder.CreateUnreachable();

}

void CodeGenLexicalScope::EndScope(void)
{
	IRBuilder<> CleanupBuilder = IRBuilder<>(CleanupEndBB);
	if (containsBlocks)
	{
		CleanupBuilder.CreateBr(PromoteBB);
	}
	else
	{
		CleanupBuilder.CreateBr(RetBB);
	}
}

void CodeGenLexicalScope::UnboxArgs(IRBuilder<> *B,
                                    Function *F,
                                    llvm::SmallVectorImpl<llvm::Value*> &argv,
                                    llvm::SmallVectorImpl<llvm::Value*> &args,
                                    const char *selTypes) 
{
	if (NULL == selTypes) 
	{
		// All types are id
		for (unsigned i=0 ; i<argv.size() ; ++i) 
		{
			args.push_back(Unbox(B, F, argv[i], "@"));
		}
	} 
	else 
	{
		SkipTypeQualifiers(&selTypes);
		//Skip return, self, cmd
		NEXT(selTypes);
		NEXT(selTypes);
		for (unsigned i=0 ; i<argv.size() ; ++i) 
		{
			NEXT(selTypes);
			SkipTypeQualifiers(&selTypes);
			args.push_back(Unbox(B, F, argv[i], selTypes));
		}
	}
}

Value *CodeGenLexicalScope::MessageSendSuper(IRBuilder<> *B, Function *F, const
		char *selName, const char *selTypes,
		llvm::SmallVectorImpl<llvm::Value*> &argv) 
{
	Value *Sender = LoadSelf();
	Value *SelfPtr = Sender;

	llvm::SmallVector<Value*, 8> args;
	UnboxArgs(B, F, argv, args, selTypes);

	bool isSRet = false;
	const Type *realReturnType = NULL;
	FunctionType *MethodTy = CGM->LLVMFunctionTypeFromString(selTypes, isSRet,
			realReturnType);

	CGObjCRuntime *Runtime = CGM->getRuntime();

	llvm::Value *msg = Runtime->GenerateMessageSendSuper(*B,
		MethodTy->getReturnType(), isSRet, Sender,
		CGM->getSuperClassName().c_str(), SelfPtr, selName, selTypes, args,
		CGM->inClassMethod, CleanupBB);
	if (MethodTy->getReturnType() == realReturnType)
	{
		msg = Builder.CreateBitCast(msg, realReturnType);
	}
	return msg;
}

// Preform a real message send.  Reveicer must be a real object, not a
// SmallInt.
Value *CodeGenLexicalScope::MessageSendId(IRBuilder<> *B,
                                          Value *receiver,
                                          const char *selName,
                                          const char *selTypes,
                                          llvm::SmallVectorImpl<llvm::Value*> &argv)
{
	//FIXME: Find out why this crashes.
	Value *SelfPtr = NULL;//LoadSelf();

	bool isSRet = false;
	const Type *realReturnType = NULL;
	FunctionType *MethodTy = CGM->LLVMFunctionTypeFromString(selTypes, isSRet,
			realReturnType);

	CGObjCRuntime *Runtime = CGM->getRuntime();

	llvm::Value *msg = Runtime->GenerateMessageSend(*B,
			MethodTy->getReturnType(), isSRet, SelfPtr, receiver, selName,
			selTypes, argv, ExceptionBB);
	if (MethodTy->getReturnType() == realReturnType)
	{
		msg = Builder.CreateBitCast(msg, realReturnType);
	}
	return msg;
}

Value *CodeGenLexicalScope::MessageSend(IRBuilder<> *B, Function *F, Value
		*receiver, const char *selName, const char *selTypes)
{
	SmallVector<Value*,0> noArgs;
	return MessageSend(B, F, receiver, selName, selTypes, noArgs);
}
Value *CodeGenLexicalScope::MessageSend(IRBuilder<> *B,
                                        Function *F,
                                        Value *receiver,
                                        const char *selName,
                                        const char *selTypes,
                                        SmallVectorImpl<Value*> &boxedArgs)
{
	//LOG("Sending message to %s", selName);
	//LOG(" (%s)\n", selTypes);
	Value *Int = B->CreatePtrToInt(receiver, IntPtrTy);
	Value *IsSmallInt = B->CreateTrunc(Int, Type::getInt1Ty(CGM->Context), "is_small_int");

	// Basic blocks for messages to SmallInts and ObjC objects:
	BasicBlock *SmallInt = BasicBlock::Create(CGM->Context, string("small_int_message") + selName, F);
	IRBuilder<> SmallIntBuilder = IRBuilder<>(SmallInt);
	BasicBlock *RealObject = BasicBlock::Create(CGM->Context, string("real_object_message") +
			selName, F);

	IRBuilder<> RealObjectBuilder = IRBuilder<>(RealObject);
	// Branch to whichever is the correct implementation
	B->CreateCondBr(IsSmallInt, SmallInt, RealObject);
	B->ClearInsertionPoint();

	Value *Result = 0;
	// Basic block for rejoining the two cases.
	BasicBlock *Continue = BasicBlock::Create(CGM->Context, "Continue", F);

	// See if there is a function defined to implement this message
	Value *SmallIntFunction =
		getSmallIntModuleFunction(CGM, FunctionNameFromSelector(selName));

	BasicBlock *smallIntContinueBB = 0;

	// Send a message to a small int, using a static function or by promoting to
	// a big int.
	if (0 != SmallIntFunction)
	{
		smallIntContinueBB = 
			BasicBlock::Create(CGM->Context, "small_int_bitcast_result", CurrentFunction);
		SmallVector<Value*, 8> Args;
		Args.push_back(receiver);
		Args.append(boxedArgs.begin(), boxedArgs.end());
		for (unsigned i=0 ; i<Args.size() ;i++)
		{
			const Type *ParamTy =
				cast<Function>(SmallIntFunction)->getFunctionType()->getParamType(i);
			if (Args[i]->getType() != ParamTy)
			{
				Args[i] = SmallIntBuilder.CreateBitCast(Args[i], ParamTy);
			}
		}
		Result = SmallIntBuilder.CreateInvoke(SmallIntFunction,
			smallIntContinueBB, ExceptionBB, Args.begin(), Args.end(),
			"small_int_message_result");
		SmallIntBuilder.ClearInsertionPoint();
	}
	else
	{
		//Promote to big int and send a real message.
		Value *BoxFunction = getSmallIntModuleFunction(CGM, "BoxSmallInt");
		Result = SmallIntBuilder.CreateBitCast(receiver, IdTy);
		Result = SmallIntBuilder.CreateCall(BoxFunction, Result,
			"boxed_small_int");
		SmallVector<Value*, 8> unboxedArgs;
		UnboxArgs(&SmallIntBuilder, F, boxedArgs, unboxedArgs, selTypes);
		Result = MessageSendId(&SmallIntBuilder, Result, selName, selTypes,
				unboxedArgs);
		smallIntContinueBB = SmallIntBuilder.GetInsertBlock();
	}
	SmallInt = 0;

	SmallVector<Value*, 8> unboxedArgs;
	UnboxArgs(&RealObjectBuilder, F, boxedArgs, unboxedArgs, selTypes);
	Value *ObjResult = MessageSendId(&RealObjectBuilder, receiver, selName,
		selTypes, unboxedArgs);
	// This will create some branches - get the new basic block.
	RealObject = RealObjectBuilder.GetInsertBlock();

	SmallIntBuilder.SetInsertPoint(smallIntContinueBB);
	if ((Result->getType() != ObjResult->getType())
			&& (ObjResult->getType() != Type::getVoidTy(CGM->Context)))
	{
		Result = SmallIntBuilder.CreateBitCast(Result, ObjResult->getType(), 
			"cast_small_int_result");
	}
	SmallIntBuilder.CreateBr(Continue);

	
	// Join the two paths together again:

	RealObjectBuilder.CreateBr(Continue);
	B->SetInsertPoint(Continue);
	if (ObjResult->getType() != Type::getVoidTy(CGM->Context))
	{
		PHINode *Phi = B->CreatePHI(Result->getType(),	selName);
		Phi->reserveOperandSpace(2);
		Phi->addIncoming(Result, smallIntContinueBB);
		Phi->addIncoming(ObjResult, RealObject);
		return Phi;
	}
	return ConstantPointerNull::get(IdTy);
}

Value *CodeGenLexicalScope::LoadArgumentAtIndex(unsigned index, unsigned depth)
{
	if (0 == depth)
	{
		return Builder.CreateLoad(Args[index]); 
	}
	Value *context = Context;
	for (unsigned i=0 ; i<depth ; ++i)
	{
		// Get a pointer to the parent in the context
		context = Builder.CreateStructGEP(context, 1);
		// Load it.
		context = Builder.CreateLoad(context);
	}
	return Builder.CreateLoad(
		Builder.CreateStructGEP(context, CONTEXT_VARIABLE_OFFSET + 1 + index));
}

Value *CodeGenLexicalScope::LoadLocalAtIndex(unsigned index, unsigned depth) 
{ 
	LOG("Loading local %d, depth %d \n", index, depth);
	if (0 == depth)
	{
		DUMP(Locals[index]);
		return Builder.CreateLoad(Locals[index]); 
	}
	CodeGenLexicalScope *scope = this;
	Value *context = Context;
	for (unsigned i=0 ; i<depth ; ++i)
	{
		// Get a pointer to the parent in the contect
		context = Builder.CreateStructGEP(context, 1);
		// Load it.
		context = Builder.CreateLoad(context);
		scope = scope->getParentScope();
	}
	return Builder.CreateLoad(Builder.CreateStructGEP(context, 
				CONTEXT_VARIABLE_OFFSET + 1 + index + scope->Args.size()));
}

Value *CodeGenLexicalScope::LoadSelf(void) 
{
	return Builder.CreateLoad(Self, true);
}

void CodeGenLexicalScope::StoreValueInLocalAtIndex(Value * value, unsigned
		index, unsigned depth)
{
	if (value->getType() != IdTy) 
	{
		value = Builder.CreateBitCast(value, IdTy);
	}
	if (0 == depth)
	{
		Builder.CreateStore(value, Locals[index]);
		return;
	}
	CodeGenLexicalScope *scope = this;
	Value *context = Context;
	for (unsigned i=0 ; i<depth ; ++i)
	{
		// Get a pointer to the parent in the contect
		context = Builder.CreateStructGEP(context, 1);
		// Load it.
		context = Builder.CreateLoad(context);
		scope = scope->getParentScope();
	}
	LOG("Storing local at index %d, depth %d.  ", index, depth);
	LOG("Offset is: %d\n", CONTEXT_VARIABLE_OFFSET + 1 + index + scope->Args.size());
	DUMP(value);
	// Locals go after args in the context
	Builder.CreateStore(value, Builder.CreateStructGEP(context,
		CONTEXT_VARIABLE_OFFSET + 1 + index + scope->Args.size()));
}

Value *CodeGenLexicalScope::LoadClass(const char *classname)
{
	return CGM->getRuntime()->LookupClass(Builder,
		CGM->MakeConstantString(classname));
}

Value *CodeGenLexicalScope::LoadValueOfTypeAtOffsetFromObject(
	const char *className,
	const char *ivarName,
	const char *type,
	unsigned offset,
	Value *object)
{
	// FIXME: Non-id loads
	assert(isLKObject(type));

	Value *Offset = 
		CGM->getRuntime()->OffsetOfIvar(Builder, className, ivarName, offset);
	Value *addr = Builder.CreatePtrToInt(object, IntTy);
	addr = Builder.CreateAdd(addr, Offset);
	addr = Builder.CreateIntToPtr(addr, PointerType::getUnqual(IdTy));
	return Builder.CreateLoad(addr, true, "ivar");
}

// Generate a printf() call with the specified string and value.  Used for
// debugging.
void CodeGenLexicalScope::CreatePrintf(IRBuilder<> &Builder,
                                       const char *str, 
                                       Value *val)
{
	std::vector<const Type*> Params;
	Params.push_back(PointerType::getUnqual(Type::getInt8Ty(CGM->Context)));
	Value *PrintF = CGM->getModule()->getOrInsertFunction("printf", 
			FunctionType::get(Type::getVoidTy(CGM->Context), Params, true));
	Builder.CreateCall2(PrintF, CGM->MakeConstantString(str), val);
}

void CodeGenLexicalScope::StoreValueOfTypeAtOffsetFromObject(
	Value *value,
	const char *className,
	const char *ivarName,
	const char* type,
	unsigned offset,
	Value *object)
{
	CGObjCRuntime *Runtime = CGM->getRuntime();
	// Turn the value into something valid for storing in this ivar
	Value *box = Unbox(&Builder, CurrentFunction, value, type);
	// Calculate the offset of the ivar
	Value *Offset = Runtime->OffsetOfIvar(Builder, className, ivarName, offset);
	Value *addr = Builder.CreatePtrToInt(object, IntTy);
	addr = Builder.CreateAdd(addr, Offset);
	addr = Builder.CreateIntToPtr(addr, PointerType::getUnqual(box->getType()),
		"ivar");
	// Do the ASSIGN() thing if it's an object.
	if (type[0] == '@')
	{
	// Some objects may return a different object when retained.	Store that
	// instead.
		box = Runtime->GenerateMessageSend(Builder, IdTy, false, NULL, box,
			"retain", NULL);
		Value *old = Builder.CreateLoad(addr);
		Runtime->GenerateMessageSend(Builder, Type::getVoidTy(CGM->Context), false, NULL, old,
			"autorelease", NULL);
	}
	else if (isLKObject(type))
	{
		box = MessageSend(&Builder, CurrentFunction, box, "retain", 0);
		Value *old = Builder.CreateLoad(addr);
		MessageSend(&Builder, CurrentFunction, old, "autorelease", 0);
	}
	Builder.CreateStore(box, addr, true);
}

void CodeGenLexicalScope::EndChildBlock(CodeGenBlock *block) {
	containsBlocks = true;
}

Value *CodeGenLexicalScope::ComparePointers(Value *lhs, Value *rhs)
{
	lhs = Builder.CreatePtrToInt(lhs, IntPtrTy);
	rhs = Builder.CreatePtrToInt(rhs, IntPtrTy);
	Value *result = Builder.CreateICmpEQ(rhs, lhs, "pointer_compare_result");
	result = Builder.CreateZExt(result, IntPtrTy);
	result = Builder.CreateShl(result, ConstantInt::get(IntPtrTy, 1));
	result = Builder.CreateOr(result, ConstantInt::get(IntPtrTy, 1));
	return Builder.CreateIntToPtr(result, IdTy);
}

Value *CodeGenLexicalScope::IntConstant(const char *value)
{
	return CGM->IntConstant(Builder, value);
}
Value *CodeGenLexicalScope::FloatConstant(const char *value)
{
	return CGM->FloatConstant(Builder, value);
}
Value *CodeGenLexicalScope::SymbolConstant(const char *symbol)
{
	return CGM->SymbolConstant(Builder, symbol);
}

Value *CodeGenLexicalScope::MessageSendId(Value *receiver,
                                          const char *selName,
                                          const char *selTypes,
                                          SmallVectorImpl<Value*> &argv)
{
	SmallVector<Value*, 8> args;
	UnboxArgs(&Builder, CurrentFunction, argv, args, selTypes);
	LOG("Generating object message send %s\n", selName);
	return BoxValue(&Builder, MessageSendId(&Builder, receiver, selName,
		selTypes, args), selTypes);
}

Value *CodeGenLexicalScope::MessageSendSuper(const char *selName,
                                             const char *selTypes,
                                             SmallVectorImpl<Value*> &argv)
{
	return BoxValue(&Builder, MessageSendSuper(&Builder, CurrentFunction, selName,
		selTypes, argv), selTypes);
}
Value *CodeGenLexicalScope::MessageSend(Value *receiver,
                                        const char *selName,
                                        const char *selTypes,
                                        SmallVectorImpl<Value*> &argv)
{
	LOG("Generating %s (%s)\n", selName, selTypes);
	Value *msg = MessageSend(&Builder, CurrentFunction, receiver, selName,
			selTypes, argv);
	return BoxValue(&Builder, msg, selTypes);
}
Value *CodeGenLexicalScope::LoadClassVariable(string className, string
		cVarName)
{
	return CGM->getRuntime()->LoadClassVariable(Builder, className, cVarName);
}
void CodeGenLexicalScope::StoreValueInClassVariable(string className, string
		cVarName, Value *object)
{
	object = MessageSend(&Builder, CurrentFunction, object, "retain", 0);
	Value *old = LoadClassVariable(className, cVarName);
	MessageSend(&Builder, CurrentFunction, old, "autorelease", 0);
	CGM->getRuntime()->StoreClassVariable(Builder, className, cVarName, object);
}

BasicBlock *CodeGenLexicalScope::StartBasicBlock(const char* BBName)
{
	BasicBlock * newBB = llvm::BasicBlock::Create(CGM->Context, BBName, CurrentFunction);
	Builder.SetInsertPoint(newBB);
	return newBB;
}

BasicBlock *CodeGenLexicalScope::CurrentBasicBlock(void)
{
	return Builder.GetInsertBlock();
}

void CodeGenLexicalScope::MoveInsertPointToBasicBlock(BasicBlock *BB)
{
	Builder.SetInsertPoint(BB);
}

void CodeGenLexicalScope::GoTo(BasicBlock *BB)
{
	Builder.CreateBr(BB);
	Builder.SetInsertPoint(BB);
}

void CodeGenLexicalScope::BranchOnCondition(Value *condition,
		BasicBlock *TrueBB, BasicBlock *FalseBB)
{
	// Make the condition an int
	Value *lhs = Builder.CreatePtrToInt(condition, IntPtrTy);
	// SmallInt value NO (0 << 1 & 1)
	Value *rhs = ConstantInt::get(IntPtrTy, 1);
	// If condition != NO
	Value *result = Builder.CreateICmpNE(rhs, lhs, "pointer_compare_result");
	Builder.CreateCondBr(result, TrueBB, FalseBB);
}

void CodeGenLexicalScope::SetReturn(Value * Ret) 
{
	if (Ret != 0)
	{
		if (Ret->getType() != IdTy) 
		{
			Ret = Builder.CreateBitCast(Ret, IdTy);
		}
		Ret = Unbox(&Builder, CurrentFunction, Ret, ReturnType);
		Builder.CreateStore(Ret, RetVal);
	}
	Builder.CreateBr(CleanupBB);
	Builder.ClearInsertionPoint();
}

CodeGenLexicalScope::~CodeGenLexicalScope()
{
	BasicBlock *BB = Builder.GetInsertBlock();
	if (0 != BB && 0 == BB->getTerminator())
	{
		SetReturn();
	}
}
