include ../../config.mak

VPATH=$(SRC_PATH)/M4Systems/MP4

# NOTE: -I.. is needed to include config.h
CFLAGS= $(OPTFLAGS) -DGCCLONGINT -Wall -I.. -I$(SRC_PATH)/include

#movie fragments enabled
ifeq ($(ISOFRAGMENTS), no)
CFLAGS+= -DM4_ISO_NO_FRAGMENTS
endif

#file format is read-only
ifeq ($(GPACREADONLY), yes)
CFLAGS+= -DM4_READ_ONLY
endif

ifeq ($(DEBUGBUILD), yes)
CFLAGS+=-g
LDFLAGS+=-g
endif


#common obj
OBJS= 3GPP_atoms_code.o \
	Atom.o \
	AtomPriv.o \
	AVC.o \
	DTE.o \
	HintPacket.o \
	HintSample.o \
	M4_3GPP.o \
	M4_Dump.o \
	M4_FileDataMap.o \
	M4_Fragments.o \
	M4_HintDump.o \
	M4_HintTrack.o \
	M4_Media.o \
	M4_Media_od.o \
	M4_MovieIntern.o \
	M4_Movie_read.o \
	M4_Movie_write.o \
	M4_Publish.o \
	M4_RTPHint.o \
	M4_Track.o \
	M4_dataMap.o \
	M4_packing.o \
	M4_stbl_read.o \
	M4_stbl_write.o \
	M4_store.o \
	RTPPacket.o \
	TimedText.o \
	atoms_code.o \
	base_sample_desc.o \
	ghnt.o


#file mapping for windows
ifeq ($(CONFIG_WIN32), yes)
OBJS += M4_FileMapping.o
else
OBJS += M4_FileMapping_fake.o
endif


SRCS := $(OBJS:.o=.c) 


LIB=../../bin/gcc/temp/libm4systems_mp4.a

all: $(LIB)


$(LIB): $(OBJS)
	$(AR) rc $@ $(OBJS)
	$(RANLIB) $@


%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $< 



dep: depend

depend:
	rm -f .depend
	$(CC) -MM $(CFLAGS) $(SRCS) 1>.depend


clean: 
	rm -f *.o $(LIB)

distclean: clean
	rm -f Makefile.bak .depend



# include dependency files if they exist
#
ifneq ($(wildcard .depend),)
include .depend
endif
