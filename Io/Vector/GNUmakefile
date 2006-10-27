include $(GNUSTEP_MAKEFILES)/common.make

ADDITIONAL_INCLUDE_DIRS += \
	-I../iovm \
	-I../basekit \
	-I../skipdb \
	-I../garbagecollector \
	-I../coroutine

ifeq ($(FOUNDATION_LIB), apple)
  	# Only for Darwin
  	#ADDITIONAL_CFLAGS += -mdynamic-no-pic -falign-loops=16 -DSANE_POPEN
  	ADDITIONAL_LDFLAGS += -framework Accelerate -faltivec
endif

SUBPROJECT_NAME = Vector

Vector_C_FILES = \
	IoVectorInit.c \
	IoBox.c \
	IoVector.c \
	Vector.c

include $(GNUSTEP_MAKEFILES)/subproject.make
