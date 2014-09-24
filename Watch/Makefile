ifeq ($(MSPPATH),)
MSPPATH = /home/junsu/mspgccx/bin
endif
CC      = clang 
OBJCOPY = objcopy
SIZE    = size
NM	= nm
MAKETXT = tools/srec_cat
ECHO	= echo

TARGET_CPU = msp430f5438a
MEMORY_MODEL = medium
#WARNING_FLAGS = -W -Wall
CFLAGS0  = -g -std=c99 -O0 $(WARNING_FLAGS)\
	-ffunction-sections -fdata-sections
LDFLAGS = -g -std=c99 -O0 $(WARNING_FLAGS)

FWVERSION="DEBUG"

CFLAGS = $(CFLAGS0)
# Support LTO, todo later
# -flto
#LDFLAGS += -flto

ALL_DEFINES = AUTOSTART_ENABLE=1 UNITTEST=1 HAVE_ALLOCA_H=0 BYTE_ORDER=LITTLE_ENDIAN PAWN_CELL_SIZE=16 AMX_NATIVETABLE=window_natives HAVE_BLE=1 FWVERSION=\"$(FWVERSION)\"
ALL_INCLUDEDIRS = \
	. \
	core \
	core/lib \
	platform/common \
	platform/native \
	platform/iwatch \
	btstack/include \
	btstack/src \
	platform/common/btstack \
	platform/iwatch/btstack \
	unittest \

#######################################
# source files
CORE   = \
	core/cfs/cfs-coffee.c \
    core/sys/autostart.c \
    core/sys/ctimer.c \
    core/sys/etimer.c \
    core/sys/energest.c \
    core/sys/process.c \
    core/sys/rtimer.c \
    core/sys/stimer.c \
    core/sys/timer.c \
    core/lib/assert.c \
    core/lib/list.c

COMMON = \
	platform/common/memlcd.c

PLATFORM = \
	platform/native/clock.c \
	platform/native/notimpl.c \
	platform/native/dev/xmem.c \
	platform/native/Template_Driver.c

GRLIB0 = \
	circle.c \
	charmap.c \
	context.c \
	image.c \
	line.c \
	rectangle.c \
	triagle.c \
	string.c

GRLIB_FONTS = \
	fontgothic14.c \
	fontgothic18.c \
	fontgothic18b.c \
	fontgothic24b.c \
	fontgothic24b.c \
	fontgothic28.c \
	fontgothic28b.c \
	fonticon16.c \
	fonticon32.c \
	fonticon48.c \
	fontnimbus30.c \
	fontnimbus38.c \
	fontnimbus46.c \
	fontnimbus52.c \
	fontnimbus34.c \
	fontnimbus40.c \
	fontnimbus50.c \
	fontnimbus91.c \
	fontunicode.c \
	logoimg.c
GRLIB = $(addprefix grlib/, $(GRLIB0)) $(addprefix grlib/fonts/, $(GRLIB_FONTS))

WATCH = \
    watch/analog-watch.c \
    watch/btconfig.c \
    watch/configtime.c \
    watch/countdown.c \
    watch/controller.c \
    watch/controls.c \
    watch/charging.c \
    watch/digit-watch.c \
    watch/stopwatch.c \
    watch/names.c \
    watch/menu.c \
    gesture/gesture.c \
    watch/cordic.c \
    watch/calendar.c \
    watch/stopwatch.c \
    watch/sportswatch.c \
    watch/sportwait.c \
    watch/sportselect.c \
    watch/sportsdata.c \
    watch/phone.c \
    watch/notification.c \
    watch/worldclock.c \
    watch/host.c \
    watch/today.c \
    watch/reset.c \
    watch/siri.c \
    watch/status.c \
    watch/test.c \
    watch/sleep.c \
    watch/upgrade.c \
    watch/welcome.c \
    watch/window.c

BTSTACK= \
	platform/common/btstack/ble_file_handler.c \
	platform/common/btstack/ble_handler.c \
	platform/common/btstack/stlv.c \
	platform/common/btstack/stlv_client.c \
	platform/common/btstack/stlv_server.c \
	platform/common/btstack/stlv_transport.c \
	platform/common/btstack/stlv_handler.c

PAWN=pawnscript/amx.c pawnscript/amxcons.c pawnscript/amxwindow.c pawnscript/amxstring.c

SRCS = $(BTSTACK) $(CORE) $(PLATFORM) $(GRLIB) $(WATCH) $(PAWN) $(COMMON) \
 unittest/CuTest.c \
 unittest/AllTests.c \
 unittest/bleHandlerTest.c \
 unittest/cfsTest.c \
 unittest/windowTest.c \
 unittest/stlvTest.c \
 unittest/sleepmock.c \
 unittest/gestureTest.c \
 unittest/TestUtility/stlv_test_stub.c \
 unittest/hfpTest.c 

OBJDIR = objs.native
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
	unix2dos $

# create firmware image from common objects and example source file

all: $(DEPFILES) $(OBJS) iwatch

# if need support loader  $(OBJDIR)/$(OBJDIR)/symbols.o
iwatch: ${OBJS}
	@$(ECHO) "Linking $@ second pass"
	@${CC} $^ ${LDFLAGS} -o $@

flash: iwatch.hex
	~/bin/mspdebug tilib 'prog iwatch.elf' run

$(OBJDIR)/iwatch.elf: ${OBJS}
	@echo "Link $@ first pass"
	@${CC} $^ ${LDFLAGS} -o $@

$(OBJDIR)/symbols.c: $(OBJDIR)/iwatch.elf
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
