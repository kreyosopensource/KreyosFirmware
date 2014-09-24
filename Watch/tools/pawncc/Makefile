
CC      = gcc
OBJCOPY = objcopy
SIZE    = size
NM	= nm
MAKETXT = tools/srec_cat
ECHO	= echo

WARNING_FLAGS = 
CFLAGS0  = -g -O2 $(WARNING_FLAGS)\
	-ffunction-sections -fdata-sections -D__NO_MINGW_LFS
LDFLAGS = -g -O2 $(WARNING_FLAGS)


CFLAGS = $(CFLAGS0) 
# Support LTO, todo later
# -flto
#LDFLAGS += -flto

ALL_DEFINES = HAVE_ALLOCA_H=0 AMX_ANSIONLY=1 PAWN_CELL_SIZE=16 PAWN_LIGHT
ifeq ($(OS),Windows_NT)
 ALL_DEFINES += __WIN32__
else
 UNAME_S := $(shell uname -s)
 ifeq ($(UNAME_S),Darwin)
   ALL_DEFINES += PAWN_NO_CODEPAGE __BYTE_ORDER=1234 MACOS HAVE_SAFESTR
 endif
endif
ALL_INCLUDEDIRS = \
	linux \
	.


SRCS =  compiler/sc6.c \
compiler/lstring.c     compiler/sc7.c \
compiler/memfile.c     compiler/scexpand.c \
compiler/sci18n.c      compiler/sclist.c \
compiler/sc1.c         compiler/scmemfil.c \
compiler/sc2.c         \
compiler/sc3.c         compiler/scstate.c \
compiler/sc4.c  \
compiler/sc5.c         compiler/scvars.c \
amx/keeloq.c

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

# create firmware image from common objects and example source file

all: $(DEPFILES) $(OBJS) pawncc

# if need support loader  $(OBJDIR)/$(OBJDIR)/symbols.o
pawncc: ${OBJS}
	@$(ECHO) "Linking $@ second pass"
	@${CC} $^ ${LDFLAGS} -o $@


ifneq ($(MAKECMDGOALS), clean)
-include $(DEPFILES)
endif
.SILENT:
.PHONY:	clean
clean:
	rm -f $ *.elf *.hex
	rm -Rf $(OBJDIR)
