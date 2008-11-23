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
				Value *calledValue = v->getCalledValue();
				while (BitCastInst *val = dyn_cast<BitCastInst>(calledValue))
				{
					calledValue = val->getOperand(0);
				}
				if (CallInst *callee = 
						dyn_cast<CallInst>(calledValue))
				{
					if (callee->getCalledFunction()->getName() 
							== "objc_msg_lookup")
					{
						Constant *sel = (Constant*)((LoadInst*)callee->getOperand(2))->getOperand(0);
						if (GlobalAlias *a = dyn_cast<GlobalAlias>(sel))
						{
							sel = a->getAliasee();
						}
						ConstantArray *sels = (ConstantArray*)((GlobalVariable*)sel->getOperand(0)->getOperand(0))->getInitializer();
						ConstantInt *idx = (ConstantInt*)sel->getOperand(0)->getOperand(2);
						//sels->getOperand(idx->getLimitedValue())->getOperand(0)->getOperand(0)->dump();
						*object = v->getOperand(1);
						selector = ((ConstantArray*)((GlobalVariable*)sels->getOperand(idx->getLimitedValue())->getOperand(0)->getOperand(0))->getInitializer())->getAsString();
						return true;
					}
				}
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
						llvm::cerr << selector << "\n";
						Value *original;
						string construtor;
						if (getSelectorForValue(obj, &original, construtor))
						{
							llvm::cerr << "created with: " <<construtor << "\n";
						}
						obj->dump();
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
