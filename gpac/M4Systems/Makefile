include ../config.mak

outlibs=libm4systems_bifs.a libm4systems_esm.a libm4systems_ietf.a libm4systems_mp4.a libm4systems_od.a libm4systems_sg.a libm4systems_auth.a libm4systems_tools.a libm4systems_render.a

#all OS and lib independent
M4LIBDIRS=Tools OD IETF MP4 authoring SceneGraph BIFS ESM render

ifeq ($(CONFIG_JS),no)
else
ifeq ($(CONFIG_JS),local)
EXTRALIBS+= -L../../extra_lib/lib/gcc
endif
EXTRALIBS+= -l$(JS_LIBS)
endif

ifeq ($(CONFIG_ZLIB), local)
CFLAGS+= -I../../extra_lib/include/zlib
EXTRALIBS+=-L../../extra_lib/lib/gcc
endif
EXTRALIBS+=-lz


ifeq ($(CONFIG_WIN32),yes)
LIB=libm4systems.dll
LDFLAGS+=-export-symbols m4systems.def
else
LIB=libm4systems.so
EXTRALIBS+=-lpthread -ldl
endif


all: lib

lib: $(LIB)

$(LIB):
	rm -f ../bin/gcc/libm4systems.a
	rm -f ../bin/gcc/$(LIB)
	for thelib in $(outlibs);	do rm -f ../bin/gcc/temp/$$thelib;	done
	for i in $(M4LIBDIRS) ; do $(MAKE) -C $$i all; done
	for thelib in $(outlibs);	do ar x ../bin/gcc/temp/$$thelib;	done
	ar cr ../bin/gcc/libm4systems_static.a *.o
	ranlib ../bin/gcc/libm4systems_static.a	
	$(CC) $(SHFLAGS) $(LDFLAGS) -o ../bin/gcc/$@ *.o $(EXTRALIBS)
	rm -f *.o

dep:
	for i in $(M4LIBDIRS) ; do $(MAKE) -C $$i dep; done

clean: 
	for i in $(M4LIBDIRS) ; do $(MAKE) -C $$i clean; done
	rm -rf ../bin/gcc/libm4systems_static.a
	rm -rf ../bin/gcc/$(LIB)

distclean: 
	for i in $(M4LIBDIRS) ; do $(MAKE) -C $$i distclean; done
