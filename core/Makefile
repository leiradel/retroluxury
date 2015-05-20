RL=../src
LUA=lua/src
PACK=../etc/luai/luai ../../etc/rlpack.lua
RLE=../etc/luai/luai ../../etc/rlrle.lua
VER=../etc/version.c.templ

INCLUDES=-I. -I$(LUA) -I$(RL)
DEFINES=-DRL_USERDATA_COUNT=4 -DRL_BG_SAVE_SIZE=131072

CFLAGS=-m32 -O0 -g --std=c99 $(INCLUDES) $(DEFINES)
CPPFLAGS=-m32 -O0 -g $(INCLUDES) $(DEFINES)
LDFLAGS=-m32 -g
LIBS=

LUAOBJS=$(LUA)/lapi.o $(LUA)/lcode.o $(LUA)/lctype.o $(LUA)/ldebug.o $(LUA)/ldo.o $(LUA)/ldump.o $(LUA)/lfunc.o $(LUA)/lgc.o $(LUA)/llex.o $(LUA)/lmem.o $(LUA)/lobject.o $(LUA)/lopcodes.o $(LUA)/lparser.o $(LUA)/lstate.o $(LUA)/lstring.o $(LUA)/ltable.o $(LUA)/ltm.o $(LUA)/lundump.o $(LUA)/lvm.o $(LUA)/lzio.o $(LUA)/lauxlib.o $(LUA)/lbaselib.o $(LUA)/lbitlib.o $(LUA)/lcorolib.o $(LUA)/ldblib.o $(LUA)/liolib.o $(LUA)/lmathlib.o $(LUA)/loslib.o $(LUA)/lstrlib.o $(LUA)/ltablib.o $(LUA)/lutf8lib.o $(LUA)/loadlib.o $(LUA)/linit.o

RLOBJS=$(RL)/rl_backgrnd.o $(RL)/rl_image.o $(RL)/rl_map.o $(RL)/rl_pack.o $(RL)/rl_rand.o $(RL)/rl_sound.o $(RL)/rl_sprite.o $(RL)/rl_tile.o $(RL)/rl_version.o

OBJS=bind.o libretro.o $(RLOBJS) $(LUAOBJS)

%.o: %.c
	gcc $(CFLAGS) -c $< -o $@

%.o: %.cpp
	g++ $(CPPFLAGS) -c $< -o $@

all: rl_libretro.dll

rl_libretro.dll: $(OBJS)
	g++ -shared $(LDFLAGS) -o $@ $+ $(LIBS)

libretro.o: boot_lua.h

boot_lua.h: boot.lua
	xxd -i $< > $@

$(RL)/rl_version.c: FORCE
	cat $(VER) | sed s/HASH/`git rev-parse HEAD | tr -d "\n"`/g > $@

clean:
	rm -rf rl_libretro.dll $(OBJS) boot_lua.h

.PHONY: clean FORCE
