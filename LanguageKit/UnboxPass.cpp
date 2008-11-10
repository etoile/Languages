#include "llvm/Pass.h"
#include "llvm/Function.h"
#include "llvm/Instructions.h"
#include "llvm/GlobalAlias.h"
#include "llvm/GlobalVariable.h"
#include "llvm/Constants.h"
#include <string>

using namespace llvm;
using std::string;

namespace 
{
	class UnboxPass : public FunctionPass 
	{

		public:
		static char ID;
		UnboxPass() : FunctionPass((intptr_t)&ID) {}

		bool getSelectorForValue(Value *b, Value **object, string &selector)
		{
			if (CallInst *v = dyn_cast<CallInst>(b))
			{
				llvm::cerr << "Found call\n";
				Value *calledValue = v->getCalledFunction();
				calledValue->dump();
				while (BitCastInst *val = dyn_cast<BitCastInst>(calledValue))
				{
					calledValue = val->getOperand(0);
					calledValue->dump();
				}
				if (CallInst *callee = 
						dyn_cast<CallInst>(v->getCalledValue()))
				{
					if (callee->getCalledFunction()->getName() 
							== "objc_msg_lookup")
					{
						llvm::cerr << "Found callInst\n";
						Constant *sel = (Constant*)((LoadInst*)callee->getOperand(2))->getOperand(0);
						if (GlobalAlias *a = dyn_cast<GlobalAlias>(sel))
						{
							sel = a->getAliasee();
						}
						ConstantArray *sels = (ConstantArray*)((GlobalVariable*)sel->getOperand(0)->getOperand(0))->getInitializer();
						ConstantInt *idx = (ConstantInt*)sel->getOperand(0)->getOperand(2);
						//sels->getOperand(idx->getLimitedValue())->getOperand(0)->getOperand(0)->dump();
						*object = v->getOperand(0);
						selector = ((ConstantArray*)((GlobalVariable*)sels->getOperand(idx->getLimitedValue())->getOperand(0)->getOperand(0))->getInitializer())->getAsString();
						return true;
					}
				}
				//v->getCalledValue()->dump();
			}
			return false;
		}

		virtual bool runOnFunction(Function &F) 
		{
			//llvm::cerr << "UnboxPass: " << F.getName() << "\n";
			for (Function::iterator i=F.begin(), e=F.end() ;
					i != e ; ++i)
			{
				for (BasicBlock::iterator b=i->begin(), last=i->end() ;
						b != last ; ++b)
				{
					string selector;
					Value *obj;
					if (getSelectorForValue(b, &obj, selector))
					{
						//obj->dump();
						llvm::cerr << selector << "\n";
					}
				}
			}
			return false;
		}
	};

	char UnboxPass::ID = 0;
	RegisterPass<UnboxPass> X("unbox", "Unbox Autoboxed Variabes Pass");
}

FunctionPass *createUnboxPass(void)
{
	return new UnboxPass();
}
