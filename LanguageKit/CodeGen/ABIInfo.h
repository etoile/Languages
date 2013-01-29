/*
	ABIInfo.h

	Abstract base class of ABI information providers.

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
#include <stdint.h>
#if (LLVM_MAJOR > 3) || (LLVM_MAJOR == 3 && LLVM_MINOR >= 3)
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Attributes.h>
#include <llvm/IR/LLVMContext.h>
#else
#include <llvm/DerivedTypes.h>
#include <llvm/Attributes.h>
#include <llvm/LLVMContext.h>
#endif
#if (LLVM_MAJOR > 3) || (LLVM_MAJOR == 3 && LLVM_MINOR >= 2)
#if (LLVM_MAJOR > 3) || (LLVM_MAJOR == 3 && LLVM_MINOR >= 3)
#include <llvm/IR/DataLayout.h>
#else
#include <llvm/DataLayout.h>
#endif
typedef llvm::DataLayout TargetData;
#else
#include <llvm/Target/TargetData.h>
typedef llvm::TargetData TargetData;
#endif
#if (LLVM_MAJOR > 3) || (LLVM_MAJOR == 3 && LLVM_MINOR > 2)
#define AttrListPtr AttributeSet
typedef std::pair<unsigned, llvm::Attribute> AttributeWithIndex;
inline AttributeWithIndex GetAttributeWithIndex(unsigned index, llvm::Attribute attr)
{
	return std::make_pair(index, attr);
}
#else
inline AttributeWithIndex GetAttributeWithIndex(unsigned index, llvm::Attribute &attr)
{
	return AttributeWithIndex::get(index, attr);
}
#endif

#if (LLVM_MINOR > 2)
typedef llvm::Attribute Attributes;
#else
typedef llvm::Attributes Attributes;
#endif
#if (LLVM_MAJOR > 3) || (LLVM_MAJOR == 3 && LLVM_MINOR >= 3)
typedef llvm::AttrBuilder ParameterAttribute;
#else
typedef Attributes ParameterAttribute;
#endif

#include "LLVMCompat.h"
#ifndef ABIInfo_h_INCLUDED
#define ABIInfo_h_INCLUDED


namespace etoile
{
namespace languagekit
{
class ABIInfo
{
protected:
	llvm::Module &md;
public:
	ABIInfo(llvm::Module &M) : md(M) {}
	/**
	 * This function can be used to determine the function return type.
	 */
	virtual LLVMType *returnTypeForRetLLVMType(LLVMType *ty,
	  bool &onStack) = 0;

	virtual bool willPassTypeAsPointer(llvm::Type *ty) = 0;

	/**
	 * Quick shim to generate attribute lists to apply to functions. retTy specifies the
	 * intended rather than the ABI-mandated return type of the function.
	 */
	virtual llvm::AttrListPtr attributeListForFunctionType(llvm::FunctionType *funTy, llvm::Type *retTy) = 0;
};
} //namespace: languagekit
} //namespace: etoile
#endif //ABIInfo_h_INCLUDED
