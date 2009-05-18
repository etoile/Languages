include $(GNUSTEP_MAKEFILES)/common.make

#
# Application
#
VERSION = 0.1
FRAMEWORK_NAME = ObjectiveC2

#
# Resource files
#
ObjectiveC2_LANGUAGES = English

ObjectiveC2_OBJC_FILES = \
				sync.m\
				properties.m

ObjectiveC2_C_FILES = \
				runtime.c

ADDITIONAL_CFLAGS = -std=c99 -fexceptions 
ADDITIONAL_OBJCFLAGS = -fobjc-exceptions -fexceptions

ObjectiveC2_OBJCFLAGS += -std=c99

include $(GNUSTEP_MAKEFILES)/framework.make
