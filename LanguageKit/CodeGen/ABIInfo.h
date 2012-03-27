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
#include <llvm/DerivedTypes.h>
#include <llvm/Target/TargetData.h>
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
	ABIInfo(llvm::Module &M) : md(M) {};
	/**
	 * This function can be used to determine the register usage for a function return.
	 */
	virtual LLVMType *returnTypeAndRegisterUsageForRetLLVMType(LLVMType *ty,
	  bool &onStack,
	  unsigned &integerRegisters,
	  unsigned &floatRegisters) = 0;

	virtual bool passStructTypeAsPointer(llvm::StructType *ty) = 0;
};
} //namespace: languagekit
} //namespace: etoile
#endif //ABIInfo_h_INCLUDED
