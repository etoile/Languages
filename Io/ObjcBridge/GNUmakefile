include $(GNUSTEP_MAKEFILES)/common.make

# Only for Darwin
#ADDITIONAL_CFLAGS += -mdynamic-no-pic -falign-loops=16 -DSANE_POPEN
ADDITIONAL_CPPFLAGS += -D__ETOILE__
ADDITIONAL_LDFLAGS +=
ADDITIONAL_INCLUDE_DIRS += \
        -I../iovm \
        -I../basekit \
        -I../skipdb \
        -I../garbagecollector \
        -I../coroutine \
	-I../Vector


SUBPROJECT_NAME = ObjcBridge

ObjcBridge_OBJC_FILES = \
	IoAppKit.m \
	IoFoundation.m \
	Runtime.m \
	Io2Objc.m \
	ObjcSubclass.m \
	IoObjcBridge.m \
	IoObjcBridgeInit.m \
	Io_NSApplicationMain.m \
	Objc2Io.m

include $(GNUSTEP_MAKEFILES)/subproject.make
