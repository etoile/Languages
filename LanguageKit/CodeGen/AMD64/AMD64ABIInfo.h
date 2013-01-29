/*
	AMD64ABIInfo.cpp

	ABI information provider for code generation on AMD64 platforms.

	Copyright (C) 2011 Niels Grewe

	Author:  Niels Grewe <niels.grewe@halbordnung.de>
	Date:  March 2011

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
#include "../ABIInfo.h"
#ifndef AMD64ABIInfo_h_INCLUDED
#define AMD64ABIInfo_h_INCLUDED

enum
{
NO_CLASS,
INTEGER,
SSE,
SSEUP,
X87,
X87UP,
COMPLEX_X87,
MEMORY,
AMD64_CLASS_MAX
};

typedef uintptr_t AMD64ABIClass;


typedef struct _AMD64ABIClassPair
{
	AMD64ABIClass low;
	AMD64ABIClass high;

} AMD64ABIClassPair;

namespace etoile
{
namespace languagekit
{
class AMD64ABIInfo : public ABIInfo
{
private:
	const TargetData *td;
	llvm::LLVMContext &context;
protected:
	AMD64ABIClass mergeClasses(const AMD64ABIClass accumulator, const AMD64ABIClass next);
	void postMergerCleanup(AMD64ABIClass *low, AMD64ABIClass *high, uint64_t size);
	void classifyLLVMType(llvm::Type *ty, uint64_t offset,
	  AMD64ABIClass *low, AMD64ABIClass *high);
	void classifyLLVMType(llvm::VectorType *ty, uint64_t offset,
	  AMD64ABIClass *low, AMD64ABIClass *high);
	void classifyLLVMType(llvm::ArrayType *ty, uint64_t offset,
	  AMD64ABIClass *low, AMD64ABIClass *high);
	void classifyLLVMType(llvm::StructType *ty, uint64_t offset,
	  AMD64ABIClass *low, AMD64ABIClass *high);
	/**
	 * This function classifies the given LLVM type and assigns it the corresponding
	 * AMD64 ABI classes. They can subsequently be used to determine the correct
	 * register and stack layout for a function call.
	 */
	AMD64ABIClassPair classPairForLLVMType(llvm::Type *ty);

	LLVMType *returnTypeAndRegisterUsageForRetLLVMType(LLVMType *ty,
	  bool &onStack,
	  unsigned &integerRegisters,
	  unsigned &floatRegisters);
public:
	AMD64ABIInfo(llvm::Module &M);
	~AMD64ABIInfo();

	LLVMType *returnTypeForRetLLVMType(LLVMType *ty,
	  bool &onStack);

	bool willPassTypeAsPointer(llvm::Type *ty);
	ParameterAttribute attributesForLLVMType(llvm::Type *ty, unsigned freeInteger, unsigned &usedInteger,
	  unsigned freeFloat,
	  unsigned &usedFloat);
	llvm::AttrListPtr attributeListForFunctionType(llvm::FunctionType *funTy, llvm::Type *retTy);
};
} // namespace: languagekit
} // namespace: etoile
#endif //AMD64ABIInfo_h_INCLUDED
