include $(GNUSTEP_MAKEFILES)/common.make

# We reset PROJECT_DIR provided by etoile.make to match the subproject since 
# etoile.make doesn't detect and handle such embedded project
PROJECT_DIR = $(CURDIR)

#
# Library
#
VERSION = 0.6
FRAMEWORK_NAME = SmalltalkKit

${FRAMEWORK_NAME}_CC_FILES = \
	CGObjCGNU.cpp\
	CodeGenBlock.cpp\
	CodeGenModule.cpp\
	CodeGenLexicalScope.cpp\
	LLVMCodeGen.cpp\
	LLVMCodeGenCAPI.cpp

${FRAMEWORK_NAME}_OBJC_FILES = \
	smalltalk.m\
	AST.m\
	ArrayExpr.m\
	AssignExpr.m\
	BlockExpr.m\
	Category.m\
	CodeGen.m\
	Comparison.m\
	Comment.m\
	DeclRef.m\
	Literal.m\
	MessageSend.m\
	Method.m\
	Module.m\
	Parser.m\
	Return.m\
	SmalltalkCompiler.m\
	Subclass.m\
	SymbolRef.m\
	SymbolTable.m

${FRAMEWORK_NAME}_HEADER_FILES = \
	AST.h\
	ArrayExpr.h\
	AssignExpr.h\
	BlockExpr.h\
	Category.h\
	CodeGen.h\
	CodeGenBlock.h\
	CodeGenModule.h\
	Comment.h\
	Comparison.h\
	DeclRef.h\
	Literal.h\
	MessageSend.h\
	Method.h\
	Module.h\
	Parser.h\
	Return.h\
	SmalltalkCompiler.h\
	SmalltalkKit.h\
	Subclass.h\
	SymbolRef.h\
	SymbolTable.h


${FRAMEWORK_NAME}_CPPFLAGS =  -D_GNU_SOURCE
${FRAMEWORK_NAME}_OBJCFLAGS = -std=c99 -g -Wno-unused-value
${FRAMEWORK_NAME}_LDFLAGS += -g -lEtoileFoundation -lstdc++
${FRAMEWORK_NAME}_CFLAGS += -Wno-implicit -g

# LLVM flags
LLVM_LIBS=all
${FRAMEWORK_NAME}_CCFLAGS += `llvm-config --cxxflags  ${LLVM_LIBS}` -g
LIBRARIES_DEPEND_UPON += `llvm-config --ldflags --libfiles ${LLVM_LIBS}` 
LD = g++

${FRAMEWORK_NAME}_RESOURCE_FILES += MsgSendSmallInt.bc\
	ObjCConstants.plist

-include ../../../etoile.make
include $(GNUSTEP_MAKEFILES)/framework.make

smalltalk.m: smalltalk.y
	@echo Generating parser...
	@lemon smalltalk.y ; mv smalltalk.c smalltalk.m

MsgSendSmallInt.bc: MsgSendSmallInt.m
	@echo "Generating LLVM bitcode for small int messages (requires clang)"
	@clang -emit-llvm-bc MsgSendSmallInt.m

