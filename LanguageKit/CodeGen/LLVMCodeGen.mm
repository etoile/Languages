extern "C"
{
#import "ObjCXXRuntime.h"
#import "LLVMCodeGen.h"
#import "../Runtime/LKObject.h"
#import "../LKSymbolTable.h"
#if __OBJC_GC__
BOOL objc_collecting_enabled(void);
#else
#	error Not compiling in GC mode!
#endif
//#include <objc/objc-api.h>
}


#include "CodeGenLexicalScope.h"
#include <llvm/Constants.h>
#include <llvm/LLVMContext.h>
#include <llvm/DerivedTypes.h>
#include <llvm/Target/TargetSelect.h>
#include <llvm/ExecutionEngine/JIT.h>

using llvm::Value;
using llvm::BasicBlock;

static NSString *SmallIntFile;
const char * LKObjectEncoding = @encode(LKObjectPtr);

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
		const char *bcFilePath = [SmallIntFile UTF8String];
		NSAssert(bcFilePath, 
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
		MsgSendSmallIntFilename = strdup(bcFilePath);
		IdTy = PointerType::getUnqual(Type::getInt8Ty(getGlobalContext()));
		IntTy = IntegerType::get(getGlobalContext(), sizeof(int) * 8);
		IntPtrTy = IntegerType::get(getGlobalContext(), sizeof(void*) * 8);
		Zeros[0] = Zeros[1] = 
			ConstantInt::get(Type::getInt32Ty(getGlobalContext()), 0);
		SelTy = IntPtrTy;
		std::vector<const Type*> IMPArgs;
		IMPArgs.push_back(IdTy);
		IMPArgs.push_back(SelTy);
		IMPTy = PointerType::getUnqual(FunctionType::get(IdTy, IMPArgs, true));
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
	const char *ModuleName = [fileName UTF8String];
	if (NULL == ModuleName) ModuleName = "Anonymous";
	Builder = new CodeGenModule(ModuleName, getGlobalContext(), objc_collecting_enabled());
}

- (void) endModule
{
	Builder->compile();
}

- (void) createSubclassWithName:(NSString*)aClass
                superclassNamed:(NSString*)aSuperclass
                  withCvarNames:(const char**)cVarNames 
                          types:(const char**)cVarTypes
                  withIvarNames:(const char**)iVarNames 
                          types:(const char**)iVarTypes
                        offsets:(int*)offsets
{
	int supersize = 0;
	Class sup = NSClassFromString(aSuperclass);
	if (Nil != sup)
	{
		supersize = class_getInstanceSize(sup);
	}
	else
	{
		LKObjectSymbolTable * symbols = (LKObjectSymbolTable*)
			[LKObjectSymbolTable symbolTableForNewClassNamed:aSuperclass];
		if (nil != symbols)
		{
			supersize = [symbols instanceSize];
		}
	}
	Builder->BeginClass([aClass UTF8String], [aSuperclass UTF8String], 
			cVarNames, cVarTypes, iVarNames, iVarTypes, offsets, supersize);
}
- (void) endClass
{
	Builder->EndClass();
}
- (void) createCategoryWithName:(NSString*)aCategory
                   onClassNamed:(NSString*)aClass
{
	Builder->BeginCategory([aClass UTF8String], [aClass UTF8String]);
}
- (void) endCategory
{
	Builder->EndCategory();
}

- (void) beginClassMethod:(const char*) aName
                withTypes:(const char*)types
                   locals: (const char**)locals
                    count:(unsigned)localsCount
{
	Builder->BeginClassMethod(aName, types, localsCount, locals);
}

- (void) beginInstanceMethod: (const char*) aName
                   withTypes: (const char*)types
                      locals: (const char**)locals
                       count: (unsigned)localsCount
{
	Builder->BeginInstanceMethod(aName, types, localsCount, locals);
}

- (void*) sendMessage:(const char*)aMessage
                types:(const char*)types
             toObject:(void*)receiver
             withArgs:(void**)argv
                count:(unsigned)argc
{
	SmallVector<Value*, 8> args;
	args.append((Value**)argv, ((Value**)argv)+argc);
	return Builder->getCurrentScope()->MessageSendId((Value*)receiver, aMessage,
		types, args);
}
- (void*) sendSuperMessage:(const char*)sel
                     types:(const char*)selTypes
                  withArgs:(void**)argv
                     count:(unsigned)argc
{
	SmallVector<Value*, 8> args;
	args.append((Value**)argv, ((Value**)argv)+argc);
	return 
		Builder->getCurrentScope()->MessageSendSuper(sel, selTypes, args);
}
- (void*) sendMessage:(const char*)aMessage
                types:(const char*)types
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
	return Builder->getCurrentScope()->CallFunction([functionName UTF8String],
		[types UTF8String], args);
}
- (void) storeValue:(void*)rval 
    inClassVariable:(NSString*) aClassVar
{
	Builder->StoreClassVar([aClassVar UTF8String], (Value*)rval);
}
- (void*) loadClassVariable:(NSString*) aSymbol
{
	return Builder->LoadClassVar([aSymbol UTF8String]);
}
- (void) storeValue: (void*)aVal 
     inLocalAtIndex: (unsigned)index
