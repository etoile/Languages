#include <llvm/Support/IRBuilder.h>
#include <llvm/Module.h>
#include "CodeGenModule.h"
#include "CodeGenBlock.h"
#include "ABI.h"
#include "LLVMCompat.h"
#import <Foundation/Foundation.h>
#import "../LanguageKit.h"
#import "../Runtime/LKObject.h"
using namespace etoile::languagekit;

/**
 * Returns true if the provided type encoding is the type encoding for a
 * LanguageKit object.  Note that this can't use -isEqualToString:, because the
 * typeEncoding string may have some trailing data.
 */
static bool isLKObject(NSString *typeEncoding)
{
	// TODO: This can all go away now that libobjc2 supports small objects.
	static const char*LKObjectEncoding = @encode(LKObject);
	static const int encodingLength = strlen(LKObjectEncoding);
	if ([typeEncoding length] < encodingLength) { return false; }
	char encoding[encodingLength+1];
	[typeEncoding getCString: encoding
	               maxLength: encodingLength+1
	                encoding: NSUTF8StringEncoding];
	encoding[encodingLength] = 0;
	return strncmp(LKObjectEncoding, encoding, encodingLength) == 0;
}

/**
 * Returns true if the type encoding represents any object type - id, Class,
 * or LKObject.
 */
static bool isObject(NSString *encoding)
{
	char first = (char)[encoding characterAtIndex: 0];
	return (first == '@') || (first == '#') || isLKObject(encoding);
}

/**
 * Returns a reference to a function in the Small Int module.  
 */
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

string CodeGenSubroutine::FunctionNameFromSelector(NSString *sel)
{
	// Special cases
	switch ([sel characterAtIndex: 0]) {
		case '+':
			return "SmallIntMsgplus_";
		case '-':
			return "SmallIntMsgsub_";
		case '/':
			return "SmallIntMsgdiv_";
		case '*':
			return "SmallIntMsgmul_";
		default: {
			// TODO: Now that we support message sends to small ints, we should
			// just use them and only try to inline methods where speed matters.
			string str = "SmallIntMsg" + string([sel UTF8String]);
			replace(str.begin(), str.end(), ':', '_');
			return str;
		}
	}
}

