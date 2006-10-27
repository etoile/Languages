include $(GNUSTEP_MAKEFILES)/common.make

SUBPROJECT_NAME = coroutine

ADDITIONAL_INCLUDE_DIRS += \
        -I../basekit

coroutine_C_FILES = \
	Coro.c \
	PortableUContext.c 

coroutine_OBJ_FILES = \
	$(GNUSTEP_OBJ_DIR)/PortableUContext_asm.o

$(GNUSTEP_OBJ_DIR)/%$(OEXT): %.S
	$(ECHO_COMPILING)$(CC) $< -c -o $@$(END_ECHO)

include $(GNUSTEP_MAKEFILES)/subproject.make

