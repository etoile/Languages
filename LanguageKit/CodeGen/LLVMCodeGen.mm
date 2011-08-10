extern "C"
{
#import "ObjCXXRuntime.h"
#import "LLVMCodeGen.h"
#import "../Runtime/LKObject.h"
#import "../LKSymbolTable.h"
#if __OBJC_GC__
#	include <objc/objc-api.h>
#else
BOOL objc_collecting_enabled(void) { return NO; }
#endif
}


#include "CodeGenModule.h"
#include "CodeGenLexicalScope.h"
#include <llvm/Constants.h>
#include <llvm/LLVMContext.h>
#include <llvm/DerivedTypes.h>
#include <llvm/Target/TargetSelect.h>
#include <llvm/ExecutionEngine/JIT.h>

using llvm::Value;
using llvm::BasicBlock;
using namespace etoile::languagekit;


static NSString *SmallIntFile;
const char * LKObjectEncoding = @encode(LKObject);

@implementation LLVMCodeGen 
+ (void) initialize
{
	if (self == [LLVMCodeGen class])
	{
		NSString *SmallIntFileName; 

		SmallIntFileName = @"MsgSendSmallInt";
		NSString *SmallIntFullName = 
			[SmallIntFileName stringByAppendingString: @".bc"];

		NSFileManager *f = [NSFileManager defaultManager];
		if ([f fileExistsAtPath: SmallIntFullName])
		{
			SmallIntFile = SmallIntFullName;
		}
		else 
		{
			SmallIntFile = [[[NSBundle bundleForClass:self] 
				pathForResource: SmallIntFileName ofType: @"bc"] retain];
		}
		NSAssert(SmallIntFile, 
		         @"Unable to find the location of MsgSendSmallInt.bc."
		         "This must be in either the current working directory or in"
		         " the Resources directory of the LanguageKit bundle "
		         "installed on your system.");
		// These two functions don't do anything.  They must be called,
		// however, to make sure that the linker doesn't optimise the JIT away.
		InitializeNativeTarget();
		LLVMLinkInJIT();

		// TODO: These should all use a per-object context so we can have
		// multiple compilers in the same class.
		MsgSendSmallIntFilename = SmallIntFile;
	}
}
- (id) init
{
	SUPERINIT;
	labelledBasicBlocks = NSCreateMapTable(NSObjectMapKeyCallBacks,
	                                       NSNonOwnedPointerMapValueCallBacks,
	                                       0);
	return self;
}
+ (NSString*) smallIntBitcodeFile
{
	return SmallIntFile;
}
- (void) startModule: (NSString*)fileName;
{
	if (nil == fileName) fileName = @"Anonymous";
	Builder = new CodeGenModule(fileName, getGlobalContext(), objc_collecting_enabled());
}

- (void) endModule
{
	Builder->compile();
}

- (void) createSubclassWithName: (NSString*)aClass
                superclassNamed: (NSString*)aSuperclass
                withSymbolTable: (LKSymbolTable*)symbolTable
{
	Builder->BeginClass(aClass, aSuperclass, symbolTable);
}
- (void) endClass
{
	Builder->EndClass();
}
- (void) createCategoryWithName:(NSString*)aCategory
                   onClassNamed:(NSString*)aClass
{
	Builder->BeginCategory(aClass, aClass);
}
- (void) endCategory
{
	Builder->EndCategory();
}

- (void) beginClassMethod: (NSString*) aName
         withTypeEncoding: (NSString*)types
                arguments: (NSArray*)args
                   locals: (NSArray*)locals
{
	Builder->BeginClassMethod(aName, types, locals, args);
}

- (void) beginInstanceMethod: (NSString*) aName
            withTypeEncoding: (NSString*)types
                   arguments: (NSArray*)args
                      locals: (NSArray*)locals
{
	Builder->BeginInstanceMethod(aName, types, locals, args);
}

- (void*) sendMessage:(NSString*)aMessage
                types:(NSArray*)types
             toObject:(void*)receiver
             withArgs:(void**)argv
                count:(unsigned)argc
{
	SmallVector<Value*, 8> args;
	args.append((Value**)argv, ((Value**)argv)+argc);
	return Builder->getCurrentScope()->MessageSendId((Value*)receiver, aMessage,
		types, args);
}
- (void*) sendSuperMessage:(NSString*)sel
                     types:(NSString*)selTypes
                  withArgs:(void**)argv
                     count:(unsigned)argc
{
	SmallVector<Value*, 8> args;
	args.append((Value**)argv, ((Value**)argv)+argc);
	return 
		Builder->getCurrentScope()->MessageSendSuper(sel, selTypes, args);
}
- (void*) sendMessage:(NSString*)aMessage
                types:(NSArray*)types
                   to:(void*)receiver
             withArgs:(void**)argv
                count:(unsigned)argc
{
	SmallVector<Value*, 8> args;
	args.append((Value**)argv, ((Value**)argv)+argc);
	return Builder->getCurrentScope()->MessageSend((Value*)receiver, aMessage,
		types, args);
}
- (void*)callFunction: (NSString*)functionName
                types: (NSString*)types
             withArgs: (void**)argv
                count: (unsigned int)argc;
{
	SmallVector<Value*, 8> args;
	args.append((Value**)argv, ((Value**)argv)+argc);
	return Builder->getCurrentScope()->CallFunction(functionName,
		types, args);
}
- (void)storeValue:(void*)aVal inVariable: (LKSymbol*)aVariable
{
	switch ([aVariable scope])
	{
		case LKSymbolScopeExternal:
		case LKSymbolScopeArgument:
		case LKSymbolScopeLocal:
			Builder->StoreScopedValue([aVariable name], (llvm::Value*)aVal);
			break;
		case LKSymbolScopeObject:
			Builder->StoreIVar([aVariable name], [aVariable typeEncoding], (Value*)aVal);
			break;
		case LKSymbolScopeClass:
			Builder->StoreCVar([aVariable name], (Value*)aVal);
			break;
		default:
			assert(0 && "Storing in unsupported variable type");
	}
}
- (void*)loadVariable: (LKSymbol*)aVariable
{
	switch ([aVariable scope])
	{
		case LKSymbolScopeExternal:
		case LKSymbolScopeArgument:
		case LKSymbolScopeLocal:
			return Builder->LoadScopedValue([aVariable name]);
		case LKSymbolScopeObject:
			return Builder->LoadIvar(aVariable);
		case LKSymbolScopeClass:
			return Builder->LoadCvar(aVariable);
		default:
			assert(0 && "Storing in unsupported variable type");
			return NULL;
	}
}
- (void) beginBlockWithArgs: (NSArray*)args
                     locals: (NSArray*)locals
                  externals: (NSArray*)externals
                  signature: (NSString*)signature
{
	Builder->BeginBlock(locals, args, externals, signature);
}
- (void*) endBlock
{
	return Builder->EndBlock();
}
- (void) endMethod
{
	Builder->EndMethod();
}

