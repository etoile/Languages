PACKAGE_NAME = Languages

include $(GNUSTEP_MAKEFILES)/common.make

#
# Variables to turn projects on and off in the build process
# (listed by alphabetical order)
#

-include ../modules.make

export io ?= no
export smalltalk ?= yes

#
# Projects (listed by dependency order, then alphabetical order)
#

ifeq ($(io), yes)
	SUBPROJECTS += Io
endif

ifeq ($(smalltalk), yes)
	SUBPROJECTS += LanguageKit 
	SUBPROJECTS += SmalltalkKit
	SUBPROJECTS += Compiler
	SUBPROJECTS += LKPlugins
endif

include $(GNUSTEP_MAKEFILES)/aggregate.make
