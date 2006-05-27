include $(GNUSTEP_MAKEFILES)/common.make

# FIXME: cannot use this one. It will not work for GNUmakefile
# Only for Darwin
#ADDITIONAL_CFLAGS += -mdynamic-no-pic -falign-loops=16

SUBPROJECT_NAME = IoVM_base

IoVM_base_C_FILES = \
	BStream.c \
	BStreamTag.c \
	ByteArray.c \
	Collector.c \
	CollectorMarker.c \
	Common.c \
	Coro.c \
	Date.c \
	Datum.c \
	Duration.c \
	DynLib.c \
	Hash.c \
	MainArgs.c \
	List.c \
	PHash.c \
	PortableGettimeofday.c \
	PortableSnprintf.c \
	PortableStrlcpy.c \
	PortableStrptime.c \
	PortableTruncate.c \
	PortableUContext.c \
	PortableUsleep.c \
	RandomGen.c \
	Sorting.c \
	Stack.c \
	UTinstant.c \
	UTzones.c 

IoVM_base_OBJ_FILES = \
	PortableUContext_asm.o

-include GNUmakefile.preamble
include $(GNUSTEP_MAKEFILES)/subproject.make
-include GNUmakefile.postamble

