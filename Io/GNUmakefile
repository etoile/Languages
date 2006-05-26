include $(GNUSTEP_MAKEFILES)/common.make

# FIXME: Try to clean this bunch of flags or put it a separate GNUmakefile.preamble
ifeq ($(FOUNDATION_LIB), apple)
  ADDITIONAL_LDFLAGS += -F/Users/yjchen/Library/Frameworks -framework StepTalk
  ADDITIONAL_LDFLAGS += -framework Accelerate -faltivec
  # Only for Darwin 
  ADDITIONAL_CFLAGS += -mdynamic-no-pic -falign-loops=16
else
  # FIXME: framework doesn't work
  # ADDITIONAL_LDFLAGS += -F/usr/local/GNUstep/Local/Library/Frameworks/ -frameworks StepTalk
  ADDITIONAL_LDFLAGS += -L/usr/local/GNUstep/Local/Library/Frameworks/StepTalk.framework/Versions/Current/ -lStepTalk -lgnustep-gui
  ADDITIONAL_INCLUDE_DIRS += -I/usr/local/GNUstep/Local/Library/Frameworks/StepTalk.framework/Headers/
endif

ADDITIONAL_INCLUDE_DIRS += -IIoVM -IIoVM/base -IIoVM/SkipDB -IAAVector -IObjcBridge

#
# Language bundle name (to use with StepTalk)
#

BUNDLE_NAME = IoLanguage

IoLanguage_SUBPROJECTS += \
	IoVM \
	AAVector \
	ObjcBridge

# MyLanguage_BUNDLE_LIBS += -lmylanguage

BUNDLE_EXTENSION  := .stlanguage
BUNDLE_INSTALL_DIR := $(GNUSTEP_INSTALLATION_DIR)/Library/StepTalk/Languages

IoLanguage_PRINCIPAL_CLASS = IoLanguageEngine

#
# Files
#

IoLanguage_OBJC_FILES = \
        IoLanguageEngine.m 

-include GNUmakefile.preamble
-include GNUmakefile.tool
include $(GNUSTEP_MAKEFILES)/bundle.make
include $(GNUSTEP_MAKEFILES)/aggregate.make
-include GNUMakefile.postamble