lexicalScopeAtDepth: (unsigned) scope
{
	Builder->getCurrentScope()->StoreValueInLocalAtIndex((Value*)aVal, index,
		scope);
}
- (void) storeValue: (void*)aVal inLocalAtIndex: (unsigned)index
{
	Builder->getCurrentScope()->StoreValueInLocalAtIndex((Value*)aVal, index, 0);
}
- (void) storeValue: (void*)aValue
             inIvar: (NSString*)anIvar
             ofType: (NSString*)aType
           atOffset: (unsigned)anOffset
         fromObject: (void*)anObject
            ofClass: (NSString*)className
{
	Builder->getCurrentScope()->StoreValueOfTypeAtOffsetFromObject(
		(Value*)aValue, [className UTF8String], [anIvar UTF8String], 
		[aType UTF8String], anOffset, (Value*)anObject);
}
- (void*) loadValueOfType: (NSString*)aType
                 fromIvar: (NSString*)anIvar
                 atOffset: (unsigned)anOffset
               fromObject: (void*)anObject
                  ofClass: (NSString*)className
{
	return Builder->getCurrentScope()->LoadValueOfTypeAtOffsetFromObject(
		[className UTF8String], [anIvar UTF8String], [aType UTF8String],
		anOffset, (Value*)anObject);
}
- (void) beginBlockWithArgs:(unsigned)args
					 locals:(unsigned)locals
{
	Builder->BeginBlock(args, locals);
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

- (void*) loadLocalAtIndex:(unsigned)index
	   lexicalScopeAtDepth:(unsigned) scope
{
	return Builder->getCurrentScope()->LoadLocalAtIndex(index, scope);
}
- (void*) loadArgumentAtIndex:(unsigned) index
		  lexicalScopeAtDepth:(unsigned) scope
{
	return Builder->getCurrentScope()->LoadArgumentAtIndex(index, scope);
}
- (void*) loadLocalAtIndex:(unsigned)index
{
	return Builder->getCurrentScope()->LoadLocalAtIndex(index, 0);
}
- (void*) loadArgumentAtIndex:(unsigned) index
{
	return Builder->getCurrentScope()->LoadArgumentAtIndex(index, 0);
}
- (void*) loadClassNamed:(NSString*)aClass
{
	return Builder->getCurrentScope()->LoadClass([aClass UTF8String]);
}
- (void*) intConstant:(NSString*)aString
{
	return Builder->getCurrentScope()->IntConstant([aString UTF8String]);
}
- (void*) floatConstant:(NSString*)aString
{
	return Builder->getCurrentScope()->FloatConstant([aString UTF8String]);
}
- (void*) stringConstant:(NSString*)aString
{
	return Builder->StringConstant([aString UTF8String]);
}
- (void*) nilConstant
{
	return ConstantPointerNull::get(IdTy);
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
	return Builder->getCurrentScope()->SymbolConstant([aSymbol UTF8String]);
}
- (void*) startBasicBlock:(NSString*)aName
{
	return Builder->getCurrentScope()->StartBasicBlock([aName UTF8String]);
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
		NSMapInsert(labelledBasicBlocks, aLabel, aBasicBlock);
	}
	else
	{
		NSMapRemove(labelledBasicBlocks, aLabel);
	}
}
- (void*) basicBlockForLabel:(NSString*)aLabel
{
	return NSMapGet(labelledBasicBlocks, aLabel);
}
- (void) goToLabelledBasicBlock:(NSString*)aLabel
{
	Builder->getCurrentScope()->GoTo((BasicBlock*)NSMapGet(labelledBasicBlocks,
				aLabel));
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
	Builder->writeBitcodeToFile((char*)[outFile UTF8String], NO);
}
- (void) startModule: (NSString*)fileName
{
	const char *ModuleName = [[outFile lastPathComponent] UTF8String];
	if (NULL == ModuleName) { ModuleName = "Anonymous"; }
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
