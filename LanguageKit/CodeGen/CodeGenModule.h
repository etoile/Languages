#ifndef __CODE_GEN_MODULE__INCLUDED__
#define __CODE_GEN_MODULE__INCLUDED__
#import "CGObjCRuntime.h"
#import "CodeGenTypes.h"
#import "CodeGenAssignments.h"
#include "llvm/ADT/StringMap.h"
#include <stdio.h>
#import "objc_pointers.h"

namespace llvm {
  class BasicBlock;
  class PointerType;
  class FunctionType;
}
using std::string;
using namespace::llvm;

@class NSString;
@class LKSymbolTable;
@class LKSymbol;

extern NSString *MsgSendSmallIntFilename;

namespace etoile
{
namespace languagekit
{
class CodeGenSubroutine;
/**
 * This class implements a streaming code generation interface designed to be
 * called directly from an AST.  
 */
class CodeGenModule {
private:
  friend class CodeGenBlock;
  friend class CodeGenSubroutine;
  friend class CodeGenMethod;
  friend class CodeGenFunction;

  LLVMContext &Context;
  Module *TheModule;

public:
  CodeGenTypes *types;
private:
  bool JIT;

  //DIFactory *Debug;
  DICompileUnit ModuleScopeDescriptor;
  DIFile ModuleSourceFile;
  llvm::StringMap<DIType> DebugTypeEncodings;

  Module *SmallIntModule;
  Function *LiteralInitFunction;
  CGBuilder InitialiseBuilder;
  llvm::Value *initializerPool;
  CGObjCRuntime * Runtime;
  CodeGenAssignments *assign;
  bool inClassMethod;
  LLVMType *CurrentClassTy;
  CGBuilder *MethodBuilder;
  NSString *ClassName;
  NSString *SuperClassName;
  NSString *CategoryName;
  int InstanceSize;
  SmallVector<CodeGenSubroutine*, 8> ScopeStack;
  llvm::SmallVector<strong_id<NSString*>, 8> IvarNames;
  llvm::SmallVector<strong_id<NSString*>, 8> CvarNames;
  // All will be "@" for now.
  llvm::SmallVector<strong_id<NSString*>, 8> IvarTypes;
  llvm::SmallVector<strong_id<NSString*>, 8> CvarTypes;
  llvm::SmallVector<int, 8> IvarOffsets;
  llvm::SmallVector<strong_id<NSString*>, 8> InstanceMethodNames;
  llvm::SmallVector<strong_id<NSString*>, 8> InstanceMethodTypes;
  llvm::SmallVector<strong_id<NSString*>, 8> ClassMethodNames;
  llvm::SmallVector<strong_id<NSString*>, 8> ClassMethodTypes;
  llvm::SmallVector<strong_id<NSString*>, 8> Protocols;

public:
  bool profilingEnabled;
private:

  object_map<NSString*, llvm::Constant*> constantStrings;
  /**
   * Returns a constant C string using Str as an initialiser.
   */
  Constant *MakeConstantString(NSString *Str, const
          std::string &Name="", unsigned GEPs=2);

  /**
   * Creates a generic constant.  This will be defined in the module load
   * function by sending a message to the specified class.
   */
	Value *GenericConstant(CGBuilder &Builder, NSString *className,
			NSString *constructor, NSString *argument);
	/**
	 * Creates a global value containing a pointer to a class.
	 */
	void CreateClassPointerGlobal(NSString *className, NSString *globalName);

	/**
	 * Finishes IR generation and prepares the module for code generation.
	 */
	void EndModule(void);

public:
	LLVMType *getCurrentClassTy() { return CurrentClassTy; }
	CGBuilder *getInitBuilder() { return &InitialiseBuilder; }
	Module *getModule() { return TheModule; }
	//DIFactory *getDebugFactory() { return Debug; }
	CGObjCRuntime *getRuntime() { return Runtime; }
	DICompileUnit getModuleDescriptor() { return ModuleScopeDescriptor; }
	DIFile getSourceFileDescriptor() { return ModuleSourceFile; }

	
	/**
	 * Returns the debug info node for an Objective-C type encoding.
	 */
	DIType DebugTypeForEncoding(const std::string &encoding);
	/**
	 * Returns an array of debug types representing the type encodings in a
	 * string.
	 */
	//DIArray DebugTypeArrayForEncoding(const string &encoding);

