include ../../config.mak

VPATH=$(SRC_PATH)/Plugins/m4_rend

# NOTE: -I.. is needed to include config.h
CFLAGS= $(OPTFLAGS) -Wall -I.. -I$(SRC_PATH)/include

ifeq ($(DEBUGBUILD), yes)
CFLAGS+=-g
LDFLAGS+=-g
endif

#big-endian config (needed for ARGB pixel format)
ifeq ($(IS_BIGENDIAN), yes)
CFLAGS+=-DM4_BIG_ENDIAN
endif


#common obj
OBJS= ftgrays.o \
	m4_load.o \
	m4_raster_565.o \
	m4_raster_argb.o \
	m4_raster_rgb.o \
	m4_stencil.o \
	m4_surface.o

SRCS := $(OBJS:.o=.c) 

ifeq ($(CONFIG_WIN32),yes)
LIB=m4_rend.dll
LDFLAGS+=-export-symbols m4_rend.def 
else
LIB=m4_rend.so
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
