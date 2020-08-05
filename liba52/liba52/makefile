#LibA52 Dreamcast K:OS Makefile (c) PH3NOM 2011

KOS_CFLAGS += -I. -I../include/

OBJS = bitstream.o bit_allocate.o downmix.o imdct.o parse.o

INCS += -I. -DLITTLE_ENDIAN=1 -DLIBA52_FIXED
CFLAGS += -Os -m4-single-only -ffast-math

all: liba52.a

liba52.a: $(OBJS)
	$(KOS_AR) rcs liba52.a $(OBJS)

include $(KOS_BASE)/Makefile.rules
