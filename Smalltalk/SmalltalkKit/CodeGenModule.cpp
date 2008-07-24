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

Value *CodeGenModule::BoxValue(IRBuilder *B, Value *V, const char *typestr) {
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
      V = B->CreateSExt(V, Type::Int64Ty);
    // Now V is 64-bits.
    case 'q': case 'Q':
    {
      // This will return a SmallInt or a promoted integer.
     Constant *BoxFunction = TheModule->getOrInsertFunction("MakeSmallInt",
          IdTy, Type::Int64Ty, (void*)0);
      return B->CreateCall(BoxFunction, V);
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
      LOG("Generating NSValue boxing type %s\n", typestr);
      return Runtime->GenerateMessageSend(*B, IdTy, NULL, NSValueClass,
          Runtime->GetSelector(*B, "valueWithBytes:objCType:", NULL), &V, 1);
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
Value *CodeGenModule::Unbox(IRBuilder *B, Function *F, Value *val, const char *Type) {
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
    case '@': {
      Value *BoxFunction = TheModule->getFunction("BoxObject");
      val = B->CreateBitCast(val, IdTy);
      return B->CreateCall(BoxFunction, val, "boxed_small_int");
    }
    case 'v':
      return val;
    default:
      castSelName = "";
      assert(false && "Unable to transmogriy object to compound type");
  }
  //TODO: We don't actually use the size numbers for anything, but someone else does, so make these sensible:
  char typeStr[] = "I12@0:4";
  typeStr[0] = *Type;
  return MessageSend(B, F, val, castSelName, typeStr);
}

void CodeGenModule::UnboxArgs(IRBuilder *B, Function *F,  Value ** argv, Value **args,
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

// Preform a real message send.  Reveicer must be a real object, not a
// SmallInt.
Value *CodeGenModule::MessageSendId(IRBuilder *B, Value *receiver, const char *selName,
    const char *selTypes, Value **argv, unsigned argc) {
  Value *SelfPtr = B->CreateLoad(Self);
  FunctionType *MethodTy = LLVMFunctionTypeFromString(selTypes);
  llvm::Value *cmd = Runtime->GetSelector(*B, selName, selTypes);
  return Runtime->GenerateMessageSend(*B, MethodTy->getReturnType(), SelfPtr,
      receiver, cmd, argv, argc);
}

Value *CodeGenModule::MessageSend(IRBuilder *B, Function *F, Value *receiver, const char
    *selName, const char *selTypes, Value **argv, Value **boxedArgs, unsigned argc) {
  Value *Int = B->CreatePtrToInt(receiver, IntPtrTy);
  Value *IsSmallInt = B->CreateTrunc(Int, Type::Int1Ty, "is_small_int");

  // Basic blocks for messages to SmallInts and ObjC objects:
  BasicBlock *SmallInt = BasicBlock::Create(string("small_int_message") + selName, F);
  IRBuilder SmallIntBuilder = IRBuilder(SmallInt);
  BasicBlock *RealObject = BasicBlock::Create(string("real_object_message") + selName,
      F);
  IRBuilder RealObjectBuilder = IRBuilder(RealObject);
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
  Builder = new IRBuilder();
  Runtime = clang::CodeGen::CreateObjCRuntime(*TheModule, IntTy,
     IntegerType::get(sizeof(long) * 8));
}

void CodeGenModule::BeginClass(const char *Class, const char *Super, const char ** Names,
    const char ** Types, int *Offsets) {
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
  CurrentClassTy = IdTy;
}

void CodeGenModule::EndClass(void) {
  // FIXME: Get the instance size from somewhere sensible.
  Runtime->GenerateClass(ClassName.c_str(), SuperClassName.c_str(), 100,
      IvarNames, IvarTypes, IvarOffsets, InstanceMethodNames,
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
}

Value *CodeGenModule::LoadArgumentAtIndex(unsigned index) {
  if (BlockStack.empty()) {
    return Builder->CreateLoad(Args[index]);
  }
  return BlockStack.back()->LoadArgumentAtIndex(index);
}

Value *CodeGenModule::MessageSendId(Value *receiver, const char *selName, const char
    *selTypes, Value **argv, unsigned argc) {
  IRBuilder *B = Builder;
  Function *F = CurrentMethod;
  if (!BlockStack.empty()) {
    CodeGenBlock *b = BlockStack.back();
    B = b->Builder;
    F = b->BlockFn;
  }
  Value *args[argc];
  UnboxArgs(B, F, argv, args, argc, selTypes);
  return BoxValue(B, MessageSendId(B, receiver, selName, selTypes, argv,
        argc), selTypes);
}

Value *CodeGenModule::MessageSend(Value *receiver, const char *selName, const char
    *selTypes, Value **argv, unsigned argc) {
  IRBuilder *B = Builder;
  Function *F = CurrentMethod;
  if (!BlockStack.empty()) {
    CodeGenBlock *b = BlockStack.back();
    B = b->Builder;
    F = b->BlockFn;
  }
  /*
  Value *args[argc];
  UnboxArgs(B, F, argv, args, argc, selTypes);
  */
  return BoxValue(B, MessageSend(B, F, receiver, selName, selTypes, argv, argv,
        argc), selTypes);
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
  return Builder->CreateLoad(Self);
}

Value *CodeGenModule::LoadLocalAtIndex(unsigned index) {
  return Builder->CreateLoad(Locals[index]);
}
Value *CodeGenModule::LoadPointerToLocalAtIndex(unsigned index) {
  return Locals[index];
}

void CodeGenModule::StoreValueInLocalAtIndex(Value * value, unsigned index) {
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
  assert(*type == '@');
  Value *addr = Builder->CreatePtrToInt(object, IntTy);
  addr = Builder->CreateAdd(addr, ConstantInt::get(IntTy, offset));
  addr = Builder->CreateIntToPtr(addr, PointerType::getUnqual(IdTy));
  return Builder->CreateLoad(addr);
}

void CodeGenModule::StoreValueOfTypeAtOffsetFromObject(Value *value,
    const char* type, unsigned offset, Value *object) {
  // FIXME: This is really ugly.  We should really create an LLVM type for
  // the object and use a GEP.
  // FIXME: Non-id stores
  assert(*type == '@');
  Value *addr = Builder->CreatePtrToInt(object, IntTy);
  addr = Builder->CreateAdd(addr, ConstantInt::get(IntTy, offset));
  addr = Builder->CreateIntToPtr(addr, PointerType::getUnqual(IdTy));
  Builder->CreateStore(value, addr);
}

void CodeGenModule::BeginBlock(unsigned args, unsigned locals, Value **promoted, int count) {
  BlockStack.push_back(new CodeGenBlock(TheModule, args, locals, promoted,
        count, Builder, this));
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
  // Free this block at the end of the current method.
  // FIXME: Should free blocks allocated in blocks inside their function.
  Value *Block = block->Block;
  Value *FreeBlockFn = TheModule->getOrInsertFunction("FreeBlock",
      Type::VoidTy, Block->getType(), (void*)0);
  Value *Args[] = {Block};
  CallInst::Create(FreeBlockFn, &Args[0], &Args[1], "",
      CleanupBB->getTerminator());
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

void CodeGenModule::InitialiseFunction(IRBuilder *B, Function *F, Value *&Self,
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
    IRBuilder ReturnBuilder = IRBuilder(RetBB);
    if (F->getFunctionType()->getReturnType() != llvm::Type::VoidTy) {
      Value * R = ReturnBuilder.CreateLoad(RetVal);
      R = Unbox(&ReturnBuilder, F, R, RetType);
      ReturnBuilder.CreateRet(R);
    } else {
      ReturnBuilder.CreateRetVoid();
    }

    // Setup the cleanup block
    CleanupBB = BasicBlock::Create("cleanup", F);
    ReturnBuilder = IRBuilder(CleanupBB);
    ReturnBuilder.CreateBr(RetBB);

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
  pm.add(createPromoteMemoryToRegisterPass());
  pm.add(createFunctionInliningPass());
  pm.add(createIPConstantPropagationPass());
  pm.add(createSimplifyLibCallsPass());
  pm.add(createCondPropagationPass());
  pm.add(createAggressiveDCEPass());
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
