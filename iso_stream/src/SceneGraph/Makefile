include ../../config.mak

VPATH=$(SRC_PATH)/M4Systems/SceneGraph

# NOTE: -I.. is needed to include config.h
CFLAGS= $(OPTFLAGS) -Wall -I.. -I$(SRC_PATH)/include

ifeq ($(DEBUGBUILD), yes)
CFLAGS+=-g
LDFLAGS+=-g
endif

#common obj
OBJS= base_scenegraph.o mpeg4_animators.o mpeg4_commands.o mpeg4_nodes.o mpeg4_valuator.o vrml_interpolators.o vrml_proto.o vrml_route.o vrml_script.o vrml_SpiderMonkey.o vrml_tools.o x3d_nodes.o

ifeq ($(DISABLE_SVG), yes)
CFLAGS+=-DM4_DISABLE_SVG
else
OBJS+=svg_nodes.o svg_tools.o
endif

ifeq ($(CONFIG_JS), no)
else
#add spiderMonkey - platform ID (XP_PC, XP_UNIX, ..) is in the global OS config file
CFLAGS+=-DM4_USE_SPIDERMONKEY $(JS_FLAGS)
endif


SRCS := $(OBJS:.o=.c) 

LIB=../../bin/gcc/temp/libm4systems_sg.a

all: $(LIB)


$(LIB): $(OBJS)
	$(AR) rc $@ $(OBJS)
	$(RANLIB) $@


%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $< 


clean: 
	rm -f *.o $(LIB)

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
