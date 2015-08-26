include ../../config.mak

VPATH=$(SRC_PATH)/Plugins/rtp_in

# NOTE: -I.. is needed to include config.h
CFLAGS= $(OPTFLAGS) -Wall -I.. -I$(SRC_PATH)/include

ifeq ($(DEBUGBUILD), yes)
CFLAGS+=-g
LDFLAGS+=-g
endif


#common obj
OBJS= rtp_in.o rtp_payloads.o rtp_session.o rtp_signaling.o rtp_stream.o sdp_fetch.o sdp_load.o


SRCS := $(OBJS:.o=.c) 

ifeq ($(CONFIG_WIN32),yes)
LIB=rtp_in.dll
LDFLAGS+=-export-symbols rtp_in.def 
else
LIB=rtp_in.so
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
