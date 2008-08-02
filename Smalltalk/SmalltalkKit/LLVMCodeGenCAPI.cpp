#include "CodeGenModule.h"
#include <llvm/Constants.h>
#include <llvm/DerivedTypes.h>
// C interface:
extern "C" {
#include "LLVMCodeGen.h"

  void LLVMinitialise(const char *bcFilePath) {
    MsgSendSmallIntFilename = strdup(bcFilePath);
    IdTy = PointerType::getUnqual(Type::Int8Ty);
    IntTy = IntegerType::get(sizeof(int) * 8);
    IntPtrTy = IntegerType::get(sizeof(void*) * 8);
    Zeros[0] = Zeros[1] = llvm::ConstantInt::get(llvm::Type::Int32Ty, 0);
    //FIXME: 
    SelTy = IntPtrTy;
    std::vector<const Type*> IMPArgs;
    IMPArgs.push_back(IdTy);
    IMPArgs.push_back(SelTy);
    IMPTy = PointerType::getUnqual(FunctionType::get(IdTy, IMPArgs, true));
  }

  ModuleBuilder newModuleBuilder(const char *ModuleName) {
    if (NULL == ModuleName) ModuleName = "Anonymous";
    return new CodeGenModule(ModuleName);
  }
  void freeModuleBuilder(ModuleBuilder aModule) {
    delete aModule;
  }

  void Compile(ModuleBuilder B) {
    //B->optimise();
    B->compile();
  }
  void BeginClass(ModuleBuilder B, const char *Class, const char *Super, const
      char ** Names, const char ** Types, int *Offsets) {
    B->BeginClass(Class, Super, Names, Types, Offsets);
  }

  LLVMValue MessageSend(ModuleBuilder B, LLVMValue receiver, const char *selname,
      const char *seltype, LLVMValue *argv, unsigned argc) {
    return B->MessageSend(receiver, selname, seltype, argv, argc);
  }
  LLVMValue MessageSendId(ModuleBuilder B, LLVMValue receiver, const char *selname,
      const char *seltype, LLVMValue *argv, unsigned argc) {
    return B->MessageSendId(receiver, selname, seltype, argv, argc);
  }

  void SetReturn(ModuleBuilder B, LLVMValue retval) {
    B->SetReturn(retval);
  }

  void BeginMethod(ModuleBuilder B, const char *methodname, const char
      *methodTypes, unsigned locals) {
    B->BeginMethod(methodname, methodTypes, locals);
  }

  void EndMethod(ModuleBuilder B) {
    B->EndMethod();
  }
  LLVMValue LoadSelf(ModuleBuilder B) {
    return B->LoadSelf();
  }
  void StoreValueInLocalAtIndex(ModuleBuilder B, LLVMValue value, unsigned
      index) {
    B->StoreValueInLocalAtIndex(value, index);
  }
  void StoreValueOfTypeAtOffsetFromObject(ModuleBuilder B, LLVMValue value,
      const char* type, unsigned offset, LLVMValue object) {
    B->StoreValueOfTypeAtOffsetFromObject(value, type, offset, object);
  }

	LLVMValue LoadPointerToLocalAtIndex(ModuleBuilder B, unsigned index) {
    return B->LoadPointerToLocalAtIndex(index);
  }
  LLVMValue LoadLocalAtIndex(ModuleBuilder B, unsigned index) {
    return B->LoadLocalAtIndex(index);
  }

  LLVMValue LoadArgumentAtIndex(ModuleBuilder B, unsigned index) {
    return B->LoadArgumentAtIndex(index);
  }

  Value *LoadValueOfTypeAtOffsetFromObject(ModuleBuilder B, const char* type,
      unsigned offset, Value *object) {
    return B->LoadValueOfTypeAtOffsetFromObject(type, offset, object);
  }

  void EndClass(ModuleBuilder B) {
    B->EndClass();
  }
	void BeginCategory(ModuleBuilder B, const char *cls, const char *cat) {
    B->BeginCategory(cls, cat);
  }

	void EndCategory(ModuleBuilder B) {
    B->EndCategory();
  }

  LLVMValue LoadClass(ModuleBuilder B, const char *classname) {
    return B->LoadClass(classname);
  }
  void BeginBlock(ModuleBuilder B, unsigned args, unsigned locals, LLVMValue
      *promoted, int count) {
    B->BeginBlock(args, locals, promoted, count);
  }
  LLVMValue LoadBlockVar(ModuleBuilder B, unsigned index, unsigned offset) {
    return B->LoadBlockVar(index, offset);
  }

  LLVMValue IntConstant(ModuleBuilder B, const char *value) {
    return B->IntConstant(value);
  }
  LLVMValue StringConstant(ModuleBuilder B, const char *value) {
    return B->StringConstant(value);
  }

  LLVMValue EndBlock(ModuleBuilder B) {
    return B->EndBlock();
  }
  LLVMValue NilConstant() {
    return ConstantPointerNull::get(IdTy);
  }
	LLVMValue ComparePointers(ModuleBuilder B, LLVMValue lhs, LLVMValue rhs) {
    return B->ComparePointers(rhs, lhs);
  }
  
  LLVMValue MessageSendSuper(ModuleBuilder B, const char *selName, const char
		  *selTypes, LLVMValue *argv, unsigned argc) {
	  return B->MessageSendSuper(selName, selTypes, argv, argc);
  }
  void SetBlockReturn(ModuleBuilder B, LLVMValue value) {
    B->SetBlockReturn(value);
  }

}
