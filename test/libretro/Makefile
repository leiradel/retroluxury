RL=../../src

INCLUDES=-I. -I$(RL)
DEFINES=-DRL_BG_SAVE_SIZE=131072

CFLAGS=-m32 -O3 --std=c99 $(INCLUDES) $(DEFINES)
LDFLAGS=-m32
LIBS=

OBJS=test.o $(RL)/rl_backgrnd.o $(RL)/rl_image.o $(RL)/rl_map.o $(RL)/rl_pack.o $(RL)/rl_rand.o $(RL)/rl_sound.o $(RL)/rl_sprite.o $(RL)/rl_tile.o $(RL)/rl_version.o
RES=res/button_x.rle res/block.rle res/sketch008.ogg res/tick.pcm res/city.tls res/city.ims res/city.map
HEADERS=res.h

%.o: %.c
	gcc $(CFLAGS) -c $< -o $@

%.h: %.pak
	xxd -i $< > $@

all: test.dll

test.dll: $(OBJS)
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
