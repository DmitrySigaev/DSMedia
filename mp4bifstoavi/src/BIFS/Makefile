include ../../config.mak

VPATH=$(SRC_PATH)/M4Systems/BIFS

# NOTE: -I.. is needed to include config.h
CFLAGS= $(OPTFLAGS) -Wall -I.. -I$(SRC_PATH)/include

ifeq ($(DEBUGBUILD), yes)
CFLAGS+=-g
LDFLAGS+=-g
endif


#common obj
OBJS= AADecoder.o BIFSComDec.o BIFSComEnc.o BIFSDec.o BIFSEnc.o BIFSMemDec.o Conditional.o FieldDec.o FieldEnc.o PredictiveMFField.o Quantize.o SFScriptDec.o SFScriptEnc.o Unquantize.o bifs_node_tables.o



SRCS := $(OBJS:.o=.c) 


LIB=../../bin/gcc/temp/libm4systems_bifs.a

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
