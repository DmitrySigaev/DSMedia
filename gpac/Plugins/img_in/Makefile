include ../../config.mak

VPATH=$(SRC_PATH)/Plugins/img_in

# NOTE: -I.. is needed to include config.h
CFLAGS= $(OPTFLAGS) -Wall -I.. -I$(SRC_PATH)/include

ifeq ($(DEBUGBUILD), yes)
CFLAGS+=-g
LDFLAGS+=-g
endif

NEED_LOCAL_LIB="no"
LOCAL_LIB=../../bin/gcc
LINKLIBS= -lm4systems

#common obj
OBJS=img_dec.o img_in.o bmp_dec.o

#png config
ifeq ($(CONFIG_PNG), no)
else
OBJS+=png_dec.o
LINKLIBS+= -lpng -lz
CFLAGS+=-DM4_HAS_PNG
#local png lib
ifeq ($(CONFIG_PNG), local)
NEED_LOCAL_LIB="yes"
CFLAGS+= -I../../extra_lib/include/png
endif
endif

ifeq ($(CONFIG_ZLIB),local)
NEED_LOCAL_LIB="yes"
CFLAGS+= -I../../extra_lib/include/zlib
endif

#jpeg config
ifeq ($(CONFIG_JPEG), no)
else
OBJS+=jpeg_dec.o
LINKLIBS+= -ljpeg
CFLAGS+=-DM4_HAS_JPEG
#local png lib
ifeq ($(CONFIG_JPEG), local)
NEED_LOCAL_LIB="yes"
CFLAGS+= -I../../extra_lib/include/jpeg
endif
endif

#big-endian config (needed for PNG dec)
ifeq ($(IS_BIGENDIAN), yes)
CFLAGS+=-DM4_BIG_ENDIAN
endif

#add local lib path
ifeq ($(NEED_LOCAL_LIB), "yes")
LOCAL_LIB+=-L../../extra_lib/lib/gcc
endif



SRCS := $(OBJS:.o=.c) 

ifeq ($(CONFIG_WIN32),yes)
LINKLIBS+=-lwinmm
LIB=img_in.dll
LDFLAGS+=-export-symbols img_in.def 
else
LIB=img_in.so
endif

all: $(LIB)


$(LIB): $(OBJS)
	$(CC) $(SHFLAGS) $(LDFLAGS) -o ../../bin/gcc/$@ $(OBJS) $(EXTRALIBS) -L$(LOCAL_LIB) $(LINKLIBS)


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