- (void) setReturn:(void*)aValue
{
	Builder->getCurrentScope()->SetReturn((Value*)aValue);
}
- (void) blockReturn:(void*)aValue
{
	Builder->SetBlockReturn((Value*)aValue);
}
- (void*) loadSelf
{
	return Builder->getCurrentScope()->LoadSelf();
}
- (void*) loadBlockContext
{
	return Builder->getCurrentScope()->LoadBlockContext();
}

- (void*) loadClassNamed:(NSString*)aClass
{
	return Builder->getCurrentScope()->LoadClass(aClass);
}
- (void*) intConstant:(NSString*)aString
{
	return Builder->getCurrentScope()->IntConstant(aString);
}
- (void*) floatConstant:(NSString*)aString
{
	return Builder->getCurrentScope()->FloatConstant(aString);
}
- (void*) stringConstant:(NSString*)aString
{
	return Builder->StringConstant(aString);
}
- (void*) nilConstant
{
	return ConstantPointerNull::get(Builder->types->idTy);
}
- (void*) comparePointer:(void*)lhs to:(void*)rhs
{
	return Builder->getCurrentScope()->ComparePointers((Value*)lhs, (Value*)rhs);
}
- (void) dealloc
{
	delete Builder;
	NSFreeMapTable(labelledBasicBlocks);
	[super dealloc];
}
- (void*) generateConstantSymbol:(NSString*)aSymbol
{
	return Builder->getCurrentScope()->SymbolConstant(aSymbol);
}
- (void*) startBasicBlock:(NSString*)aName
{
	return Builder->getCurrentScope()->StartBasicBlock(aName);
}
- (void*) currentBasicBlock
{
	return Builder->getCurrentScope()->CurrentBasicBlock();
}
- (void) moveInsertPointToBasicBlock:(void*)aBasicBlock
{
	Builder->getCurrentScope()->MoveInsertPointToBasicBlock(
		(BasicBlock*)aBasicBlock);
}
- (void) branchOnCondition:(void*)aCondition
                      true:(void*)trueBlock
                     false:(void*)falseBlock
{
	Builder->getCurrentScope()->BranchOnCondition((Value*)aCondition, 
		(BasicBlock*)trueBlock, (BasicBlock*)falseBlock);
}
- (void) goToBasicBlock:(void*)aBasicBlock
{
	Builder->getCurrentScope()->GoTo((BasicBlock*)aBasicBlock);
}
- (void) setBasicBlock:(void*)aBasicBlock forLabel:(NSString*)aLabel
{
	if (aBasicBlock)
	{
		NSMapInsert(labelledBasicBlocks, (__bridge void*)aLabel, aBasicBlock);
	}
	else
	{
		NSMapRemove(labelledBasicBlocks, (__bridge void*)aLabel);
	}
}
- (void*) basicBlockForLabel:(NSString*)aLabel
{
	return NSMapGet(labelledBasicBlocks, (__bridge void*)aLabel);
}
- (void) goToLabelledBasicBlock:(NSString*)aLabel
{
	Builder->getCurrentScope()->GoTo((BasicBlock*)NSMapGet(labelledBasicBlocks,
				(__bridge void*)aLabel));
}
@end
@interface LLVMStaticCodeGen : LLVMCodeGen {
	NSString *outFile;
}
- (id) initWithFile:(NSString*)file;
@end
@implementation LLVMStaticCodeGen
- (id) initWithFile:(NSString*)file
{
	SELFINIT;
	ASSIGN(outFile, file);
	return self;
}
- (void) endModule
{
	Builder->writeBitcodeToFile(outFile, NO);
}
- (void) startModule: (NSString*)fileName
{
	NSString *ModuleName = [outFile lastPathComponent];
	if (NULL == ModuleName) { ModuleName = @"Anonymous"; }
	Builder = new CodeGenModule(ModuleName, getGlobalContext(), false, false);
}
@end
id <LKCodeGenerator> defaultJIT(void)
{
	return [LLVMCodeGen new];
}
id <LKCodeGenerator> defaultStaticCompilterWithFile(NSString* outFile)
{
	return [[LLVMStaticCodeGen alloc] initWithFile:outFile];
}
