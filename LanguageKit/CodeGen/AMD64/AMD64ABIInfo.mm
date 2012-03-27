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
#include <llvm/DerivedTypes.h>

#include <string>
#include <errno.h>

#include "../ABI.h"
#include "AMD64ABIInfo.h"
#include "../CodeGenModule.h"
#include <llvm/Module.h>

/*
 * Useful sources of information on the AMD64 ABI:
 *
 * - AMD64 System V Application Binary Interface documentation
 *   (http://www.x86-64.org/documentation/abi.pdf)
 *
 * - clang X86_64ABIInfo class
 *   (http://llvm.org/svn/llvm-project/cfe/trunk/lib/CodeGen/TargetInfo.cpp)
 */

using namespace llvm;

static inline bool willCrossWordBoundary(uint64_t offset,
  uint64_t size)
{
	uint64_t boundaryIs = (offset / 64);
	uint64_t boundaryWould = ((offset + size - 1) / 64);
	return (boundaryIs != boundaryWould);
}


namespace etoile
{
namespace languagekit
{
AMD64ABIInfo::AMD64ABIInfo(llvm::Module &M) : ABIInfo(M), context(md.getContext())
{
	td = new TargetData(&md);
}

AMD64ABIInfo::~AMD64ABIInfo()
{
	delete td;
}

inline void AMD64ABIInfo::classifyLLVMType(ArrayType *ty,
  uint64_t offset,
  AMD64ABIClass *high,
  AMD64ABIClass *low)
{
	AMD64ABIClass *current = offset < 64 ? low : high;
	uint64_t aSize = td->getTypeAllocSizeInBits(cast<Type>(ty));
	uint64_t elementCount = ty->getNumElements();
	Type *elementTy = ty->getElementType();
	uint64_t eSize = td->getTypeAllocSizeInBits(elementTy);

	// AMD64 ABI 3.2.3 (1): Objects greater than 128bit are passed in MEMORY
	if (aSize > 128)
	{
		*current = MEMORY;
		return;
	}
	// AMD64 ABI 3.2.3 (1): Objects with unaligned fields are passed in MEMORY
	if (0 != (offset % td->getABITypeAlignment(elementTy)))
	{
		*current = MEMORY;
		return;
	}

	// Otherwise do a merge on the array fields
	*current = NO_CLASS;
	for (uint64_t i = 0, newOffset = offset; i < elementCount; i++, newOffset += eSize)
	{
		AMD64ABIClass thisLow = NO_CLASS;
		AMD64ABIClass thisHigh = NO_CLASS;
		classifyLLVMType(elementTy, newOffset, &thisLow, &thisHigh);
		*low = mergeClasses(*low, thisLow);
		*high = mergeClasses(*high,thisHigh);
		if ((MEMORY == *low) || (MEMORY == *high))
			return;
	}
}

inline void AMD64ABIInfo::classifyLLVMType(StructType *ty,
  uint64_t offset,
  AMD64ABIClass *high,
  AMD64ABIClass *low)
{
	AMD64ABIClass *current = offset < 64 ? low : high;
	const StructLayout *layout = td->getStructLayout(ty);
	uint64_t sSize = layout->getSizeInBits();
	uint64_t fieldCount = ty->getNumElements();

	// AMD64 ABI 3.2.3 (1): Objects greater than 256bit are passed in MEMORY
	if (sSize > 256)
	{
		*current = MEMORY;
		return;
	}

	for (uint64_t i = 0, newOffset = offset; i < fieldCount ; i++)
	{
		unsigned elementAtOffset = layout->getElementContainingOffset(newOffset/8);
		newOffset += layout->getElementOffsetInBits(elementAtOffset);
		Type *thisFieldTy = ty->getElementType(i);
		/*
		 * AMD64 ABI 3.2.3 (1): Objects with unaligned fields are passed in MEMORY
		 */
		if (0 != (newOffset % td->getABITypeAlignment(thisFieldTy)))
		{
			*current = MEMORY;
			return;
		}

		// Do the merge
		AMD64ABIClass fieldLow = NO_CLASS;
		AMD64ABIClass fieldHigh = NO_CLASS;
		classifyLLVMType(thisFieldTy, newOffset, &fieldLow, &fieldHigh);
		*low = mergeClasses(*low, fieldLow);
		*high = mergeClasses(*high, fieldHigh);
		if ((MEMORY == *low) || (MEMORY == *high))
		{
			return;
		}

	}

}

inline void AMD64ABIInfo::classifyLLVMType(VectorType *ty,
  uint64_t offset,
  AMD64ABIClass *low,
  AMD64ABIClass *high)
{
	AMD64ABIClass *current = offset < 64 ? low : high;
	Type *elTy = ty->getElementType();
	uint64_t vSize = td->getTypeAllocSizeInBits(ty);

	if (32 == vSize)
	{
		/*
		 * GCC compatibility:
		 * We pass 4*char, 2*short, 1*int, 1*float as INTEGER
		 */
		*current = INTEGER;

		// If we cross a machine word boundary, we should split this type
		if (willCrossWordBoundary(offset, vSize))
		{
			*high = *low;
		}
	}
	else if (64 == vSize)
	{
		if(elTy->isDoubleTy())
		{
			/*
 		 	 * GCC compatibility:
		 	 * We pass 1*double in MEMORY
		 	 */
			*current = MEMORY;
			return;
		}

		if ((elTy->isIntegerTy()) && (64 == td->getTypeAllocSizeInBits(elTy)))
		{
			/*
			 * GCC compatibility:
			 * 1*long, 1*unsigned long, 1*long long, 1*unsigned long long
			 * as INTEGER.
			 */
			*current = INTEGER;
		}
		else
		{
			*current = SSE;
		}

		// If we cross a machine word boundary, we should split this type
		if (willCrossWordBoundary(offset, vSize))
		{
			*high = *low;
		}
		else if (128 == vSize)
		{
			*low = SSE;
			*high = SSEUP;
		}
	}

}

void AMD64ABIInfo::classifyLLVMType(Type *ty,
  uint64_t offset,
  AMD64ABIClass *low,
  AMD64ABIClass *high)
{
	AMD64ABIClass *current = offset < 64 ? low : high;

	uint64_t rawBits = td->getTypeAllocSizeInBits(ty);
	if (ty->isIntegerTy())
	{
		if (rawBits > 64)
		{
			*low = *high = INTEGER;
		}
		else
		{
			*current = INTEGER;
		}
		return;
	}

	if (ty->isPointerTy())
	{
		// Pointers are treated as integers and they are always register sized.
		*current = INTEGER;
		return;
	}

	if (ty->isX86_FP80Ty())
	{
		*low = X87;
		*high = X87UP;
		return;
	}

	if (ty->isFP128Ty())
	{
		//Might not actually happen
		*low = SSE;
		*high = SSEUP;
		return;
	}

	if (ty->isFloatTy() || ty->isDoubleTy() || ty->isX86_MMXTy())
	{
		*current = SSE;
		return;
	}

	if (ty->isVectorTy())
	{
		classifyLLVMType(cast<VectorType>(ty), offset, high, low);
		return;
	}

	if (ty->isArrayTy())
	{
		// We have already eliminated pointers and vectors as the other composites.
		classifyLLVMType(cast<ArrayType>(ty), offset, high, low);
		return;
	}

	if (ty->isStructTy())
	{
		// We have already eliminated pointers and vectors as the other composites.
		classifyLLVMType(cast<StructType>(ty), offset, high, low);
		return;
	}
	/*
	 * FIXME: C++ objects are replaced by pointers to them if they have
	 * non-trivial copy ctors or dtors. (AMD64 ABI 3.2.3 2.) We don't
	 * support C++ with LanguageKit, so we do not handle them here.
	 *
	 * Also, bitfields don't need to respect alignment but they are not
	 * explicitly present in the LLVM type system, so we cannot do anything
	 * about that.
	 */
}

AMD64ABIClass AMD64ABIInfo::mergeClasses(const AMD64ABIClass accumulator, const AMD64ABIClass next)
{
	/*
	 * AMD64 ABI 3.2.3 4. (a)
	 * If two classes are equal, this is the resulting class.
	 */
    if (accumulator == next)
	{
		return next;
	}

	/*
	 * AMD64 ABI 3.2.3 4. (b)
	 * If one class is NO_CLASS, the other class is the result.
	 */
	if (NO_CLASS == accumulator)
	{
		return next;
	}
	else if (NO_CLASS == next)
	{
		return accumulator;
	}

	/*
	 * AMD64 ABI 3.2.3 4. (c)
	 * If one class is MEMORY, MEMORY is the result.
	 */
	if ((MEMORY == accumulator) || (MEMORY == next))
	{
		return MEMORY;
	}

	/*
	 * AMD64 ABI 3.2.3 4. (d)
	 * If one class is INTEGER, INTEGER is the result.
	 */
	if ((INTEGER == accumulator) || (INTEGER == next))
	{
		return INTEGER;
	}


	/*
	 * AMD64 ABI 3.2.3 4. (e)
	 * If one one of the classes is a X87 FPU class, MEMORY is the result.
	 */
	if (((X87 == accumulator) || (X87UP == accumulator) || (COMPLEX_X87 == accumulator))
	  || ((X87 == next) || (X87UP == next) || (COMPLEX_X87 == next)))
	{
		return MEMORY;
	}

	/*
	 * AMD64 ABI 3.2.3 4. (f)
	 * Otherwise, SSE is used.
	 */
	return SSE;
}

void AMD64ABIInfo::postMergerCleanup(AMD64ABIClass *low, AMD64ABIClass *high, uint64_t size)
{
	/*
	 * AMD64 ABI 3.2.3 5. (a)
	 * If one of the classes is memory, pass everything in memory.
	 */
	if ((MEMORY == *low) || (MEMORY == *high))
	{
		*low = *high = MEMORY;
		return;
	}

	/*
	 * AMD64 ABI 3.2.3 5. (b)
	 * If X87UP is not preceeded by X87, pass everything in memory.
	 */
	if ((X87UP == *high) && (X87 != *low))
	{
		*low = *high = MEMORY;
		return;
	}

	/*
	 * AMD64 ABI 3.2.3 5. (b)
	 * If X87UP is not preceeded by X87, pass everything in memory.
	 */
	if ((X87UP == *high) && (X87 != *low))
	{
		*low = *high = MEMORY;
		return;
	}

	/*
	 * AMD64 ABI 3.2.3 5. (c)
	 * If the argument size exceeds two eightbytes, and the pair is not
	 * <SSE, SSEUP>, pass everything in memoery
	 */
	if ((size > 128) && ((SSEUP != *high) || (SSE != *low)))
	{
		*low = *high = MEMORY;
		return;
	}


	/*
	 * AMD64 ABI 3.2.3 5. (d)
	 * If SSEUP is not preceeded by SSE or SSEUP, convert it to SSE
	 */
	if ((SSEUP == *high) && ((SSE != *low) && (SSEUP != *low)))
	{
		*high = SSE;
		return;
	}
}

AMD64ABIClassPair AMD64ABIInfo::classPairForLLVMType(Type *ty)
{
	AMD64ABIClassPair pair = {NO_CLASS, NO_CLASS};
	if(ty->getTypeID() == Type::VoidTyID)
	{
		return pair;
	}

	classifyLLVMType(ty, 0, &(pair.low), &(pair.high));
	postMergerCleanup(&(pair.low), &(pair.high), td->getTypeAllocSizeInBits(ty));
	return pair;
}


LLVMType* AMD64ABIInfo::returnTypeAndRegisterUsageForRetLLVMType(LLVMType *ty,
  bool &onStack,
  unsigned &integerRegisters,
  unsigned &floatRegisters)
{
	// This is really a rough approximation of how we actually want to return values.
	onStack = false;
	AMD64ABIClassPair pair = classPairForLLVMType(ty);
	switch (pair.low)
	{
		case NO_CLASS:
			if (NO_CLASS == pair.high)
			{
				return  llvm::Type::getVoidTy(context);
			}
			break;
		case MEMORY:
			onStack = true;
			return llvm::Type::getVoidTy(context);
		case INTEGER:
			integerRegisters++;
			break;
		case SSE:
			floatRegisters++;
			break;
		case SSEUP:
			if (0 == (floatRegisters % 2))
			{
				floatRegisters++;
			}
			break;
		case X87:
		case X87UP:
		case COMPLEX_X87:
		default:
			// I think we need to ignore these
			break;
	}

	switch (pair.high)
	{
		case NO_CLASS:
		case MEMORY:
		case X87:
		case X87UP:
		case COMPLEX_X87:
			// these are not or have already been handled
			break;
		case INTEGER:
			integerRegisters++;
			break;
		case SSE:
			floatRegisters++;
			break;
		case SSEUP:
			if (0 == (floatRegisters % 2))
			{
				floatRegisters++;
			}
			break;
	}
	return ty;
}

bool AMD64ABIInfo::passStructTypeAsPointer(llvm::StructType *ty)
{
	return false;
}

} //namespace: languagekit
} //namepsace: etoile
