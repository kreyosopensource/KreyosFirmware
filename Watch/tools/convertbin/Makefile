
CC      = gcc
OBJCOPY = objcopy
SIZE    = size
NM	= nm
MAKETXT = tools/srec_cat
ECHO	= echo

TARGET_CPU = msp430f5438a
MEMORY_MODEL = medium
WARNING_FLAGS = 
CFLAGS0  = -g -std=c99 -O2 $(WARNING_FLAGS)\
	-ffunction-sections -fdata-sections -D__NO_MINGW_LFS
LDFLAGS = -g -std=c99 -O2 $(WARNING_FLAGS)

CFLAGS = $(CFLAGS0) 
# Support LTO, todo later
# -flto
#LDFLAGS += -flto

ALL_DEFINES = 
ALL_INCLUDEDIRS = \

SRCS =  TextFileIO.c \
             CRC.c main.c

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

all: $(DEPFILES) $(OBJS) convert

# if need support loader  $(OBJDIR)/$(OBJDIR)/symbols.o
convert: ${OBJS}
	@$(ECHO) "Linking $@ second pass"
	@${CC} $^ ${LDFLAGS} -o $@

size: all
	msp430-size *.elf


ifneq ($(MAKECMDGOALS), clean)
-include $(DEPFILES)
endif
.SILENT:
.PHONY:	clean
clean:
	rm -f $ *.elf *.hex
	rm -Rf $(OBJDIR)
