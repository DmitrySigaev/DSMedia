include ../../config.mak

VPATH=$(SRC_PATH)/Plugins/codec_pack

# NOTE: -I.. is needed to include config.h - DIRECTSOUND_VERSION is needed for GCC compil..
CFLAGS= $(OPTFLAGS) -Wall -I.. -I$(SRC_PATH)/include -I$(DX_INCLUDE) -DDIRECTSOUND_VERSION=0x0500

ifeq ($(DEBUGBUILD), yes)
CFLAGS+=-g
LDFLAGS+=-g
endif

LINKDXPATH=-L$(DX_LIB)
LINKDXLIBS=-ldsound -ldxguid -lddraw -lole32 -lgdi32 -lopenGL32

#common obj
OBJS=dx_audio.o dx_video.o dx_window.o dx_2d.o


SRCS := $(OBJS:.o=.c) 

ifeq ($(CONFIG_WIN32),yes)
LIB=dx_hw.dll
LDFLAGS+=-export-symbols dx_hw.def
else
LIB=dx_hw.so
endif

all: $(LIB)


$(LIB): $(OBJS)
	windres dx_hw.rc dw_hw.o
	$(CC) $(SHFLAGS) $(LDFLAGS) -o ../../bin/gcc/$@ $(OBJS) dw_hw.o $(LINKDXPATH) $(LINKDXLIBS) -L../../bin/gcc -lm4systems $(EXTRALIBS) 


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
