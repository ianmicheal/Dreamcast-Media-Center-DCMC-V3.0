#
# DCMC v.2.0 - Dreamcast Media Center 2.0
# (C) 2011-2013 Josh Pearson
#   

TARGET = dcmc.elf

CFLAGS += -O3 -m4-single-only -ffast-math  -funroll-loops

.SRCS = main.c

#LibDCMC - DC Hardware Support Routines
KOS_CFLAGS += -I. -Igl/
KOS_CFLAGS += -I. -Ilibdcmc/
.SRCS += libdcmc/dcmc.c
.SRCS += libdcmc/dcmc-audio.c
.SRCS += libdcmc/aica_cmd.c
.SRCS += libdcmc/bin_core.c
.SRCS += libdcmc/container.c
.SRCS += libdcmc/cstring.c
.SRCS += libdcmc/fifo_buffer.c
.SRCS += libdcmc/font.c
.SRCS += libdcmc/fs.c
.SRCS += libdcmc/input.c
.SRCS += libdcmc/render.c
.SRCS += libdcmc/snddrv.c
.SRCS += libdcmc/texture.c
.SRCS += libdcmc/thumbnail.c
.SRCS += libdcmc/timer.c

#LibPNG Decoder
KOS_CFLAGS+= -I$(KOS_BASE)/../kos-ports/libpng
KOS_CFLAGS+= -I$(KOS_BASE)/../kos-ports/include/png
.SRCS += libdcmc/png_core.c

#LibJPG Decoder
.SRCS += libdcmc/jpg_core.c

#LibAVI - ADX Format Support
KOS_CFLAGS += -I. -Ilibadx/
.SRCS += libadx/LibADX.c

#LibAVI - AVI Format Support
KOS_CFLAGS += -I. -Ilibavi/
.SRCS += libavi/LibAVI.c

#LibMPEG - MPEG Parser Support
KOS_CFLAGS += -I. -Ilibmpeg/
.SRCS += libmpeg/LibMPEG.c

#LibXviD Decoder
KOS_CFLAGS += -I. -Ilibxvid/src/
KOS_CFLAGS += -I. -Ilibxvid/player/
.SRCS += libxvid/player/LibXVID.c

#LibMPEG2 Decoder
KOS_CFLAGS += -I. -Ilibmpeg2/include/
.SRCS += libmpeg2/player/LibMPEG2.c
.SRCS += libmpeg2/player/LibMPEG2SFD.c

#LibMPG123 Decoder
KOS_CFLAGS += -I. -Ilibmpg123/src/libmpg123/
KOS_CFLAGS += -I. -Ilibmpg123/player/
.SRCS += libmpg123/player/LibMPG123.c

#LibA52 Decoder
KOS_CFLAGS += -I. -Iliba52/include/
KOS_CFLAGS += -I. -Iliba52/liba52/
.SRCS += liba52/player/LibA52.c \
	     liba52/player/audio_out.c \
	     liba52/player/audio_out_dc.c \
	     liba52/player/convert2s16.c

#LibFAAD Decoder
KOS_CFLAGS += -I. -Ilibfaad/include/
KOS_CFLAGS += -I. -Ilibfaad/common/mp4ff/
.SRCS += libfaad/player/LibFAAD.c

#LibFLAC Decoder
KOS_CFLAGS += -I. -Ilibflac/include/
KOS_CFLAGS += -I. -Ilibflac/libFLAC/include
.SRCS += libflac/player/LibFLAC.c \

OBJS = $(.SRCS:.c=.o)

all: rm-elf $(TARGET)

include $(KOS_BASE)/Makefile.rules

clean:
	-rm -f $(TARGET) $(OBJS)

rm-elf:
	-rm -f $(TARGET)

$(TARGET): $(OBJS)
	$(KOS_CC) $(KOS_CFLAGS) $(KOS_LDFLAGS) -o $(TARGET) $(KOS_START) \
		$(OBJS) $(OBJEXTRA) -L$(KOS_BASE)/lib  -lflac -lfaad_2.27 -lmp4ff -lxvidcore_1.3.0 -lmpeg2 -lmpg123_1.13.1 -la52 -ljpeg -lpng -lz -lopengl -lm $(KOS_LIBS)

run: $(TARGET)
	$(KOS_LOADER) $(TARGET)

dist:
	rm -f $(OBJS)
	$(KOS_STRIP) $(TARGET)

