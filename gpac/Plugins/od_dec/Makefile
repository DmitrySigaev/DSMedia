include ../../config.mak

VPATH=$(SRC_PATH)/Plugins/od_dec

# NOTE: -I.. is needed to include config.h
CFLAGS= $(OPTFLAGS) -Wall -I.. -I$(SRC_PATH)/include

ifeq ($(DEBUGBUILD), yes)
CFLAGS+=-g
LDFLAGS+=-g
endif


#common obj
OBJS= od_dec.o

SRCS := $(OBJS:.o=.c) 

ifeq ($(CONFIG_WIN32),yes)
LIB=od_dec.dll
LDFLAGS+=-export-symbols od_dec.def 
else
LIB=od_dec.so
endif

all: $(LIB)


$(LIB): $(OBJS)
	$(CC) $(SHFLAGS) $(LDFLAGS) -o ../../bin/gcc/$@ $(OBJS) $(EXTRALIBS) -L../../bin/gcc -lm4systems


%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $< 


clean: 
	rm -f $(OBJS) ../../bin/gcc/$(LIB)

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
