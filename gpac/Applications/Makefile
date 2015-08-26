include ../config.mak

APPDIRS=MP4Box MP4Client MP42AVI SceneGenerators
V4STUDIODIR=
INSTDIRS=MP4Client

ifeq ($(USE_WXWIDGETS), yes)
APPDIRS+=Osmo4_wx
V4STUDIODIR=V4Studio
INSTDIRS+=Osmo4_wx
endif

ALLDIRS=$(APPDIRS)

all: apps


apps: 
	for i in $(APPDIRS) ; do $(MAKE) -C $$i all; done 

sggen:
	$(MAKE) -C SceneGenerators all

V4Studio:
	for i in $(V4STUDIODIR) ; do $(MAKE) -C $$i dep; done 

dep:
	for i in $(ALLDIRS) ; do $(MAKE) -C $$i dep; done 

clean: 
	for i in $(ALLDIRS) ; do $(MAKE) -C $$i clean; done 

distclean: 
	for i in $(ALLDIRS) ; do $(MAKE) -C $$i distclean; done 

install:
	for i in $(INSTDIRS) ; do $(MAKE) -C $$i install; done

uninstall:
	for i in $(INSTDIRS) ; do $(MAKE) -C $$i uninstall; done