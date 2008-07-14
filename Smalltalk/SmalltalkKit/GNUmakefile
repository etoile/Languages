include $(GNUSTEP_MAKEFILES)/common.make

#
# Library
#
VERSION = 0.5
LIBRARY_NAME = SmalltalkKit

${LIBRARY_NAME}_CC_FILES = \
	CGObjCGNU.cpp\
	LLVMCodeGen.cpp

${LIBRARY_NAME}_OBJC_FILES = \
	smalltalk.m\
	AST.m\
	ArrayExpr.m\
	AssignExpr.m\
	BlockExpr.m\
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
	Subclass.m\
	SymbolTable.m

${LIBRARY_NAME}_OBJCFLAGS = -std=c99 -g -Wno-unused-value
${LIBRARY_NAME}_LDFLAGS += -g -lEtoileFoundation
${LIBRARY_NAME}_CFLAGS += -Wno-implicit -g

# LLVM flags
LLVM_LIBS=all
${LIBRARY_NAME}_CCFLAGS += `llvm-config --cxxflags  ${LLVM_LIBS}` -g
LIBRARIES_DEPEND_UPON += `llvm-config --ldflags --libfiles ${LLVM_LIBS}` 
#${LIBRARY_NAME}_LDFLAGS += `llvm-config --ldflags --libfiles ${LLVM_LIBS}` 
LD = g++

include $(GNUSTEP_MAKEFILES)/library.make

smalltalk.m: smalltalk.y
	@echo Generating parser...
	@lemon smalltalk.y ; mv smalltalk.c smalltalk.m

# FIXME: The generated bc file should be installed in a Library directory somewhere.

MsgSendSmallInt.bc: MsgSendSmallInt.m
	@echo "Generating LLVM bitcode for small int messages (requires clang)"
	@clang -emit-llvm-bc MsgSendSmallInt.m
