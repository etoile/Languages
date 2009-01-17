PACKAGE_NAME = LKPlugins

include $(GNUSTEP_MAKEFILES)/common.make

#
# Variables to turn projects on and off in the build process
# (listed by alphabetical order)
#

-include ../../modules.make

export  commentsToLogs ?= yes

#
# Projects (listed by dependency order, then alphabetical order)
#

ifeq ($(commentsToLogs), yes)
	SUBPROJECTS += CommentsToLogs
endif

include $(GNUSTEP_MAKEFILES)/aggregate.make
