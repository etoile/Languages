#include "CodeGenLexicalScope.h"
#include "CodeGenBlock.h"
#include "CodeGenModule.h"
#include <llvm/Module.h>

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

string CodeGenLexicalScope::FunctionNameFromSelector(const char *sel) {
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

Value *CodeGenLexicalScope::BoxValue(IRBuilder<> *B, Value *V, const char *typestr) {
  CGObjCRuntime *Runtime = CGM->getRuntime();
  // Untyped selectors return id
  if (NULL == typestr || '\0' == *typestr) return V;
  // FIXME: Other function type qualifiers
  while(*typestr == 'V' || *typestr == 'r')
  {
    typestr++;
  }
  switch(*typestr) {
    // All integer primitives smaller than a 64-bit value
    case 'B': case 'c': case 'C': case 's': case 'S': case 'i': case 'I':
    case 'l': case 'L':
      LOG("Boxing return value %s\n", typestr);
      V = B->CreateSExt(V, Type::Int64Ty);
    // Now V is 64-bits.
    case 'q': case 'Q':
    {
      // This will return a SmallInt or a promoted integer.
	  Constant *BoxFunction = getSmallIntModuleFunction(CGM, "MakeSmallInt");
      CallInst *boxed = B->CreateCall(BoxFunction, V);
      boxed->setOnlyReadsMemory();
      return boxed;
    }
    case ':': {
      // TODO: Store this in a global.
      Value *SymbolCalss = Runtime->LookupClass(*B,
          CGM->MakeConstantString("Symbol"));
      return Runtime->GenerateMessageSend(*B, IdTy, NULL, SymbolCalss,
              Runtime->GetSelector(*B, "SymbolForCString:", NULL), &V, 1);
    }
    case '{': {
      Value *NSValueClass = Runtime->LookupClass(*B,
        CGM->MakeConstantString("NSValue"));
      const char * castSelName = "valueWithBytes:objCType:";
      bool passValue = false;
        if (0 == strncmp(typestr, "{_NSRect", 8)) {
        castSelName = "valueWithRect:";
        passValue = true;
      } else if (0 == strncmp(typestr, "{_NSRange", 9)) {
        castSelName = "valueWithRange:";
        passValue = true;
      } else if (0 == strncmp(typestr, "{_NSPoint", 9)) {
        castSelName = "valueWithPoint:";
        passValue = true;
      } else if (0 == strncmp(typestr, "{_NSSize", 8)) {
        castSelName = "valueWithSize:";
        passValue = true;
      }
      if (passValue) {
        Value *boxed = Runtime->GenerateMessageSend(*B, IdTy, NULL, NSValueClass,
          Runtime->GetSelector(*B, castSelName, NULL), &V, 1);
        if (CallInst *call = dyn_cast<llvm::CallInst>(boxed)) {
            call->setOnlyReadsMemory();
        }
        return boxed;
      }
      assert(0 && "Boxing arbitrary structures doesn't work yet");
    }
    // Other types, just wrap them up in an NSValue
    default:
    {
      // TODO: Store this in a global.
      Value *NSValueClass = Runtime->LookupClass(*B,
          CGM->MakeConstantString("NSValue"));
      // TODO: We should probably copy this value somewhere, maybe with a
      // custom object instead of NSValue?
      // TODO: Should set sender to self.
      const char *end = typestr;
      while (!isdigit(*end)) { end++; }
      string typestring = string(typestr, end - typestr);
      Value *args[] = {V, CGM->MakeConstantString(typestring.c_str())};
      return Runtime->GenerateMessageSend(*B, IdTy, NULL, NSValueClass,
              Runtime->GetSelector(*B, "valueWithBytesOrNil:objCType:", NULL),
              args, 2);
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
Value *CodeGenLexicalScope::Unbox(IRBuilder<> *B, Function *F, Value
    *val, const char *Type) {
  string returnTypeString = string(1, *Type);
  const char *castSelName;
  // TODO: Factor this out into a name-for-type function
  switch(*Type) {
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
    case '@': {
      Value *BoxFunction = getSmallIntModuleFunction(CGM, "BoxObject");
      val = B->CreateBitCast(val, IdTy);
      return B->CreateCall(BoxFunction, val, "boxed_small_int");
    }
    case 'v':
      return val;
  case '{': {
    const char *end = Type;
    while(!isdigit(*end)) { end++; }
    returnTypeString = string(Type, (int)(end - Type));
    //Special cases for NSRect and NSPoint
    if (0 == strncmp(Type, "{_NSRect", 8)) {
      castSelName = "rectValue";
      break;
    }
    if (0 == strncmp(Type, "{_NSRange", 9)) {
      castSelName = "rangeValue";
      break;
    }
    if (0 == strncmp(Type, "{_NSPoint", 9)) {
      castSelName = "pointValue";
      break;
    }
    if (0 == strncmp(Type, "{_NSSize", 8)) {
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
  SmallVectorImpl<Value*> &Locals, unsigned locals, const char *MethodTypes) {
	ReturnType = MethodTypes;
	// Create the skeleton
    BasicBlock * EntryBB = llvm::BasicBlock::Create("entry", CurrentFunction);
    Builder.SetInsertPoint(EntryBB);

    // Set up the arguments
    llvm::Function::arg_iterator AI = CurrentFunction->arg_begin();
    Self = Builder.CreateAlloca(AI->getType(), 0, "self.addr");
    Builder.CreateStore(AI, Self);
    ++AI; ++AI;
    // Skip return value, self, _cmd
    NEXT(MethodTypes);
    NEXT(MethodTypes);
    NEXT(MethodTypes);
    for(Function::arg_iterator end = CurrentFunction->arg_end() ; AI != end ;
        ++AI) {
      Value * arg = Builder.CreateAlloca(IdTy, 0, "arg");
      Args.push_back(arg);
      // Initialise the local to nil
      Builder.CreateStore(BoxValue(&Builder, AI, MethodTypes), arg);
      NEXT(MethodTypes);
    }
    // Create the locals and initialise them to nil
    for (unsigned i = 0 ; i < locals ; i++) {
      Value * local = Builder.CreateAlloca(IdTy, 0, "local");
      Locals.push_back(local);
      // Initialise the local to nil
      Builder.CreateStore(ConstantPointerNull::get(IdTy),
          local);
    }

    // Create a basic block for returns, reached only from the cleanup block
	const Type *RetTy = LLVMTypeFromString(ReturnType);
	RetVal = 0;
	if (RetTy != Type::VoidTy)
	{
    	RetVal = Builder.CreateAlloca(RetTy, 0, "return_value");
		Builder.CreateStore(Constant::getNullValue(RetTy), RetVal);
	}
    BasicBlock * RetBB = llvm::BasicBlock::Create("return", CurrentFunction);
    IRBuilder<> ReturnBuilder = IRBuilder<>(RetBB);
	if (CurrentFunction->getFunctionType()->getReturnType() !=
			llvm::Type::VoidTy) {
      Value * R = ReturnBuilder.CreateLoad(RetVal);
      ReturnBuilder.CreateRet(R);
    } else {
      ReturnBuilder.CreateRetVoid();
    }

    // Setup the cleanup block
    CleanupBB = BasicBlock::Create("cleanup", CurrentFunction);
    ReturnBuilder = IRBuilder<>(CleanupBB);
    ReturnBuilder.CreateBr(RetBB);

}

void CodeGenLexicalScope::UnboxArgs(IRBuilder<> *B, Function *F,  Value **
    argv, Value **args, unsigned argc, const char *selTypes) {
  // FIXME: For objects, we need to turn SmallInts into ObjC objects
  if (NULL == selTypes) {
    // All types are id, so do nothing
    memcpy(args, argv, sizeof(Value*) * argc);
  } else {
    SkipTypeQualifiers(&selTypes);
    //Skip return, self, cmd
    NEXT(selTypes);
    NEXT(selTypes);
    for (unsigned i=0 ; i<argc ; ++i) {
      NEXT(selTypes);
      SkipTypeQualifiers(&selTypes);
      args[i] = Unbox(B, F, argv[i], selTypes);
    }
  }
}

Value *CodeGenLexicalScope::MessageSendSuper(IRBuilder<> *B, Function *F, const
		char *selName, const char *selTypes, Value **argv, unsigned argc) 
{
	Value *Sender = LoadSelf();
	Value *SelfPtr = Sender;

	Value *args[argc];
	UnboxArgs(B, F, argv, args, argc, selTypes);

	FunctionType *MethodTy = LLVMFunctionTypeFromString(selTypes);

	CGObjCRuntime *Runtime = CGM->getRuntime();

	llvm::Value *cmd = Runtime->GetSelector(*B, selName, selTypes);
	return Runtime->GenerateMessageSendSuper(*B, MethodTy->getReturnType(),
			Sender, CGM->getSuperClassName().c_str(), SelfPtr, cmd, args, argc,
			CGM->inClassMethod);
}

// Preform a real message send.  Reveicer must be a real object, not a
// SmallInt.
Value *CodeGenLexicalScope::MessageSendId(IRBuilder<> *B, Value *receiver,
    const char *selName, const char *selTypes, Value **argv, unsigned argc) {
	//FIXME: Find out why this crashes.
  Value *SelfPtr = NULL;//LoadSelf();

  FunctionType *MethodTy = LLVMFunctionTypeFromString(selTypes);
  CGObjCRuntime *Runtime = CGM->getRuntime();
  llvm::Value *cmd = Runtime->GetSelector(*B, selName, selTypes);
  return Runtime->GenerateMessageSend(*B, MethodTy->getReturnType(), SelfPtr,
      receiver, cmd, argv, argc);
}

Value *CodeGenLexicalScope::MessageSend(IRBuilder<> *B, Function *F, Value *receiver,
    const char *selName, const char *selTypes, Value **argv, Value **boxedArgs,
    unsigned argc) {
  //LOG("Sending message to %s", selName);
  //LOG(" (%s)\n", selTypes);
  Value *Int = B->CreatePtrToInt(receiver, IntPtrTy);
  Value *IsSmallInt = B->CreateTrunc(Int, Type::Int1Ty, "is_small_int");

  // Basic blocks for messages to SmallInts and ObjC objects:
  BasicBlock *SmallInt = BasicBlock::Create(string("small_int_message") + selName, F);
  IRBuilder<> SmallIntBuilder = IRBuilder<>(SmallInt);
  BasicBlock *RealObject = BasicBlock::Create(string("real_object_message") +
      selName, F);

  IRBuilder<> RealObjectBuilder = IRBuilder<>(RealObject);
  // Branch to whichever is the correct implementation
  B->CreateCondBr(IsSmallInt, SmallInt, RealObject);
  B->ClearInsertionPoint();

  Value *Result = 0;

  // See if there is a function defined to implement this message
  Value *SmallIntFunction =
    getSmallIntModuleFunction(CGM, FunctionNameFromSelector(selName));

  // Send a message to a small int, using a static function or by promoting to
  // a big int.
  if (0 != SmallIntFunction) {
    SmallVector<Value*, 8> Args;
    Args.push_back(receiver);
    Args.insert(Args.end(), boxedArgs, boxedArgs+argc);
    for (unsigned i=0 ; i<Args.size() ;i++) {
      const Type *ParamTy =
        cast<Function>(SmallIntFunction)->getFunctionType()->getParamType(i);
      if (Args[i]->getType() != ParamTy) {
        Args[i] = SmallIntBuilder.CreateBitCast(Args[i], ParamTy);
      }
    }
    Result = SmallIntBuilder.CreateCall(SmallIntFunction, Args.begin(),
        Args.end(), "small_int_message_result");
  } else {
    //Promote to big int and send a real message.
    Value *BoxFunction = getSmallIntModuleFunction(CGM, "BoxSmallInt");
    Result = SmallIntBuilder.CreateBitCast(receiver, IdTy);
    Result = SmallIntBuilder.CreateCall(BoxFunction, Result,
        "boxed_small_int");
    Result = MessageSendId(&SmallIntBuilder, Result, selName, selTypes, argv,
        argc);
  }
  Value *args[argc];
  UnboxArgs(&RealObjectBuilder, F, argv, args, argc, selTypes);
  // This will create some branches - get the new basic block.
  RealObject = RealObjectBuilder.GetInsertBlock();
  Value *ObjResult = MessageSendId(&RealObjectBuilder, receiver, selName,
      selTypes, args, argc);

  if ((Result->getType() != ObjResult->getType())
      && (ObjResult->getType() != Type::VoidTy)) {
    Result = new BitCastInst(Result, ObjResult->getType(),
        "cast_small_int_result", SmallInt);
  }
  
  // Join the two paths together again:
  BasicBlock *Continue = BasicBlock::Create("Continue", F);

  RealObjectBuilder.CreateBr(Continue);
  SmallIntBuilder.CreateBr(Continue);
  B->SetInsertPoint(Continue);
  if (ObjResult->getType() != Type::VoidTy) {
    PHINode *Phi = B->CreatePHI(Result->getType(),  selName);
    Phi->reserveOperandSpace(2);
    Phi->addIncoming(Result, SmallInt);
    Phi->addIncoming(ObjResult, RealObject);
    return Phi;
  }
  return ConstantPointerNull::get(IdTy);
}

Value *CodeGenLexicalScope::LoadArgumentAtIndex(unsigned index) { 
  return Builder.CreateLoad(Args[index]); 
}

Value *CodeGenLexicalScope::LoadLocalAtIndex(unsigned index) { 
  return Builder.CreateLoad(Locals[index]); 
}

Value *CodeGenLexicalScope::LoadSelf(void) {
  return Builder.CreateLoad(Self);
}

Value *CodeGenLexicalScope::LoadPointerToArgumentAtIndex(unsigned index) {
  return Args[index];
}
Value *CodeGenLexicalScope::LoadPointerToLocalAtIndex(unsigned index) {
  return Locals[index];
}

void CodeGenLexicalScope::StoreValueInLocalAtIndex(Value * value, unsigned index) {
  if (value->getType() != IdTy) {
    value = Builder.CreateBitCast(value, IdTy);
  }
  Builder.CreateStore(value, Locals[index]);
}

Value *CodeGenLexicalScope::LoadClass(const char *classname) {
  return CGM->getRuntime()->LookupClass(Builder,
      CGM->MakeConstantString(classname));
}

Value *CodeGenLexicalScope::LoadValueOfTypeAtOffsetFromObject( const char*
    type, unsigned offset, Value *object) {
  // FIXME: This is really ugly.  We should really create an LLVM type for
  // the object and use a GEP.
  // FIXME: Non-id loads
  assert(*type == '@' || *type == '#');
  Value *addr = Builder.CreatePtrToInt(object, IntTy);
  addr = Builder.CreateAdd(addr, ConstantInt::get(IntTy, offset));
  addr = Builder.CreateIntToPtr(addr, PointerType::getUnqual(IdTy));
  return Builder.CreateLoad(addr);
}

void CodeGenLexicalScope::StoreValueOfTypeAtOffsetFromObject(Value *value,
    const char* type, unsigned offset, Value *object) {
  // Turn the value into something valid for storing in this ivar
  Value *box = Unbox(&Builder, CurrentFunction, value, type);
  // Calculate the offset of the ivar
  Value *addr = Builder.CreatePtrToInt(object, IntTy);
  addr = Builder.CreateAdd(addr, ConstantInt::get(IntTy, offset));
  addr = Builder.CreateIntToPtr(addr, PointerType::getUnqual(box->getType()));
  // Do the ASSIGN() thing if it's an object.
  if (type[0] == '@') {
    CGObjCRuntime *Runtime = CGM->getRuntime();
    Runtime->GenerateMessageSend(Builder, IdTy, NULL, box,
          Runtime->GetSelector(Builder, "retain", NULL), 0, 0);
    Value *old = Builder.CreateLoad(addr);
    Runtime->GenerateMessageSend(Builder, Type::VoidTy, NULL, old,
          Runtime->GetSelector(Builder, "release", NULL), 0, 0);
  }
  Builder.CreateStore(box, addr);
}

void CodeGenLexicalScope::EndChildBlock(CodeGenBlock *block) {
  Value *Block = block->Block;
  Value *FreeBlockFn =
	  CGM->getModule()->getOrInsertFunction("FreeBlock", Type::VoidTy,
			  Block->getType(), (void*)0);
  Value *Args[] = {Block};
  CallInst::Create(FreeBlockFn, &Args[0], &Args[1], "",
      CleanupBB->getTerminator());
}

Value *CodeGenLexicalScope::ComparePointers(Value *lhs, Value *rhs) {
  lhs = Builder.CreatePtrToInt(lhs, IntPtrTy);
  rhs = Builder.CreatePtrToInt(rhs, IntPtrTy);
  Value *result = Builder.CreateICmpEQ(rhs, lhs, "pointer_compare_result");
  result = Builder.CreateZExt(result, IntPtrTy);
  result = Builder.CreateShl(result, ConstantInt::get(IntPtrTy, 1));
  result = Builder.CreateOr(result, ConstantInt::get(IntPtrTy, 1));
  return Builder.CreateIntToPtr(result, IdTy);
}

Value *CodeGenLexicalScope::SymbolConstant(const char *symbol) {
	// TODO: Duplicate elimination
	CGObjCRuntime *Runtime = CGM->getRuntime();
	IRBuilder<> * initBuilder = CGM->getInitBuilder();
	Value *SymbolClass = Runtime->LookupClass(*initBuilder,
		CGM->MakeConstantString("Symbol"));
	Value *V = CGM->MakeConstantString(symbol);
	Value *S = Runtime->GenerateMessageSend(*initBuilder, IdTy, NULL,
			SymbolClass, Runtime->GetSelector(*initBuilder, 
				"SymbolForCString:", NULL), &V, 1);
	GlobalVariable *GS = new GlobalVariable(IdTy, false,
			GlobalValue::InternalLinkage, ConstantPointerNull::get(IdTy),
			symbol, CGM->getModule()); initBuilder->CreateStore(S, GS);
	return Builder.CreateLoad(GS);
}

Value *CodeGenLexicalScope::MessageSendId(Value *receiver, const char *selName,
    const char *selTypes, Value **argv, unsigned argc) {
  Value *args[argc];
  UnboxArgs(&Builder, CurrentFunction, argv, args, argc, selTypes);
  LOG("Generating object message send %s\n", selName);
  return BoxValue(&Builder, MessageSendId(&Builder, receiver, selName,
        selTypes, args, argc), selTypes);
}

Value *CodeGenLexicalScope::MessageSendSuper(const char *selName, const char
    *selTypes, Value **argv, unsigned argc) {
  return BoxValue(&Builder, MessageSendSuper(&Builder, CurrentFunction, selName,
			  selTypes, argv, argc), selTypes);
}
Value *CodeGenLexicalScope::MessageSend(Value *receiver, const char *selName,
    const char *selTypes, Value **argv, unsigned argc) {
  LOG("Generating %s (%s)\n", selName, selTypes);
  return BoxValue(&Builder, MessageSend(&Builder, CurrentFunction, receiver,
			  selName, selTypes, argv, argv, argc), selTypes);
}
Value *CodeGenLexicalScope::LoadClassVariable(string className, string
		cVarName)
{
	return CGM->getRuntime()->LoadClassVariable(Builder, className, cVarName);
}
void CodeGenLexicalScope::StoreValueInClassVariable(string className, string
		cVarName, Value *object)
{
	CGM->getRuntime()->StoreClassVariable(Builder, className, cVarName, object);
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
}
