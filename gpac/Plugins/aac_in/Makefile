include ../../config.mak

VPATH=$(SRC_PATH)/Plugins/aac_in

# NOTE: -I.. is needed to include config.h
CFLAGS= $(OPTFLAGS) -Wall -I.. -I$(SRC_PATH)/include

ifeq ($(DEBUGBUILD), yes)
CFLAGS+=-g
LDFLAGS+=-g
endif


#common obj
OBJS= aac_in.o

SRCS := $(OBJS:.o=.c) 

#faad config
ifeq ($(CONFIG_FAAD), no)
else
OBJS+=faad_dec.o
CFLAGS+=-DM4_HAS_FAAD
#local faad lib
ifeq ($(CONFIG_FAAD), local)
EXTRALIBS+=-L../../extra_lib/lib/gcc
CFLAGS+= -I../../extra_lib/include/faad
endif
EXTRALIBS+= -lfaad
endif

ifeq ($(CONFIG_WIN32),yes)
LIB=aac_in.dll
LDFLAGS+=-export-symbols aac_in.def 
else
LIB=aac_in.so
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
