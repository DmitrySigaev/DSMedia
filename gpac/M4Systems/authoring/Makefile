include ../../config.mak

VPATH=$(SRC_PATH)/M4Systems/authoring

# NOTE: -I.. is needed to include config.h
CFLAGS= $(OPTFLAGS) -Wall -I.. -I$(SRC_PATH)/include

ifeq ($(DEBUGBUILD), yes)
CFLAGS+=-g
LDFLAGS+=-g
endif

#file format is read-only
ifeq ($(GPACREADONLY), yes)
CFLAGS+= -DM4_READ_ONLY
endif


#big-endian config
ifeq ($(IS_BIGENDIAN), yes)
CFLAGS+=-DM4_BIG_ENDIAN
endif

#common obj
OBJS=	AVParsers.o\
	gpac_ogg.o \
	ISMA.o\
	LoadBT.o\
	LoadXMTA.o\
	LoadMP4.o\
	M4TrackHinter.o\
	MediaExport.o\
	MediaImport.o\
	RTPPacketizer.o\
	RTPPck3GPP.o\
	RTPPckMPEG4.o\
	RTPPckMPEG12.o\
	SceneDump.o\
	SceneManager.o\
	SceneStats.o\
	TrackTools.o\
	TextImport.o\
	xml_parser.o\
	avilib.o \
	SWFParse.o \
	SWFShape.o	\
	mpeg2_ps.o	\


SRCS := $(OBJS:.o=.c) 

LIB=../../bin/gcc/temp/libm4systems_auth.a

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