Value *CodeGenSubroutine::BoxValue(CGBuilder *B, Value *V, NSString *typestr)
{
	CGObjCRuntime *Runtime = CGM->getRuntime();
	// Untyped selectors return id
	if (NULL == typestr || ([typestr length] == 0)) return V;
	// Special case for LKObjects
	if (isLKObject(typestr))
	{
		return V;
	}
	NSUInteger typeIndex = 0;
	unichar type = [typestr characterAtIndex: 0];
	// FIXME: Other function type qualifiers
	while(type == 'V' || type == 'r')
	{
		typeIndex++;
		type = [typestr characterAtIndex: typeIndex];
	}
	switch(type)
	{
		// All integer primitives smaller than a 64-bit value
		case 'B': case 'c': case 'C': case 's': case 'S': case 'i': case 'I':
		case 'l': case 'L':
			LOG("Boxing return value %s\n", [typestr UTF8String]);
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
			NSString *castSelName;
			if (type == 'f')
			{
				castSelName = @"boxedFloatWithFloat:";
			}
			else
			{
				castSelName = @"boxedFloatWithDouble:";
			}
			Value *boxed = Runtime->GenerateMessageSend(*B, types.idTy, false,
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
			return Runtime->GenerateMessageSend(*B, types.idTy, false, NULL,
					SymbolCalss, @"SymbolForSelector:", NULL, V);
		}
		case '{':
		{
			Value *NSValueClass = B->CreateLoad(
				CGM->getModule()->getGlobalVariable(
					".smalltalk_nsvalue_class", true));
			NSString * castSelName = @"valueWithBytes:objCType:";
			bool passValue = false;
			if ([typestr rangeOfString: @"{_NSRect"].location == 0)
			{
				castSelName = @"valueWithRect:";
				passValue = true;
			}
			else if ([typestr rangeOfString: @"{_NSRange"].location == 0)
			{
				castSelName = @"valueWithRange:";
				passValue = true;
			}
			else if ([typestr rangeOfString: @"{_NSPoint"].location == 0)
			{
				castSelName = @"valueWithPoint:";
				passValue = true;
			}
			else if ([typestr rangeOfString: @"{_NSSize"].location == 0)
			{
				castSelName = @"valueWithSize:";
				passValue = true;
			}
			if (passValue)
			{
				Value *boxed = Runtime->GenerateMessageSend(*B, types.idTy, false,
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
			NSUInteger end = 0;
			while (!isdigit([typestr characterAtIndex: end])) { end++; }
			SmallVector<Value*,2> args;
			args.push_back(V);
			args.push_back(CGM->MakeConstantString([typestr substringToIndex: end]));
			return Runtime->GenerateMessageSend(*B, types.idTy, false, LoadSelf(),
					NSValueClass, @"valueWithBytesOrNil:objCType:", NULL, args);
		}
		// Map void returns to nil
		case 'v':
		{
			return ConstantPointerNull::get(types.idTy);
		}
		// If it's already an object, we don't need to do anything
		case '@': case '#':
			return V;
	}
}


Value *CodeGenSubroutine::Unbox(CGBuilder *B,
                                  Function *F,
                                  Value *val,
                                  NSString *type)
{
	// Special case for LKObjects
	if (isLKObject(type))
	{
		return val;
	}
	NSString *returnTypeString = [type substringToIndex: 1];
	NSString *castSelName;
	switch([type characterAtIndex: 0])
	{
		case 'c':
			castSelName = @"charValue";
			break;
		case 'C':
			castSelName = @"unsignedCharValue";
			break;
		case 's':
			castSelName = @"shortValue";
			break;
		case 'S':
			castSelName = @"unsignedShortValue";
			break;
		case 'i':
			castSelName = @"intValue";
			break;
		case 'I':
			castSelName = @"unsignedIntValue";
			break;
		case 'l':
			castSelName = @"longValue";
			break;
		case 'L':
			castSelName = @"unsignedLongValue";
			break;
		case 'q':
			castSelName = @"longLongValue";
			break;
		case 'Q':
			castSelName = @"unsignedLongLongValue";
			break;
		case 'f':
			castSelName = @"floatValue";
			break;
		case 'd':
			castSelName = @"doubleValue";
			break;
		case 'B':
			castSelName = @"boolValue";
			break;
		case ':':
			castSelName = @"selValue";
			break;
		case '#':
		case '@':
		{
			Value *BoxFunction = getSmallIntModuleFunction(CGM, "BoxObject");
			val = B->CreateBitCast(val, types.idTy);
			return B->CreateCall(BoxFunction, val, "boxed_small_int");
		}
		case 'v':
			return val;
		case '{':
		{
			NSUInteger end = 0;
			NSUInteger length = [type length];
			int braceCount = 1;
			while (end < length)
			{
				unichar c = [type characterAtIndex: end];
				if (c == '{')
				{
					braceCount++;
				}
				else if (c == '}')
				{
					braceCount--;
					if (0 == braceCount)
					{
						break;
					}
				}
				end++;
			}
			returnTypeString = [type substringToIndex: end];
			//Special cases for NSRect and NSPoint
			if ([type rangeOfString: @"{_NSRect"].location == 0)
			{
				castSelName = @"rectValue";
				break;
			}
			if ([type rangeOfString: @"{_NSRange"].location == 0)
			{
				castSelName = @"rangeValue";
				break;
			}
			if ([type rangeOfString: @"{_NSPoint"].location == 0)
			{
				castSelName = @"pointValue";
				break;
			}
			if ([type rangeOfString: @"{_NSSize"].location == 0)
			{
				castSelName = @"sizeValue";
				break;
			}
		}
		default:
			LOG("Found type value: %s\n", [type UTF8String]);
			castSelName = @"";
			assert(false && "Unable to transmogriy object to compound type");
	}
	//TODO: We don't actually use the size numbers for anything, but someone else
	//does, so make these sensible:
	returnTypeString = [returnTypeString stringByAppendingString: @"12@0:4"];
	return MessageSend(B, F, val, castSelName, returnTypeString);
}

llvm::Value *CodeGenSubroutine::loadByRefPointer(llvm::Value *byRefPointer)
{
	// Load the forwarding pointer
	llvm::Value *forward = Builder.CreateLoad(Builder.CreateStructGEP(byRefPointer, 1));
	// Cast it to a block type
	forward = Builder.CreateBitCast(forward, byRefPointer->getType());
	// Now return the address of the real value.
	return Builder.CreateStructGEP(forward, 6);
}

llvm::Value *CodeGenSubroutine::emitByRefStructure(void)
{
	llvm::Constant *copyFn = getSmallIntModuleFunction(CGM, "LKByRefKeep");
	llvm::Constant *disposeFn = getSmallIntModuleFunction(CGM, "LKByRefDispose");
	// TODO: Currently, we are emitting one byref structure per variable.  We
	// should really emit one per block, for groups of variables passed to a
	// single block.
	llvm::StructType *byRefType = GetStructType(CGM->Context,
	                                            types.idTy,         // 0 isa 
	                                            types.ptrToVoidTy,  // 1 forwarding
	                                            types.intTy,        // 2 flags
	                                            types.intTy,        // 3 size
	                                            copyFn->getType(),   // 4 keep 
	                                            disposeFn->getType(),// 5 dispose
	                                            types.idTy,          // 6 value
	                                            NULL);
	llvm::Value *byRef = Builder.CreateAlloca(byRefType);
	llvm::Constant *nilPtr = llvm::ConstantPointerNull::get(types.idTy);
	Builder.CreateStore(nilPtr, Builder.CreateStructGEP(byRef, 0));
	// Forwarding pointer points to self initially.
	Builder.CreateStore(Builder.CreateBitCast(byRef, types.ptrToVoidTy),
		Builder.CreateStructGEP(byRef, 1));
	// Flags set to 1<<25 to indicate copy-dispose helpers.
	Builder.CreateStore(ConstantInt::get(types.intTy, (1<<25)),
	                    Builder.CreateStructGEP(byRef, 2));
	llvm::Constant *size = llvm::ConstantExpr::getSizeOf(byRefType);
	if (size->getType() != types.intTy)
	{
		size = llvm::ConstantExpr::getTrunc(size, types.intTy);
	}
	// Size of the byref structure.  When we are storing multiple bound values
	Builder.CreateStore(size,
	                    Builder.CreateStructGEP(byRef, 3));
	Builder.CreateStore(copyFn, Builder.CreateStructGEP(byRef, 4));
	Builder.CreateStore(disposeFn, Builder.CreateStructGEP(byRef, 5));
	Builder.CreateStore(nilPtr, Builder.CreateStructGEP(byRef, 6));
	return byRef;
}

void CodeGenSubroutine::initializeVariableWithValue(LKSymbol *aSym, llvm::Value *val)
{
	NSString *typeEncoding = [aSym typeEncoding];
	// TODO: Eventually we want to allow local variables of non-object types.
	llvm::Type *type = types.idTy;
	if (0 != val)
	{
		val = BoxValue(&Builder, val, typeEncoding);
	}

	// If this variable is referenced by blocks, make it indirect.  Otherwise,
	// make it direct.
	if ([aSym referencingScopes] > 0)
	{
		llvm::Value *var = emitByRefStructure();
		if (0 != val)
		{
			Builder.CreateStore(val, loadByRefPointer(var));
		}
		indirect_variables[[aSym name]] = var;
	}
	else
	{
		if (0 == val)
		{
			val = llvm::Constant::getNullValue(type);
		}
		llvm::Value *var = Builder.CreateAlloca(type);
		Builder.CreateStore(val, var);
		variables[[aSym name]] = var;
	}
}
void CodeGenSubroutine::releaseVariable(llvm::Value *val)
{
	CGBuilder cleanupBuilder(CleanupEndBB);
	CGBuilder smallIntBuilder(CGM->Context);
	splitSmallIntCase(val, cleanupBuilder, smallIntBuilder);
	CGM->assign->releaseValue(cleanupBuilder, val);
	llvm::PHINode *unused;
	combineSmallIntCase(0, 0, unused, cleanupBuilder, smallIntBuilder);
	CleanupEndBB = cleanupBuilder.GetInsertBlock();
}

void CodeGenSubroutine::InitialiseFunction(NSString *functionName,
                                           NSArray *arguments,
                                           NSArray *locals,
                                           NSString *typeEncoding,
                                           BOOL returnsRetained)
{
	ReturnType = [typeEncoding retain];
	// FIXME: This is a very long function and difficult to follow.  Split it
	// up into more sensibly-sized chunks.
	Module *TheModule = CGM->getModule();

	bool isSRet;
	llvm::Type *retTy;
	llvm::FunctionType *type = types.functionTypeFromString(typeEncoding, isSRet, retTy);

	CurrentFunction = llvm::Function::Create(type,
		llvm::GlobalValue::InternalLinkage,
		[functionName UTF8String],
		TheModule);
	llvm::BasicBlock *entryBB =
		llvm::BasicBlock::Create(CGM->Context, "entry", CurrentFunction);
	Builder.SetInsertPoint(entryBB);

	llvm::Function::arg_iterator AI = CurrentFunction->arg_begin();
	if (isSRet)
	{
		++AI;
	}
	for (LKSymbol *symbol in arguments)
	{
		initializeVariableWithValue(symbol, AI);
		++AI;
	}
	// Entry point into the clanup
	CleanupBB = BasicBlock::Create(CGM->Context, "cleanup", CurrentFunction);
	// End point of the cleanup
	CleanupEndBB = CleanupBB;
	for (LKSymbol *symbol in locals)
	{
		initializeVariableWithValue(symbol, 0);
		// Make sure that it's released
		llvm::Value *var = variables[[symbol name]];
		if (0 == var)
		{
			var = indirect_variables[[symbol name]];
			assert(0 != var);
			var = loadByRefPointer(var);
		}
		CGBuilder cleanupBuilder(CleanupEndBB);
		releaseVariable(cleanupBuilder.CreateLoad(var));
	}

	LLVMType *Int1Ty = Type::getInt1Ty(CGM->Context);
	// Flag indicating if we are in an exception handler.  Used for branching
	// later - should be removed by mem2reg and subsequent passes.
	Value *inException = Builder.CreateAlloca(Int1Ty, 0, "inException");
	Value *is_catch =
		Builder.CreateAlloca(Int1Ty, 0, "is_catch");
	Value *exceptionPtr =
		Builder.CreateAlloca(types.ptrToVoidTy, 0, "exception_pointer");
	Context = Builder.CreateAlloca(types.ptrToVoidTy, 0, "context");

	Builder.CreateStore(ConstantInt::get(Type::getInt1Ty(CGM->Context), 0), inException);
	Builder.CreateStore(ConstantPointerNull::get(types.ptrToVoidTy), exceptionPtr);

	// Create a basic block for returns, reached only from the cleanup block
	RetVal = 0;
	if (retTy != Type::getVoidTy(CGM->Context))
	{
		if (isSRet)
		{
			RetVal = CurrentFunction->arg_begin();
		}
		else
		{
			RetVal = Builder.CreateAlloca(retTy, 0, "return_value");
		}
		Builder.CreateStore(Constant::getNullValue(retTy), RetVal);
	}
	/// Handle returns

	// Create the real return handler
	BasicBlock *realRetBB = llvm::BasicBlock::Create(CGM->Context, "return", CurrentFunction);
	CGBuilder ReturnBuilder(realRetBB);

	// If this is returning an object, autorelease it.
	if (retTy != Type::getVoidTy(CGM->Context) && isObject(ReturnType) && !returnsRetained)
	{
		Value *retObj = ReturnBuilder.CreateLoad(RetVal);
		if (isLKObject(ReturnType))
		{
			CGBuilder smallIntBuilder(CGM->Context);
			splitSmallIntCase(retObj, ReturnBuilder, smallIntBuilder);
			llvm::Value *autoreleased =
				CGM->assign->autoreleaseReturnValue(ReturnBuilder, retObj);
			PHINode *phi;
			combineSmallIntCase(autoreleased, retObj, phi, ReturnBuilder,
					smallIntBuilder);
			retObj = phi;
		}
		else
		{
			retObj = CGM->assign->autoreleaseReturnValue(ReturnBuilder, retObj);
		}
		ReturnBuilder.CreateStore(retObj, RetVal);
	}
	LLVMType *functionRetTy =
		CurrentFunction->getFunctionType()->getReturnType();
	if (functionRetTy != llvm::Type::getVoidTy(CGM->Context))
	{
		Value * R = ReturnBuilder.CreateLoad(RetVal);
		if (functionRetTy != R->getType())
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


	//// Handle an exception

	//// Set up the exception landing pad.

	ExceptionBB =
		BasicBlock::Create(CGM->Context, "non_local_return_handler", CurrentFunction);
	CGBuilder ExceptionBuilder(ExceptionBB);
	Value *exception = ExceptionBuilder.CreateCall(
			llvm::Intrinsic::getDeclaration(TheModule,
				llvm::Intrinsic::eh_exception ));
	ExceptionBuilder.CreateStore(exception, exceptionPtr);
	Value *ehPersonality =
		ExceptionBuilder.CreateBitCast(TheModule->getOrInsertFunction(
			"__LanguageKitEHPersonalityRoutine", Type::getVoidTy(CGM->Context), NULL),
			types.ptrToVoidTy);

	Value *eh_selector = ExceptionBuilder.CreateCall4(
		llvm::Intrinsic::getDeclaration(TheModule,
			llvm::Intrinsic::eh_selector),
		exception, ehPersonality,
		TheModule->getOrInsertGlobal("__LanguageKitNonLocalReturn", Type::getInt8Ty(CGM->Context)),
		ConstantPointerNull::get(types.ptrToVoidTy));

	ExceptionBuilder.CreateStore(ExceptionBuilder.CreateTrunc(eh_selector, Int1Ty), is_catch);
	ExceptionBuilder.CreateStore(ConstantInt::get(Type::getInt1Ty(CGM->Context), 1), inException);
	CreatePrintf(ExceptionBuilder, @"Caught exeptioni %d\n", ExceptionBuilder.CreateTrunc(eh_selector, Int1Ty));
	ExceptionBuilder.CreateBr(CleanupBB);
	ExceptionBuilder.ClearInsertionPoint();


	BasicBlock *EHBlock = BasicBlock::Create(CGM->Context, "exception_handler", CurrentFunction);
	// Set the return block to jump to the EH block instead of the real return block
	// if we are unwinding.
	ReturnBuilder.SetInsertPoint(RetBB);
	ReturnBuilder.CreateCondBr(ReturnBuilder.CreateLoad(inException),
		EHBlock, realRetBB);
	// Jump to the exception handler if we did a cleanup after
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
	LLVMType *PtrTy = types.ptrToVoidTy;
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
			Type::getVoidTy(CGM->Context), PtrTy, PtrTy, PtrTy, NULL));
	// Note: This is not an invoke - if this throws we want it to unwind up the
	// stack past the current frame.  If it didn't, we'd get an infinite loop,
	// with the function continually catching the non-local return exception
	// and rethrowing it.
	ExceptionBuilder.CreateCall3(EHFunction,
		ExceptionBuilder.CreateBitCast(Context, PtrTy),
		ExceptionBuilder.CreateLoad(exceptionPtr),
	   	RetPtr);
	ExceptionBuilder.CreateBr(realRetBB);

	// Rethrow the exception
	ExceptionBuilder.SetInsertPoint(rethrowBB);

	// Free the return value before rethrowing - all other locals should
	// already have been freed by this point.
	if (retTy != Type::getVoidTy(CGM->Context) && isObject(ReturnType))
	{
		Value *retObj = ExceptionBuilder.CreateLoad(RetVal);
		if (isLKObject(ReturnType))
		{
			CGBuilder smallIntBuilder(CGM->Context);
			splitSmallIntCase(retObj, ExceptionBuilder, smallIntBuilder);
			CGM->assign->releaseValue(ExceptionBuilder, retObj);
			llvm::PHINode *unused;
			combineSmallIntCase(0, 0, unused, ExceptionBuilder, smallIntBuilder);
		}
		else
		{
			CGM->assign->releaseValue(ExceptionBuilder, retObj);
		}
	}

	Function *rethrowFunction = cast<Function>(
		TheModule->getOrInsertFunction("_Unwind_Resume_or_Rethrow",
			Type::getVoidTy(CGM->Context), PtrTy, NULL));
	ExceptionBuilder.CreateCall(rethrowFunction,
			ExceptionBuilder.CreateLoad(exceptionPtr));
	ExceptionBuilder.CreateUnreachable();

}

void CodeGenSubroutine::EndScope(void)
{
	CGBuilder CleanupBuilder(CleanupEndBB);
	CleanupBuilder.CreateBr(RetBB);
}
void CodeGenSubroutine::storeValueInVariable(llvm::Value *value, NSString *aVariable)
{
	llvm::Value *var = variables[aVariable];
	if (0 == var)
	{
		var = indirect_variables[aVariable];
		assert(0 != var);
		var = loadByRefPointer(var);
	}
	CGM->assign->storeLocal(Builder, var, value);
}
llvm::Value* CodeGenSubroutine::loadVariable(NSString *aVariable)
{
	llvm::Value *var = variables[aVariable];
	if (0 == var)
	{
		var = indirect_variables[aVariable];
		assert(0 != var);
		var = loadByRefPointer(var);
	}
	return CGM->assign->loadLocal(Builder, var);
}

static NSString *skipQualifiers(NSString *types)
{
	NSUInteger start = 0;
	while (1)
	{
		switch ([types characterAtIndex: start])
		{
			default: 
				return [types substringFromIndex: start];
			case 'r': case 'n': case 'N': 
			case 'o': case 'O': case 'V': 
				start++;
		}
	}
	return types;
}
static NSString *nextArgument(NSString *types)
{
	const char *start = [types UTF8String];
	const char *typestr = start;
	while (!(*typestr == '\0') && !isdigit(*typestr)) { typestr++; }
	while (isdigit(*typestr)) { typestr++; }
	return [types substringFromIndex: (typestr - start)];
}

void CodeGenSubroutine::UnboxArgs(CGBuilder *B,
                                    Function *F,
                                    llvm::SmallVectorImpl<llvm::Value*> &argv,
                                    llvm::SmallVectorImpl<llvm::Value*> &args,
                                    NSString *selTypes,
                                    bool skipImplicit)
{
	if (NULL == selTypes)
	{
		// All types are id
		for (unsigned i=0 ; i<argv.size() ; ++i)
		{
			args.push_back(Unbox(B, F, argv[i], @"@"));
		}
	}
	else
	{
		selTypes = skipQualifiers(selTypes);
		//Skip return, self, cmd
		if (skipImplicit)
		{
			selTypes = nextArgument(selTypes);
			selTypes = nextArgument(selTypes);
		}
		for (unsigned i=0 ; i<argv.size() ; ++i)
		{
			selTypes = nextArgument(selTypes);
			selTypes = skipQualifiers(selTypes);
			args.push_back(Unbox(B, F, argv[i], selTypes));
		}
	}
}

Value *CodeGenSubroutine::MessageSendSuper(CGBuilder *B, Function *F,
		NSString *selName, NSString *selTypes,
		llvm::SmallVectorImpl<llvm::Value*> &argv)
{
	Value *Sender = LoadSelf();
	Value *SelfPtr = Sender;

	llvm::SmallVector<Value*, 8> args;
	UnboxArgs(B, F, argv, args, selTypes);

	bool isSRet = false;
	LLVMType *realReturnType = NULL;
	FunctionType *MethodTy = types.functionTypeFromString(selTypes,
		isSRet, realReturnType);

	MethodFamily family = MethodFamilyForSelector(selName);

	CGObjCRuntime *Runtime = CGM->getRuntime();

	llvm::Value *msg = Runtime->GenerateMessageSendSuper(*B,
		MethodTy->getReturnType(), isSRet, Sender,
		CGM->SuperClassName, SelfPtr, selName, selTypes, args,
		CGM->inClassMethod, ExceptionBB);
	if (MethodTy->getReturnType() == realReturnType)
	{
		msg = Builder.CreateBitCast(msg, realReturnType);
	}
	// If we send a super message in the init family, it may consume self and
	// generate a new self.  We store this to self, because no other use is
	// valid.
	if (family == Init)
	{
		CGM->assign->storeLocal(*B, Self, msg);
	}
	if (isObject(selTypes))
	{
		// Retain the result, and then release it at the end.
		msg = CGM->assign->retainResult(*B, selName, msg);
	}
	return msg;
}
llvm::Value* CodeGenSubroutine::LoadBlockContext()
{
	return ConstantPointerNull::get(types.idTy);
}

// Preform a real message send.  Reveicer must be a real object, not a
// SmallInt.
Value *CodeGenSubroutine::MessageSendId(CGBuilder *B,
                                        Value *receiver,
                                        NSString *selName,
                                        NSString *selTypes,
                                        llvm::SmallVectorImpl<llvm::Value*> &argv)
{
	//FIXME: Find out why this crashes.
	Value *SelfPtr = NULL;//LoadSelf();

	bool isSRet = false;
	LLVMType *realReturnType = NULL;
	FunctionType *MethodTy =
		types.functionTypeFromString(selTypes, isSRet,
			realReturnType);

	CGObjCRuntime *Runtime = CGM->getRuntime();
	if (MethodFamilyForSelector(selName) == Init)
	{
		CGM->assign->retainValue(*B, receiver);
	}

	llvm::Value *msg = Runtime->GenerateMessageSend(*B,
			MethodTy->getReturnType(), isSRet, SelfPtr, receiver, selName,
			selTypes, argv, ExceptionBB);
	if (MethodTy->getReturnType() == realReturnType)
	{
		msg = B->CreateBitCast(msg, realReturnType);
	}
	if (isObject(selTypes))
	{
		// Retain the result, and then release it at the end.
		msg = CGM->assign->retainResult(*B, selName, msg);
	}
	return msg;
}

Value *CodeGenSubroutine::MessageSend(CGBuilder *B, Function *F, Value
		*receiver, NSString *selName, NSString *selTypes)
{
	SmallVector<Value*,0> noArgs;
	return MessageSend(B, F, receiver, selName, selTypes, noArgs);
}
Value *CodeGenSubroutine::MessageSend(CGBuilder *B,
                                        Function *F,
                                        Value *receiver,
                                        NSString *selName,
                                        NSString *selTypes,
                                        SmallVectorImpl<Value*> &boxedArgs)
{
	//LOG("Sending message to %s", selName);
	//LOG(" (%s)\n", selTypes);
	Value *Int = B->CreatePtrToInt(receiver, types.intPtrTy);
	Value *IsSmallInt = B->CreateTrunc(Int, Type::getInt1Ty(CGM->Context), "is_small_int");

	// Basic blocks for messages to SmallInts and ObjC objects:
	BasicBlock *SmallInt =
		BasicBlock::Create(CGM->Context, "small_int_message" , F);
	CGBuilder SmallIntBuilder(SmallInt);
	BasicBlock *RealObject = BasicBlock::Create(CGM->Context,
			"real_object_message", F);

	CGBuilder RealObjectBuilder(RealObject);
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
			LLVMType *ParamTy =
				cast<Function>(SmallIntFunction)->getFunctionType()->getParamType(i);
			if (Args[i]->getType() != ParamTy)
			{
				Args[i] = SmallIntBuilder.CreateBitCast(Args[i], ParamTy);
			}
		}
		Result = IRBuilderCreateInvoke(&SmallIntBuilder, SmallIntFunction,
				smallIntContinueBB, ExceptionBB, Args,
				"small_int_message_result");
		SmallIntBuilder.ClearInsertionPoint();
	}
	else
	{
		//Promote to big int and send a real message.
		Value *BoxFunction = getSmallIntModuleFunction(CGM, "BoxSmallInt");
		Result = SmallIntBuilder.CreateBitCast(receiver, types.idTy);
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
		PHINode *Phi = IRBuilderCreatePHI(B, Result->getType(), 2);
		Phi->addIncoming(Result, smallIntContinueBB);
		Phi->addIncoming(ObjResult, RealObject);
		return Phi;
	}
	return ConstantPointerNull::get(types.idTy);
}


