include $(GNUSTEP_MAKEFILES)/common.make

# We reset PROJECT_DIR provided by etoile.make to match the subproject since 
# etoile.make doesn't detect and handle such embedded project
PROJECT_DIR = $(CURDIR)

#
# Library
#
VERSION = 0.6
FRAMEWORK_NAME = LanguageKitCodeGen

${FRAMEWORK_NAME}_CC_FILES = \
	CGObjCGNU.cpp\
	CodeGenBlock.cpp\
	CodeGenModule.cpp\
	CodeGenLexicalScope.cpp\
	LLVMCodeGen.cpp

${FRAMEWORK_NAME}_OBJCC_FILES = \
	LKCompiler+JTL.mm\
	LLVMCodeGen.mm

${FRAMEWORK_NAME}_HEADER_FILES = \
	CodeGenBlock.h\
	CodeGenModule.h

# uncomment this line when debugging if you have problems with over-aggressive
# inlining throwing away useful debugging info.
#${FRAMEWORK_NAME}_CXXFLAGS =  -fno-inline
${FRAMEWORK_NAME}_CPPFLAGS =  -D_GNU_SOURCE 
${FRAMEWORK_NAME}_OBJCFLAGS = -std=c99 -g -Wno-unused-value
${FRAMEWORK_NAME}_LDFLAGS += -g -lstdc++
${FRAMEWORK_NAME}_CFLAGS += -Wno-implicit -g

# LLVM flags
LLVM_LIBS=analysis archive backend bitreader bitwriter codegen core engine executionengine instrumentation interpreter ipa ipo jit linker native nativecodegen scalaropts selectiondag support system target transformutils #x86 x86asmprinter x86codegen

${FRAMEWORK_NAME}_CCFLAGS += `llvm-config --cxxflags  ${LLVM_LIBS}` -g
${FRAMEWORK_NAME}_OBJCCFLAGS += `llvm-config --cxxflags  ${LLVM_LIBS}` -g
LIBRARIES_DEPEND_UPON += `llvm-config --ldflags --libfiles ${LLVM_LIBS}` 

LD = g++
CC = clang
CXX = clang++

${FRAMEWORK_NAME}_RESOURCE_FILES += MsgSendSmallInt.bc MsgSendSmallInt64.bc

include $(GNUSTEP_MAKEFILES)/framework.make
-include ../../../etoile.make

all:: MsgSendSmallInt.bc

MsgSendSmallInt.bc: MsgSendSmallInt.m
	@echo "Generating LLVM bitcode for small int messages..."
	@clang -c -ftrapv `gnustep-config --objc-flags` -emit-llvm -fexceptions MsgSendSmallInt.m -o MsgSendSmallInt.bc 