	/**
	 * Returns the code generator for the current scope
	 */
	CodeGenSubroutine *getCurrentScope() { return ScopeStack.back(); }
	/**
	 * Initialise for the specified module.  
	 */
	CodeGenModule(NSString *ModuleName, LLVMContext &C, bool gc=false,
			bool jit=true, bool profiling=false);

	/**
	 * Start generating code for a class.
	 */
	void BeginClass(NSString *className,
	                NSString *superclassName,
	                LKSymbolTable *symbolTable);

	/**
	 * End a class.
	 */
	void EndClass(void);

	/**
	 * Start generating code for a category.
	 */
	void BeginCategory(NSString *Class, NSString *CategoryName);

	/**
	 * Finish generating a category.
	 */
	void EndCategory(void);

	/**
	 * Start a function method.
	 */
	void BeginFunction(NSString *methodName,
	                   NSString *methodTypes,
	                   NSArray *locals,
	                   NSArray *arguments);
	/**
	 * Ends a function.
	 */
	void EndFunction(void);
	/**
	 * Start a class method.
	 */
	void BeginClassMethod(NSString *methodName,
	                      NSString *methodTypes,
	                      NSArray *locals,
	                      NSArray *arguments);
	/**
	 * Start a method.
	 */
	void BeginInstanceMethod(NSString *methodName,
	                         NSString *methodTypes,
	                         NSArray *locals,
	                         NSArray *arguments);
	
	/**
	 * End the current method.
	 */
	void EndMethod();

	/**
	 * Begin a BlockClosure.
	 */
	void BeginBlock(NSArray *locals,
	                NSArray *arguments,
	                NSArray *bound,
	                NSString *signature);
	/**
	 * End the current block.  Returns a pointer to the block object.
	 */
	Value *EndBlock(void);
	/**
	 * Store the class variable for the current class.
	 */
	void StoreCVar(NSString *cVarName, Value *value);
	/**
	 * Stores the instance variable for the current class.
	 */
	void StoreIVar(LKSymbol *ivar, Value *value);
	/**
	 * Stores a variable in the specified local value.
	 */
	void StoreScopedValue(NSString *varialbe, Value *value);
	/**
	 * Load the class variable for the current class.
	 */
	Value *LoadCvar(LKSymbol *cvar);
	/**
	 * Loads the instance variable from the current class.
	 */
	llvm::Value * LoadIvar(LKSymbol *ivar);
	/**
	 * Loads a variable in the specified local value.
	 */
	llvm::Value *LoadScopedValue(NSString *variable);

	/**
	 * Set the (local) return value for a block.
	 */
	void SetBlockReturn(Value *value);
	/**
	 * Create an integer constant.  Either a SmallInt or a BigInt, depending on
	 * the size.  
	 */
	Value *IntConstant(CGBuilder &Builder, NSString *value);
	/**
	 * Creates a floating point constant.
	 */
	Value *FloatConstant(CGBuilder &Builder, NSString *value);
	/**
	 * Create a symbol (selector) constant.
	 */
	Value *SymbolConstant(CGBuilder &Builder, NSString *symbol);
	/**
	 * Create a string (object) constant.
	 */
	Value *StringConstant(NSString *value);

	/**
	 * Get the module which provides static definitions of small int messages.
	 */
	Module *getSmallIntModule() { return SmallIntModule; }

	/**
	 * Compile and load this module.
	 */
	void compile(void);
	/**
	 * Write the module as a bitcode file.  If isAsm is true then this writes
	 * LLVM 'assembly language' instead of bitcode.
	 */
	void writeBitcodeToFile(NSString* filename, bool isAsm=false);
	void CreateClassPointerGlobal(NSString *className, const char *globalName);

};
// Debugging macros:
extern "C" {
  extern int DEBUG_DUMP_MODULES;
}
/** Debugging macro: dumps the object if the debug flag is set */
#define DUMP(x) do { if (DEBUG_DUMP_MODULES) x->dump(); } while(0)
/** Debugging macro: dumps the object's type if the debug flag is set */
#define DUMPT(x) DUMP((x->getType()))
/** Debugging macro: logs an error message to stderr if the debug flag is set. */
#define LOG(x,...) \
	do { if (DEBUG_DUMP_MODULES) fprintf(stderr, x,##__VA_ARGS__); } while(0)
}}


#endif // __CODE_GEN_MODULE__INCLUDED__
