include ../../config.mak

VPATH=$(SRC_PATH)/M4Systems/OD

# NOTE: -I.. is needed to include config.h
CFLAGS= $(OPTFLAGS) -Wall -I.. -I$(SRC_PATH)/include

ifeq ($(DEBUGBUILD), yes)
CFLAGS+=-g
LDFLAGS+=-g
endif


#common obj
OBJS= ESDescriptor.o \
	InitialObjectDescriptor.o \
	M4F_InitialObjectDescriptor.o \
	M4F_ObjectDescriptor.o \
	OCICodec.o \
	ODCoDec.o \
	ODCommand.o \
	OD_Dump.o \
	OD_Parse.o \
	ObjectDescriptor.o \
	QoSDescriptor.o \
	SLConfigDescriptor.o \
	SyncLayer.o \
	comPrivate.o \
	descPrivate.o \
	descriptors.o \
	od_code.o

SRCS := $(OBJS:.o=.c) 


LIB=../../bin/gcc/temp/libm4systems_od.a

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