Value *CodeGenSubroutine::LoadSelf(void)
{
	return Builder.CreateLoad(Self, true);
}

Value *CodeGenSubroutine::LoadClass(NSString *classname)
{
	return CGM->getRuntime()->LookupClass(Builder,
		CGM->MakeConstantString(classname));
}

Value *CodeGenSubroutine::LoadValueOfTypeAtOffsetFromObject(
	NSString *className,
	NSString *ivarName,
	NSString *type,
	unsigned offset,
	Value *object)
{
	// Get the offset
	Value *Offset =
		CGM->getRuntime()->OffsetOfIvar(Builder, className, ivarName, offset);
	// Add the offset to the object address
	Value *addr = Builder.CreatePtrToInt(object, types.intTy);
	addr = Builder.CreateAdd(addr, Offset);
	// Cast it to a pointer of the correct type
	addr = Builder.CreateIntToPtr(addr, PointerType::getUnqual(types.typeFromString(type)));
	// Load it and box it.
	return BoxValue(&Builder, Builder.CreateLoad(addr, true, "ivar"), type);
}

// Generate a printf() call with the specified string and value.  Used for
// debugging.
void CodeGenSubroutine::CreatePrintf(CGBuilder &Builder,
                                       NSString *str,
                                       Value *val)
{
	std::vector<LLVMType*> Params;
	Params.push_back(PointerType::getUnqual(Type::getInt8Ty(CGM->Context)));
	Value *PrintF = CGM->getModule()->getOrInsertFunction("printf",
			FunctionType::get(Type::getVoidTy(CGM->Context), Params, true));
	Builder.CreateCall2(PrintF, CGM->MakeConstantString(str), val);
}

