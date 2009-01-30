include $(GNUSTEP_MAKEFILES)/common.make

# We reset PROJECT_DIR provided by etoile.make to match the subproject since 
# etoile.make doesn't detect and handle such embedded project
PROJECT_DIR = $(CURDIR)

#
# Library
#
VERSION = 0.6
FRAMEWORK_NAME = LanguageKit

SUBPROJECTS = CodeGen Runtime

${FRAMEWORK_NAME}_OBJC_FILES = \
	LKAST.m\
	LKASTVisitor.m\
	LKArrayExpr.m\
	LKAssignExpr.m\
	LKBlockExpr.m\
	LKCategory.m\
	LKCodeGen.m\
	LKComment.m\
	LKComparison.m\
	LKCompiler.m\
	LKDeclRef.m\
	LKIfStatement.m\
	LKLiteral.m\
	LKMessageSend.m\
	LKMethod.m\
	LKModule.m\
	LKReturn.m\
	LKSubclass.m\
	LKSymbolRef.m\
	LKSymbolTable.m\
	LKToken.m\
	LKVariableDecl.m

${FRAMEWORK_NAME}_HEADER_FILES = \
	LKAST.h\
	LKASTVisitor.h\
	LKArrayExpr.h\
	LKAssignExpr.h\
	LKBlockExpr.h\
	LKCategory.h\
	LKCodeGen.h\
	LKComment.h\
	LKComparison.h\
	LKCompiler.h\
	LKDeclRef.h\
	LKIfStatement.h\
	LKLiteral.h\
	LKMessageSend.h\
	LKMethod.h\
	LKModule.h\
	LKReturn.h\
	LKSubclass.h\
	LKSymbolRef.h\
	LKSymbolTable.h\
	LKToken.h\
	LKVariableDecl.h\
	LanguageKit.h

#FIXME: -fno-inline is just for debugging and will make stuff very slow.  Remove it before committing.
${FRAMEWORK_NAME}_CPPFLAGS =  -D_GNU_SOURCE -fno-inline
${FRAMEWORK_NAME}_OBJCFLAGS = -std=c99 -g -Wno-unused-value
${FRAMEWORK_NAME}_LDFLAGS += -g -lEtoileFoundation -lstdc++
${FRAMEWORK_NAME}_CFLAGS += -Wno-implicit -g

${FRAMEWORK_NAME}_RESOURCE_FILES += ObjCConstants.plist

include $(GNUSTEP_MAKEFILES)/framework.make
include $(GNUSTEP_MAKEFILES)/aggregate.make
-include ../../etoile.make
