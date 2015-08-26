include ../../config.mak

VPATH=$(SRC_PATH)/Plugins/svg_loader

# NOTE: -I.. is needed to include config.h
CFLAGS= $(OPTFLAGS) -Wall -I.. -I$(SRC_PATH)/include

ifeq ($(DEBUGBUILD), yes)
CFLAGS+=-g
LDFLAGS+=-g
endif

#common obj
OBJS= svg_loader.o

SRCS := $(OBJS:.o=.c) 

CFLAGS+=$(XML2_CFLAGS)

LOCAL_LIB=
ifeq ($(CONFIG_ZLIB), local)
CFLAGS+= -I../../extra_lib/include/zlib
LOCAL_LIB+=-L../../extra_lib/lib/gcc
endif

ifeq ($(CONFIG_WIN32),yes)
LIB=svg_loader.dll
LDFLAGS+=-export-symbols svg_loader.def 
else
LIB=svg_loader.so
endif

all: $(LIB)


$(LIB): $(OBJS)
	$(CC) $(SHFLAGS) $(LDFLAGS) -o ../../bin/gcc/$@ $(OBJS) -L../../bin/gcc -lm4systems $(XML2_LFLAGS) $(LOCAL_LIB)


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