void CodeGenSubroutine::StoreValueOfTypeAtOffsetFromObject(
	Value *value,
	NSString *className,
	NSString *ivarName,
	NSString* type,
	unsigned offset,
	Value *object)
{
	CGObjCRuntime *Runtime = CGM->getRuntime();
	// Turn the value into something valid for storing in this ivar
	Value *box = Unbox(&Builder, CurrentFunction, value, type);
	// Calculate the offset of the ivar
	Value *Offset = Runtime->OffsetOfIvar(Builder, className, ivarName, offset);
	// Do the ASSIGN() thing if it's an object.
	if ([type characterAtIndex: 0] == '@' || isLKObject(type))
	{
		// FIXME: With libobjc2 1.5, we need to special case this for small
		// ints.  That should probably be done in the CodeGenAssignments class,
		// which can just check for the small objects capability.
		CGM->assign->storeIvar(Builder, object, Offset, box);
		return;
	}
	Value *addr = Builder.CreatePtrToInt(object, types.intTy);
	addr = Builder.CreateAdd(addr, Offset);
	addr = Builder.CreateIntToPtr(addr, PointerType::getUnqual(box->getType()),
		"ivar");
	Builder.CreateStore(box, addr, true);
}

void CodeGenSubroutine::EndChildBlock(CodeGenBlock *block) {
	containsBlocks = true;
}

