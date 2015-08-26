include ../../config.mak

VPATH=$(SRC_PATH)/Plugins/ffmpeg_in

# NOTE: -I.. is needed to include config.h
CFLAGS= $(OPTFLAGS) -Wall -I.. -I$(SRC_PATH)/include

ifeq ($(DEBUGBUILD), yes)
CFLAGS+=-g
LDFLAGS+=-g
endif

LOCAL_LIB=
LINKLIBS=-lm4systems -lavcodec -lavformat -lz

#common obj
OBJS=ffmpeg_decode.o ffmpeg_demux.o ffmpeg_load.o

#local ffmpeg lib
ifeq ($(CONFIG_FFMPEG), local)
LOCAL_LIB=-L../../extra_lib/lib/gcc
CFLAGS+= -I../../extra_lib/include
endif

ifeq ($(CONFIG_ZLIB), local)
LOCAL_LIB=-L../../extra_lib/lib/gcc
endif


SRCS := $(OBJS:.o=.c) 

ifeq ($(CONFIG_WIN32),yes)
LIB=ffmpeg_in.dll
LDFLAGS+=-export-symbols ffmpeg_in.def 
else
LIB=ffmpeg_in.so
endif

all: $(LIB)


$(LIB): $(OBJS)
	$(CC) $(SHFLAGS) $(LDFLAGS) -o ../../bin/gcc/$@ $(OBJS) -L../../bin/gcc $(LOCAL_LIB) $(LINKLIBS) $(EXTRALIBS)


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
