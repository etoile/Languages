#include "CodeGenModule.h"
#include "LLVMCompat.h"

#include "llvm/LinkAllPasses.h"
#include <llvm/Bitcode/ReaderWriter.h>
#include <llvm/Constants.h>
#include <llvm/DerivedTypes.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/GlobalVariable.h>
#include <llvm/Module.h>
#include <llvm/LLVMContext.h>
#include <llvm/PassManager.h>
#include "llvm/Analysis/Verifier.h"
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Target/TargetData.h>

#include <string>
#include <algorithm>
#include <errno.h>

#ifdef __amd64__
#include "AMD64/AMD64ABIInfo.h"
#else
#include "GenericABIInfo.h"
#endif
#import <Foundation/Foundation.h>
#import "../Runtime/LKObject.h"

using namespace etoile::languagekit;

static const char *LKObjectEncoding = @encode(LKObject);


NSString *MsgSendSmallIntFilename;

// TODO: This should be rewritten to use the encoding2 stuff in libobjc2

void SkipTypeQualifiers(const char **typestr)
{
	while (1)
	{
		switch (**typestr)
		{
			default: return;
			case 'r': case 'n': case 'N': 
			case 'o': case 'O': case 'V': 
				(*typestr)++;
		}
	}
}


static LLVMType *LLVMTypeFromString2(LLVMContext &Context, const char ** typestr)
{
	// FIXME: Other function type qualifiers
	SkipTypeQualifiers(typestr);
	// Special case for LKObject
	if (strncmp(*typestr, LKObjectEncoding, strlen(LKObjectEncoding)) == 0)
	{
		*typestr += strlen(LKObjectEncoding);
		return PointerType::getUnqual(Type::getInt8Ty(Context));
	}
	switch(**typestr)
	{
		case 'c':
		case 'C':
			(*typestr)++;
			return IntegerType::get(Context, sizeof(char) * 8);
		case 's':
		case 'S':
			(*typestr)++;
			return IntegerType::get(Context, sizeof(short) * 8);
		case 'i':
		case 'I':
			(*typestr)++;
			return IntegerType::get(Context, sizeof(int) * 8);
		case 'l':
		case 'L':
			(*typestr)++;
			return IntegerType::get(Context, sizeof(long) * 8);
		case 'q':
		case 'Q':
			(*typestr)++;
			return IntegerType::get(Context, sizeof(long long) * 8);
		case 'f':
			(*typestr)++;
			return Type::getFloatTy(Context);
		case 'd':
			(*typestr)++;
			return Type::getDoubleTy(Context);
		case 'B':
			(*typestr)++;
			return IntegerType::get(Context, sizeof(bool) * 8);
		case '^':
		{
			(*typestr)++;
			LLVMType *pointeeType = LLVMTypeFromString2(Context, typestr);
			if (pointeeType == Type::getVoidTy(Context))
			{
				pointeeType = Type::getInt8Ty(Context);
			}
			return PointerType::getUnqual(pointeeType);
		}
		case '@':
			// Handle block encodings.
			if (*((*typestr)+1) == '?') (*typestr)++;
		case ':':
		case '#':
		case '*':
			(*typestr)++;
			return PointerType::getUnqual(Type::getInt8Ty(Context));
		case 'v':
			(*typestr)++;
			return Type::getVoidTy(Context);
		case '{':
		{
			while (**typestr != '=') 
			{ 
				(*typestr)++; 
			}
			(*typestr)++;
			std::vector<LLVMType*> types;
			while (**typestr != '}')
			{
				// FIXME: Doesn't work with nested structs
				types.push_back(LLVMTypeFromString2(Context, typestr));
			}
			(*typestr)++;
			// If this struct has no fields, then it is opqaue.  Return the
			// void type for it.
			if (types.size() == 0)
			{
				return Type::getVoidTy(Context);
			}
			return StructType::get(Context, types);
		}
		case '[':
		{
			(*typestr)++;
			uint64_t elements = strtoll(*typestr, (char**)typestr, 10);
			LLVMType *elementTy = LLVMTypeFromString2(Context, typestr);
			// Skip the trailing ]
			(*typestr)++;
			return ArrayType::get(elementTy, elements);
		}
		default:
		//FIXME: Structure and array types
			return NULL;
	}
}

