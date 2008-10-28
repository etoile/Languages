include $(GNUSTEP_MAKEFILES)/common.make

# We reset PROJECT_DIR provided by etoile.make to match the subproject since 
# etoile.make doesn't detect and handle such embedded project
PROJECT_DIR = $(CURDIR)

#
# Library
#
VERSION = 0.6
FRAMEWORK_NAME = LanguageKit

${FRAMEWORK_NAME}_CC_FILES = \
	CGObjCGNU.cpp\
	CodeGenBlock.cpp\
	CodeGenModule.cpp\
	CodeGenLexicalScope.cpp\
	LLVMCodeGen.cpp\
	LLVMCodeGenCAPI.cpp

${FRAMEWORK_NAME}_OBJC_FILES = \
	LKAST.m\
	LKArrayExpr.m\
	LKAssignExpr.m\
	LKBlockExpr.m\
	LKCategory.m\
	LKCodeGen.m\
	LKComparison.m\
	LKComment.m\
	LKDeclRef.m\
	LKLiteral.m\
	LKCompiler.m\
	LKMessageSend.m\
	LKMethod.m\
	LKModule.m\
	LKReturn.m\
	LKSubclass.m\
	LKSymbolRef.m\
	LKSymbolTable.m

${FRAMEWORK_NAME}_HEADER_FILES = \
	LKAST.h\
	LKArrayExpr.h\
	LKAssignExpr.h\
	LKBlockExpr.h\
	LKCategory.h\
	LKCodeGen.h\
	CodeGenBlock.h\
	CodeGenModule.h\
	LKComment.h\
	LKComparison.h\
	LKDeclRef.h\
	LKLiteral.h\
	LKCompiler.h\
	LKMessageSend.h\
	LKMethod.h\
	LKModule.h\
	LKReturn.h\
	LanguageKit.h\
	LKSubclass.h\
	LKSymbolRef.h\
	LKSymbolTable.h


${FRAMEWORK_NAME}_CPPFLAGS =  -D_GNU_SOURCE
${FRAMEWORK_NAME}_OBJCFLAGS = -std=c99 -g -Wno-unused-value
${FRAMEWORK_NAME}_LDFLAGS += -g -lEtoileFoundation -lstdc++
${FRAMEWORK_NAME}_CFLAGS += -Wno-implicit -g

# LLVM flags
LLVM_LIBS=analysis archive backend bitreader bitwriter codegen core debugger engine executionengine instrumentation interpreter ipa ipo jit linker native nativecodegen scalaropts selectiondag support system target transformutils #x86 x86asmprinter x86codegen

${FRAMEWORK_NAME}_CCFLAGS += `llvm-config --cxxflags  ${LLVM_LIBS}` -g
LIBRARIES_DEPEND_UPON += `llvm-config --ldflags --libfiles ${LLVM_LIBS}` 
LD = g++

${FRAMEWORK_NAME}_RESOURCE_FILES += MsgSendSmallInt.bc\
	ObjCConstants.plist

include $(GNUSTEP_MAKEFILES)/framework.make
-include ../../etoile.make

MsgSendSmallInt.bc: MsgSendSmallInt.m
	@echo "Generating LLVM bitcode for small int messages (requires clang)"
	@clang -emit-llvm-bc MsgSendSmallInt.m

