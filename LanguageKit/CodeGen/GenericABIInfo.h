/*
	GenericABIInfo.h

	ABI information provider for platforms that don't have an explicit provider.

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
#include "ABIInfo.h"
#include "LLVMCompat.h"
#include <llvm/Module.h>
#include <llvm/LLVMContext.h>
#ifndef GenericABIInfo_h_INCLUDED
#define GenericABIInfo_h_INCLUDED


namespace etoile
{
namespace languagekit
{
class GenericABIInfo : public ABIInfo
{
private:
	llvm::LLVMContext &context;
public:
	GenericABIInfo(llvm::Module &M) : ABIInfo(M), context(md.getContext()) {};

	LLVMType *returnTypeAndRegisterUsageForRetLLVMType(LLVMType *ty,
	  bool &onStack,
	  unsigned &integerRegisters,
	  unsigned &floatRegisters);

	bool passStructTypeAsPointer(llvm::StructType *ty);
};
} //namespace: languagekit
} //namespace: etoile
#endif //GenericABIInfo_h_INCLUDED
