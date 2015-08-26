include ../../config.mak

VPATH=$(SRC_PATH)/Plugins/xvid_dec

# NOTE: -I.. is needed to include config.h
CFLAGS= $(OPTFLAGS) -Wall -I.. -I$(SRC_PATH)/include

ifeq ($(DEBUGBUILD), yes)
CFLAGS+=-g
LDFLAGS+=-g
endif

#common obj
OBJS=xvid_dec.o

#local xvid lib
ifeq ($(CONFIG_XVID), local)
CFLAGS+= -I../../extra_lib/include/xvid
EXTRALIBS+= -L../../extra_lib/lib/gcc
endif
EXTRALIBS+= -lxvidcore

SRCS := $(OBJS:.o=.c) 

ifeq ($(CONFIG_WIN32),yes)
LIB=xvid_dec.dll
LDFLAGS+=-export-symbols xvid_dec.def 
else
LIB=xvid_dec.so
endif

all: $(LIB)


$(LIB): $(OBJS)
	$(CC) $(SHFLAGS) $(LDFLAGS) -o ../../bin/gcc/$@ $(OBJS) -L../../bin/gcc -lm4systems $(EXTRALIBS)


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
