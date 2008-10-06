PACKAGE_NAME = Languages

include $(GNUSTEP_MAKEFILES)/common.make

#
# Variables check
#

export etoile ?= yes
export etoile-extensions ?= yes

ifeq ($(etoile-extensions), yes)

    #export io ?= yes
    export smalltalk ?= yes

endif

ifeq ($(etoile-extensions), no)

    export io ?= no
    export smalltalk ?= no

endif

#
# Subprojects choice
# 

ifeq ($(io), yes)
	SUBPROJECTS += Io
endif

ifeq ($(smalltalk), yes)
	SUBPROJECTS += LanguageKit 
	SUBPROJECTS += SmalltalkKit
	SUBPROJECTS += Compiler
endif

include $(GNUSTEP_MAKEFILES)/aggregate.make
