/*
	GenericABIInfo.mm

	ABI information provider for arbitrary (potentially unsupported) platforms.

	Copyright (C) 2012 Niels Grewe

	Author:  Niels Grewe <niels.grewe@halbordnung.de>
	Date:  March 2012

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:

	* Redistributions of source code must retain the above copyright notice,
	  this list of conditions and the following disclaimer.
	* Redistributions in binary form must reproduce the above copyright notice,
	  this list of conditions and the following disclaimer in the documentation
	  and/or other materials provided with the distribution.
	* Neither the name of the Etoile project nor the names of its contributors
	  may be used to endorse or promote products derived from this software
	  without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
	AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
	IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
	ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
	LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
	CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
	CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
	ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
	THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "GenericABIInfo.h"
#include "ABI.h"
#include "CodeGenModule.h"

using namespace llvm;
static void const countIntsAndFloats(const LLVMType *ty,
                                     unsigned &ints,
                                     unsigned &floats)
{
	if(ty->getTypeID() == Type::VoidTyID)
	{
		return;
	}
	if (ty->isIntegerTy())
	{
		ints++;
	}
	else if (ty->isFloatingPointTy())
	{
		floats++;
	}
	// Assume that pointers count as integers for now.
	else if(ty->getTypeID() == Type::PointerTyID)
	{
		ints++;
	}
	else if (ty->isArrayTy())
	{
		unsigned i=0;
		unsigned f=0;
		const LLVMArrayType *arr = cast<LLVMArrayType>(ty);
		countIntsAndFloats(arr->getElementType(), i, f);
		uint64_t elements = arr->getNumElements();
		ints += i * elements;
		floats += f * elements;
	}
	else if (ty->isAggregateType())
	{
		for (Type::subtype_iterator i=ty->subtype_begin(), end=ty->subtype_end()
		     ; i!=end ; ++i)
		{
			countIntsAndFloats(*i, ints, floats);
		}
	}
	else
	{
		ty->dump();
		assert(0 && "Unrecognised type.");
	}
}

namespace etoile
{
namespace languagekit
{

LLVMType* GenericABIInfo::returnTypeForRetLLVMType(LLVMType *ty, bool &onStack)
{
	unsigned ints = 0;
	unsigned floats = 0;
	return returnTypeAndRegisterUsageForRetLLVMType(ty, onStack, ints, floats);
}

LLVMType* GenericABIInfo::returnTypeAndRegisterUsageForRetLLVMType(LLVMType *ty,
          bool &onStack,
          unsigned &integerRegisters,
          unsigned &floatRegisters)
{
	Type *realRetTy = ty;
	onStack = false;
	if (isa<StructType>(ty))
	{
		countIntsAndFloats(ty, integerRegisters, floatRegisters);
		LOG("Found %d ints and %d floats in ", integerRegisters, floatRegisters);
		DUMP(ty);
		if (integerRegisters > MAX_INTS_IN_REGISTERS || floatRegisters > MAX_FLOATS_IN_REGISTERS)
		{
			LOG("Returning value on stack\n");
			onStack = true;
			realRetTy = llvm::Type::getVoidTy(context);
		}
	}
	if (SMALL_FLOAT_STRUCTS_ON_STACK && isa<StructType>(ty)
	  && ty == GetStructType(context, Type::getFloatTy(context), Type::getFloatTy(context), NULL))
	{
		onStack = false;
		realRetTy = Type::getInt64Ty(context);
		countIntsAndFloats(realRetTy, integerRegisters, floatRegisters);
        }
	if (onStack)
	{
		// If we are returning this on stack, we don't need any registers
		integerRegisters = floatRegisters = 0;
	}
	return realRetTy;
}

bool GenericABIInfo::willPassTypeAsPointer(llvm::Type *ty)
{
	if (isa<StructType>(ty))
	{
		return PASS_STRUCTS_AS_POINTER;
	}
	return false;
}

llvm::AttrListPtr GenericABIInfo::attributeListForFunctionType(llvm::FunctionType *funTy, llvm::Type *retType)
{
	unsigned intReg;
	unsigned floatReg;
	bool isSRet = false;
	GenericABIInfo::returnTypeAndRegisterUsageForRetLLVMType(retType, isSRet, intReg, floatReg);
	if (isSRet)
	{
		AttributeWithIndex stackRetAttr = AttributeWithIndex::get(1, Attribute::StructRet);
		return AttrListPtr::get(&stackRetAttr, 1);
	}
	return AttrListPtr::get((AttributeWithIndex*)NULL, 0);
}

} //namespace: languagekit
} //namespace: etoile
