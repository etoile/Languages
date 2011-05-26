include $(GNUSTEP_MAKEFILES)/common.make

# We reset PROJECT_DIR provided by etoile.make to match the subproject since 
# etoile.make doesn't detect and handle such embedded project
PROJECT_DIR = $(CURDIR)
# We redefine the project name since the framework name doesn't match the name
# of the project directory (CodeGen)
PROJECT_NAME = LanguageKitCodeGen

VERSION = 0.6
FRAMEWORK_NAME = ${PROJECT_NAME}

${FRAMEWORK_NAME}_CC_FILES = \
	CGObjCGNU.cpp\
	CodeGenBlock.cpp\
	CodeGenModule.cpp\
	CodeGenLexicalScope.cpp\
	LLVMCodeGen.cpp

ifeq ($(GNUSTEP_TARGET_CPU), x86_64)
${FRAMEWORK_NAME}_CC_FILES += \
	AMD64/AMD64ABIInfo.cpp
endif

${FRAMEWORK_NAME}_OBJCC_FILES = \
	LKCompiler+JTL.mm\
	LLVMCodeGen.mm

${FRAMEWORK_NAME}_HEADER_FILES = \
	CodeGenBlock.h\
	CodeGenModule.h

# uncomment this line when debugging if you have problems with over-aggressive
# inlining throwing away useful debugging info.
${FRAMEWORK_NAME}_CXXFLAGS =  -fno-inline
${FRAMEWORK_NAME}_CPPFLAGS =  -D_GNU_SOURCE 
${FRAMEWORK_NAME}_OBJCFLAGS = -std=c99 -g -Wno-unused-value
${FRAMEWORK_NAME}_LDFLAGS += -g -lstdc++
${FRAMEWORK_NAME}_CFLAGS += -Wno-implicit -g

# LLVM doesn't provide its version in any header files, so define it here using
# some ugly sed tricks to parse the output from llvm-config
${FRAMEWORK_NAME}_CPPFLAGS =  `llvm-config --version | sed 's/\([0-9]*\).\([0-9]*\).*/-DLLVM_MAJOR=\1 -DLLVM_MINOR=\2/'`

# LLVM flags
#LLVM_LIBS=analysis archive backend bitreader bitwriter codegen core engine executionengine instrumentation interpreter ipa ipo jit linker native nativecodegen scalaropts selectiondag support system target transformutils #x86 x86asmprinter x86codegen
LLVM_LIBS=all

${FRAMEWORK_NAME}_CCFLAGS += `llvm-config --cxxflags  ${LLVM_LIBS}` -g
${FRAMEWORK_NAME}_OBJCCFLAGS += `llvm-config --cxxflags  ${LLVM_LIBS}` -g
LIBRARIES_DEPEND_UPON += `llvm-config --ldflags --libfiles ${LLVM_LIBS}`

LD = g++
CC = clang
CXX = clang++

${FRAMEWORK_NAME}_RESOURCE_FILES += MsgSendSmallInt.bc 

include $(GNUSTEP_MAKEFILES)/framework.make
-include ../../../etoile.make
-include ../../../documentation.make

all:: MsgSendSmallInt.bc

MsgSendSmallInt.bc: ../Runtime/MsgSendSmallInt.m
	@echo "Generating LLVM bitcode for small int messages..."
	@clang -c -ftrapv  -ftrapv-handler=smalltalk_overflow_handler `gnustep-config --objc-flags` -emit-llvm -fexceptions ../Runtime/MsgSendSmallInt.m -o MsgSendSmallInt.private.bc 
	@cp MsgSendSmallInt.private.bc MsgSendSmallInt.bc
	@#opt -make-runtime-lib-interface MsgSendSmallInt.private.bc -o MsgSendSmallInt.bc
	@rm MsgSendSmallInt.private.bc
