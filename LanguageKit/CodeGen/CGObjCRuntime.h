//===----- CGObjCRuntime.h - Emit LLVM Code from ASTs for a Module --------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This provides an abstract class for Objective-C code generation.  Concrete
// subclasses of this implement code generation for specific Objective-C
// runtime libraries.
//
//===----------------------------------------------------------------------===//

#ifndef CLANG_CODEGEN_OBCJRUNTIME_H
#define CLANG_CODEGEN_OBCJRUNTIME_H
#include "llvm/ADT/SmallVector.h"
#include "llvm/Support/IRBuilder.h"
#include <string>
#import "CodeGenTypes.h"
#import "objc_pointers.h"


@class NSString;

namespace llvm {
  class Constant;
  class Type;
  class Value;
  class Module;
  class Function;
}

typedef const llvm::SmallVectorImpl<strong_id<NSString *> > StringVector;

/// Implements runtime-specific code generation functions.
class CGObjCRuntime {
public:
  virtual ~CGObjCRuntime();
  
  /// Generate an Objective-C message send operation
  virtual llvm::Value *GenerateMessageSend(CGBuilder &Builder,
                                           LLVMType *ReturnTy,
	                                       bool isSRet,
                                           llvm::Value *Sender,
                                           llvm::Value *Receiver,
	                                       NSString *selName,
	                                       NSString *selTypes,
	                                       llvm::SmallVectorImpl<llvm::Value*> &ArgV,
                                           llvm::BasicBlock *CleanupBlock=0,
                                           NSString *ReceiverClass=0,
										   bool isClassMessage=false)=0;
	llvm::Value *GenerateMessageSend(CGBuilder &Builder,
	                                LLVMType *ReturnTy,
	                                bool isSRet,
	                                llvm::Value *Sender,
	                                llvm::Value *Receiver,
	                                NSString *selName,
	                                NSString *selTypes)
	{
		llvm::SmallVector<llvm::Value*,0> noArgs;
		return GenerateMessageSend(Builder, ReturnTy, isSRet, Sender, Receiver,
				selName, selTypes, noArgs);
	}
	llvm::Value *GenerateMessageSend(CGBuilder &Builder,
	                                LLVMType *ReturnTy,
	                                bool isSRet,
	                                llvm::Value *Sender,
	                                llvm::Value *Receiver,
	                                NSString *selName,
	                                NSString *selTypes,
	                                llvm::Value *Value)
	{
		llvm::SmallVector<llvm::Value*,1> arg = llvm::SmallVector<llvm::Value*,1>(1, Value);
		return GenerateMessageSend(Builder, ReturnTy, isSRet, Sender, Receiver,
				selName, selTypes, arg);
	}
  /// Generate the function required to register all Objective-C components in
  /// this compilation unit with the runtime library.
  virtual llvm::Function *ModuleInitFunction() =0;
  /// Get a selector for the specified name and type values
  virtual llvm::Value *GetSelector(CGBuilder &Builder, 
                                   llvm::Value *SelName, 
                                   llvm::Value *SelTypes) = 0;
  /// Get a selector whose names and types are known at compile time
  virtual llvm::Value *GetSelector(CGBuilder &Builder,
      NSString *SelName,
      NSString *SelTypes) =0;
  /// Generate a constant string object
  virtual llvm::Constant *GenerateConstantString(NSString *String) = 0;
  /// Generate a category.  A category contains a list of methods (and
  /// accompanying metadata) and a list of protocols.
  virtual void GenerateCategory(NSString *ClassName, NSString *CategoryName,
           StringVector  &InstanceMethodNames,
           StringVector  &InstanceMethodTypes,
           StringVector  &ClassMethodNames,
           StringVector  &ClassMethodTypes,
           StringVector &Protocols) = 0;
  /// Generate a class stucture for this class.
  virtual void GenerateClass(
             NSString *ClassName,
             NSString *SuperClassName,
             const int instanceSize,
             StringVector  &IvarNames,
             StringVector  &IvarTypes,
             const llvm::SmallVectorImpl<int>  &IvarOffsets,
             StringVector  &InstanceMethodNames,
             StringVector  &InstanceMethodTypes,
             StringVector  &ClassMethodNames,
             StringVector  &ClassMethodTypes,
             StringVector &Protocols) =0;
  /// Generate a reference to the named protocol.
  virtual llvm::Value *GenerateProtocolRef(CGBuilder &Builder, NSString
      *ProtocolName) =0;
  virtual llvm::Value *GenerateMessageSendSuper(CGBuilder &Builder,
                                                LLVMType *ReturnTy,
	                                            bool isSRet,
                                                llvm::Value *Sender,
                                                NSString *SuperClassName,
                                                llvm::Value *Receiver,
	                                            NSString *selName,
	                                            NSString *selTypes,
                                                llvm::SmallVectorImpl<llvm::Value*> &ArgV,
												bool isClassMessage,
											    llvm::BasicBlock *CleanupBlock=0)=0;
  /// Generate the named protocol.  Protocols contain method metadata but no 
  /// implementations. 
  virtual void GenerateProtocol(NSString *ProtocolName,
    StringVector &Protocols,
    const llvm::SmallVectorImpl<llvm::Constant *>  &InstanceMethodNames,
    const llvm::SmallVectorImpl<llvm::Constant *>  &InstanceMethodTypes,
    const llvm::SmallVectorImpl<llvm::Constant *>  &ClassMethodNames,
    const llvm::SmallVectorImpl<llvm::Constant *>  &ClassMethodTypes) =0;
  /// Generate a function preamble for a method with the specified types
  virtual llvm::Function *MethodPreamble(
                                         const NSString* ClassName,
                                         const NSString* CategoryName,
                                         const NSString* MethodName,
                                         LLVMType *ReturnTy,
                                         LLVMType *SelfTy,
                                         const llvm::SmallVectorImpl<LLVMType*> &ArgTy,
                                         bool isClassMethod=false,
	                                     bool isSRet=false,
                                         bool isVarArg=false) = 0;
	/// Look up the class for the specified name
	virtual llvm::Value *LookupClass(CGBuilder &Builder,
	                                 NSString *ClassName) =0;
	// Define class variables for a specific class
	virtual void DefineClassVariables(
			NSString* ClassName,
			StringVector  &CvarNames,
			StringVector  &CvarTypes) = 0;
	/// Returns the address used to store a specific class variable
	virtual llvm::Value *AddressOfClassVariable(CGBuilder &Builder,
			 NSString* ClassName, NSString* CvarName) = 0;
	/// Store a value to a class variable
	// Look up the offset of an instance variable.
	virtual llvm::Value *OffsetOfIvar(CGBuilder &Builder,
	                                  NSString *className,
	                                  NSString *ivarName,
	                                  int offsetGuess) = 0;
  /// If instance variable addresses are determined at runtime then this should
  /// return true, otherwise instance variables will be accessed directly from
  /// the structure.  If this returns true then @defs is invalid for this
  /// runtime and a warning should be generated.
  virtual bool LateBoundIVars() { return false; }
};

/// Creates an instance of an Objective-C runtime class.  
//TODO: This should include some way of selecting which runtime to target.
CGObjCRuntime *CreateObjCRuntime(
    etoile::languagekit::CodeGenTypes *types,
    llvm::Module &M,
    llvm::LLVMContext &C,
    LLVMType *LLVMIntType,
    LLVMType *LLVMLongType,
    bool enableGC,
    bool isJit);
#endif
