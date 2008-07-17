#include "CodeGenModule.h"

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

// FIXME: This file is a horrible, unstructured, mess.  Split it into separate parts.

// C++ Implementation
using namespace llvm;
using std::string;
using namespace clang::CodeGen;
extern "C" {
  int DEBUG_DUMP_MODULES = 0;
}

void SkipTypeQualifiers(const char **typestr) {
  if (*typestr == NULL) return;
  while(**typestr == 'V' || **typestr == 'r')
  {
    (*typestr)++;
  }
}
PointerType *IdTy;
const Type *IntTy;
const Type *IntPtrTy;
const Type *SelTy;
const PointerType *IMPTy;
const char *MsgSendSmallIntFilename;
Constant *Zeros[2];


const Type *LLVMTypeFromString(const char * typestr) {
  // FIXME: Other function type qualifiers
  SkipTypeQualifiers(&typestr);
  switch(*typestr) {
    case 'c':
    case 'C':
      return IntegerType::get(sizeof(char) * 8);
    case 's':
    case 'S':
      return IntegerType::get(sizeof(short) * 8);
    case 'i':
    case 'I':
      return IntegerType::get(sizeof(int) * 8);
    case 'l':
    case 'L':
      return IntegerType::get(sizeof(long) * 8);
    case 'q':
    case 'Q':
      return IntegerType::get(sizeof(long long) * 8);
    case 'f':
      return Type::FloatTy;
    case 'd':
      return Type::DoubleTy;
    case 'B':
      return IntegerType::get(sizeof(bool) * 8);
    case '^': {
      const Type *pointeeType = LLVMTypeFromString(typestr+1);
      if (pointeeType == Type::VoidTy) {
        pointeeType = Type::Int8Ty;
      }
      return PointerType::getUnqual(pointeeType);
    }
      //FIXME:
    case ':':
    case '@':
    case '#':
    case '*':
      return PointerType::getUnqual(Type::Int8Ty);
    case 'v':
      return Type::VoidTy;
    default:
    //FIXME: Structure and array types
      return NULL;
  }
}

#define NEXT(typestr) \
  while (!isdigit(*typestr)) { typestr++; }\
  while (isdigit(*typestr)) { typestr++; }
FunctionType *LLVMFunctionTypeFromString(const char *typestr) {
  std::vector<const Type*> ArgTypes;
  if (NULL == typestr) {
    ArgTypes.push_back(LLVMTypeFromString("@"));
    ArgTypes.push_back(LLVMTypeFromString(":"));
    return FunctionType::get(LLVMTypeFromString("@"), ArgTypes, true);
  }
  // Function encodings look like this:
  // v12@0:4@8 - void f(id, SEL, id)
  const Type * ReturnTy = LLVMTypeFromString(typestr);
  NEXT(typestr);
  while(*typestr) {
    ArgTypes.push_back(LLVMTypeFromString(typestr));
    NEXT(typestr);
  }
  return FunctionType::get(ReturnTy, ArgTypes, false);
}


CGObjCRuntime::~CGObjCRuntime() {}
