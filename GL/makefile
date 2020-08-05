#LibOpenGL Makefile (C) Josh PH3NOM Pearson 2013

KOS_CFLAGS += -I. -I../include/

OBJS = gl-matrix.o gl-render.o

INCS += -I. -DLITTLE_ENDIAN=1
CFLAGS += -O3 -m4-single-only -ffast-math

all: libopengl.a

libopengl.a: $(OBJS)
	$(KOS_AR) rcs libopengl.a $(OBJS)

include $(KOS_BASE)/Makefile.rules
