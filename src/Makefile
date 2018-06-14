INCLUDES = -I.
DEFINES = -DOUTSIDE_SPEEX -DRANDOM_PREFIX=speex -DEXPORT= -DFIXED_POINT
CFLAGS = --std=c99 -Wall -fPIC $(INCLUDES) $(DEFINES)

ifneq ($(DEBUG),)
  CFLAGS   += -O0 -g
  CXXFLAGS += -O0 -g
else
  CFLAGS   += -O3 -DNDEBUG
  CXXFLAGS += -O3 -DNDEBUG
endif

OBJS = rl_backgrnd.o rl_base64.o rl_bdffont.o rl_config.o rl_image.o rl_pixelsrc.o rl_pack.o rl_rand.o rl_resample.o \
       rl_sound.o rl_snddata.o rl_sprite.o rl_utf8.o rl_version.o rl_xml.o external/resample.o

%.o: %.c
	gcc $(CFLAGS) -c $< -o $@

all: libretroluxury.a

libretroluxury.a: $(OBJS)
	ar rcs $@ $+

rl_version.c: FORCE
	cat ../etc/version.c.templ | sed s/HASH/`git rev-parse HEAD | tr -d "\n"`/g > $@

clean:
	rm -f libretroluxury.a $(OBJS)

.PHONY: clean FORCE
