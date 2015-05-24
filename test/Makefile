PACK=../etc/luai/luai ../etc/rlpack.lua
RLE=../etc/luai/luai ../etc/rlrle.lua
IMS=../etc/luai/luai ../etc/rlimageset.lua

RES=res/sketch008.ogg res/city.tls res/city.ims res/city.map res/hero.ims lua/main.lua lua/class.lua

all: test.rlp

test.rlp: $(RES)
	$(PACK) 1 $@ $+

res/hero.ims: res/char_d1.png res/char_d2.png res/char_d3.png res/char_l1.png res/char_l2.png res/char_l3.png res/char_r1.png res/char_r2.png res/char_r3.png res/char_u1.png res/char_u2.png res/char_u3.png
	$(IMS) --output $@ --margin 32 $+

res.h: res.pak
	xxd -i $< > $@

clean:
	rm -rf test.rlp res/button_x.rle res/block.rle res/tick.pcm res/hero.ims
