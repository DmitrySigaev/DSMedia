include ../../config.mak

VPATH=$(SRC_PATH)/Plugins/mp3_in

# NOTE: -I.. is needed to include config.h
CFLAGS= $(OPTFLAGS) -Wall -I.. -I$(SRC_PATH)/include

ifeq ($(DEBUGBUILD), yes)
CFLAGS+=-g
LDFLAGS+=-g
endif


#common obj
OBJS= mp3_in.o



#mad config
ifeq ($(CONFIG_MAD), no)
else
OBJS+=mad_dec.o
CFLAGS+=-DM4_HAS_MAD
#local mad lib
ifeq ($(CONFIG_MAD), local)
EXTRALIBS+=-L../../extra_lib/lib/gcc
CFLAGS+= -I../../extra_lib/include/mad
endif
EXTRALIBS+= -lmad
endif

SRCS := $(OBJS:.o=.c) 

ifeq ($(CONFIG_WIN32),yes)
LIB=mp3_in.dll
LDFLAGS+=-export-symbols mp3_in.def 
else
LIB=mp3_in.so
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
