OBJS=test.o src/rl_backgrnd.o src/rl_image.o src/rl_sound.o src/rl_sprite.o
HEADERS=button_x.h sketch008.h tick.h

%.o: %.c
	gcc -O3 --std=c99 -Wall -I. -Isrc -c -o $@ $<

%.h: %.rle
	xxd -i $< | sed "s/unsigned/const unsigned/g" > $@

%.h: %.ogg
	xxd -i $< | sed "s/unsigned/const unsigned/g" > $@

%.h: %.pcm
	xxd -i $< | sed "s/unsigned/const unsigned/g" > $@

all: test.dll

test.dll: $(OBJS)
	gcc -o $@ -shared $+

test.o: $(HEADERS)

button_x.rle: button_x.png
	etc/luai/luai etc/rlrle.lua --margin 32 $<

button_x.h: button_x.rle

sketch008.h: sketch008.ogg

tick.h: tick.pcm

tick.pcm: tick.wav
	sox $< -B -t s16 -c 1 -r 44100 $@

clean:
	rm -rf test.dll button_x.rle tick.pcm $(OBJS) $(HEADERS)
