INCLUDES=-I. -I../../src
DEFINES=

CFLAGS=-m32 -O3 --std=c99 $(INCLUDES) $(DEFINES)
LDFLAGS=-m32 -L../../src
LIBS=-lretroluxury

OBJS=test.o
RES=res/button_x.rle res/block.rle res/sketch008.ogg res/tick.pcm res/city.tls res/city.ims res/city.map
HEADERS=res.h

%.o: %.c
	gcc $(CFLAGS) -c $< -o $@

%.h: %.pak
	xxd -i $< > $@

all: test.dll

test.dll: $(OBJS) ../../src/libretroluxury.a
	gcc -shared $(LDFLAGS) -o $@ $+ $(LIBS)

test.o: $(HEADERS)

res.pak: $(RES)
	../../etc/luai/luai ../../etc/rlpack.lua 1 $@ $+

res/button_x.rle: res/button_x.png
	../../etc/luai/luai ../../etc/rlrle.lua --margin 32 $<

res/block.rle: res/block.png
	../../etc/luai/luai ../../etc/rlrle.lua --margin 32 $<

res/tick.pcm: res/tick.wav
	sox $< -B -t s16 -c 1 -r 44100 $@

clean:
	rm -rf test.dll res.pak $(OBJS) $(HEADERS) res/button_x.rle res/block.rle res/tick.pcm