LLVMType *CodeGenTypes::typeFromString(NSString *typeEncoding)
{
	const char *typestr = [typeEncoding UTF8String];
	if (NULL == typestr || '\0' == typestr[0])
	{
		typestr = @encode(LKObject);
	}
	return LLVMTypeFromString2(Mod.getContext(), &typestr);
}

#define NEXT(typestr) \
	while ((*typestr) && !isdigit(*typestr)) { typestr++; }\
	while (isdigit(*typestr)) { typestr++; }

namespace etoile
{
namespace languagekit
{

CodeGenTypes::CodeGenTypes(llvm::Module &M) : Mod(M)
{
	llvm::LLVMContext &context = M.getContext();
#	ifdef __amd64__
	AI = new AMD64ABIInfo(M);
#	else
	AI = new GenericABIInfo(M);
#	endif
	voidTy = llvm::Type::getVoidTy(context);
	idTy = llvm::Type::getInt8PtrTy(context);
	ptrToIdTy = llvm::PointerType::getUnqual(idTy);
	switch (Mod.getPointerSize())
	{
		case llvm::Module::Pointer32:
			intPtrTy = llvm::Type::getInt32Ty(context);
			break;
		case llvm::Module::Pointer64:
			intPtrTy = llvm::Type::getInt64Ty(context);
			break;
		case llvm::Module::AnyPointerSize:
			intPtrTy = llvm::Type::getIntNTy(context, sizeof(void*) * 8);
	}
	// Is this ever wrong?  Segmented architectures, with 64-bit address space
	// of 2^32 x 2^32 segments?
	ptrDiffTy = intPtrTy;
	ptrToVoidTy = idTy;
	selTy = ptrToVoidTy;
	charTy = IntegerType::get(context, sizeof(char) * 8);
	shortTy = IntegerType::get(context, sizeof(short) * 8);
	intTy = IntegerType::get(context, sizeof(int) * 8);
	longTy = IntegerType::get(context, sizeof(long) * 8);
	longLongTy = IntegerType::get(context, sizeof(long long) * 8);
	zeros[0] = llvm::Constant::getNullValue(intTy);
	zeros[1] = zeros[0];
	genericByRefType = GetStructType(CGM->Context,
	                                 idTy,         // 0 isa 
	                                 ptrToVoidTy,  // 1 forwarding
	                                 intTy,        // 2 flags
	                                 intTy,        // 3 size
	                                 ptrToVoidTy,  // 4 keep 
	                                 ptrToVoidTy,  // 5 dispose
	                                 idTy,         // 6 value
	                                 NULL);
}



FunctionType *CodeGenTypes::functionTypeFromString(NSString *type,
                                                   bool &isSRet,
                                                   LLVMType *&realRetTy)
{
	std::vector<LLVMType*> ArgTypes;
	llvm::FunctionType *functionType;
	if (NULL == type)
	{
		ArgTypes.push_back(idTy);
		ArgTypes.push_back(selTy);
		isSRet = false;
		realRetTy = idTy;
		functionType = FunctionType::get(idTy, ArgTypes, true);
		return functionType;
	}
	const char *typestr = [type UTF8String];
	llvm::LLVMContext &context = Mod.getContext();
	// Function encodings look like this:
	// v12@0:4@8 - void f(id, SEL, id)
	LLVMType * ReturnTy = LLVMTypeFromString2(context, &typestr);	
	realRetTy = ReturnTy;

	ReturnTy = AI->returnTypeForRetLLVMType(ReturnTy, isSRet);
	if (isSRet)
	{
		ArgTypes.push_back(llvm::PointerType::getUnqual(realRetTy));
	}
	NEXT(typestr);
	while(*typestr)
	{
		LLVMType *argTy = LLVMTypeFromString2(context, &typestr);
		if (AI->willPassTypeAsPointer(argTy))
		{
			argTy = llvm::PointerType::getUnqual(argTy);
		}
		ArgTypes.push_back(argTy);
		NEXT(typestr);
	}
	functionType = FunctionType::get(ReturnTy, ArgTypes, false);
	return functionType;
}

}}
