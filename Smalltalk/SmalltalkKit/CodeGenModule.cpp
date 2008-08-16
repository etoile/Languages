#include "CodeGenModule.h"
#include "CodeGenBlock.h"
#include "LLVMCodeGen.h"

#include "llvm/LinkAllPasses.h"
#include <llvm/Bitcode/ReaderWriter.h>
#include <llvm/Constants.h>
#include <llvm/DerivedTypes.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/GlobalVariable.h>
#include <llvm/Module.h>
#include <llvm/ModuleProvider.h>
#include <llvm/PassManager.h>
#include "llvm/Analysis/Verifier.h"
#include <llvm/Support/IRBuilder.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Target/TargetData.h>

#include <string>
#include <algorithm>
#include <errno.h>


Constant *CodeGenModule::MakeConstantString(const std::string &Str, const
        std::string &Name, unsigned GEPs) {
  Constant * ConstStr = ConstantArray::get(Str);
  ConstStr = new GlobalVariable(ConstStr->getType(), true,
      GlobalValue::InternalLinkage, ConstStr, Name, TheModule);
  return ConstantExpr::getGetElementPtr(ConstStr, Zeros, GEPs);
}

string CodeGenModule::FunctionNameFromSelector(const char *sel) {
  // Special cases
  switch (*sel) {
    case '+':
      return "SmallIntMsgadd_";
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

Value *CodeGenModule::BoxValue(IRBuilder<> *B, Value *V, const char *typestr) {
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
      Constant *BoxFunction = TheModule->getOrInsertFunction("MakeSmallInt",
          IdTy, Type::Int64Ty, (void*)0);
      CallInst *boxed = B->CreateCall(BoxFunction, V);
      boxed->setOnlyReadsMemory();
      return boxed;
    }
    case ':': {
      // TODO: Store this in a global.
      Value *SymbolCalss = Runtime->LookupClass(*B,
          MakeConstantString("Symbol"));
      return Runtime->GenerateMessageSend(*B, IdTy, NULL, SymbolCalss,
              Runtime->GetSelector(*B, "SymbolForCString:", NULL), &V, 1);
    }
    case '{': {
      Value *NSValueClass = Runtime->LookupClass(*B,
        MakeConstantString("NSValue"));
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
          MakeConstantString("NSValue"));
      // TODO: We should probably copy this value somewhere, maybe with a
      // custom object instead of NSValue?
      // TODO: Should set sender to self.
      const char *end = typestr;
      while (!isdigit(*end)) { end++; }
      string typestring = string(typestr, end - typestr);
      Value *args[] = {V, MakeConstantString(typestring.c_str())};
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
Value *CodeGenModule::Unbox(IRBuilder<> *B, Function *F, Value *val, const char *Type) {
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
      Value *BoxFunction = TheModule->getFunction("BoxObject");
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
  //TODO: We don't actually use the size numbers for anything, but someone else does, so make these sensible:
  returnTypeString += "12@0:4";
  return MessageSend(B, F, val, castSelName, returnTypeString.c_str());
}

void CodeGenModule::UnboxArgs(IRBuilder<> *B, Function *F,  Value ** argv, Value **args,
    unsigned argc, const char *selTypes) {
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

Value *CodeGenModule::MessageSendSuper(IRBuilder<> *B, Function *F, const char
        *selName, const char *selTypes, Value **argv, unsigned argc) {
  Value *Sender = LoadSelf();
  Value *SelfPtr = Sender;

  Value *args[argc];
  UnboxArgs(B, F, argv, args, argc, selTypes);

  FunctionType *MethodTy = LLVMFunctionTypeFromString(selTypes);
  llvm::Value *cmd = Runtime->GetSelector(*B, selName, selTypes);
  return Runtime->GenerateMessageSendSuper(*B, MethodTy->getReturnType(),
          Sender, SuperClassName.c_str(), SelfPtr, cmd, args, argc);
}

// Preform a real message send.  Reveicer must be a real object, not a
// SmallInt.
Value *CodeGenModule::MessageSendId(IRBuilder<> *B, Value *receiver, const char
    *selName, const char *selTypes, Value **argv, unsigned argc) {
	//FIXME: Find out why this crashes.
  Value *SelfPtr = NULL;//LoadSelf();

  FunctionType *MethodTy = LLVMFunctionTypeFromString(selTypes);
  llvm::Value *cmd = Runtime->GetSelector(*B, selName, selTypes);
  return Runtime->GenerateMessageSend(*B, MethodTy->getReturnType(), SelfPtr,
      receiver, cmd, argv, argc);
}

Value *CodeGenModule::MessageSend(IRBuilder<> *B, Function *F, Value *receiver,
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
    TheModule->getFunction(FunctionNameFromSelector(selName));

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
    Value *BoxFunction = TheModule->getFunction("BoxSmallInt");
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

CodeGenModule::CodeGenModule(const char *ModuleName) {
  TheModule = ParseBitcodeFile(MemoryBuffer::getFile(MsgSendSmallIntFilename));
  Builder = new IRBuilder<>();
  Runtime = clang::CodeGen::CreateObjCRuntime(*TheModule, IntTy,
     IntegerType::get(sizeof(long) * 8));
}

void CodeGenModule::BeginClass(const char *Class, const char *Super, const
  char ** Names, const char ** Types, int *Offsets, int SuperclassSize) {
  ClassName = string(Class);
  SuperClassName = string(Super);
  CategoryName = "";
  InstanceMethodNames.clear();
  InstanceMethodTypes.clear();
  IvarNames.clear();
  while (*Names) {
    IvarNames.push_back(*Names);
    Names++;
  }
  IvarTypes.clear();
  while (*Types) {
    IvarTypes.push_back(*Types);
    Types++;
  }
  IvarOffsets.clear();
  while (*Offsets) {
    IvarOffsets.push_back(*Offsets);
    Offsets++;
  }
  InstanceSize = SuperclassSize + sizeof(void*) * IvarNames.size();
  CurrentClassTy = IdTy;
}

void CodeGenModule::EndClass(void) {
  Runtime->GenerateClass(ClassName.c_str(), SuperClassName.c_str(),
    InstanceSize, IvarNames, IvarTypes, IvarOffsets, InstanceMethodNames,
    InstanceMethodTypes, ClassMethodNames, ClassMethodTypes, Protocols);
}
void CodeGenModule::BeginCategory(const char *Class, const char *Category) {
  ClassName = string(Class);
  SuperClassName = "";
  CategoryName = string(CategoryName); 
  InstanceMethodNames.clear();
  InstanceMethodTypes.clear();
  IvarNames.clear();
  CurrentClassTy = IdTy;
}
void CodeGenModule::EndCategory(void) {
  Runtime->GenerateCategory(ClassName.c_str(), CategoryName.c_str(),
      InstanceMethodNames, InstanceMethodTypes, ClassMethodNames,
      ClassMethodTypes, Protocols);
}

void CodeGenModule::BeginMethod(const char *MethodName, const char
    *MethodTypes, int locals) {
  // Log the method name and types so that we can use it to set up the class
  // structure.
  InstanceMethodNames.push_back(MethodName);
  InstanceMethodTypes.push_back(MethodTypes);

  // Generate the method function
  FunctionType *MethodTy = LLVMFunctionTypeFromString(MethodTypes);
  unsigned argc = MethodTy->getNumParams() - 2;
  const Type *argTypes[argc];
  FunctionType::param_iterator arg = MethodTy->param_begin();
  ++arg; ++arg;
  for (unsigned i=0 ; i<argc ; ++i) {
    argTypes[i] = MethodTy->getParamType(i+2);
  }


  CurrentMethod = Runtime->MethodPreamble(ClassName, CategoryName, MethodName,
      MethodTy->getReturnType(), CurrentClassTy, 
      argTypes, argc, false, false);
  BasicBlock * EntryBB = llvm::BasicBlock::Create("entry", CurrentMethod);
  Builder->SetInsertPoint(EntryBB);

  Args.clear();
  Locals.clear();
  InitialiseFunction(Builder, CurrentMethod, Self, Args, Locals, locals,
      RetVal, CleanupBB, MethodTypes);
  MethodBuilder = Builder;
  CurrentFunction = CurrentMethod;
  //StructType *sty = StructType::get(IntTy, IntTy, NULL);
}

Value *CodeGenModule::LoadArgumentAtIndex(unsigned index) {
  if (BlockStack.empty()) {
    return Builder->CreateLoad(Args[index]);
  }
  return BlockStack.back()->LoadArgumentAtIndex(index);
}

Value *CodeGenModule::MessageSendId(Value *receiver, const char *selName, const
    char *selTypes, Value **argv, unsigned argc) {
  Value *args[argc];
  UnboxArgs(Builder, CurrentFunction, argv, args, argc, selTypes);
  return BoxValue(Builder, MessageSendId(Builder, receiver, selName, selTypes,
			  args, argc), selTypes);
}

Value *CodeGenModule::MessageSendSuper(const char *selName, const char
        *selTypes, Value **argv, unsigned argc) {
  return BoxValue(Builder, MessageSendSuper(Builder, CurrentFunction, selName,
			  selTypes, argv, argc), selTypes);
}
Value *CodeGenModule::MessageSend(Value *receiver, const char *selName, const char
    *selTypes, Value **argv, unsigned argc) {
  LOG("Generating %s (%s)\n", selName, selTypes);
  return BoxValue(Builder, MessageSend(Builder, CurrentFunction, receiver,
			  selName, selTypes, argv, argv, argc), selTypes);
}

void CodeGenModule::SetReturn(Value * Ret) {
  if (Ret != 0) {
  if (Ret->getType() != IdTy) {
      Ret = Builder->CreateBitCast(Ret, IdTy);
    }
    Builder->CreateStore(Ret, RetVal);
  }
  Builder->CreateBr(CleanupBB);
}

void CodeGenModule::EndMethod() {
  if (0 == Builder->GetInsertBlock()->getTerminator()) {
    SetReturn();
  }
}

Value *CodeGenModule::LoadSelf(void) {
  if (!BlockStack.empty()) {
    return BlockStack.back()->LoadBlockVar(0, 0);
  }
  return MethodBuilder->CreateLoad(Self);
}

Value *CodeGenModule::LoadLocalAtIndex(unsigned index) {
  return Builder->CreateLoad(Locals[index]);
}
Value *CodeGenModule::LoadPointerToArgumentAtIndex(unsigned index) {
  if (!BlockStack.empty()) {
	LOG("Returning arg from block");
	BlockStack.back()->Args[index]->dump();
    return BlockStack.back()->Args[index];
  }
	LOG("Returning arg from method");
Args[index]->dump();
  return Args[index];
}
Value *CodeGenModule::LoadPointerToLocalAtIndex(unsigned index) {
  return Locals[index];
}

void CodeGenModule::StoreValueInLocalAtIndex(Value * value, unsigned index) {
  if (value->getType() != IdTy) {
  value = Builder->CreateBitCast(value, IdTy);
  }
  Builder->CreateStore(value, Locals[index]);
}

Value *CodeGenModule::LoadClass(const char *classname) {
  return Runtime->LookupClass(*Builder, MakeConstantString(classname));
}

Value *CodeGenModule::LoadValueOfTypeAtOffsetFromObject( const char* type, unsigned offset,
    Value *object) {
  // FIXME: This is really ugly.  We should really create an LLVM type for
  // the object and use a GEP.
  // FIXME: Non-id loads
  assert(*type == '@' || *type == '#');
  Value *addr = Builder->CreatePtrToInt(object, IntTy);
  addr = Builder->CreateAdd(addr, ConstantInt::get(IntTy, offset));
  addr = Builder->CreateIntToPtr(addr, PointerType::getUnqual(IdTy));
  return Builder->CreateLoad(addr);
}

void CodeGenModule::StoreValueOfTypeAtOffsetFromObject(Value *value,
    const char* type, unsigned offset, Value *object) {
  // Turn the value into something valid for storing in this ivar
  Value *box = Unbox(Builder, CurrentFunction, value, type);
  // Calculate the offset of the ivar
  Value *addr = Builder->CreatePtrToInt(object, IntTy);
  addr = Builder->CreateAdd(addr, ConstantInt::get(IntTy, offset));
  addr = Builder->CreateIntToPtr(addr, PointerType::getUnqual(box->getType()));
  // Do the ASSIGN() thing if it's an object.
  if (type[0] == '@') {
    Runtime->GenerateMessageSend(*Builder, IdTy, NULL, box,
          Runtime->GetSelector(*Builder, "retain", NULL), 0, 0);
    Value *old = Builder->CreateLoad(addr);
    Runtime->GenerateMessageSend(*Builder, Type::VoidTy, NULL, old,
          Runtime->GetSelector(*Builder, "release", NULL), 0, 0);
  }
  Builder->CreateStore(box, addr);
}

void CodeGenModule::BeginBlock(unsigned args, unsigned locals, Value **promoted, int count) {
  BlockStack.push_back(new CodeGenBlock(TheModule, args, locals, promoted,
        count, LoadSelf(), Builder, this));
  Builder = BlockStack.back()->Builder;
  CurrentFunction = BlockStack.back()->BlockFn;
}
Value *CodeGenModule::LoadBlockVar(unsigned index, unsigned offset) {
  return BlockStack.back()->LoadBlockVar(index, offset);
}

void CodeGenModule::SetBlockReturn(Value *value) {
  BlockStack.back()->SetReturn(value);
}

Value *CodeGenModule::EndBlock(void) {
  CodeGenBlock *block = BlockStack.back();
  BlockStack.pop_back();
  block->EndBlock();
  BasicBlock *Cleanup = CleanupBB;
  if (!BlockStack.empty()) {
    Builder = BlockStack.back()->Builder;
    CurrentFunction = BlockStack.back()->BlockFn;
	Cleanup = BlockStack.back()->CleanupBB;
  } else {
    Builder = MethodBuilder;
	CurrentFunction = CurrentMethod;
  }
  // Free this block at the end of the current scope.
  Value *Block = block->Block;
  Value *FreeBlockFn = TheModule->getOrInsertFunction("FreeBlock",
      Type::VoidTy, Block->getType(), (void*)0);
  Value *Args[] = {Block};
  CallInst::Create(FreeBlockFn, &Args[0], &Args[1], "",
      Cleanup->getTerminator());
  return block->Block;
}

Value *CodeGenModule::IntConstant(const char *value) {
  long long val = strtoll(value, NULL, 10);
  intptr_t ptrVal = (val << 1);
  if ((0 == val && errno == EINVAL) || ((ptrVal >> 1) != val)) {
    //FIXME: Implement BigInt
    assert(false && "Number needs promoting to BigInt and BigInt isn't implemented yet");
  }
  ptrVal |= 1;
  Constant *Val = ConstantInt::get(IntPtrTy, ptrVal);
  Val = ConstantExpr::getIntToPtr(Val, IdTy);
  Val->setName("SmallIntConstant");
  return Val;
}
Value *CodeGenModule::StringConstant(const char *value) {
  return Runtime->GenerateConstantString(value, strlen(value));
}
Value *CodeGenModule::ComparePointers(Value *lhs, Value *rhs) {
  lhs = Builder->CreatePtrToInt(lhs, IntPtrTy);
  rhs = Builder->CreatePtrToInt(rhs, IntPtrTy);
  Value *result = Builder->CreateICmpEQ(rhs, lhs, "pointer_compare_result");
  result = Builder->CreateZExt(result, IntPtrTy);
  result = Builder->CreateShl(result, ConstantInt::get(IntPtrTy, 1));
  result = Builder->CreateOr(result, ConstantInt::get(IntPtrTy, 1));
  return Builder->CreateIntToPtr(result, IdTy);
}

void CodeGenModule::InitialiseFunction(IRBuilder<> *B, Function *F, Value *&Self,
    SmallVectorImpl<Value*> &Args, SmallVectorImpl<Value*> &Locals, unsigned
    locals, Value *&RetVal, BasicBlock *&CleanupBB, const char *MethodTypes) {

    const char *RetType = MethodTypes;
    // Set up the arguments
    llvm::Function::arg_iterator AI = F->arg_begin();
    Self = B->CreateAlloca(AI->getType(), 0, "self.addr");
    B->CreateStore(AI, Self);
    ++AI; ++AI;
    // Skip return value, self, _cmd
    NEXT(MethodTypes);
    NEXT(MethodTypes);
    NEXT(MethodTypes);
    for(Function::arg_iterator end = F->arg_end() ; AI != end ;
        ++AI) {
      Value * arg = B->CreateAlloca(IdTy, 0, "arg");
      Args.push_back(arg);
      // Initialise the local to nil
      B->CreateStore(BoxValue(B, AI, MethodTypes), arg);
      NEXT(MethodTypes);
    }
    // Create the locals and initialise them to nil
    for (unsigned i = 0 ; i < locals ; i++) {
      Value * local = B->CreateAlloca(IdTy, 0, "local");
      Locals.push_back(local);
      // Initialise the local to nil
      B->CreateStore(ConstantPointerNull::get(IdTy),
          local);
    }

    // Create a basic block for returns, reached only from the cleanup block
    RetVal = B->CreateAlloca(IdTy, 0, "return_value");
    B->CreateStore(ConstantPointerNull::get(IdTy),
        RetVal);
    BasicBlock * RetBB = llvm::BasicBlock::Create("return", F);
    IRBuilder<> ReturnBuilder = IRBuilder<>(RetBB);
    if (F->getFunctionType()->getReturnType() != llvm::Type::VoidTy) {
      Value * R = ReturnBuilder.CreateLoad(RetVal);
      R = Unbox(&ReturnBuilder, F, R, RetType);
      ReturnBuilder.CreateRet(R);
    } else {
      ReturnBuilder.CreateRetVoid();
    }

    // Setup the cleanup block
    CleanupBB = BasicBlock::Create("cleanup", F);
    ReturnBuilder = IRBuilder<>(CleanupBB);
    ReturnBuilder.CreateBr(RetBB);

}
Value *CodeGenModule::SymbolConstant(const char *symbol) {
  // FIXME: Make constant symbols global objects instead of creating them once
  // for every use.
  Value *SymbolCalss = Runtime->LookupClass(*Builder,
      MakeConstantString("Symbol"));
  Value *V = MakeConstantString(symbol);
  return Runtime->GenerateMessageSend(*Builder, IdTy, NULL, SymbolCalss,
          Runtime->GetSelector(*Builder, "SymbolForCString:", NULL), &V, 1);
}

void CodeGenModule::compile(void) {
  llvm::Function *init = Runtime->ModuleInitFunction();
  // Make the init function external so the optimisations won't remove it.
  init->setLinkage(GlobalValue::ExternalLinkage);
  DUMP(TheModule);
  LOG("\n\n\n Optimises to:\n\n\n");
  PassManager pm;
  pm.add(createVerifierPass());
  pm.add(new TargetData(TheModule));
  pm.add(createAggressiveDCEPass());
  pm.add(createPromoteMemoryToRegisterPass());
  pm.add(createFunctionInliningPass());
  pm.add(createIPConstantPropagationPass());
  pm.add(createSimplifyLibCallsPass());
  pm.add(createPredicateSimplifierPass());
  pm.add(createCondPropagationPass());
  pm.add(createInstructionCombiningPass());
  pm.add(createTailDuplicationPass());
  pm.add(createCFGSimplificationPass());
  pm.add(createStripDeadPrototypesPass());
  pm.run(*TheModule);
  DUMP(TheModule);
  ExecutionEngine *EE = ExecutionEngine::create(TheModule);
  LOG("Compiling...\n");
  EE->runStaticConstructorsDestructors(false);
  void(*f)(void) = (void(*)(void))EE->getPointerToFunction(init);
  LOG("Loading %x...\n", (unsigned)(unsigned long)f);
  f();
  LOG("Loaded.\n");
}
