include $(GNUSTEP_MAKEFILES)/common.make

#
# Application
#
VERSION = 0.1
TOOL_NAME = st

#
# Resource files
#
${TOOL_NAME}_LANGUAGES = English

${TOOL_NAME}_CC_FILES = \
	CGObjCGNU.cpp\
	LLVMCodeGen.cpp

${TOOL_NAME}_OBJC_FILES = \
	smalltalk.m\
	AST.m\
	ArrayExpr.m\
	AssignExpr.m\
	BlockExpr.m\
	CodeGen.m\
	DeclRef.m\
	Literal.m\
	MessageSend.m\
	Method.m\
	Module.m\
	Parser.m\
	Return.m\
	StringMap.m\
	Subclass.m\
	SymbolTable.m\
	main.m

${TOOL_NAME}_OBJCFLAGS = -std=c99 -g -Wno-unused-value
${TOOL_NAME}_LDFLAGS += -g -lEtoileFoundation -lgnustep-gui 
${TOOL_NAME}_CFLAGS += -Wno-implicit -g

# LLVM flags
LLVM_LIBS = all
#LLVM_LIBS=engine bitreader linker
${TOOL_NAME}_CCFLAGS += `llvm-config --cxxflags  ${LLVM_LIBS}` -g
#${TOOL_NAME}_LDFLAGS += `llvm-config --ldflags --libfiles ${LLVM_LIBS}` 
LD = g++

${TOOL_NAME}_SUBPROJECTS += Support
include $(GNUSTEP_MAKEFILES)/aggregate.make
include $(GNUSTEP_MAKEFILES)/tool.make
# This is an ugly hack which wouldn't be required if GS Make sucked a bit less.
CONFIG_SYSTEM_LIBS += `llvm-config --ldflags --libfiles ${LLVM_LIBS}` 

smalltalk.m: smalltalk.y
	@echo Generating parser...
	@lemon smalltalk.y ; mv smalltalk.c smalltalk.m
