
CC      = gcc
OBJCOPY = objcopy
SIZE    = size
NM	= nm
MAKETXT = tools/srec_cat
ECHO	= echo

TARGET_CPU = msp430f5438a
MEMORY_MODEL = medium
WARNING_FLAGS = 
CFLAGS0  = -g -std=c99 -O0 $(WARNING_FLAGS)\
	-ffunction-sections -fdata-sections -D__NO_MINGW_LFS
LDFLAGS = -g -std=c99 -O0 $(WARNING_FLAGS) -Wl,-gc-sections hid.lib -lsetupapi -luuid


CFLAGS = $(CFLAGS0) 
# Support LTO, todo later
# -flto
#LDFLAGS += -flto

ALL_DEFINES = 
ALL_INCLUDEDIRS = \
	.


SRCS = BSL_Commands_5xx.c  BSL_IO_UART.c  BSL_Interface.c          BSL_UART_Protocol_ROM.c  CRC.c        TextFileIO.c \
BSL_Commands_ROM.c  BSL_IO_USB.c   BSL_UART_Protocol_5xx.c  BSL_USB_Protocol_5xx.c   HidDevice.c  main.c

OBJDIR = objs.win
OBJS0 = $(SRCS:.c=.o)
OBJS = $(addprefix $(OBJDIR)/, $(OBJS0))
DEPFILES = $(OBJS:.o=.d)

#####################
# rules to build the object files
$(OBJDIR)/%.o: %.c
	@$(ECHO) "Compiling $<"
	@test -d $(OBJDIR) || mkdir -pm 775 $(OBJDIR)
	@test -d $(@D) || mkdir -pm 775 $(@D)
	@-$(RM) $@
	@$(CC) $(CFLAGS) $(ALL_DEFINES:%=-D%) $(ALL_INCLUDEDIRS:%=-I%) -c $< -o $@

$(OBJDIR)/%.d: %.c
	@test -d $(OBJDIR) || mkdir -pm 775 $(OBJDIR)
	@test -d $(@D) || mkdir -pm 775 $(@D)
	@-$(RM) $@
	@$(CC) $(ALL_DEFINES:%=-D%) $(ALL_INCLUDEDIRS:%=-I%) -MM $< -MT $(patsubst %.d, %.o, $@)> $@


# create .hex file from .elf
%.hex: %.elf
	@$(OBJCOPY) -O ihex $< $@
	@$(SIZE) $<
%.txt: %.hex
	$(MAKETXT) -O $@ -TITXT $< -I
	unix2dos $@

# create firmware image from common objects and example source file

all: $(DEPFILES) $(OBJS) BSL.exe

# if need support loader  $(OBJDIR)/$(OBJDIR)/symbols.o
BSL.exe: ${OBJS}
	@$(ECHO) "Linking $@ second pass"
	@${CC} $^ ${LDFLAGS} -o $@

size: all
	msp430-size *.elf

flash: BSL.hex
	~/bin/mspdebug tilib 'prog BSL.elf' run

$(OBJDIR)/BSL.elf: ${OBJS}
	@echo "Link $@ first pass"
	@${CC} $^ ${LDFLAGS} -o $@

$(OBJDIR)/symbols.c: $(OBJDIR)/BSL.elf
	@echo "Generate symbol table"
	@echo "#include \"loader/symbols-def.h\"" > $@
	@echo "const struct symbols symbols[] = {" >> $@
	@$(NM) $^ | awk '/([0-9a-f])+ [ABDRST] ([a-zA-Z][0-9A-Za-z_]+)$$/{print "{\"" $$3 "\",(void*)0x" $$1 "}," }' | sort -f -t ','>> $@
	@echo "{0, 0}};" >> $@

ifneq ($(MAKECMDGOALS), clean)
-include $(DEPFILES)
endif
.SILENT:
.PHONY:	clean
clean:
	rm -f $ *.elf *.hex
	rm -Rf $(OBJDIR)
