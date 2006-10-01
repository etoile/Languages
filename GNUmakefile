PACKAGE_NAME = Languages

include $(GNUSTEP_MAKEFILES)/common.make

#
# Variables check
#

export etoile ?= yes
export etoile-extensions ?= yes

ifeq ($(etoile-extensions), yes)

    export io ?= yes

endif

ifeq ($(etoile-extensions), no)

    export io ?= no
    
endif

#
# Subprojects choice
# 

ifeq ($(io), yes)
	SUBPROJECTS += Io
endif

include $(GNUSTEP_MAKEFILES)/aggregate.make
