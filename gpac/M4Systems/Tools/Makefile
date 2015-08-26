include ../../config.mak

VPATH=$(SRC_PATH)/M4Systems/Tools

# NOTE: -I.. is needed to include config.h
CFLAGS= $(OPTFLAGS) -Wall -I.. -I$(SRC_PATH)/include

ifeq ($(DEBUGBUILD), yes)
CFLAGS+=-g
LDFLAGS+=-g
endif


#common obj
OBJS= chain.o bitstream.o base_encoding.o error.o inifile.o url.o stringparser.o UTF8.o plugin.o


#per-platform source 
ifeq ($(CONFIG_WIN32), yes)
OBJS += ./w32/os_divers.o ./w32/os_net.o ./w32/os_plug.o ./w32/os_thread.o
endif
ifeq ($(CONFIG_LINUX), yes)
OBJS += ./linux/os_divers.o ./linux/os_net.o ./linux/os_plug.o ./linux/os_thread.o
endif

SRCS := $(OBJS:.o=.c) 

LIB=../../bin/gcc/temp/libm4systems_tools.a

all: $(LIB)


$(LIB): $(OBJS)
	$(AR) rc $@ $(OBJS)
	$(RANLIB) $@

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $< 


clean: 
	rm -f $(OBJS) $(LIB)

dep: depend

depend:
	rm -f .depend	
	$(CC) -MM $(CFLAGS) $(SRCS) 1>.depend

distclean: clean
	rm -f Makefile.bak .depend



# include dependency files if they exist
#
ifneq ($(wildcard .depend),)
include .depend
endif
