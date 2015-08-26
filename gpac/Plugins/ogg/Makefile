include ../../config.mak

VPATH=$(SRC_PATH)/Plugins/ogg

# NOTE: -I.. is needed to include config.h
CFLAGS= $(OPTFLAGS) -Wall -I.. -I$(SRC_PATH)/include $(FT_CFLAGS)

ifeq ($(DEBUGBUILD), yes)
CFLAGS+=-g
LDFLAGS+=-g
endif

#common obj
OBJS= ogg_in.o ogg_load.o

SRCS := $(OBJS:.o=.c) 

NEED_LOCAL_LIB="no"
LOCAL_LIB=../../bin/gcc
LINKLIBS= -lm4systems -logg

ifeq ($(CONFIG_OGG), local)
NEED_LOCAL_LIB="yes"
endif

ifeq ($(CONFIG_VORBIS), no)
else
OBJS+= vorbis_dec.o
LINKLIBS+= -lvorbis
CFLAGS+=-DM4_HAS_VORBIS
ifeq ($(CONFIG_VORBIS), local)
NEED_LOCAL_LIB="yes"
CFLAGS+= -I../../extra_lib/include
endif
endif

ifeq ($(CONFIG_THEORA), no)
else
OBJS+= theora_dec.o
LINKLIBS+= -ltheora
CFLAGS+=-DM4_HAS_THEORA
ifeq ($(CONFIG_THEORA), local)
NEED_LOCAL_LIB="yes"
CFLAGS+= -I../../extra_lib/include
endif
endif


#add local lib path
ifeq ($(NEED_LOCAL_LIB), "yes")
LOCAL_LIB+=-L../../extra_lib/lib/gcc
endif


ifeq ($(CONFIG_WIN32),yes)
LIB=ogg_xiph.dll
LDFLAGS+=-export-symbols ogg.def 
else
LIB=ogg_xiph.so
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
