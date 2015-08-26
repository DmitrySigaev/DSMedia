include ../../config.mak

VPATH=$(SRC_PATH)/M4Systems/ESM

# NOTE: -I.. is needed to include config.h
CFLAGS= $(OPTFLAGS) -Wall -I.. -I$(SRC_PATH)/include

ifeq ($(DEBUGBUILD), yes)
CFLAGS+=-g
LDFLAGS+=-g
endif


#common obj
OBJS= InlineScene.o InputSensor.o M4NodeInit.o M4Term.o MediaControl.o MediaManager.o MediaMemory.o MediaObject.o MediaSensor.o NetService.o ODBrowser.o ODManager.o channel.o clock.o decoder.o

SRCS := $(OBJS:.o=.c) 


LIB=../../bin/gcc/temp/libm4systems_esm.a

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
