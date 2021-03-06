include ../../config.mak

VPATH=$(SRC_PATH)/Plugins/render2d

# NOTE: -I.. is needed to include config.h
CFLAGS= $(OPTFLAGS) -Wall -I.. -I$(SRC_PATH)/include

ifeq ($(DEBUGBUILD), yes)
CFLAGS+=-g
LDFLAGS+=-g
endif


#common obj
OBJS= background2d.o \
	copy_pixels.o \
	drawable.o \
	flashshape_proto.o \
	form.o \
	geometry_stacks.o \
	grouping.o \
	grouping_stacks.o \
	ifs2d.o \
	ils2d.o \
	layout.o \
	path_layout.o \
	render2d.o \
	render2d_nodes.o \
	sensor_stacks.o \
	sound.o \
	text.o \
	texture_stacks.o \
	viewport.o \
	visualsurface2d.o \
	visualsurface2d_draw.o	\
	./SVG/render_svg_nodes.o	\
	./SVG/render_svg_text.o	\
	./SVG/svg_animation.o	\
	./SVG/svg_media.o	\
	./SMIL/smil_animation.o	\


LINKFLAGS=-L../../bin/gcc -lm4systems

ifeq ($(DISABLE_SVG), yes)
CFLAGS+=-DM4_DISABLE_SVG
endif


SRCS := $(OBJS:.o=.c) 

ifeq ($(CONFIG_WIN32),yes)
LIB=render2d.dll
LDFLAGS+=-export-symbols render2d.def 
else
LIB=render2d.so
endif

all: $(LIB)


$(LIB): $(OBJS)
	$(CC) $(SHFLAGS) $(LDFLAGS) -o ../../bin/gcc/$@ $(OBJS) $(LINKFLAGS) $(EXTRALIBS)


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
