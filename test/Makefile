PACK=../etc/luai/luai ../etc/rlpack.lua
RLE=../etc/luai/luai ../etc/rlrle.lua

RES=res/button_x.rle res/block.rle res/sketch008.ogg res/tick.pcm res/city.tls res/city.ims res/city.map main.lua

all: test.rlp

test.rlp: $(RES)
	$(PACK) 1 $@ $+

res/button_x.rle: res/button_x.png
	$(RLE) --margin 32 $<

res/block.rle: res/block.png
	$(RLE) --margin 32 $<

res/tick.pcm: res/tick.wav
	sox $< -B -t s16 -c 1 -r 44100 $@

res.h: res.pak
	xxd -i $< > $@

clean:
	rm -rf test.rlp res/button_x.rle res/block.rle res/tick.pcm
