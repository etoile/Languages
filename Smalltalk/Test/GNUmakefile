include $(GNUSTEP_MAKEFILES)/common.make

BUNDLE_NAME = SmalltalkTest
$(BUNDLE_NAME)_OBJC_FILES = $(wildcard *.m)
$(BUNDLE_NAME)_RESOURCE_FILES = $(wildcard *.st)

${BUNDLE_NAME}_LDFLAGS += -lUnitKit

#disable treating warnings as errors
unexport ADDITIONAL_OBJCFLAGS

# Hacks to compile without installing the frameworks first.
${BUNDLE_NAME}_LDFLAGS += -lSmalltalkKit -L../SmalltalkKit/SmalltalkKit.framework/Versions/Current
${BUNDLE_NAME}_LDFLAGS += -lSmalltalkSupport -L../Support/obj
${BUNDLE_NAME}_CFLAGS += -g
${BUNDLE_NAME}_CPPFLAGS += -I..//SmalltalkKit/SmalltalkKit.framework

include $(GNUSTEP_MAKEFILES)/bundle.make