Value *CodeGenSubroutine::ComparePointers(Value *lhs, Value *rhs)
{
	lhs = Builder.CreatePtrToInt(lhs, types.intPtrTy);
	rhs = Builder.CreatePtrToInt(rhs, types.intPtrTy);
	Value *result = Builder.CreateICmpEQ(rhs, lhs, "pointer_compare_result");
	result = Builder.CreateZExt(result, types.intPtrTy);
	result = Builder.CreateShl(result, ConstantInt::get(types.intPtrTy, 1));
	result = Builder.CreateOr(result, ConstantInt::get(types.intPtrTy, 1));
	return Builder.CreateIntToPtr(result, types.idTy);
}

Value *CodeGenSubroutine::IntConstant(NSString *value)
{
	return CGM->IntConstant(Builder, value);
}
Value *CodeGenSubroutine::FloatConstant(NSString *value)
{
	return CGM->FloatConstant(Builder, value);
}
Value *CodeGenSubroutine::SymbolConstant(NSString *symbol)
{
	return CGM->SymbolConstant(Builder, symbol);
}

Value *CodeGenSubroutine::MessageSendId(Value *receiver,
                                          NSString *selName,
                                          NSString *selTypes,
                                          SmallVectorImpl<Value*> &argv)
{
	SmallVector<Value*, 8> args;
	UnboxArgs(&Builder, CurrentFunction, argv, args, selTypes);
	LOG("Generating object message send %s\n", [selName UTF8String]);
	return BoxValue(&Builder, MessageSendId(&Builder, receiver, selName,
		selTypes, args), selTypes);
}
Value *CodeGenSubroutine::CallFunction(NSString *functionName,
                                         NSString *argTypes,
                                         SmallVectorImpl<Value*> &argv)
{
	SmallVector<Value*, 8> args;
	UnboxArgs(&Builder, CurrentFunction, argv, args, argTypes, false);

	bool isSRet = false;
	LLVMType *realReturnType = NULL;

	FunctionType *functionTy = types.functionTypeFromString(argTypes,
			isSRet, realReturnType);

	Function *f =
		static_cast<Function*>(CGM->getModule()->getOrInsertFunction([functionName
					UTF8String], functionTy));

	// Construct the call
	llvm::SmallVector<llvm::Value*, 8> callArgs;
	llvm::Value *sret = 0;
	if (isSRet)
	{
		sret = Builder.CreateAlloca(realReturnType);
		callArgs.push_back(sret);
	}
	if (PASS_STRUCTS_AS_POINTER)
	{
		llvm::Value* callArg;
		for (unsigned int i = 0; i < args.size() ; i++) {
			callArg = args[i];
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
		callArgs.insert(callArgs.end(), args.begin(), args.end());
	}
	llvm::Instruction *ret = 0;
	if (0 != CleanupBB)
	{
		llvm::BasicBlock *continueBB =
			llvm::BasicBlock::Create(CGM->Context, "invoke_continue",
					Builder.GetInsertBlock()->getParent());
		ret = IRBuilderCreateInvoke(&Builder, f, continueBB, CleanupBB,
			callArgs);
		Builder.SetInsertPoint(continueBB);
		if (isSRet)
		{
			cast<llvm::InvokeInst>(ret)->addAttribute(1, Attribute::StructRet);
		}
	}
	else
	{
		ret = IRBuilderCreateCall(&Builder, f, callArgs);
		if (isSRet)
		{
			cast<llvm::CallInst>(ret)->addAttribute(1, Attribute::StructRet);
		}
	}
	if (isSRet)
	{
		ret = Builder.CreateLoad(sret);
	}
	return BoxValue(&Builder, ret, argTypes);
}

Value *CodeGenSubroutine::MessageSendSuper(NSString *selName,
                                             NSString *selTypes,
                                             SmallVectorImpl<Value*> &argv)
{
	return BoxValue(&Builder, MessageSendSuper(&Builder, CurrentFunction, selName,
		selTypes, argv), selTypes);
}
Value *CodeGenSubroutine::MessageSend(Value *receiver,
                                      NSString *selName,
                                      NSString *selTypes,
                                      SmallVectorImpl<Value*> &argv)
{
	LOG("Generating %s (%s)\n", [selName UTF8String], [selTypes UTF8String]);
	Value *msg = MessageSend(&Builder, CurrentFunction, receiver, selName,
			selTypes, argv);
	return BoxValue(&Builder, msg, selTypes);
}
Value *CodeGenSubroutine::LoadClassVariable(NSString *className,
                                            NSString *cVarName)
{
	return
		Builder.CreateLoad(CGM->getRuntime()->AddressOfClassVariable(Builder,
					className, cVarName));
}
void CodeGenSubroutine::StoreValueInClassVariable(NSString *className, 
                                                  NSString *cVarName,
                                                  Value *object)
{
	CGM->assign->storeGlobal(Builder,
			CGM->getRuntime()->AddressOfClassVariable(Builder, className,
				cVarName), object);
}

BasicBlock *CodeGenSubroutine::StartBasicBlock(NSString* BBName)
{
	BasicBlock * newBB = llvm::BasicBlock::Create(CGM->Context, [BBName
			UTF8String], CurrentFunction);
	Builder.SetInsertPoint(newBB);
	return newBB;
}

BasicBlock *CodeGenSubroutine::CurrentBasicBlock(void)
{
	return Builder.GetInsertBlock();
}

void CodeGenSubroutine::MoveInsertPointToBasicBlock(BasicBlock *BB)
{
	Builder.SetInsertPoint(BB);
}

void CodeGenSubroutine::GoTo(BasicBlock *BB)
{
	Builder.CreateBr(BB);
	Builder.SetInsertPoint(BB);
}

void CodeGenSubroutine::BranchOnCondition(Value *condition,
		BasicBlock *TrueBB, BasicBlock *FalseBB)
{
	// Make the condition an int
	Value *lhs = Builder.CreatePtrToInt(condition, types.intPtrTy);
	// SmallInt value NO (0 << 1 & 1)
	Value *rhs = ConstantInt::get(types.intPtrTy, 1);
	// If condition != NO
	Value *result = Builder.CreateICmpNE(rhs, lhs, "pointer_compare_result");
	Builder.CreateCondBr(result, TrueBB, FalseBB);
}

void CodeGenSubroutine::SetReturn(Value * Ret)
{
	if (Ret != 0)
	{
		if (Ret->getType() != types.idTy)
		{
			Ret = Builder.CreateBitCast(Ret, types.idTy);
		}
		Ret = Unbox(&Builder, CurrentFunction, Ret, ReturnType);
		CGM->assign->storeLocal(Builder, RetVal, Ret);
	}
	Builder.CreateBr(CleanupBB);
	Builder.ClearInsertionPoint();
}

void CodeGenSubroutine::splitSmallIntCase(llvm::Value *anObject,
                                          CGBuilder &aBuilder,
                                          CGBuilder &smallIntBuilder)
{
	Value *Int = aBuilder.CreatePtrToInt(anObject, types.intPtrTy);
	Value *IsSmallInt = aBuilder.CreateTrunc(Int,
			Type::getInt1Ty(CGM->Context), "is_small_int");

	BasicBlock *smallIntBB = BasicBlock::Create(CGM->Context, "small_int", CurrentFunction);
	BasicBlock *objectBB = BasicBlock::Create(CGM->Context, "real_object", CurrentFunction);

	aBuilder.CreateCondBr(IsSmallInt, smallIntBB, objectBB);
	aBuilder.SetInsertPoint(objectBB);
	smallIntBuilder.SetInsertPoint(smallIntBB);
}
void CodeGenSubroutine::combineSmallIntCase(llvm::Value *anObject,
                                            llvm::Value *aSmallInt,
                                            llvm::PHINode *&phi,
                                            CGBuilder &objectBuilder,
                                            CGBuilder &smallIntBuilder)
{
	BasicBlock *continueBB = BasicBlock::Create(CGM->Context, "continue", CurrentFunction);
	BasicBlock *objectBB = objectBuilder.GetInsertBlock();
	BasicBlock *smallIntBB = smallIntBuilder.GetInsertBlock();
	phi = 0;
	objectBuilder.CreateBr(continueBB);
	smallIntBuilder.CreateBr(continueBB);
	// Invalidate the two builders so we'll get an error if we try using them again.
	objectBuilder.SetInsertPoint(continueBB);
	smallIntBuilder.ClearInsertionPoint();

	if (0 != anObject)
	{
		phi = IRBuilderCreatePHI(&objectBuilder, anObject->getType(), 2);
		if (anObject->getType() != aSmallInt->getType())
		{
			aSmallInt = smallIntBuilder.CreateBitCast(aSmallInt, anObject->getType());
		}
		phi->addIncoming(anObject, objectBB);
		phi->addIncoming(aSmallInt, smallIntBB);
	}
}

CodeGenSubroutine::~CodeGenSubroutine()
{
	BasicBlock *BB = Builder.GetInsertBlock();
	if (0 != BB && 0 == BB->getTerminator())
	{
		SetReturn();
	}
}
