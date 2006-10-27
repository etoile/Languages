include $(GNUSTEP_MAKEFILES)/common.make

# Only for Darwin
#ADDITIONAL_CFLAGS += -mdynamic-no-pic -falign-loops=16 -DSANE_POPEN
ADDITIONAL_LDFLAGS +=
ADDITIONAL_INCLUDE_DIRS += -I../basekit

SUBPROJECT_NAME = skipdb

skipdb_C_FILES = \
	JFile.c \
	SkipDBCursor.c \
	SkipDBRecord.c \
	UDBIndex.c \
	UDBRecords.c \
	SkipDB.c \
	SkipDBM.c \
	UDB.c \
	UDBRecord.c

include $(GNUSTEP_MAKEFILES)/subproject.make

